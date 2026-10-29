#pragma once

#include <ArduinoJson.h>
#include <DNSServer.h>

#include "settings-store.h"
#include "status-light.h"

class NetworkManager {
 public:
  explicit NetworkManager(StatusLight &light);
  void begin(const Settings &settings);
  void loop();
  void advertisePrinter(const String &printerName);
  void listPeers(JsonArray out);
  void scanNetworks(JsonArray out);

  const String &hostname() const { return hostname_; }
  bool setupPortalActive() const { return portalActive_; }
  bool connected() const;
  String ipAddress() const;

 private:
  void startPortal();
  void stopPortal();
  void startMdns();

  StatusLight &light_;
  DNSServer dns_;
  String hostname_;
  String printerName_;
  bool hasCredentials_ = false;
  bool portalActive_ = false;
  bool mdnsStarted_ = false;
  uint32_t disconnectedSince_ = 0;
};
