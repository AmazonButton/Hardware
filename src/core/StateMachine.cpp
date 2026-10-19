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
  Serial.printf("Device ID: %s | Model: ESP32-S3-WROOM-1\n", DEVICE_ID.c_str());
  Serial.printf("Hardware Mode: %s\n", 
    ButtonManager::getVariant() == HARDWARE_VARIANT_TRIPLE_BUTTON 
      ? "BẢN 3 NÚT (SW1: Order, SW2: Bill, SW3: Cancel)" 
      : "BẢN 1 NÚT (SW1: Multi-gesture)");
  Serial.println("Schematic: Smart_Order_Button.kicad_sch");
  Serial.println("==================================================");

  // Initialize hardware drivers
  LedManager::init();
  BuzzerManager::init();
  ButtonManager::init();
  PowerManager::init();

  // Hiệu ứng khởi động hệ thống
  BuzzerManager::beepBoot();

  uint32_t batteryMv = PowerManager::readBatteryMillivolts();
  uint8_t batteryPct = PowerManager::calculateBatteryPercentage(batteryMv);
  Serial.printf("[POWER] MAX17048 Fuel Gauge: %u mV (%u%%)\n", batteryMv, batteryPct);

  // ⚡ PHÁT SÓNG BLUETOOTH NGAY KHI VỪA NẠP CODE XONG / KHỞI ĐỘNG
  String bleName = "SOB-" + DEVICE_ID;
  Serial.printf("\n[BLE] >>> KHỞI ĐỘNG PHÁT SÓNG BLUETOOTH: '%s' <<<\n", bleName.c_str());
  Serial.println("[BLE] Đèn LED Xanh dương nhấp nháy: Đang chờ điện thoại/thiết bị kết nối Bluetooth...");
  Serial.println("[BLE] Khi điện thoại bắt được kết nối -> Đèn sẽ tự động đổi màu XANH LÁ!");
  BLEProvisioning::start(bleName, DEVICE_ID);

  // Nếu người dùng đang giữ nút trong lúc cắm nguồn (Cold boot gesture)
  if (ButtonManager::isPressed()) {
    ButtonGesture gesture = ButtonManager::detectGesture(false);
    handleGesture(gesture);
  }
}

void StateMachine::loop() {
  static unsigned long lastBlinkMs = 0;
  static bool blinkState = false;
  static bool wasConnected = false;

  bool isConnected = BLEProvisioning::isConnected();

  if (!isConnected) {
    // 1. CHƯA BẮT ĐƯỢC BLUETOOTH:
    // Đèn LED nhấp nháy màu XANH DƯƠNG (Blue) chu kỳ 500ms báo đang phát Bluetooth chờ bắt
    if (millis() - lastBlinkMs >= 500) {
      lastBlinkMs = millis();
      blinkState = !blinkState;
      LedManager::setColor(blinkState ? LED_BLUE : LED_OFF);
    }

    // Nếu trước đó đang kết nối mà bị ngắt kết nối
    if (wasConnected) {
      wasConnected = false;
      Serial.println("\n[BLE] Thiết bị đã ngắt kết nối! Đèn nhấp nháy Xanh dương chờ bắt lại...");
      BuzzerManager::beepCancel();
    }
  } else {
    // 2. ĐÃ BẮT ĐƯỢC BLUETOOTH ("BẮT ĐƯỢC RỒI")!
    if (!wasConnected) {
      wasConnected = true;
      Serial.println("\n========================================================");
      Serial.println("[BLE] >>> ĐÃ BẮT ĐƯỢC BLUETOOTH THÀNH CÔNG! <<<");
      Serial.println("[LED] >>> THAY ĐỔI ĐÈN: CHUYỂN SANG MÀU XANH LÁ (CONNECTED)! <<<");
      Serial.println("========================================================");

      // THAY ĐỔI ĐÈN: Chuyển sang màu Xanh Lá (GREEN) sáng cố định báo đã kết nối
      LedManager::setColor(LED_GREEN);

      // Bíp âm thanh chào mừng kết nối thành công
      BuzzerManager::beepSuccess();
    }
  }

  // 3. Đọc cử chỉ phím SW1 nếu người dùng bấm
  if (ButtonManager::isPressed()) {
    ButtonGesture gesture = ButtonManager::detectGesture(false);
    handleGesture(gesture);

    // Khôi phục trạng thái đèn tương ứng với Bluetooth sau khi thực hiện xong cử chỉ
    if (BLEProvisioning::isConnected()) {
      LedManager::setColor(LED_GREEN);
    } else {
      LedManager::setColor(LED_BLUE);
    }
  }

  delay(20);
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

  // 1. Khôi phục cài đặt gốc:
  // - Bản 1 nút: Nhấn giữ SW1 15s
  // - Bản 3 nút: Giữ đồng thời SW1 + SW3 trong 5s (hoặc giữ SW1/SW3 15s)
  if (gesture == GESTURE_HOLD_15S_FACTORY_RESET || gesture == GESTURE_ALL_HOLD_RESET) {
    Serial.println("[RESET] Factory reset requested. Clearing Wi-Fi credentials...");
    WiFiManager::clearCredentials();
    LedManager::blink(LED_RED, 5, 100);
    BuzzerManager::beepError();
    return;
  }

  // 2. Chế độ cấu hình BLE & SoftAP:
  // - Bản 1 nút: 5 Clicks hoặc giữ SW1 10s
  // - Bản 3 nút: Giữ SW2 trong 5s (hoặc giữ SW1 10s)
  if (gesture == GESTURE_FIVE_CLICKS_PAIR || 
      gesture == GESTURE_HOLD_10S_PROVISION || 
      gesture == GESTURE_BTN2_HOLD_CONFIG) {
    Serial.println("[MODE] Starting BLE Auto-Pairing & SoftAP Provisioning...");
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

    WiFiManager::stopWiFi();
    BLEProvisioning::startAdvertising();
    return;
  }

  // 3. Phân loại nghiệp vụ theo cử chỉ & cấu hình từng nút (Món hàng / Dịch vụ)
  String eventTypeStr = "ORDER";
  LedColor actionColor = LED_BLUE;
  ButtonConfig activeConfig;
  bool hasActiveConfig = false;

  if (gesture == GESTURE_SHORT_PRESS || gesture == GESTURE_BTN1_ORDER) {
    activeConfig = ButtonManager::getButtonConfig(0);
    hasActiveConfig = true;
    if (activeConfig.actionType == ACTION_ORDER_ITEM) {
      eventTypeStr = "ORDER_ITEM";
      actionColor = LED_BLUE;
      Serial.printf("[ACTION] 🔘 SW1: ĐẶT MÓN HÀNG [%s] (Mã SKU: %s, Số lượng: %d)\n", 
                    activeConfig.itemName, activeConfig.itemId, activeConfig.quantity);
    } else {
      eventTypeStr = "ORDER";
      actionColor = LED_BLUE;
      Serial.println("[ACTION] 🔘 SW1: TẠO ĐƠN HÀNG MỚI (NEW ORDER)");
    }
  } else if (gesture == GESTURE_BTN2_BILL) {
    activeConfig = ButtonManager::getButtonConfig(1);
    hasActiveConfig = true;
    if (activeConfig.actionType == ACTION_ORDER_ITEM) {
      eventTypeStr = "ORDER_ITEM";
      actionColor = LED_CYAN;
      Serial.printf("[ACTION] 🔘 SW2: ĐẶT MÓN HÀNG [%s] (Mã SKU: %s, Số lượng: %d)\n", 
                    activeConfig.itemName, activeConfig.itemId, activeConfig.quantity);
    } else if (activeConfig.actionType == ACTION_CALL_STAFF) {
      eventTypeStr = "CALL_STAFF";
      actionColor = LED_CYAN;
      Serial.printf("[ACTION] 🔘 SW2: GỌI NHÂN VIÊN PHỤC VỤ (CALL STAFF)\n");
    } else {
      eventTypeStr = "REQUEST_BILL";
      actionColor = LED_CYAN;
      Serial.println("[ACTION] 🔘 SW2: YÊU CẦU THANH TOÁN / GỌI PHỤC VỤ (REQUEST BILL)");
    }
  } else if (gesture == GESTURE_DOUBLE_PRESS || gesture == GESTURE_BTN3_CANCEL) {
    activeConfig = ButtonManager::getButtonConfig(2);
    hasActiveConfig = true;
    if (activeConfig.actionType == ACTION_ORDER_ITEM) {
      eventTypeStr = "ORDER_ITEM";
      actionColor = LED_YELLOW;
      Serial.printf("[ACTION] 🔘 SW3: ĐẶT MÓN HÀNG [%s] (Mã SKU: %s, Số lượng: %d)\n", 
                    activeConfig.itemName, activeConfig.itemId, activeConfig.quantity);
    } else {
      eventTypeStr = "CANCEL_ORDER";
      actionColor = LED_YELLOW;
      Serial.println("[ACTION] 🔘 SW3 / DOUBLE-PRESS: HỦY ĐƠN HÀNG (CANCEL ORDER)");
    }
  } else if (gesture == GESTURE_HOLD_5S_ACTIVATE) {
    eventTypeStr = "ACTIVATE";
    actionColor = LED_PURPLE;
    Serial.println("[ACTION] 🔘 HOLD 5S: KÍCH HOẠT THIẾT BỊ / ĐỒNG BỘ CẤU HÌNH");
  }

  LedManager::setColor(actionColor);

  // 4. Kết nối Wi-Fi & Truyền sự kiện bảo mật
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

  int8_t rssi = WiFi.RSSI();
  Serial.printf("[API] Transmitting event: %s (Batt: %d%%, RSSI: %d dBm)\n",
                eventTypeStr.c_str(), batteryPct, rssi);

  OrderResult result = ApiClient::sendButtonEvent(
      BACKEND_URL,
      DEVICE_ID,
      DEVICE_SECRET,
      eventTypeStr,
      batteryPct,
      rssi,
      hasActiveConfig ? &activeConfig : nullptr
  );

  // 5. Phản hồi LED & Còi Buzzer trực quan theo từng nghiệp vụ
  if (result.success) {
    if (eventTypeStr == "CANCEL_ORDER") {
      Serial.println("[SUCCESS] Hủy đơn hàng thành công!");
      LedManager::setColor(LED_YELLOW);
      BuzzerManager::beepCancel();
    } else if (eventTypeStr == "REQUEST_BILL" || eventTypeStr == "CALL_STAFF") {
      Serial.println("[SUCCESS] Đã gửi yêu cầu dịch vụ tới phục vụ!");
      LedManager::setColor(LED_CYAN);
      BuzzerManager::beepBill();
    } else {
      if (hasActiveConfig && activeConfig.actionType == ACTION_ORDER_ITEM) {
        Serial.printf("[SUCCESS] GREEN LED: Đặt món [%s] thành công! Mã đơn: %s\n", 
                      activeConfig.itemName, result.orderNumber.c_str());
      } else {
        Serial.printf("[SUCCESS] GREEN LED: Đặt hàng thành công! Mã đơn: %s\n", result.orderNumber.c_str());
      }
      LedManager::setColor(LED_GREEN);
      BuzzerManager::beepSuccess();
    }
    delay(1200);
  } else {
    Serial.printf("[ERROR] RED LED: Máy chủ từ chối (%s)\n", result.errorMessage.c_str());
    LedManager::setColor(LED_RED);
    BuzzerManager::beepError();
    delay(1200);
  }

  // 6. Tắt Wi-Fi và LED trước khi quay lại chế độ ngủ
  WiFiManager::stopWiFi();
  LedManager::setColor(LED_OFF);
}
