#include "bambuddy-client.h"

#include <HTTPClient.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>

#include <memory>

namespace {
constexpr const char *API_PREFIX = "/api/v1";
constexpr const char *API_KEY_HEADER = "X-API-Key";
constexpr uint32_t TIMEOUT_MS = 4000;

class Request {
 public:
  Request(const BambuddyServer &server, const String &path) {
    String base = server.url;
    while (base.endsWith("/")) base.remove(base.length() - 1);
    if (base.startsWith("https://")) {
      auto secure = std::make_unique<WiFiClientSecure>();
      secure->setInsecure();
      client_ = std::move(secure);
    } else {
      client_ = std::make_unique<WiFiClient>();
    }
    http_.setTimeout(TIMEOUT_MS);
    http_.setConnectTimeout(TIMEOUT_MS);
    http_.begin(*client_, base + API_PREFIX + path);
    http_.addHeader(API_KEY_HEADER, server.apiKey);
  }

  ~Request() { http_.end(); }

  ApiResult get() { return toResult(http_.GET()); }
  ApiResult post() { return toResult(http_.POST("")); }
  String body() { return http_.getString(); }

 private:
  static ApiResult toResult(int code) {
    if (code < 0) return ApiResult::NETWORK_ERROR;
    if (code == HTTP_CODE_UNAUTHORIZED || code == HTTP_CODE_FORBIDDEN) return ApiResult::UNAUTHORIZED;
    if (code == HTTP_CODE_NOT_FOUND) return ApiResult::NOT_FOUND;
    if (code >= HTTP_CODE_OK && code < HTTP_CODE_MULTIPLE_CHOICES) return ApiResult::OK;
    return ApiResult::REJECTED;
  }

  std::unique_ptr<WiFiClient> client_;
  HTTPClient http_;
};
}  // namespace

const char *apiResultMessage(ApiResult result) {
  switch (result) {
    case ApiResult::OK:
      return "";
    case ApiResult::UNAUTHORIZED:
      return "API key was rejected";
    case ApiResult::NOT_FOUND:
      return "Printer not found";
    case ApiResult::REJECTED:
      return "Bambuddy rejected the request";
    case ApiResult::NETWORK_ERROR:
      return "Could not reach Bambuddy";
  }
  return "Unknown error";
}

ApiResult BambuddyClient::fetchStatus(const BambuddyServer &server, int printerId, PrinterStatus &out) {
  Request request(server, "/printers/" + String(printerId) + "/status");
  ApiResult result = request.get();
  if (result != ApiResult::OK) return result;

  JsonDocument filter;
  filter["connected"] = true;
  filter["state"] = true;
  filter["awaiting_plate_clear"] = true;

  JsonDocument doc;
  if (deserializeJson(doc, request.body(), DeserializationOption::Filter(filter))) return ApiResult::NETWORK_ERROR;

  out.connected = doc["connected"] | false;
  out.state = doc["state"] | "";
  out.awaitingPlateClear = doc["awaiting_plate_clear"] | false;
  return ApiResult::OK;
}

ApiResult BambuddyClient::clearPlate(const BambuddyServer &server, int printerId) {
  Request request(server, "/printers/" + String(printerId) + "/clear-plate");
  return request.post();
}

ApiResult BambuddyClient::listPrinters(const BambuddyServer &server, JsonArray out) {
  Request request(server, "/printers/");
  ApiResult result = request.get();
  if (result != ApiResult::OK) return result;

  JsonDocument filter;
  filter[0]["id"] = true;
  filter[0]["name"] = true;
  filter[0]["model"] = true;

  JsonDocument doc;
  if (deserializeJson(doc, request.body(), DeserializationOption::Filter(filter))) return ApiResult::NETWORK_ERROR;

  for (JsonObject printer : doc.as<JsonArray>()) out.add(printer);
  return ApiResult::OK;
}
