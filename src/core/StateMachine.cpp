#include "StateMachine.h"
#include "../power/PowerManager.h"
#include "../led/LedManager.h"
#include "../buzzer/BuzzerManager.h"
#include "../button/ButtonManager.h"
#include "../wifi/WiFiManager.h"
#include "../ble/BLEProvisioning.h"
#include "../api/ApiClient.h"
#include "config.h"

// Device credentials for this firmware build (or read from NVS)
static const String DEVICE_ID = "BTN-8829-WTR";
static const String DEVICE_SECRET = "sec_smart_button_8829_wtr_key_99";
static const String BACKEND_URL = DEFAULT_BACKEND_URL;

void StateMachine::run() {
  Serial.begin(115200);
  delay(50);

  Serial.println("\n==================================================");
  Serial.println("🔘 SMART ORDER BUTTON — ESP32-S3 ULTRA LOW POWER FIRMWARE");
  Serial.printf("Device ID: %s | Model: ESP32-S3-WROOM-1\n", DEVICE_ID.c_str());
  Serial.println("Schematic: Smart_Order_Button.kicad_sch");
  Serial.println("==================================================");

  // Initialize hardware drivers
  LedManager::init();
  BuzzerManager::init();
  ButtonManager::init();
  PowerManager::init();

  // CYAN: Device Boot visual indicator
  LedManager::setColor(LED_WHITE);
  delay(120);

  uint32_t batteryMv = PowerManager::readBatteryMillivolts();
  uint8_t batteryPct = PowerManager::calculateBatteryPercentage(batteryMv);
  Serial.printf("[POWER] MAX17048 Fuel Gauge: %u mV (%u%%)\n", batteryMv, batteryPct);

  // 1. Detect Gesture (Non-blocking thresholds on SW1 / IO39)
  ButtonGesture gesture = ButtonManager::detectGesture();

  // Gesture: Hold 15s -> Factory Reset
  if (gesture == GESTURE_HOLD_15S_FACTORY_RESET) {
    Serial.println("[RESET] Hold 15s detected: FACTORY RESETTING DEVICE...");
    WiFiManager::clearCredentials();
    LedManager::blink(LED_RED, 5, 100);
    PowerManager::enterDeepSleep();
    return;
  }

  // Gesture: 5 Clicks (Quick Pairing Mode) or Hold 10s -> BLE 5 Auto-Pairing & WiFi Provisioning Mode
  if (gesture == GESTURE_FIVE_CLICKS_PAIR || gesture == GESTURE_HOLD_10S_PROVISION || !WiFiManager::hasCredentials()) {
    Serial.println("[MODE] BLUE LED: 5-Clicks or Provision detected -> Starting BLE Auto-Pairing");
    LedManager::setColor(LED_BLUE);
    BuzzerManager::beepBoot();

    // Name format: SOB-<DEVICE_ID> (e.g. SOB-BTN-8829-WTR) so Mobile app can auto-detect without typing ID
    String bleName = "SOB-" + DEVICE_ID;
    BLEProvisioning::start(bleName, DEVICE_ID);
    WiFiManager::startCaptivePortal(bleName + "-AP");

    unsigned long provisionStart = millis();
    while (millis() - provisionStart < (BLE_PROVISION_TIMEOUT_S * 1000)) {
      // Blink BLUE LED softly during pairing
      if ((millis() / 500) % 2 == 0) {
        LedManager::setColor(LED_BLUE);
      } else {
        LedManager::setColor(LED_OFF);
      }

      WiFiManager::handleCaptivePortal();
      delay(20);
    }

    BLEProvisioning::stop();
    WiFiManager::stopWiFi();
    LedManager::setColor(LED_OFF);
    PowerManager::enterDeepSleep();
    return;
  }

  // Gesture: Hold 5s -> Wake / Activate / Config Sync
  if (gesture == GESTURE_HOLD_5S_ACTIVATE) {
    Serial.println("[MODE] PURPLE LED: Device Configuration / Activation Sync");
    LedManager::setColor(LED_PURPLE);
    // Continue to connect Wi-Fi and sync config
  }

  if (gesture == GESTURE_NONE) {
    Serial.println("[GESTURE] Transient electrical bounce. Deep sleep immediately.");
    PowerManager::enterDeepSleep();
    return;
  }

  // 2. Wi-Fi Connection & Data Transmission -> YELLOW LED
  LedManager::setColor(LED_YELLOW);

  bool connected = false;
  for (int retry = 1; retry <= WIFI_MAX_RETRIES; retry++) {
    Serial.printf("[WIFI] Attempt %d/%d...\n", retry, WIFI_MAX_RETRIES);
    if (WiFiManager::connect(WIFI_CONNECT_TIMEOUT_MS)) {
      connected = true;
      break;
    }
    delay(200);
  }

  if (!connected) {
    Serial.println("[WIFI] Connection failed. RED LED Error indication.");
    LedManager::setColor(LED_RED);
    BuzzerManager::beepError();
    delay(1500);
    WiFiManager::stopWiFi();
    LedManager::setColor(LED_OFF);
    PowerManager::enterDeepSleep();
    return;
  }

  // 3. Dispatch Event to Cloud via HMAC-SHA256
  String eventTypeStr = "SINGLE_PRESS";
  if (gesture == GESTURE_DOUBLE_PRESS) {
    eventTypeStr = "DOUBLE_PRESS"; // Cancel order gesture
  } else if (gesture == GESTURE_HOLD_5S_ACTIVATE) {
    eventTypeStr = "ACTIVATE";
  }

  int8_t rssi = WiFi.RSSI();

  Serial.printf("[API] Transmitting event: %s (Batt: %d%%, RSSI: %d dBm)\n",
                eventTypeStr.c_str(), batteryPct, rssi);

  OrderResult result = ApiClient::sendButtonEvent(
      BACKEND_URL,
      DEVICE_ID,
      DEVICE_SECRET,
      eventTypeStr,
      batteryPct,
      rssi
  );

  // 4. Visual & Audio Feedback
  if (result.success) {
    if (gesture == GESTURE_DOUBLE_PRESS) {
      Serial.println("[SUCCESS] Order cancelled successfully by Double-Press.");
      LedManager::setColor(LED_YELLOW);
      BuzzerManager::beepCancel();
    } else {
      Serial.printf("[SUCCESS] GREEN LED: Order created! Code: %s\n", result.orderNumber.c_str());
      LedManager::setColor(LED_GREEN);
      BuzzerManager::beepSuccess();
    }
    delay(1500);
  } else {
    Serial.printf("[ERROR] RED LED: Cloud rejected request (%s)\n",
                  result.errorMessage.c_str());
    LedManager::setColor(LED_RED);
    BuzzerManager::beepError();
    delay(1500);
  }

  // 5. Radio Shutdown & Deep Sleep Entry
  Serial.println("[POWER] Shutting down Wi-Fi radio and entering Deep Sleep (~15uA)...");
  WiFiManager::stopWiFi();
  LedManager::setColor(LED_OFF);
  PowerManager::enterDeepSleep();
}
