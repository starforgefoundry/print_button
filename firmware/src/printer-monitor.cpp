#include "printer-monitor.h"

#include <WiFi.h>

namespace {
constexpr uint32_t POLL_INTERVAL_MS = 3000;
constexpr const char *BUSY_STATES[] = {"PREPARE", "SLICING", "RUNNING", "PAUSE"};

bool isBusy(const String &state) {
  for (const char *busy : BUSY_STATES) {
    if (state == busy) return true;
  }
  return false;
}
}  // namespace

PrinterMonitor::PrinterMonitor(BambuddyClient &client, StatusLight &light) : client_(client), light_(light) {}

void PrinterMonitor::configure(const Settings &settings) {
  server_ = {settings.bambuddyUrl, settings.apiKey};
  printerId_ = settings.hasBambuddy() ? settings.printerId : 0;
  status_ = {};
  lastResult_ = ApiResult::OK;
  refreshNow();
}

void PrinterMonitor::refreshNow() { pollDue_ = true; }

void PrinterMonitor::loop() {
  if (!WiFi.isConnected()) return;

  if (printerId_ == 0) {
    light_.setState(IndicatorState::UNCONFIGURED);
    light_.takePress();
    return;
  }

  if (light_.takePress() && light_.state() == IndicatorState::PLATE_READY) clearPlate();

  if (pollDue_ || millis() - lastPollAt_ >= POLL_INTERVAL_MS) poll();
}

ApiResult PrinterMonitor::clearPlate() {
  ApiResult result = client_.clearPlate(server_, printerId_);
  if (result == ApiResult::OK) {
    light_.acknowledge();
    status_.awaitingPlateClear = false;
    light_.setState(IndicatorState::IDLE);
  }
  refreshNow();
  return result;
}

void PrinterMonitor::poll() {
  pollDue_ = false;
  lastPollAt_ = millis();
  lastResult_ = client_.fetchStatus(server_, printerId_, status_);
  light_.setState(indicatorFor(lastResult_));
}

IndicatorState PrinterMonitor::indicatorFor(ApiResult result) const {
  if (result == ApiResult::NETWORK_ERROR) return IndicatorState::OFFLINE;
  if (result != ApiResult::OK) return IndicatorState::ERROR;
  if (status_.awaitingPlateClear) return IndicatorState::PLATE_READY;
  if (!status_.connected) return IndicatorState::OFFLINE;
  if (isBusy(status_.state)) return IndicatorState::BUSY;
  return IndicatorState::IDLE;
}
