#include "StateMachine.h"
#include "../power/PowerManager.h"
#include "../led/LedManager.h"
#include "../buzzer/BuzzerManager.h"
#include "../button/ButtonManager.h"
#include "../wifi/WiFiManager.h"
#include "../ble/BLEProvisioning.h"
#include "../api/ApiClient.h"
#include "../hmi/HmiManager.h"
#include "../display/TftDisplayManager.h"
#include "../../include/config.h"

// Device credentials for this firmware build (hoặc lưu trong NVS Flash)
static const String DEVICE_ID = "BTN-8829-WTR";
static const String DEVICE_SECRET = "sec_smart_button_8829_wtr_key_99";
static const String BACKEND_URL = DEFAULT_BACKEND_URL;

void StateMachine::init() {
  Serial.begin(115200);
  delay(50);

  Serial.println("\n==================================================");
  Serial.printf("Device ID: %s | Model: ESP32-S3-WROOM-1\n", DEVICE_ID.c_str());
#if CURRENT_BOARD_TARGET == BOARD_CHATBOT_V2_ESP32S3
  Serial.println("Hardware Board: CHATBOT V2_2025 (ESP32-S3 IoT Node)");
  Serial.println("Features: BẢN 1 NÚT (1 Nhấn = Đặt Hàng | 2 Nhấn = Hủy Đơn | Giữ 5s = Phát Bluetooth)");
#else
  Serial.printf("Hardware Mode: %s\n", 
    ButtonManager::getVariant() == HARDWARE_VARIANT_TRIPLE_BUTTON 
      ? "BẢN 3 NÚT (SW1: Order, SW2: Bill, SW3: Cancel)" 
      : "BẢN 1 NÚT (1 Nhấn = Đặt Hàng | 2 Nhấn = Hủy Đơn | Giữ 5s = Phát Bluetooth)");
  Serial.println("Schematic: Smart_Order_Button.kicad_sch");
#endif
  Serial.println("==================================================");

  // Initialize hardware drivers
  LedManager::init();
  BuzzerManager::init();
  ButtonManager::init();
  PowerManager::init();

  // Khởi tạo các giao tiếp hiển thị trên bo mạch CHATBOT V2
#if ENABLE_HMI_UART
  HmiManager::init();
  HmiManager::showBoot(DEVICE_ID, "ESP32-S3-WROOM-1");
#endif
#if ENABLE_TFT_SPI
  TftDisplayManager::init();
#endif

  // Hiệu ứng khởi động hệ thống
  BuzzerManager::beepBoot();

  uint32_t batteryMv = PowerManager::readBatteryMillivolts();
  uint8_t batteryPct = PowerManager::calculateBatteryPercentage(batteryMv);
  Serial.printf("[POWER] Battery Status: %lu mV (%u%%)\n", (unsigned long)batteryMv, batteryPct);
#if ENABLE_HMI_UART
  HmiManager::showBatteryStatus(batteryMv, batteryPct);
#endif

  // Nếu người dùng đang giữ nút trong lúc cắm nguồn (Cold boot gesture)
  if (ButtonManager::isPressed()) {
    ButtonGesture gesture = ButtonManager::detectGesture(false);
    handleGesture(gesture);
  } else {
    Serial.println("[READY] Đã sẵn sàng (Bản 1 Nút):");
    Serial.println("  👉 Nhấn 1 lần : ĐẶT HÀNG (ORDER)");
    Serial.println("  👉 Nhấn 2 lần : HỦY ĐƠN (CANCEL)");
    Serial.println("  👉 Giữ 5 giây : PHÁT BLUETOOTH (SOB-BTN-8829-WTR) để bắt trên FE");
  }
}

void StateMachine::loop() {
  // Theo dõi trạng thái kết nối Bluetooth (không khóa mạch)
  bool nowConnected = false;
  if (BLEProvisioning::checkConnectionChanged(nowConnected)) {
    if (nowConnected) {
      LedManager::setColor(LED_GREEN);
      BuzzerManager::beepSuccess();
#if ENABLE_HMI_UART
      HmiManager::showBleConnected();
#endif
    } else {
      BuzzerManager::beepCancel();
    }
  }

  // Nếu đang phát Bluetooth chờ kết nối thì nháy đèn Xanh Dương nhẹ
  if (BLEProvisioning::isActive() && !BLEProvisioning::isConnected() && !ButtonManager::isPressed()) {
    if ((millis() / 450) % 2 == 0) {
      LedManager::setColor(LED_BLUE);
    } else {
      LedManager::setColor(LED_OFF);
    }
  }

  // Đọc cử chỉ phím (Nút BOOT GPIO 0 hoặc SW1) nếu người dùng bấm
  if (ButtonManager::isPressed()) {
    ButtonGesture gesture = ButtonManager::detectGesture(false);
    handleGesture(gesture);
    if (!BLEProvisioning::isActive() || BLEProvisioning::isConnected()) {
      LedManager::setColor(BLEProvisioning::isConnected() ? LED_GREEN : LED_OFF);
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
    Serial.println("[GESTURE] Transient electrical bounce. Resuming...");
    return;
  }

  // 1. Khôi phục cài đặt gốc (Giữ 15s)
  if (gesture == GESTURE_HOLD_15S_FACTORY_RESET || gesture == GESTURE_ALL_HOLD_RESET) {
    Serial.println("[RESET] Factory reset requested. Clearing Wi-Fi credentials...");
    WiFiManager::clearCredentials();
    LedManager::blink(LED_RED, 5, 100);
    BuzzerManager::beepError();
    return;
  }

  // 2. Giữ nút 5 giây -> PHÁT BLUETOOTH (Không khóa mạch, không bắt buộc cấu hình lại)
  if (gesture == GESTURE_FIVE_CLICKS_PAIR || 
      gesture == GESTURE_HOLD_10S_PROVISION || 
      gesture == GESTURE_HOLD_5S_ACTIVATE ||
      gesture == GESTURE_BTN2_HOLD_CONFIG) {
    Serial.println("\n[BLE-MODE] >>> ĐÃ GIỮ NÚT 5 GIÂY: BẬT PHÁT BLUETOOTH (KHÔNG KHÓA MẠCH) <<<");
    LedManager::setColor(LED_BLUE);
    BuzzerManager::beepSuccess();

    String bleName = "SOB-" + DEVICE_ID;
    BLEProvisioning::start(bleName, DEVICE_ID);

#if ENABLE_HMI_UART
    HmiManager::showBleAdvertising(bleName, DEVICE_ID);
#endif

    // Chờ người dùng nhả nút ra nếu vẫn đang giữ
    while (ButtonManager::isPressed()) {
      delay(20);
    }
    return;
  }

  // 3. Phân loại nghiệp vụ Bản 1 Nút:
  // - Nhấn 1 lần (GESTURE_SHORT_PRESS / GESTURE_BTN1_ORDER) -> ĐẶT HÀNG
  // - Nhấn 2 lần (GESTURE_DOUBLE_PRESS / GESTURE_BTN3_CANCEL) -> HỦY ĐƠN
  String eventTypeStr = "ORDER";
  LedColor actionColor = LED_GREEN;
  ButtonConfig activeConfig;
  bool hasActiveConfig = false;

  if (gesture == GESTURE_SHORT_PRESS || gesture == GESTURE_BTN1_ORDER) {
    activeConfig = ButtonManager::getButtonConfig(0);
    hasActiveConfig = true;
    eventTypeStr = (activeConfig.actionType == ACTION_ORDER_ITEM) ? "ORDER_ITEM" : "ORDER";
    actionColor = LED_GREEN;
    Serial.printf("[ACTION] 🔘 NHẤN 1 LẦN (ĐẶT HÀNG): [%s] (Số lượng: %d)\n", 
                  activeConfig.itemName, activeConfig.quantity);
    BLEProvisioning::notifyEvent("EVENT:ORDER:" + String(activeConfig.itemName) + ":" + String(activeConfig.quantity));
  } else if (gesture == GESTURE_DOUBLE_PRESS || gesture == GESTURE_BTN3_CANCEL) {
    activeConfig = ButtonManager::getButtonConfig(2);
    hasActiveConfig = true;
    eventTypeStr = "CANCEL_ORDER";
    actionColor = LED_YELLOW;
    Serial.printf("[ACTION] 🔘 NHẤN 2 LẦN (HỦY ĐƠN): [%s]\n", activeConfig.itemName);
    BLEProvisioning::notifyEvent("EVENT:CANCEL:" + String(activeConfig.itemName));
  } else if (gesture == GESTURE_BTN2_BILL) {
    activeConfig = ButtonManager::getButtonConfig(1);
    hasActiveConfig = true;
    eventTypeStr = "REQUEST_BILL";
    actionColor = LED_CYAN;
    Serial.printf("[ACTION] 🔘 YÊU CẦU THANH TOÁN: [%s]\n", activeConfig.itemName);
    BLEProvisioning::notifyEvent("EVENT:BILL:" + String(activeConfig.itemName));
  }

  LedManager::setColor(actionColor);

#if ENABLE_HMI_UART
  if (eventTypeStr == "CANCEL_ORDER") {
    HmiManager::showCancelOrder();
  } else if (eventTypeStr == "REQUEST_BILL" || eventTypeStr == "CALL_STAFF") {
    HmiManager::showBillRequest();
  } else {
    HmiManager::showOrderSending(hasActiveConfig ? activeConfig.itemName : "Don Hang", hasActiveConfig ? activeConfig.quantity : 1);
  }
#endif

  // Nếu chưa lưu Wi-Fi, vẫn xác nhận Đặt Hàng / Hủy Đơn ngay qua Bluetooth & Đèn Còi (Không bắt cấu hình lại!)
  if (!WiFiManager::hasCredentials()) {
    if (eventTypeStr == "CANCEL_ORDER") {
      LedManager::setColor(LED_YELLOW);
      BuzzerManager::beepCancel();
      Serial.println("[SUCCESS] ✅ ĐÃ HỦY ĐƠN HÀNG (1 Nút - Nhấn 2 lần)!");
    } else {
      LedManager::setColor(LED_GREEN);
      BuzzerManager::beepSuccess();
      Serial.println("[SUCCESS] ✅ ĐÃ ĐẶT HÀNG THÀNH CÔNG (1 Nút - Nhấn 1 lần)!");
    }
    delay(700);
    return;
  }

  // 4. Kết nối Wi-Fi & Truyền sự kiện bảo mật
  uint32_t batteryMv = PowerManager::readBatteryMillivolts();
  uint8_t batteryPct = PowerManager::calculateBatteryPercentage(batteryMv);

  bool connected = false;
  for (int retry = 1; retry <= WIFI_MAX_RETRIES; retry++) {
    Serial.printf("[WIFI] Attempt %d/%d...\n", retry, WIFI_MAX_RETRIES);
#if ENABLE_HMI_UART
    HmiManager::showWifiConnecting(retry, WIFI_MAX_RETRIES);
#endif
    if (WiFiManager::connect(WIFI_CONNECT_TIMEOUT_MS)) {
      connected = true;
      break;
    }
    delay(100);
  }

  if (!connected) {
    // Vẫn phản hồi hoàn tất cử chỉ nút bấm qua Bluetooth
    if (eventTypeStr == "CANCEL_ORDER") {
      LedManager::setColor(LED_YELLOW);
      BuzzerManager::beepCancel();
    } else {
      LedManager::setColor(LED_GREEN);
      BuzzerManager::beepSuccess();
    }
    delay(700);
    return;
  }

  int8_t rssi = WiFi.RSSI();
  Serial.printf("[API] Transmitting event: %s (Batt: %d%%, RSSI: %d dBm)\n",
                eventTypeStr.c_str(), batteryPct, rssi);

#if ENABLE_HMI_UART
  HmiManager::showWifiConnected(WiFi.SSID(), WiFi.localIP().toString(), rssi, batteryPct);
#endif

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
  if (eventTypeStr == "CANCEL_ORDER") {
    Serial.println("[SUCCESS] Hủy đơn hàng thành công!");
#if ENABLE_HMI_UART
    HmiManager::showCancelSuccess();
#endif
    LedManager::setColor(LED_YELLOW);
    BuzzerManager::beepCancel();
  } else if (eventTypeStr == "REQUEST_BILL" || eventTypeStr == "CALL_STAFF") {
    Serial.println("[SUCCESS] Đã gửi yêu cầu dịch vụ tới phục vụ!");
#if ENABLE_HMI_UART
    HmiManager::showBillSuccess();
#endif
    LedManager::setColor(LED_CYAN);
    BuzzerManager::beepBill();
  } else {
    Serial.printf("[SUCCESS] GREEN LED: Đặt món [%s] thành công!\n", 
                  hasActiveConfig ? activeConfig.itemName : "Don Hang");
#if ENABLE_HMI_UART
    HmiManager::showOrderSuccess(result.orderNumber, hasActiveConfig ? activeConfig.itemName : "Don Hang", hasActiveConfig ? activeConfig.quantity : 1);
#endif
    LedManager::setColor(LED_GREEN);
    BuzzerManager::beepSuccess();
  }
  delay(800);
}
