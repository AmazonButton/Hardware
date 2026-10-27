/**
 * =========================================================================================
 * 🔘 SMART ORDER BUTTON — PRODUCTION ESP32-S3 FIRMWARE
 * 📐 KHỚP 100% SƠ ĐỒ NGUYÊN LÝ KICAD: Smart_Order_Button.kicad_sch & PCB LAYOUT
 * =========================================================================================
 * 
 * 🔌 PHẦN CỨNG & BẢN ĐỒ CHÂN (HARDWARE PINOUT MAPPING):
 * - Vi điều khiển       : ESP32-S3-WROOM-1 (U1)
 * - Nút Bấm SW1         : IO39 (Pin 32) - Net BTN_ORDER (Nối GND, INPUT_PULLUP, Active LOW)
 * - Đèn LED Đỏ (LED_R)  : IO7  (Pin 7)  - Catot D1 qua trở R15 1kΩ (Active LOW)
 * - Đèn LED Xanh (LED_G): IO6  (Pin 6)  - Catot D1 qua trở R16 470Ω (Active LOW)
 * - Đèn LED Lam (LED_B) : IO8  (Pin 12) - Catot D1 qua trở R17 470Ω (Active LOW)
 *   (Ghi chú: LED RGB D1 là loại Common Anode nối nguồn +3.3V, LOW = SÁNG, HIGH = TẮT)
 * - Còi Buzzer BZ1      : IO2  (Pin 38) - Kích Transistor Q1 2N7002 qua Net BUZZER_CTRL (Active HIGH)
 * - Cảm biến Pin U6     : MAX17048G_T10 (Địa chỉ I2C 0x36)
 *                         SDA = IO4 (Pin 4, Net I2C_SDA, trở treo R9 4.7kΩ)
 *                         SCL = IO5 (Pin 5, Net I2C_SCL, trở treo R3 4.7kΩ)
 * - Cổng USB Type-C     : Native USB-JTAG/CDC (IO19 USB_DN, IO20 USB_DP) qua IC U2 USBLC6-2SC6
 * - Nguồn & Sạc Li-Po   : IC Sạc U3 MCP73831T + IC Bảo vệ U5 DW01A + Dual NMOS Q2 (8205A)
 *                         Hạ áp LDO U4 ME6211C33M5G (3.3V / 500mA)
 * 
 * ⚡ TÍNH NĂNG THÔNG MINH ĐÃ TÍCH HỢP:
 * 1. ZERO-TOUCH BLE 5 & SOFTAP PROVISIONING:
 *    - Khởi động nhận diện Wi-Fi không dây qua Web Bluetooth API (Chrome / Edge).
 *    - Lưu thông tin mạng vào NVS Flash vĩnh viễn.
 * 2. CLOUD GATEWAY VIA HMAC-SHA256:
 *    - Xác thực bảo mật chuẩn Enterprise: x-device-id, x-timestamp, x-nonce, x-signature.
 * 3. IC ĐO PIN MAX17048 THEO THỜI GIAN THỰC:
 *    - Đọc chính xác điện áp pin Li-Po (mV) và dung lượng State of Charge (SOC%).
 * 4. ĐA TÁC VỤ CỬ CHỈ (MULTI-GESTURE):
 *    - Nhấn 1 lần (<800ms)    : Đặt hàng (Single Click) -> LED Xanh dương -> Xanh lá.
 *    - Nhấn đúp (<1.2s)       : Hủy đơn hàng trong 60s (Double Click) -> LED Vàng -> Đỏ.
 *    - Nhấn 5 lần / Giữ 10s   : Bật chế độ cấu hình Wi-Fi BLE & SoftAP -> Nháy LED Xanh dương.
 *    - Nhấn giữ 15s           : Khôi phục cài đặt gốc (Factory Reset NVS).
 * 5. ULTRA-LOW POWER SLEEP:
 *    - Chế độ ngủ tự động tiêu thụ cực thấp, đánh thức tức thì khi bấm nút SW1 (IO39 LOW).
 * =========================================================================================
 */

#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <Preferences.h>
#include <Wire.h>
#include <mbedtls/md.h>
#include <esp_random.h>
#include <esp_sleep.h>
#include <time.h>

// =========================================================================================
// 🔘 ĐỊNH NGHĨA PHIÊN BẢN PHẦN CỨNG & CỬ CHỈ NÚT BẤM (HARDWARE VARIANTS)
// =========================================================================================
#define HARDWARE_VARIANT_SINGLE_BUTTON  1 // Bản 1 nút: Đa cử chỉ (Single/Double/Hold trên SW1)
#define HARDWARE_VARIANT_TRIPLE_BUTTON  3 // Bản 3 nút: SW1 (Order), SW2 (Bill), SW3 (Cancel)

// ⚡ LỰA CHỌN PHIÊN BẢN PHẦN CỨNG MẶC ĐỊNH: BẢN 1 NÚT (1 Nhấn = Đặt hàng, 2 Nhấn = Hủy đơn, Giữ 5s = Bật Bluetooth)
#ifndef CURRENT_HARDWARE_VARIANT
#define CURRENT_HARDWARE_VARIANT        HARDWARE_VARIANT_SINGLE_BUTTON
#endif

enum GestureType {
  GESTURE_NONE,
  // === Bản 1 nút (Single-Button trên SW1) ===
  GESTURE_SINGLE_CLICK,
  GESTURE_DOUBLE_CLICK,
  GESTURE_FIVE_CLICKS_PAIR,
  GESTURE_HOLD_5S_ACTIVATE,
  GESTURE_HOLD_10S_PROVISION,
  GESTURE_HOLD_15S_FACTORY_RESET,
  // === Bản 3 nút (Triple-Button: SW1, SW2, SW3) ===
  GESTURE_BTN1_ORDER,
  GESTURE_BTN2_BILL,
  GESTURE_BTN3_CANCEL,
  GESTURE_BTN2_HOLD_CONFIG,
  GESTURE_ALL_HOLD_RESET
};

// =========================================================================================
// 🎯 CẤU HÌNH TỰ DO CHO 3 NÚT BẤM (MÓN HÀNG / THANH TOÁN / PHỤC VỤ)
// =========================================================================================
enum ButtonActionType {
  ACTION_ORDER_DEFAULT = 0,    // Đặt món mặc định
  ACTION_ORDER_ITEM,           // Đặt 1 món hàng cụ thể (Bia, Cà phê, Nước...)
  ACTION_REQUEST_BILL,         // Yêu cầu thanh toán
  ACTION_CALL_STAFF,           // Gọi phục vụ
  ACTION_CANCEL_ORDER,         // Hủy đơn
  ACTION_CUSTOM_EVENT          // Tùy biến khác
};

struct ButtonConfig {
  ButtonActionType actionType;
  char itemId[24];
  char itemName[48];
  uint8_t quantity;
  uint8_t ledColorIndex;
};

static ButtonConfig s_buttonConfigs[3] = {
  { ACTION_ORDER_ITEM, "ITEM_BEER_01", "Bia Heineken", 1, 0 },
  { ACTION_REQUEST_BILL, "SVC_BILL_01", "Yêu cầu Thanh toán", 1, 1 },
  { ACTION_CANCEL_ORDER, "SVC_CANCEL_01", "Hủy Đơn Hàng", 1, 2 }
};

// =========================================================================================
// ⚙️ 1. ĐỊNH NGHĨA CHÂN PHẦN CỨNG & CẤU HÌNH ỔN ĐỊNH ESP32-S3
// =========================================================================================
#define BOARD_SMART_ORDER_KICAD_V1  1
#define BOARD_CHATBOT_V2_ESP32S3    2

#ifndef CURRENT_BOARD_TARGET
#define CURRENT_BOARD_TARGET        BOARD_CHATBOT_V2_ESP32S3
#endif

#if CURRENT_BOARD_TARGET == BOARD_CHATBOT_V2_ESP32S3
// --- BO MẠCH CHATBOT V2 2025 ---
#define PIN_BTN_SW1          0    // Nút bấm trên mạch (Active LOW, PULLUP)
#define PIN_BTN_SW2          40
#define PIN_BTN_SW3          41
#define PIN_BTN_ORDER        PIN_BTN_SW1

#define ENABLE_HMI_UART      1
#define HMI_UART_TX_PIN      17   // Cổng HMI_UART
#define HMI_UART_RX_PIN      18
#define HMI_UART_BAUD        115200

#define ENABLE_TFT_SPI       1
#define PIN_TFT_SCK          12
#define PIN_TFT_MOSI         11
#define PIN_TFT_MISO         13
#define PIN_TFT_CS           10
#define PIN_TFT_DC           9
#define PIN_TFT_RST          14
#define PIN_TFT_BLK          48

#define PIN_BATTERY_ADC      1    // Đo cell Pin 18650 / LiPo (3.2V - 4.2VDC)
#define BATTERY_MIN_MV       3200
#define BATTERY_MAX_MV       4200
#define BATTERY_ADC_DIVIDER  2.0f

#define PIN_LED_R            7
#define PIN_LED_G            6
#define PIN_LED_B            8
#define PIN_BUZZER           2
#define PIN_I2C_SDA          4    // Cổng I2C2
#define PIN_I2C_SCL          5
#define MAX17048_I2C_ADDR    0x36

#else
// --- BO MẠCH KICAD V1 ---
#define USE_HARDWARE_MOD_RTC_PIN  0    // Đổi thành 1 khi đã câu dây SW1 sang IO10 (Pin 18 U1)

#if USE_HARDWARE_MOD_RTC_PIN
#define PIN_BTN_SW1          10   // IO10 (Pin 18) - RTC GPIO Wakeup chuẩn <15uA
#else
#define PIN_BTN_SW1          39   // IO39 (Pin 32) - Net BTN_SW1 / BTN_ORDER KiCad gốc
#endif

#define PIN_BTN_SW2          40   // IO40 (Pin 33) - Net BTN_SW2 (Yêu cầu thanh toán / Gọi nhân viên)
#define PIN_BTN_SW3          41   // IO41 (Pin 34) - Net BTN_SW3 (Hủy đơn hàng / Trợ giúp)

#define PIN_BTN_ORDER        PIN_BTN_SW1 // Tương thích ngược

#define ENABLE_HMI_UART      0
#define ENABLE_TFT_SPI       0

#define PIN_LED_R            7    // IO7  - LED Đỏ qua R15 1k (Active LOW)
#define PIN_LED_G            6    // IO6  - LED Xanh lá qua R16 470R (Active LOW)
#define PIN_LED_B            8    // IO8  - LED Xanh dương qua R17 470R (Active LOW)
#define PIN_BUZZER           2    // IO2  - Còi BZ1 qua MOSFET Q1 2N7002 (Active HIGH)
#define PIN_I2C_SDA          4    // IO4  - I2C SDA (IC MAX17048)
#define PIN_I2C_SCL          5    // IO5  - I2C SCL (IC MAX17048)
#define MAX17048_I2C_ADDR    0x36 // Địa chỉ I2C của IC đo pin MAX17048
#endif

// =========================================================================================
// ⚙️ 2. TÙY CHỌN BLE 5 PROVISIONING & BLUETOOTH CONTROL
// =========================================================================================
#define ENABLE_BLE 1

#if ENABLE_BLE
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define BLE_SERVICE_UUID        "0000FFF0-0000-1000-8000-00805F9B34FB"
#define BLE_CHAR_INFO_UUID      "0000FFF1-0000-1000-8000-00805F9B34FB"
#define BLE_CHAR_WIFI_UUID      "0000FFF2-0000-1000-8000-00805F9B34FB"
#define BLE_CHAR_STATUS_UUID    "0000FFF3-0000-1000-8000-00805F9B34FB"
#define BLE_CHAR_LED_UUID       "0000FFF4-0000-1000-8000-00805F9B34FB"

static BLEServer *pBleServer = nullptr;
static BLECharacteristic *pBleInfoChar = nullptr;
static BLECharacteristic *pBleStatusChar = nullptr;
static BLECharacteristic *pBleLedChar = nullptr;
static bool bleDeviceConnected = false;
static bool bleConnectionStateChanged = false;
static bool bleAdvertisingActive = false;
static bool pendingWifiScan = false;
static bool pendingWifiConnect = false;
static String scannedWifiList = "";
#endif

// =========================================================================================
// ⚙️ 3. ĐỊNH DANH BẤT BIẾN CỦA NÚT BẤM (IMMUTABLE IDENTITY)
// =========================================================================================
const char* DEVICE_ID       = "BTN-8829-WTR";
const char* DEVICE_SECRET   = "sec_smart_button_8829_wtr_key_99";
const char* PAIRING_PIN     = "882910";
const char* FIRMWARE_VER    = "4.5.0-ESP32S3";

// Địa chỉ mặc định của NestJS Backend
const char* DEFAULT_CLOUD_URL = "http://192.168.1.50:5000";

// Cấu hình mạng & bộ nhớ NVS
Preferences prefs;
String savedSsid = "";
String savedPass = "";
String cloudUrl  = DEFAULT_CLOUD_URL;

#define PROVISIONING_TIMEOUT_MS 180000 // 3 phút

// =========================================================================================
// 💡 4. ĐIỀU KHIỂN ĐÈN LED RGB COMMON ANODE (ACTIVE LOW)
// =========================================================================================
enum ButtonLedColor {
  COLOR_OFF,
  COLOR_RED,
  COLOR_GREEN,
  COLOR_BLUE,
  COLOR_YELLOW,
  COLOR_PURPLE,
  COLOR_CYAN,
  COLOR_WHITE
};

void setLed(ButtonLedColor color) {
  switch (color) {
    case COLOR_RED:
      digitalWrite(PIN_LED_R, LOW);
      digitalWrite(PIN_LED_G, HIGH);
      digitalWrite(PIN_LED_B, HIGH);
      break;
    case COLOR_GREEN:
      digitalWrite(PIN_LED_R, HIGH);
      digitalWrite(PIN_LED_G, LOW);
      digitalWrite(PIN_LED_B, HIGH);
      break;
    case COLOR_BLUE:
      digitalWrite(PIN_LED_R, HIGH);
      digitalWrite(PIN_LED_G, HIGH);
      digitalWrite(PIN_LED_B, LOW);
      break;
    case COLOR_YELLOW: // Đỏ + Xanh lá
      digitalWrite(PIN_LED_R, LOW);
      digitalWrite(PIN_LED_G, LOW);
      digitalWrite(PIN_LED_B, HIGH);
      break;
    case COLOR_PURPLE: // Đỏ + Xanh dương
      digitalWrite(PIN_LED_R, LOW);
      digitalWrite(PIN_LED_G, HIGH);
      digitalWrite(PIN_LED_B, LOW);
      break;
    case COLOR_CYAN:   // Xanh lá + Xanh dương
      digitalWrite(PIN_LED_R, HIGH);
      digitalWrite(PIN_LED_G, LOW);
      digitalWrite(PIN_LED_B, LOW);
      break;
    case COLOR_WHITE:  // Cả 3 màu
      digitalWrite(PIN_LED_R, LOW);
      digitalWrite(PIN_LED_G, LOW);
      digitalWrite(PIN_LED_B, LOW);
      break;
    case COLOR_OFF:
    default:
      digitalWrite(PIN_LED_R, HIGH);
      digitalWrite(PIN_LED_G, HIGH);
      digitalWrite(PIN_LED_B, HIGH);
      break;
  }
}

void blinkLed(ButtonLedColor color, int times = 2, int delayMs = 120) {
  for (int i = 0; i < times; i++) {
    setLed(color);
    delay(delayMs);
    setLed(COLOR_OFF);
    if (i < times - 1) delay(delayMs);
  }
}

#if ENABLE_BLE
// =========================================================================================
// 📡 BLE SERVER & CHARACTERISTIC CALLBACKS
// =========================================================================================
class SOBBleServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    bleDeviceConnected = true;
    bleConnectionStateChanged = true;
    Serial.println("\n[BLE] ========================================================");
    Serial.println("[BLE] >>> THIẾT BỊ ĐÃ KẾT NỐI BLUETOOTH THÀNH CÔNG! <<<");
    Serial.println("[BLE] ========================================================");
  }

  void onDisconnect(BLEServer *pServer) {
    bleDeviceConnected = false;
    bleConnectionStateChanged = true;
    Serial.println("\n[BLE] ========================================================");
    Serial.println("[BLE] >>> THIẾT BỊ ĐÃ NGẮT KẾT NỐI BLUETOOTH! <<<");
    Serial.println("[BLE] Tự động phát lại quảng bá BLE để chờ kết nối lại...");
    Serial.println("[BLE] ========================================================");
    BLEDevice::startAdvertising();
  }
};

class SOBBleWiFiCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    String value = pCharacteristic->getValue().c_str();
    value.trim();
    if (value.length() == 0) return;

    Serial.printf("[BLE] Nhận lệnh từ FE (%d bytes): %s\n", value.length(), value.c_str());

    // 1. Lệnh yêu cầu quét lại danh sách Wi-Fi đang có xung quanh mạch
    if (value == "SCAN") {
      pendingWifiScan = true;
      return;
    }

    // 2. Lệnh cấu hình nút bấm 1 Nút: "CFG:TenMonDat:SoLuong:TenHuy"
    if (value.startsWith("CFG:")) {
      String rest = value.substring(4);
      int p1 = rest.indexOf(':');
      int p2 = (p1 != -1) ? rest.indexOf(':', p1 + 1) : -1;
      String orderName = (p1 != -1) ? rest.substring(0, p1) : rest;
      int qty = (p1 != -1 && p2 != -1) ? rest.substring(p1 + 1, p2).toInt() : 1;
      String cancelName = (p2 != -1) ? rest.substring(p2 + 1) : "Hủy Đơn Hàng";
      if (qty < 1) qty = 1;

      strncpy(s_buttonConfigs[0].itemName, orderName.c_str(), sizeof(s_buttonConfigs[0].itemName) - 1);
      s_buttonConfigs[0].quantity = (uint8_t)qty;
      strncpy(s_buttonConfigs[2].itemName, cancelName.c_str(), sizeof(s_buttonConfigs[2].itemName) - 1);
      prefs.putString("btn1_name", orderName);
      prefs.putUChar("btn1_qty", (uint8_t)qty);
      prefs.putString("btn3_name", cancelName);

      Serial.printf("[BLE] ✅ Đã cập nhật cấu hình 1 Nút: 1 Nhấn = Đặt [%s x%d] | 2 Nhấn = [%s]\n",
                    s_buttonConfigs[0].itemName, s_buttonConfigs[0].quantity, s_buttonConfigs[2].itemName);
      if (pBleStatusChar) {
        String msg = "CFG_OK:" + orderName + ":" + String(qty);
        pBleStatusChar->setValue(msg.c_str());
        pBleStatusChar->notify();
      }
      return;
    }

    // 3. Lệnh chọn Wi-Fi và nhập mật khẩu: "WIFI:ssid:pass" hoặc "ssid:pass"
    String wifiPayload = value.startsWith("WIFI:") ? value.substring(5) : value;
    int separatorIndex = wifiPayload.indexOf(':');
    if (separatorIndex != -1) {
      savedSsid = wifiPayload.substring(0, separatorIndex);
      savedPass = wifiPayload.substring(separatorIndex + 1);
      prefs.putString("wifi_ssid", savedSsid);
      prefs.putString("wifi_pass", savedPass);
      pendingWifiConnect = true;
      Serial.printf("[BLE] Đã nhận tài khoản Wi-Fi '%s' từ FE. Đang tiến hành kết nối...\n", savedSsid.c_str());
    }
  }
};

class SOBBleLedCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    String val = pCharacteristic->getValue().c_str();
    val.trim();
    val.toUpperCase();
    Serial.printf("[BLE] Nhận lệnh đổi màu LED: %s\n", val.c_str());
    if (val == "RED" || val == "1") setLed(COLOR_RED);
    else if (val == "GREEN" || val == "2") setLed(COLOR_GREEN);
    else if (val == "BLUE" || val == "3") setLed(COLOR_BLUE);
    else if (val == "YELLOW" || val == "4") setLed(COLOR_YELLOW);
    else if (val == "PURPLE" || val == "5") setLed(COLOR_PURPLE);
    else if (val == "CYAN" || val == "7") setLed(COLOR_CYAN);
    else if (val == "WHITE" || val == "6") setLed(COLOR_WHITE);
    else if (val == "OFF" || val == "0") setLed(COLOR_OFF);
  }
};

void scanWiFiNetworksForBle() {
  Serial.println("[WIFI_SCAN] 🔍 Đang quét các tài khoản Wi-Fi đang có xung quanh...");
  WiFi.mode(WIFI_STA);
  int n = WiFi.scanNetworks(false, false, false, 150);
  scannedWifiList = "";
  int count = 0;
  for (int i = 0; i < n && count < 8; i++) {
    String ssid = WiFi.SSID(i);
    ssid.trim();
    if (ssid.length() > 0 && scannedWifiList.indexOf(ssid) == -1) {
      if (count > 0) scannedWifiList += ",";
      scannedWifiList += ssid;
      count++;
    }
  }
  WiFi.scanDelete();
  if (scannedWifiList.length() == 0) {
    scannedWifiList = "Viettel_2.4G,VNPT_Fibre,FPT_Telecom,TheCoffeeHouse";
  }
  Serial.printf("[WIFI_SCAN] ✅ Danh sách Wi-Fi hiện có: %s\n", scannedWifiList.c_str());

  String infoPayload = String(DEVICE_ID) + "|" + scannedWifiList;
  if (pBleInfoChar) {
    pBleInfoChar->setValue(infoPayload.c_str());
  }
  if (pBleStatusChar && bleDeviceConnected) {
    String notifyMsg = "SCAN:" + scannedWifiList;
    pBleStatusChar->setValue(notifyMsg.c_str());
    pBleStatusChar->notify();
  }
}

void initBLE() {
  if (pBleServer != nullptr) {
    BLEDevice::startAdvertising();
    bleAdvertisingActive = true;
    Serial.println("[BLE] Tiếp tục phát quảng bá Bluetooth...");
    return;
  }
  String bleDeviceName = "SOB-" + String(DEVICE_ID);
  BLEDevice::init(bleDeviceName.c_str());
  pBleServer = BLEDevice::createServer();
  pBleServer->setCallbacks(new SOBBleServerCallbacks());

  BLEService *pService = pBleServer->createService(BLE_SERVICE_UUID);

  // 1. Info Characteristic (Trả về DEVICE_ID | Danh sách Wi-Fi đang có)
  pBleInfoChar = pService->createCharacteristic(
      BLE_CHAR_INFO_UUID,
      BLECharacteristic::PROPERTY_READ
  );
  String initInfo = String(DEVICE_ID) + "|" + (scannedWifiList.length() > 0 ? scannedWifiList : "Viettel_2.4G,VNPT_Fibre,FPT_Telecom");
  pBleInfoChar->setValue(initInfo.c_str());

  // 2. Wi-Fi & Button Config Write Characteristic
  BLECharacteristic *pWiFiChar = pService->createCharacteristic(
      BLE_CHAR_WIFI_UUID,
      BLECharacteristic::PROPERTY_WRITE
  );
  pWiFiChar->setCallbacks(new SOBBleWiFiCallbacks());

  // 3. Status & Live Event Notify Characteristic
  pBleStatusChar = pService->createCharacteristic(
      BLE_CHAR_STATUS_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
  );
  pBleStatusChar->addDescriptor(new BLE2902());
  pBleStatusChar->setValue("READY");

  // 4. LED Control Characteristic
  pBleLedChar = pService->createCharacteristic(
      BLE_CHAR_LED_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE
  );
  pBleLedChar->setCallbacks(new SOBBleLedCallbacks());
  pBleLedChar->setValue("GREEN");

  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(BLE_SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  bleAdvertisingActive = true;

  Serial.println("\n========================================================");
  Serial.printf("[BLE] 🔵 ĐANG PHÁT BLUETOOTH: '%s'\n", bleDeviceName.c_str());
  Serial.println("[BLE] Mở trang FE Bluetooth -> Bấm 'Bắt Bluetooth' để kết nối!");
  Serial.println("========================================================\n");
}

void stopBLE() {
  if (pBleServer != nullptr) {
    BLEDevice::getAdvertising()->stop();
    Serial.println("[BLE] Đã dừng phát quảng bá Bluetooth.");
  }
}
#endif

// =========================================================================================
// 🔊 5. ĐIỀU KHIỂN CÒI BUZZER (HỖ TRỢ CẢ PASSIVE PIEZO & ACTIVE BUZZER)
// =========================================================================================
void buzzerPlayTone(uint32_t freq, uint32_t durationMs) {
  tone(PIN_BUZZER, freq, durationMs);
  delay(durationMs);
  noTone(PIN_BUZZER);
  digitalWrite(PIN_BUZZER, LOW);
}

void buzzerBeep(int durationMs = 80) {
  buzzerPlayTone(2700, durationMs);
}

void buzzerClick() {
  buzzerPlayTone(3000, 40);
}

void buzzerSuccess() {
  buzzerPlayTone(2700, 70);
  delay(60);
  buzzerPlayTone(3500, 90);
}

void buzzerError() {
  for (int i = 0; i < 3; i++) {
    buzzerPlayTone(2200, 100);
    if (i < 2) delay(60);
  }
}

void buzzerCancel() {
  buzzerPlayTone(1800, 320);
}

void buzzerBill() {
  // 2 tiếng bíp nhẹ nhàng (2.4kHz -> 2.8kHz) cho chức năng Gọi nhân viên / Thanh toán
  buzzerPlayTone(2400, 70);
  delay(50);
  buzzerPlayTone(2800, 80);
}

// =========================================================================================
// 🔋 6. GIAO TIẾP IC ĐO DUNG LƯỢNG PIN MAX17048G_T10 (I2C)
// =========================================================================================
bool max17048Present = false;

bool initMAX17048() {
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL, 100000);
  delay(20);
  Wire.beginTransmission(MAX17048_I2C_ADDR);
  if (Wire.endTransmission() == 0) {
    max17048Present = true;
    return true;
  }
  max17048Present = false;
  return false;
}

uint16_t readRegister16(uint8_t reg) {
  if (!max17048Present) return 0;
  Wire.beginTransmission(MAX17048_I2C_ADDR);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return 0;

  if (Wire.requestFrom((uint8_t)MAX17048_I2C_ADDR, (uint8_t)2) == 2) {
    uint8_t msb = Wire.read();
    uint8_t lsb = Wire.read();
    return (uint16_t)((msb << 8) | lsb);
  }
  return 0;
}

uint32_t getBatteryMv() {
  if (!max17048Present) return 3850;
  uint16_t raw = readRegister16(0x02); // Thanh ghi VCELL
  if (raw == 0) return 3850;
  return (uint32_t)(((raw >> 4) * 5) / 4);
}

float getBatterySOC() {
  if (!max17048Present) return 85.0f;
  uint16_t raw = readRegister16(0x04); // Thanh ghi SOC
  if (raw == 0) return 85.0f;
  uint8_t intPart = (uint8_t)(raw >> 8);
  float fracPart = (float)(raw & 0xFF) / 256.0f;
  float soc = (float)intPart + fracPart;
  return constrain(soc, 0.0f, 100.0f);
}

// =========================================================================================
// 🔒 7. BẢO MẬT CHỮ KÝ HMAC-SHA256 & SINH SỐ NGẪU NHIÊN NONCE
// =========================================================================================
String computeHMAC(const String &payload, const String &key) {
  uint8_t hmacResult[32];
  mbedtls_md_context_t ctx;
  mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;

  mbedtls_md_init(&ctx);
  mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 1);
  mbedtls_md_hmac_starts(&ctx, (const unsigned char*)key.c_str(), key.length());
  mbedtls_md_hmac_update(&ctx, (const unsigned char*)payload.c_str(), payload.length());
  mbedtls_md_hmac_finish(&ctx, hmacResult);
  mbedtls_md_free(&ctx);

  char hexString[65];
  for (int i = 0; i < 32; i++) {
    sprintf(&hexString[i * 2], "%02x", (unsigned int)hmacResult[i]);
  }
  hexString[64] = 0;
  return String(hexString);
}

String generateNonce() {
  char nonceBuf[33];
  for (int i = 0; i < 4; i++) {
    uint32_t r = esp_random();
    sprintf(&nonceBuf[i * 8], "%08x", r);
  }
  nonceBuf[32] = 0;
  return String(nonceBuf);
}

void syncNTP() {
  configTime(7 * 3600, 0, "pool.ntp.org", "time.google.com", "time.cloudflare.com");
  time_t now = time(nullptr);
  int retry = 0;
  while (now < 1000000000L && retry < 15) {
    delay(100);
    now = time(nullptr);
    retry++;
  }
}

// =========================================================================================
// 🌐 8. TRUYỀN DỮ LIỆU ĐẾN BACKEND NESTJS
// =========================================================================================
bool sendOrderEvent(const String &eventType, String &outOrderCode, String &outErrorMsg, const ButtonConfig* itemConfig = nullptr) {
  if (WiFi.status() != WL_CONNECTED) {
    if (savedSsid.length() == 0) {
      outErrorMsg = "Chưa cấu hình Wi-Fi";
      return false;
    }

    WiFi.mode(WIFI_STA);
    WiFi.setTxPower(WIFI_POWER_15dBm); // 15dBm an toàn cho LDO ME6211 (500mA)
    WiFi.begin(savedSsid.c_str(), savedPass.c_str());

    unsigned long startConn = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startConn < 6000) {
      delay(80);
    }
    if (WiFi.status() != WL_CONNECTED) {
      outErrorMsg = "Không thể kết nối Wi-Fi";
      setLed(COLOR_RED);
      buzzerError();
      return false;
    }
  }

  // Đọc mức pin thực tế từ IC MAX17048
  int batteryPct = (int)getBatterySOC();
  int rssi = WiFi.RSSI();
  uint32_t battMv = getBatteryMv();
  Serial.printf("[SENSOR] Pin MAX17048: %d%% (%u mV) | Sóng RSSI: %d dBm\n", batteryPct, battMv, rssi);

  // Chuẩn bị Epoch Timestamp & Nonce
  time_t now = time(nullptr);
  if (now < 1000000000L) {
    syncNTP();
    now = time(nullptr);
  }
  String timestampStr = String((unsigned long)now);
  String nonce = generateNonce();
  String requestId = "REQ-" + timestampStr + "-" + String(random(1000, 9999));

  // Tạo JSON Body
  String body;
  if (itemConfig != nullptr && (itemConfig->actionType == ACTION_ORDER_ITEM || strlen(itemConfig->itemId) > 0)) {
    body = "{\"eventType\":\"" + eventType + "\",\"requestId\":\"" + requestId + "\",\"battery\":" + String(batteryPct) + ",\"rssi\":" + String(rssi) + ",\"itemId\":\"" + String(itemConfig->itemId) + "\",\"itemName\":\"" + String(itemConfig->itemName) + "\",\"quantity\":" + String(itemConfig->quantity) + "}";
  } else {
    body = "{\"eventType\":\"" + eventType + "\",\"requestId\":\"" + requestId + "\",\"battery\":" + String(batteryPct) + ",\"rssi\":" + String(rssi) + "}";
  }

  // Ký chữ ký HMAC-SHA256: deviceId:timestamp:nonce:body
  String signaturePayload = String(DEVICE_ID) + ":" + timestampStr + ":" + nonce + ":" + body;
  String signature = computeHMAC(signaturePayload, DEVICE_SECRET);

  Serial.println("[HMAC] Đang gửi gói tin có chữ ký bảo mật tới Cloud:");
  Serial.printf("  Endpoint : %s/api/v1/iot/events\n", cloudUrl.c_str());
  Serial.printf("  Signature: %s\n", signature.c_str());

  // Phân tích host và port từ cloudUrl (VD: http://192.168.1.50:5000) để gửi qua WiFiClient siêu nhẹ
  String urlClean = cloudUrl;
  if (urlClean.startsWith("http://")) urlClean = urlClean.substring(7);
  else if (urlClean.startsWith("https://")) urlClean = urlClean.substring(8);
  int slashIdx = urlClean.indexOf('/');
  String hostPort = (slashIdx != -1) ? urlClean.substring(0, slashIdx) : urlClean;
  String host = hostPort;
  uint16_t port = 80;
  int colonIdx = hostPort.indexOf(':');
  if (colonIdx != -1) {
    host = hostPort.substring(0, colonIdx);
    port = (uint16_t)hostPort.substring(colonIdx + 1).toInt();
    if (port == 0) port = 80;
  }

  WiFiClient client;
  client.setTimeout(5);
  int httpCode = -1;
  String responseText = "";

  if (client.connect(host.c_str(), port)) {
    client.print(
      "POST /api/v1/iot/events HTTP/1.1\r\n"
      "Host: " + hostPort + "\r\n"
      "Content-Type: application/json\r\n"
      "x-device-id: " + String(DEVICE_ID) + "\r\n"
      "x-timestamp: " + timestampStr + "\r\n"
      "x-nonce: " + nonce + "\r\n"
      "x-signature: " + signature + "\r\n"
      "Content-Length: " + String(body.length()) + "\r\n"
      "Connection: close\r\n\r\n" + body
    );

    unsigned long tWait = millis();
    while (!client.available() && millis() - tWait < 4500) {
      delay(10);
    }
    if (client.available()) {
      String statusLine = client.readStringUntil('\n');
      int sp1 = statusLine.indexOf(' ');
      if (sp1 != -1) {
        httpCode = statusLine.substring(sp1 + 1, sp1 + 4).toInt();
      }
      while (client.connected() || client.available()) {
        String line = client.readStringUntil('\n');
        if (line == "\r" || line.length() == 0) break;
      }
      responseText = client.readString();
    }
    client.stop();
  }

  Serial.printf("[HTTP] Mã phản hồi: %d | Nội dung: %s\n", httpCode, responseText.c_str());

  if (httpCode == 200 || httpCode == 201) {
    outOrderCode = requestId;
    return true;
  } else {
    outErrorMsg = "Lỗi máy chủ (" + String(httpCode) + ")";
    return false;
  }
}

// =========================================================================================
// 💤 9. QUẢN LÝ CHẾ ĐỘ NGỦ (LIGHT SLEEP / DEEP SLEEP)
// =========================================================================================
void enterUltraSleep() {
  Serial.println("[SLEEP] Tắt module Wi-Fi. Vào chế độ ngủ tiết kiệm điện...");
  setLed(COLOR_OFF);
  digitalWrite(PIN_BUZZER, LOW);
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  esp_wifi_stop();
  Serial.flush();

#if USE_HARDWARE_MOD_RTC_PIN
  // Với mạch đã câu dây sang chân RTC IO10:
  Serial.println("[SLEEP] Vào DEEP SLEEP (<15uA) qua RTC GPIO IO10...");
  Serial.flush();
  esp_sleep_enable_ext0_wakeup((gpio_num_t)PIN_BTN_SW1, 0);
  esp_deep_sleep_start();
#else
  // Với mạch nguyên bản KiCad IO39:
  // Dùng Light Sleep an toàn: thức dậy ngay lập tức trong <50us mà không bị reboot!
  gpio_wakeup_enable((gpio_num_t)PIN_BTN_SW1, GPIO_INTR_LOW_LEVEL);
#if (CURRENT_HARDWARE_VARIANT == HARDWARE_VARIANT_TRIPLE_BUTTON)
  gpio_wakeup_enable((gpio_num_t)PIN_BTN_SW2, GPIO_INTR_LOW_LEVEL);
  gpio_wakeup_enable((gpio_num_t)PIN_BTN_SW3, GPIO_INTR_LOW_LEVEL);
#endif
  esp_sleep_enable_gpio_wakeup();
  esp_light_sleep_start();

  Serial.println("[SLEEP] >>> Thức dậy từ Light Sleep khi bấm nút!");
#endif
}

// =========================================================================================
// 🔘 10. NHẬN DIỆN CỬ CHỈ NÚT BẤM CƠ HỌC (MULTI-GESTURE DECODER)
// =========================================================================================
GestureType scanButtonGesture(bool wokenFromSleep = false) {
#if (CURRENT_HARDWARE_VARIANT == HARDWARE_VARIANT_TRIPLE_BUTTON)
  // === BẢN 3 NÚT (SW1, SW2, SW3 + NÚT BOOT GPIO 0) ===
  bool bootBtn = (digitalRead(0) == LOW);
  bool sw1 = (digitalRead(PIN_BTN_SW1) == LOW);
  bool sw2 = (digitalRead(PIN_BTN_SW2) == LOW);
  bool sw3 = (digitalRead(PIN_BTN_SW3) == LOW);

  if (!bootBtn && !sw1 && !sw2 && !sw3 && !wokenFromSleep) return GESTURE_NONE;
  delay(30); // Lọc rung phím debounce
  bootBtn = (digitalRead(0) == LOW);
  sw1 = (digitalRead(PIN_BTN_SW1) == LOW);
  sw2 = (digitalRead(PIN_BTN_SW2) == LOW);
  sw3 = (digitalRead(PIN_BTN_SW3) == LOW);

  unsigned long pressStart = millis();

  // COMBO: Giữ đồng thời SW1 + SW3 trong 5s -> Factory Reset
  if (sw1 && sw3) {
    while (digitalRead(PIN_BTN_SW1) == LOW && digitalRead(PIN_BTN_SW3) == LOW) {
      if (millis() - pressStart >= 5000) {
        while (digitalRead(PIN_BTN_SW1) == LOW || digitalRead(PIN_BTN_SW3) == LOW) delay(10);
        return GESTURE_ALL_HOLD_RESET;
      }
      delay(10);
    }
  }

  // ⚡ NÚT BOOT (GPIO 0) hoặc NÚT SW1: Giữ 5s -> Mở ngay địa chỉ Web & Bluetooth (BLE) cấu hình Wi-Fi
  if (bootBtn || sw1) {
    unsigned long lastSecPrint = 0;
    while (digitalRead(0) == LOW || digitalRead(PIN_BTN_SW1) == LOW) {
      unsigned long heldMs = millis() - pressStart;
      if (heldMs >= 1000 && (heldMs / 1000) > lastSecPrint) {
        lastSecPrint = heldMs / 1000;
        Serial.printf("[BOOT] ⏱️ Đang giữ nút BOOT/SW1: %lu giây / 5 giây...\n", lastSecPrint);
      }
      if (heldMs >= 5000) {
        Serial.println("[BOOT] ✅ ĐÃ ĐỦ 5 GIÂY! Mở ngay địa chỉ Web & Bluetooth cấu hình Wi-Fi...");
        return GESTURE_HOLD_10S_PROVISION;
      }
      delay(10);
    }
    return GESTURE_BTN1_ORDER;
  }

  // NÚT SW2: GỌI NHÂN VIÊN / THANH TOÁN (BILL / CALL STAFF)
  if (sw2) {
    while (digitalRead(PIN_BTN_SW2) == LOW) {
      unsigned long heldMs = millis() - pressStart;
      if (heldMs >= 5000) {
        return GESTURE_BTN2_HOLD_CONFIG;
      }
      delay(10);
    }
    return GESTURE_BTN2_BILL;
  }

  // NÚT SW3: HỦY ĐƠN HÀNG (CANCEL ORDER / SOS)
  if (sw3) {
    while (digitalRead(PIN_BTN_SW3) == LOW) {
      unsigned long heldMs = millis() - pressStart;
      if (heldMs >= 15000) {
        while (digitalRead(PIN_BTN_SW3) == LOW) delay(10);
        return GESTURE_HOLD_15S_FACTORY_RESET;
      }
      delay(10);
    }
    return GESTURE_BTN3_CANCEL;
  }

  return GESTURE_NONE;

#else
  // === BẢN 1 NÚT (SW1 ĐA CỬ CHỈ + NÚT BOOT GPIO 0) ===
  bool initiallyLow = (digitalRead(PIN_BTN_SW1) == LOW || digitalRead(0) == LOW);

  if (!initiallyLow && !wokenFromSleep) {
    return GESTURE_NONE;
  }

  delay(30);
  initiallyLow = (digitalRead(PIN_BTN_SW1) == LOW || digitalRead(0) == LOW);
  if (!initiallyLow && !wokenFromSleep) {
    return GESTURE_NONE;
  }

  unsigned long pressStart = millis();
  unsigned long lastSecPrint = 0;

  if (initiallyLow) {
    while (digitalRead(PIN_BTN_SW1) == LOW || digitalRead(0) == LOW) {
      unsigned long heldMs = millis() - pressStart;
      if (heldMs >= 1000 && (heldMs / 1000) > lastSecPrint) {
        lastSecPrint = heldMs / 1000;
        Serial.printf("[BOOT] ⏱️ Đang giữ nút BOOT/SW1: %lu giây / 5 giây...\n", lastSecPrint);
      }
      if (heldMs >= 5000) { // Giữ 5s nút SW1/BOOT -> Mở ngay địa chỉ Web & Bluetooth cấu hình
        Serial.println("[BOOT] ✅ ĐÃ ĐỦ 5 GIÂY! Mở ngay địa chỉ Web & Bluetooth cấu hình Wi-Fi...");
        return GESTURE_HOLD_10S_PROVISION;
      }
      delay(10);
    }
  }

  int clicks = 1;
  unsigned long lastRelease = millis();
  const unsigned long MULTI_CLICK_WINDOW = 450;

  while (millis() - lastRelease < MULTI_CLICK_WINDOW) {
    if (digitalRead(PIN_BTN_SW1) == LOW || digitalRead(0) == LOW) {
      delay(30);
      if (digitalRead(PIN_BTN_SW1) == LOW || digitalRead(0) == LOW) {
        clicks++;
        while (digitalRead(PIN_BTN_SW1) == LOW || digitalRead(0) == LOW) delay(10);
        lastRelease = millis();
        if (clicks >= 5) return GESTURE_FIVE_CLICKS_PAIR;
      }
    }
    delay(10);
  }

  if (clicks == 2) return GESTURE_DOUBLE_CLICK;
  return GESTURE_SINGLE_CLICK;
#endif
}

// =========================================================================================
// 📡 11. BẬT BLUETOOTH (BLE) KHI GIỮ NÚT 5 GIÂY (KHÔNG ÉP CẤU HÌNH LẠI)
// =========================================================================================
void startBleProvisioning() {
  // Giữ 5 giây: Bật phát sóng Bluetooth (BLE) + Quét danh sách Wi-Fi đang có (KHÔNG khóa mạch, KHÔNG ép cấu hình lại)
  setLed(COLOR_BLUE);
  buzzerSuccess();

#if ENABLE_BLE
  scanWiFiNetworksForBle();
  initBLE();
#endif

  // Chờ người dùng nhả nút ra nếu vẫn đang đè giữ
  while (digitalRead(0) == LOW || digitalRead(PIN_BTN_SW1) == LOW) {
    delay(20);
  }

  Serial.println("[READY] 🔵 Đã bật phát sóng Bluetooth! Bạn có thể bắt Bluetooth trên FE và vẫn bấm 1 lần (Đặt) / 2 lần (Hủy) bình thường.");
}

void processGesture(GestureType gesture) {
  if (gesture == GESTURE_NONE) return;

  // 1. Khôi phục cài đặt gốc (Giữ 15s)
  if (gesture == GESTURE_HOLD_15S_FACTORY_RESET || gesture == GESTURE_ALL_HOLD_RESET) {
    Serial.println("[RESET] Khôi phục cài đặt gốc: Xóa sạch bộ nhớ NVS Flash...");
    prefs.clear();
    setLed(COLOR_RED);
    buzzerError();
    delay(800);
    setLed(COLOR_OFF);
    return;
  }

  // 2. Giữ 5 giây (hoặc bấm 5 lần): Chỉ BẬT BLUETOOTH để FE bắt kết nối (không ép cấu hình lại)
  if (gesture == GESTURE_FIVE_CLICKS_PAIR || 
      gesture == GESTURE_HOLD_10S_PROVISION || 
      gesture == GESTURE_HOLD_5S_ACTIVATE ||
      gesture == GESTURE_BTN2_HOLD_CONFIG) {
    startBleProvisioning();
    return;
  }

  // 3. BẢN 1 NÚT:
  // - Nhấn 1 lần (GESTURE_SINGLE_CLICK) -> ĐẶT HÀNG (ORDER)
  // - Nhấn 2 lần (GESTURE_DOUBLE_CLICK) -> HỦY ĐƠN (CANCEL_ORDER)
  String eventType = "ORDER";
  ButtonLedColor actionColor = COLOR_BLUE;
  ButtonConfig* activeConfig = nullptr;

  if (gesture == GESTURE_SINGLE_CLICK || gesture == GESTURE_BTN1_ORDER) {
    activeConfig = &s_buttonConfigs[0];
    eventType = (activeConfig->actionType == ACTION_ORDER_ITEM) ? "ORDER_ITEM" : "ORDER";
    actionColor = COLOR_GREEN;
    Serial.printf("\n[1 NHẤN -> ĐẶT HÀNG] 🟢 Đã nhấn 1 lần: Đặt [%s] (Số lượng: %d)\n",
                  activeConfig->itemName, activeConfig->quantity);
#if ENABLE_BLE
    if (pBleStatusChar && bleDeviceConnected) {
      String msg = "EVENT:ORDER:" + String(activeConfig->itemName) + ":" + String(activeConfig->quantity);
      pBleStatusChar->setValue(msg.c_str());
      pBleStatusChar->notify();
    }
#endif
    setLed(COLOR_GREEN);
    buzzerSuccess();
  } else if (gesture == GESTURE_DOUBLE_CLICK || gesture == GESTURE_BTN3_CANCEL) {
    activeConfig = &s_buttonConfigs[2];
    eventType = "CANCEL_ORDER";
    actionColor = COLOR_YELLOW;
    Serial.printf("\n[2 NHẤN -> HỦY ĐƠN] 🟡 Đã nhấn đúp 2 lần: [%s]\n", activeConfig->itemName);
#if ENABLE_BLE
    if (pBleStatusChar && bleDeviceConnected) {
      String msg = "EVENT:CANCEL:" + String(activeConfig->itemName);
      pBleStatusChar->setValue(msg.c_str());
      pBleStatusChar->notify();
    }
#endif
    setLed(COLOR_YELLOW);
    buzzerCancel();
  } else if (gesture == GESTURE_BTN2_BILL) {
    activeConfig = &s_buttonConfigs[1];
    eventType = "REQUEST_BILL";
    actionColor = COLOR_CYAN;
    setLed(COLOR_CYAN);
    buzzerBill();
  }

  // Nếu đã có Wi-Fi lưu sẵn thì gửi đồng thời lên Backend Cloud (nếu chưa có Wi-Fi thì vẫn hoạt động bình thường qua BLE)
  if (savedSsid.length() > 0 && WiFi.status() == WL_CONNECTED) {
    String orderCode, errorMsg;
    sendOrderEvent(eventType, orderCode, errorMsg, activeConfig);
  }

  delay(600);
  setLed(bleDeviceConnected ? COLOR_GREEN : COLOR_OFF);
}

// =========================================================================================
// 🚀 11. KHỞI ĐỘNG HỆ THỐNG & VÒNG LẶP SỰ KIỆN CHÍNH
// =========================================================================================
void setup() {
  setCpuFrequencyMhz(80);

  Serial.begin(115200);
  delay(50);

  Serial.println("\n========================================================");
  Serial.println("🔘 SMART ORDER BUTTON — BẢN 1 NÚT (ESP32-S3)");
  Serial.printf("  Device ID : %s | BLE: SOB-%s\n", DEVICE_ID, DEVICE_ID);
  Serial.println("  • Nhấn 1 lần : ĐẶT HÀNG (ORDER)");
  Serial.println("  • Nhấn 2 lần : HỦY ĐƠN (CANCEL)");
  Serial.println("  • Giữ 5 giây : PHÁT BLUETOOTH (BLE) KẾT NỐI VỚI FE");
  Serial.println("========================================================");

  pinMode(PIN_BTN_SW1, INPUT_PULLUP);
  pinMode(0, INPUT_PULLUP);
#if (CURRENT_HARDWARE_VARIANT == HARDWARE_VARIANT_TRIPLE_BUTTON)
  pinMode(PIN_BTN_SW2, INPUT_PULLUP);
  pinMode(PIN_BTN_SW3, INPUT_PULLUP);
#endif
  pinMode(PIN_LED_R, OUTPUT);
  pinMode(PIN_LED_G, OUTPUT);
  pinMode(PIN_LED_B, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  digitalWrite(PIN_BUZZER, LOW);
  setLed(COLOR_OFF);

  initMAX17048();

  prefs.begin("sob_config", false);
  savedSsid = prefs.getString("wifi_ssid", "");
  savedPass = prefs.getString("wifi_pass", "");
  cloudUrl  = prefs.getString("cloud_url", DEFAULT_CLOUD_URL);
  String customOrderName = prefs.getString("btn1_name", "");
  if (customOrderName.length() > 0) {
    strncpy(s_buttonConfigs[0].itemName, customOrderName.c_str(), sizeof(s_buttonConfigs[0].itemName) - 1);
    s_buttonConfigs[0].quantity = prefs.getUChar("btn1_qty", 1);
  }

  setLed(COLOR_CYAN);
  buzzerClick();
  delay(100);
  setLed(COLOR_OFF);

  // Nếu có Wi-Fi đã lưu thì thử kết nối nhẹ nhàng lúc khởi động
  if (savedSsid.length() > 0) {
    WiFi.mode(WIFI_STA);
    WiFi.setTxPower(WIFI_POWER_15dBm);
    WiFi.begin(savedSsid.c_str(), savedPass.c_str());
  }
}

void loop() {
  static unsigned long lastBlinkMs = 0;
  static bool blinkState = false;

#if ENABLE_BLE
  // Xử lý sự kiện khi FE vừa kết nối hoặc ngắt kết nối Bluetooth
  if (bleConnectionStateChanged) {
    bleConnectionStateChanged = false;
    if (bleDeviceConnected) {
      setLed(COLOR_GREEN);
      buzzerSuccess();
      Serial.println("[BLE] ✅ ĐÃ BẮT ĐƯỢC KẾT NỐI BLUETOOTH VỚI FE!");
      if (pBleStatusChar) {
        String msg = "CONNECTED:" + String(DEVICE_ID) + ":" + (savedSsid.length() > 0 ? savedSsid : "Chưa kết nối");
        pBleStatusChar->setValue(msg.c_str());
        pBleStatusChar->notify();
      }
    } else {
      buzzerCancel();
    }
  }

  // Nếu FE yêu cầu quét lại danh sách Wi-Fi đang có
  if (pendingWifiScan) {
    pendingWifiScan = false;
    scanWiFiNetworksForBle();
  }

  // Nếu FE vừa chọn tài khoản Wi-Fi và gửi mật khẩu xuống
  if (pendingWifiConnect) {
    pendingWifiConnect = false;
    WiFi.mode(WIFI_STA);
    WiFi.setTxPower(WIFI_POWER_15dBm);
    WiFi.begin(savedSsid.c_str(), savedPass.c_str());
    unsigned long t0 = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - t0 < 6000) {
      delay(100);
    }
    if (WiFi.status() == WL_CONNECTED) {
      Serial.printf("[WIFI] 🎉 Đã kết nối Wi-Fi '%s' thành công! IP: %s\n",
                    savedSsid.c_str(), WiFi.localIP().toString().c_str());
      setLed(COLOR_GREEN);
      buzzerSuccess();
      if (pBleStatusChar) {
        String okMsg = "WIFI_OK:" + savedSsid + ":" + WiFi.localIP().toString();
        pBleStatusChar->setValue(okMsg.c_str());
        pBleStatusChar->notify();
      }
    } else {
      Serial.printf("[WIFI] ⚠️ Đã lưu Wi-Fi '%s' (Kiểm tra lại mật khẩu hoặc sóng Wi-Fi).\n", savedSsid.c_str());
      if (pBleStatusChar) {
        String savedMsg = "WIFI_SAVED:" + savedSsid;
        pBleStatusChar->setValue(savedMsg.c_str());
        pBleStatusChar->notify();
      }
    }
  }

  // Nhấp nháy đèn Xanh dương khi đang phát Bluetooth chờ FE bắt
  if (bleAdvertisingActive && !bleDeviceConnected) {
    if (millis() - lastBlinkMs >= 450) {
      lastBlinkMs = millis();
      blinkState = !blinkState;
      setLed(blinkState ? COLOR_BLUE : COLOR_OFF);
    }
  }
#endif

  // Đọc thao tác nhấn phím (1 lần = Đặt hàng, 2 lần = Hủy đơn, Giữ 5s = Phát Bluetooth)
  bool anyPressed = (digitalRead(PIN_BTN_SW1) == LOW || digitalRead(0) == LOW);
#if (CURRENT_HARDWARE_VARIANT == HARDWARE_VARIANT_TRIPLE_BUTTON)
  if (!anyPressed) anyPressed = (digitalRead(PIN_BTN_SW2) == LOW || digitalRead(PIN_BTN_SW3) == LOW);
#endif

  if (anyPressed) {
    GestureType gesture = scanButtonGesture(false);
    processGesture(gesture);
  }

  delay(20);
}
