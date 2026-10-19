#include "ApiClient.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include "../security/DeviceAuth.h"

OrderResult ApiClient::sendButtonEvent(
    const String &baseUrl,
    const String &deviceId,
    const String &deviceSecret,
    const String &eventType,
    uint8_t battery,
    int rssi,
    const ButtonConfig* itemConfig
) {
  OrderResult res;
  res.success = false;

  HTTPClient http;
  String url = baseUrl;
  while (url.endsWith("/")) url = url.substring(0, url.length() - 1);
  if (!url.endsWith("/api/v1")) {
    url += "/api/v1";
  }
  url += "/iot/events";

  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(4000);

  // Build JSON body
  String requestId = "esp_req_" + String(millis()) + "_" + DeviceAuth::generateNonce().substring(0, 6);
  JsonDocument doc;
  doc["eventType"] = eventType;
  doc["requestId"] = requestId;
  doc["battery"] = battery;
  doc["rssi"] = rssi;

  if (itemConfig != nullptr) {
    if (itemConfig->actionType == ACTION_ORDER_ITEM || strlen(itemConfig->itemId) > 0) {
      doc["itemId"] = itemConfig->itemId;
      doc["itemName"] = itemConfig->itemName;
      doc["quantity"] = itemConfig->quantity;
    }
  }

  String bodyString;
  serializeJson(doc, bodyString);

  // SNTP Unix epoch (seconds since 1970-01-01)
  time_t now = time(nullptr);
  if (now < 1000000000L) {
    now = (time_t)(millis() / 1000);
    Serial.println("[API][WARN] SNTP not synced yet, using fallback timestamp.");
  }
  String timestamp = String((unsigned long)now);

  String nonce = DeviceAuth::generateNonce();
  String signaturePayload = deviceId + ":" + timestamp + ":" + nonce + ":" + bodyString;
  String signature = DeviceAuth::calculateHmac(deviceSecret, signaturePayload);

  http.addHeader("x-device-id", deviceId);
  http.addHeader("x-timestamp", timestamp);
  http.addHeader("x-nonce", nonce);
  http.addHeader("x-signature", signature);

  Serial.printf("[API] POST %s | Event: %s | ReqId: %s | UnixTS: %s\n",
    url.c_str(), eventType.c_str(), requestId.c_str(), timestamp.c_str());

  int httpCode = http.POST(bodyString);

  if (httpCode > 0) {
    String response = http.getString();
    Serial.printf("[API] Response: HTTP %d\n", httpCode);

    JsonDocument resDoc;
    DeserializationError error = deserializeJson(resDoc, response);
    if (!error && resDoc["success"] == true) {
      res.success = true;
      if (resDoc["data"]["order"]["orderNumber"].is<const char*>()) {
        res.orderNumber = resDoc["data"]["order"]["orderNumber"].as<String>();
      } else {
        res.orderNumber = requestId;
      }
      res.cancelWindowSeconds = resDoc["data"]["cancelWindowSeconds"] | 60;
    } else {
      res.errorMessage = resDoc["message"].as<String>();
      Serial.printf("[API] Server error: %s\n", res.errorMessage.c_str());
    }
  } else {
    res.errorMessage = "HTTP connection failed: " + http.errorToString(httpCode);
    Serial.printf("[API] Connection error (code %d): %s\n", httpCode, res.errorMessage.c_str());
  }

  http.end();
  return res;
}
