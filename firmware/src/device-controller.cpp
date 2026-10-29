#include "device-controller.h"

#include <WiFi.h>

#include "web-ui.h"

namespace {
constexpr uint16_t HTTP_PORT = 80;
constexpr uint32_t REBOOT_DELAY_MS = 1000;
constexpr const char *FIRMWARE_VERSION = "1.0.0";
constexpr const char *CONTENT_JSON = "application/json";
constexpr const char *CONTENT_HTML = "text/html";
}  // namespace

DeviceController::DeviceController(Settings &settings, SettingsStore &store, BambuddyClient &client,
                                   PrinterMonitor &monitor, NetworkManager &network, StatusLight &light)
    : settings_(settings),
      store_(store),
      client_(client),
      monitor_(monitor),
      network_(network),
      light_(light),
      server_(HTTP_PORT) {}

void DeviceController::begin() {
  server_.on("/", HTTP_GET, [this] { serveUi(); });
  server_.on("/api/device", HTTP_GET, [this] { getDevice(); });
  server_.on("/api/settings", HTTP_POST, [this] { saveSettings(); });
  server_.on("/api/printers", HTTP_POST, [this] { listPrinters(); });
  server_.on("/api/identify", HTTP_POST, [this] { identify(); });
  server_.on("/api/clear-plate", HTTP_POST, [this] { clearPlate(); });
  server_.on("/api/wifi/networks", HTTP_GET, [this] { scanNetworks(); });
  server_.on("/api/wifi", HTTP_POST, [this] { saveWifi(); });
  server_.on("/api/peers", HTTP_GET, [this] { listPeers(); });
  server_.onNotFound([this] { notFound(); });
  server_.begin();
}

void DeviceController::loop() {
  server_.handleClient();
  if (rebootAt_ && int32_t(millis() - rebootAt_) >= 0) ESP.restart();
}

void DeviceController::serveUi() {
  server_.sendHeader("Content-Encoding", "gzip");
  server_.send_P(200, CONTENT_HTML, reinterpret_cast<const char *>(WEB_UI_GZ), WEB_UI_GZ_LENGTH);
}

void DeviceController::getDevice() {
  JsonDocument doc;
  doc["hostname"] = network_.hostname();
  doc["ip"] = network_.ipAddress();
  doc["mac"] = WiFi.macAddress();
  doc["firmwareVersion"] = FIRMWARE_VERSION;
  doc["setupPortal"] = network_.setupPortalActive();
  doc["wifiSsid"] = settings_.wifiSsid;
  doc["wifiConnected"] = network_.connected();
  doc["rssi"] = WiFi.RSSI();
  doc["bambuddyUrl"] = settings_.bambuddyUrl;
  doc["hasApiKey"] = settings_.apiKey.length() > 0;
  doc["printerId"] = settings_.printerId;
  doc["printerName"] = settings_.printerName;
  doc["indicator"] = indicatorStateName(light_.state());
  doc["printerState"] = monitor_.status().state;
  doc["error"] = monitor_.lastError();
  sendJson(200, doc);
}

void DeviceController::saveSettings() {
  JsonDocument body;
  if (!readBody(body)) return;

  String url = body["bambuddyUrl"] | settings_.bambuddyUrl;
  String apiKey = body["apiKey"] | "";
  if (apiKey.isEmpty()) apiKey = settings_.apiKey;

  settings_.bambuddyUrl = url;
  settings_.apiKey = apiKey;
  settings_.printerId = body["printerId"] | 0;
  settings_.printerName = body["printerName"] | "";
  store_.saveBambuddy(settings_.bambuddyUrl, settings_.apiKey, settings_.printerId, settings_.printerName);

  monitor_.configure(settings_);
  network_.advertisePrinter(settings_.printerName);
  sendOk();
}

void DeviceController::listPrinters() {
  JsonDocument body;
  if (!readBody(body)) return;

  BambuddyServer server{body["bambuddyUrl"] | settings_.bambuddyUrl, body["apiKey"] | ""};
  if (server.apiKey.isEmpty()) server.apiKey = settings_.apiKey;

  JsonDocument printers;
  ApiResult result = client_.listPrinters(server, printers.to<JsonArray>());
  if (result != ApiResult::OK) return sendError(502, apiResultMessage(result));
  sendJson(200, printers);
}

void DeviceController::identify() {
  light_.identify();
  sendOk();
}

void DeviceController::clearPlate() {
  if (!settings_.hasPrinter()) return sendError(400, "No printer selected");
  ApiResult result = monitor_.clearPlate();
  if (result != ApiResult::OK) return sendError(502, apiResultMessage(result));
  sendOk();
}

void DeviceController::scanNetworks() {
  JsonDocument networks;
  network_.scanNetworks(networks.to<JsonArray>());
  sendJson(200, networks);
}

void DeviceController::saveWifi() {
  JsonDocument body;
  if (!readBody(body)) return;

  String ssid = body["ssid"] | "";
  if (ssid.isEmpty()) return sendError(400, "SSID is required");

  store_.saveWifi(ssid, body["password"] | "");
  sendOk();
  rebootAt_ = millis() + REBOOT_DELAY_MS;
}

void DeviceController::listPeers() {
  JsonDocument peers;
  network_.listPeers(peers.to<JsonArray>());
  sendJson(200, peers);
}

void DeviceController::notFound() {
  if (network_.setupPortalActive() && !server_.uri().startsWith("/api/")) {
    server_.sendHeader("Location", "http://" + network_.ipAddress() + "/");
    server_.send(302);
    return;
  }
  sendError(404, "Not found");
}

bool DeviceController::readBody(JsonDocument &body) {
  if (deserializeJson(body, server_.arg("plain")) == DeserializationError::Ok) return true;
  sendError(400, "Invalid JSON body");
  return false;
}

void DeviceController::sendJson(int code, const JsonDocument &doc) {
  String payload;
  serializeJson(doc, payload);
  server_.send(code, CONTENT_JSON, payload);
}

void DeviceController::sendError(int code, const char *message) {
  JsonDocument doc;
  doc["error"] = message;
  sendJson(code, doc);
}

void DeviceController::sendOk() {
  JsonDocument doc;
  doc["ok"] = true;
  sendJson(200, doc);
}
