#pragma once

#include <ArduinoJson.h>
#include <WebServer.h>

#include "bambuddy-client.h"
#include "network-manager.h"
#include "printer-monitor.h"
#include "settings-store.h"
#include "status-light.h"

class DeviceController {
 public:
  DeviceController(Settings &settings, SettingsStore &store, BambuddyClient &client, PrinterMonitor &monitor,
                   NetworkManager &network, StatusLight &light);
  void begin();
  void loop();

 private:
  void serveUi();
  void getDevice();
  void saveSettings();
  void listPrinters();
  void identify();
  void clearPlate();
  void scanNetworks();
  void saveWifi();
  void listPeers();
  void notFound();

  bool readBody(JsonDocument &body);
  void sendJson(int code, const JsonDocument &doc);
  void sendError(int code, const char *message);
  void sendOk();

  Settings &settings_;
  SettingsStore &store_;
  BambuddyClient &client_;
  PrinterMonitor &monitor_;
  NetworkManager &network_;
  StatusLight &light_;
  WebServer server_;
  uint32_t rebootAt_ = 0;
};
