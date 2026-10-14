#include "StateMachine.h"
#include "../power/PowerManager.h"
#include "../led/LedManager.h"
#include "../buzzer/BuzzerManager.h"
#include "../button/ButtonManager.h"
#include "../wifi/WiFiManager.h"
#include "../ble/BLEProvisioning.h"
#include "../api/ApiClient.h"
#include "config.h"

// Device credentials for this firmware build (hoặc lưu trong NVS Flash)
static const String DEVICE_ID = "BTN-8829-WTR";
static const String DEVICE_SECRET = "sec_smart_button_8829_wtr_key_99";
static const String BACKEND_URL = DEFAULT_BACKEND_URL;

void StateMachine::init() {
  Serial.begin(115200);
  delay(50);

  Serial.println("\n==================================================");
  Serial.println("🔘 SMART ORDER BUTTON — ESP32-S3 PRODUCTION FIRMWARE");
  Serial.printf("Device ID: %s | Model: ESP32-S3-WROOM-1\n", DEVICE_ID.c_str());
  Serial.println("Schematic: Smart_Order_Button.kicad_sch");
  Serial.println("==================================================");

  // Initialize hardware drivers
  LedManager::init();
  BuzzerManager::init();
  ButtonManager::init();
  PowerManager::init();

  // Hiệu ứng khởi động hệ thống: LED Trắng nhẹ 100ms & bíp khởi động
  LedManager::setColor(LED_WHITE);
  BuzzerManager::beepBoot();
  delay(100);
  LedManager::setColor(LED_OFF);

  uint32_t batteryMv = PowerManager::readBatteryMillivolts();
  uint8_t batteryPct = PowerManager::calculateBatteryPercentage(batteryMv);
  Serial.printf("[POWER] MAX17048 Fuel Gauge: %u mV (%u%%)\n", batteryMv, batteryPct);

  // Nếu người dùng đang giữ nút trong lúc cắm nguồn (Cold boot gesture)
  if (ButtonManager::isPressed()) {
    ButtonGesture gesture = ButtonManager::detectGesture(false);
    handleGesture(gesture);
  }
}

void StateMachine::loop() {
  // Đi vào chế độ ngủ tiết kiệm điện (Light Sleep qua IO39 hoặc Deep Sleep qua RTC IO10)
  PowerManager::enterSleep();

  // Khi bấm nút SW1, ESP32-S3 thức dậy ngay lập tức!
  // Đọc cử chỉ phím với cờ wokenFromSleep = true
  ButtonGesture gesture = ButtonManager::detectGesture(true);
  handleGesture(gesture);
}

void StateMachine::run() {
  init();
  while (true) {
    loop();
  }
}

void StateMachine::handleGesture(ButtonGesture gesture) {
  if (gesture == GESTURE_NONE) {
    Serial.println("[GESTURE] Transient electrical bounce. Resuming sleep...");
    return;
  }

  // 1. Gesture: Hold 15s -> Factory Reset
  if (gesture == GESTURE_HOLD_15S_FACTORY_RESET) {
    Serial.println("[RESET] Hold 15s detected: FACTORY RESETTING DEVICE...");
    WiFiManager::clearCredentials();
    LedManager::blink(LED_RED, 5, 100);
    BuzzerManager::beepError();
    return;
  }

  // 2. Gesture: 5 Clicks (Quick Pairing) hoặc Hold 10s hoặc chưa có thông tin Wi-Fi -> Bật BLE & SoftAP
  if (gesture == GESTURE_FIVE_CLICKS_PAIR || gesture == GESTURE_HOLD_10S_PROVISION || !WiFiManager::hasCredentials()) {
    Serial.println("[MODE] BLUE LED: 5-Clicks or Provision detected -> Starting BLE Auto-Pairing");
    LedManager::setColor(LED_BLUE);
    BuzzerManager::beepBoot();

    String bleName = "SOB-" + DEVICE_ID;
    BLEProvisioning::start(bleName, DEVICE_ID);
    WiFiManager::startCaptivePortal(bleName + "-AP");

    unsigned long provisionStart = millis();
    while (millis() - provisionStart < (BLE_PROVISION_TIMEOUT_S * 1000)) {
      if ((millis() / 500) % 2 == 0) {
        LedManager::setColor(LED_BLUE);
      } else {
        LedManager::setColor(LED_OFF);
      }

      WiFiManager::handleCaptivePortal();
      delay(20);

      // Nếu đã lưu được thông tin Wi-Fi mới thì thoát chế độ cấu hình
      if (WiFiManager::hasCredentials() && millis() - provisionStart > 3000) {
        break;
      }
    }

    BLEProvisioning::stop();
    WiFiManager::stopWiFi();
    LedManager::setColor(LED_OFF);
    return;
  }

  // 3. Gesture: Hold 5s -> Wake / Activate / Config Sync
  if (gesture == GESTURE_HOLD_5S_ACTIVATE) {
    Serial.println("[MODE] PURPLE LED: Device Configuration / Activation Sync");
    LedManager::setColor(LED_PURPLE);
  } else if (gesture == GESTURE_DOUBLE_PRESS) {
    Serial.println("[GESTURE] DOUBLE PRESS -> CANCEL ORDER");
    LedManager::setColor(LED_YELLOW);
  } else {
    Serial.println("[GESTURE] SINGLE PRESS -> NEW ORDER");
    LedManager::setColor(LED_BLUE);
  }

  // 4. Kết nối Wi-Fi & Truyền sự kiện
  uint32_t batteryMv = PowerManager::readBatteryMillivolts();
  uint8_t batteryPct = PowerManager::calculateBatteryPercentage(batteryMv);

  bool connected = false;
  for (int retry = 1; retry <= WIFI_MAX_RETRIES; retry++) {
    Serial.printf("[WIFI] Attempt %d/%d...\n", retry, WIFI_MAX_RETRIES);
    if (WiFiManager::connect(WIFI_CONNECT_TIMEOUT_MS)) {
      connected = true;
      break;
    }
    delay(100);
  }

  if (!connected) {
    Serial.println("[WIFI] Connection failed. RED LED Error indication.");
    LedManager::setColor(LED_RED);
    BuzzerManager::beepError();
    delay(1200);
    WiFiManager::stopWiFi();
    LedManager::setColor(LED_OFF);
    return;
  }

  // 5. Chuẩn bị sự kiện gửi lên Cloud Backend
  String eventTypeStr = "SINGLE_PRESS";
  if (gesture == GESTURE_DOUBLE_PRESS) {
    eventTypeStr = "DOUBLE_PRESS";
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

  // 6. Phản hồi LED & Còi Buzzer
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
    delay(1200);
  } else {
    Serial.printf("[ERROR] RED LED: Cloud rejected request (%s)\n",
                  result.errorMessage.c_str());
    LedManager::setColor(LED_RED);
    BuzzerManager::beepError();
    delay(1200);
  }

  // 7. Tắt Wi-Fi và LED trước khi quay lại chế độ ngủ
  WiFiManager::stopWiFi();
  LedManager::setColor(LED_OFF);
}
