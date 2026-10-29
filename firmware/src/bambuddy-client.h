#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

enum class ApiResult {
  OK,
  UNAUTHORIZED,
  NOT_FOUND,
  REJECTED,
  NETWORK_ERROR,
};

const char *apiResultMessage(ApiResult result);

struct PrinterStatus {
  bool connected = false;
  String state;
  bool awaitingPlateClear = false;
};

struct BambuddyServer {
  String url;
  String apiKey;
};

class BambuddyClient {
 public:
  ApiResult fetchStatus(const BambuddyServer &server, int printerId, PrinterStatus &out);
  ApiResult clearPlate(const BambuddyServer &server, int printerId);
  ApiResult listPrinters(const BambuddyServer &server, JsonArray out);
};
