#include <Arduino.h>

#include "bambuddy-client.h"
#include "device-controller.h"
#include "network-manager.h"
#include "printer-monitor.h"
#include "settings-store.h"
#include "status-light.h"

namespace {
SettingsStore store;
Settings settings;
StatusLight light;
BambuddyClient client;
PrinterMonitor monitor(client, light);
NetworkManager network(light);
DeviceController controller(settings, store, client, monitor, network, light);
}  // namespace

void setup() {
  Serial.begin(115200);
  settings = store.load();
  light.begin();
  network.begin(settings);
  monitor.configure(settings);
  controller.begin();
}

void loop() {
  network.loop();
  controller.loop();
  monitor.loop();
  delay(1);
}
