#pragma once

#include <Arduino.h>

struct Settings {
  String wifiSsid;
  String wifiPassword;
  String bambuddyUrl;
  String apiKey;
  int printerId = 0;
  String printerName;

  bool hasPrinter() const { return printerId > 0; }
  bool hasBambuddy() const { return bambuddyUrl.length() > 0 && apiKey.length() > 0; }
};

class SettingsStore {
 public:
  Settings load();
  void saveWifi(const String &ssid, const String &password);
  void saveBambuddy(const String &url, const String &apiKey, int printerId, const String &printerName);
};
