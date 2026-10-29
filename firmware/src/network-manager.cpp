#include "network-manager.h"

#include <ESPmDNS.h>
#include <WiFi.h>

namespace {
constexpr const char *HOSTNAME_PREFIX = "printbutton-";
constexpr const char *MDNS_SERVICE = "printbutton";
constexpr const char *MDNS_PROTOCOL = "tcp";
constexpr const char *TXT_PRINTER = "printer";
constexpr uint16_t HTTP_PORT = 80;
constexpr uint16_t DNS_PORT = 53;
constexpr uint32_t PORTAL_AFTER_MS = 20000;

String chipSuffix() {
  uint8_t mac[6];
  WiFi.macAddress(mac);
  char suffix[7];
  snprintf(suffix, sizeof(suffix), "%02x%02x%02x", mac[3], mac[4], mac[5]);
  return suffix;
}
}  // namespace

NetworkManager::NetworkManager(StatusLight &light) : light_(light) {}

void NetworkManager::begin(const Settings &settings) {
  WiFi.mode(WIFI_STA);
  hostname_ = HOSTNAME_PREFIX + chipSuffix();
  printerName_ = settings.printerName;
  WiFi.setHostname(hostname_.c_str());
  WiFi.setAutoReconnect(true);

  hasCredentials_ = settings.wifiSsid.length() > 0;
  disconnectedSince_ = millis();
  if (hasCredentials_) {
    WiFi.begin(settings.wifiSsid.c_str(), settings.wifiPassword.c_str());
  } else {
    startPortal();
  }
}

void NetworkManager::loop() {
  if (portalActive_) dns_.processNextRequest();

  if (connected()) {
    disconnectedSince_ = millis();
    if (portalActive_) stopPortal();
    if (!mdnsStarted_) startMdns();
    return;
  }

  if (!portalActive_ && millis() - disconnectedSince_ >= PORTAL_AFTER_MS) startPortal();
  if (portalActive_) light_.setState(IndicatorState::SETUP);
}

bool NetworkManager::connected() const { return WiFi.isConnected(); }

String NetworkManager::ipAddress() const {
  return connected() ? WiFi.localIP().toString() : WiFi.softAPIP().toString();
}

void NetworkManager::startPortal() {
  WiFi.mode(hasCredentials_ ? WIFI_AP_STA : WIFI_AP);
  WiFi.softAP(hostname_.c_str());
  dns_.start(DNS_PORT, "*", WiFi.softAPIP());
  portalActive_ = true;
  light_.setState(IndicatorState::SETUP);
  if (!mdnsStarted_) startMdns();
}

void NetworkManager::stopPortal() {
  dns_.stop();
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_STA);
  portalActive_ = false;
}

void NetworkManager::startMdns() {
  if (!MDNS.begin(hostname_.c_str())) return;
  MDNS.addService(MDNS_SERVICE, MDNS_PROTOCOL, HTTP_PORT);
  MDNS.addServiceTxt(MDNS_SERVICE, MDNS_PROTOCOL, TXT_PRINTER, printerName_);
  mdnsStarted_ = true;
}

void NetworkManager::advertisePrinter(const String &printerName) {
  printerName_ = printerName;
  if (mdnsStarted_) MDNS.addServiceTxt(MDNS_SERVICE, MDNS_PROTOCOL, TXT_PRINTER, printerName_);
}

void NetworkManager::listPeers(JsonArray out) {
  int count = MDNS.queryService(MDNS_SERVICE, MDNS_PROTOCOL);
  for (int i = 0; i < count; i++) {
    JsonObject peer = out.add<JsonObject>();
    peer["hostname"] = MDNS.hostname(i);
    peer["ip"] = MDNS.IP(i).toString();
    peer["printerName"] = MDNS.txt(i, TXT_PRINTER);
  }
}

void NetworkManager::scanNetworks(JsonArray out) {
  int count = WiFi.scanNetworks();
  for (int i = 0; i < count; i++) {
    JsonObject network = out.add<JsonObject>();
    network["ssid"] = WiFi.SSID(i);
    network["rssi"] = WiFi.RSSI(i);
    network["secure"] = WiFi.encryptionType(i) != WIFI_AUTH_OPEN;
  }
  WiFi.scanDelete();
}
