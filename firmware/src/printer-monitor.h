#pragma once

#include "bambuddy-client.h"
#include "settings-store.h"
#include "status-light.h"

class PrinterMonitor {
 public:
  PrinterMonitor(BambuddyClient &client, StatusLight &light);
  void configure(const Settings &settings);
  void loop();
  ApiResult clearPlate();
  void refreshNow();
  const PrinterStatus &status() const { return status_; }
  const char *lastError() const { return apiResultMessage(lastResult_); }

 private:
  void poll();
  IndicatorState indicatorFor(ApiResult result) const;

  BambuddyClient &client_;
  StatusLight &light_;
  BambuddyServer server_;
  int printerId_ = 0;
  PrinterStatus status_;
  ApiResult lastResult_ = ApiResult::OK;
  uint32_t lastPollAt_ = 0;
  bool pollDue_ = true;
};
