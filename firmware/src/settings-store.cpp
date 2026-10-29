#include "settings-store.h"

#include <Preferences.h>

#ifndef DEFAULT_WIFI_SSID
#define DEFAULT_WIFI_SSID ""
#endif
#ifndef DEFAULT_WIFI_PASSWORD
#define DEFAULT_WIFI_PASSWORD ""
#endif
#ifndef DEFAULT_BAMBUDDY_URL
#define DEFAULT_BAMBUDDY_URL ""
#endif
#ifndef DEFAULT_BAMBUDDY_API_KEY
#define DEFAULT_BAMBUDDY_API_KEY ""
#endif

namespace {
constexpr const char *NAMESPACE = "printbutton";
constexpr const char *KEY_WIFI_SSID = "wifiSsid";
constexpr const char *KEY_WIFI_PASSWORD = "wifiPass";
constexpr const char *KEY_BAMBUDDY_URL = "bambuddyUrl";
constexpr const char *KEY_API_KEY = "apiKey";
constexpr const char *KEY_PRINTER_ID = "printerId";
constexpr const char *KEY_PRINTER_NAME = "printerName";
}  // namespace

Settings SettingsStore::load() {
  Preferences prefs;
  prefs.begin(NAMESPACE, true);
  Settings settings;
  settings.wifiSsid = prefs.getString(KEY_WIFI_SSID, DEFAULT_WIFI_SSID);
  settings.wifiPassword = prefs.getString(KEY_WIFI_PASSWORD, DEFAULT_WIFI_PASSWORD);
  settings.bambuddyUrl = prefs.getString(KEY_BAMBUDDY_URL, DEFAULT_BAMBUDDY_URL);
  settings.apiKey = prefs.getString(KEY_API_KEY, DEFAULT_BAMBUDDY_API_KEY);
  settings.printerId = prefs.getInt(KEY_PRINTER_ID, 0);
  settings.printerName = prefs.getString(KEY_PRINTER_NAME, "");
  prefs.end();
  return settings;
}

void SettingsStore::saveWifi(const String &ssid, const String &password) {
  Preferences prefs;
  prefs.begin(NAMESPACE, false);
  prefs.putString(KEY_WIFI_SSID, ssid);
  prefs.putString(KEY_WIFI_PASSWORD, password);
  prefs.end();
}

void SettingsStore::saveBambuddy(const String &url, const String &apiKey, int printerId, const String &printerName) {
  Preferences prefs;
  prefs.begin(NAMESPACE, false);
  prefs.putString(KEY_BAMBUDDY_URL, url);
  prefs.putString(KEY_API_KEY, apiKey);
  prefs.putInt(KEY_PRINTER_ID, printerId);
  prefs.putString(KEY_PRINTER_NAME, printerName);
  prefs.end();
}
