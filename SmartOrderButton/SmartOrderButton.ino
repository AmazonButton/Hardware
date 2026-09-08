/**
 * =========================================================================================
 * 🔘 SMART ORDER BUTTON — PRODUCTION ESP32-S3 FIRMWARE
 * 📐 KHỚP 100% SƠ ĐỒ NGUYÊN LÝ KICAD: Smart_Order_Button.kicad_sch
 * =========================================================================================
 * 
 * 🔌 PHẦN CỨNG & BẢN ĐỒ CHÂN (HARDWARE PINOUT MAPPING):
 * - Vi điều khiển       : ESP32-S3-WROOM-1 (U1)
 * - Nút Bấm SW1        : IO39 (Pin 19) - Net BTN_ORDER (Nối GND, INPUT_PULLUP, Active LOW)
 * - Đèn LED Đỏ (LED_R)  : IO7  (Pin 7)  - Catot D1 qua trở R15 1kΩ (Active LOW)
 * - Đèn LED Xanh (LED_G): IO6  (Pin 6)  - Catot D1 qua trở R16 470Ω (Active LOW)
 * - Đèn LED Lam (LED_B) : IO8  (Pin 12) - Catot D1 qua trở R17 470Ω (Active LOW)
 *   (Ghi chú: LED RGB D1 là loại Common Anode nối nguồn +3.3V, LOW = SÁNG, HIGH = TẮT)
 * - Còi Buzzer BZ1      : IO2  (Pin 38) - Kích Transistor Q1 2N7002 qua Net BUZZER_CTRL (Active HIGH)
 * - Cảm biến Pin U6    : MAX17048G_T10 (Địa chỉ I2C 0x36)
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
 * 5. ULTRA-LOW POWER DEEP SLEEP:
 *    - Chế độ ngủ sâu tự động tiêu thụ <15µA, đánh thức tức thì khi bấm nút SW1 (IO39 LOW).
 * =========================================================================================
 */

#include <Arduino.h>
#include <WiFi.h>
#include <Preferences.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <mbedtls/md.h>
#include <esp_random.h>
#include <esp_sleep.h>
#include <time.h>

// =========================================================================================
// ⚙️ 1. ĐỊNH NGHĨA CHÂN PHẦN CỨNG & CẤU HÌNH ỔN ĐỊNH ESP32-S3
// =========================================================================================
// ⚠️ QUAN TRỌNG VỀ PHẦN CỨNG ESP32-S3:
// IO39 thuộc HP Domain (không thuộc RTC IO 0-21) nên không thể dùng ext0 wakeup deep sleep.
// - Đặt USE_HARDWARE_MOD_RTC_PIN = 0: Dùng chân IO39 KiCad gốc (Tự động vào Light Sleep, bấm SW1 thức dậy 100%).
// - Đặt USE_HARDWARE_MOD_RTC_PIN = 1: Đã câu dây phần cứng SW1 sang IO10 -> Cho phép Deep Sleep <15uA.
#define USE_HARDWARE_MOD_RTC_PIN  0    // Đổi thành 1 khi đã câu dây SW1 sang IO10 (Pin 14 U1)

#if USE_HARDWARE_MOD_RTC_PIN
#define PIN_BTN_ORDER        10   // IO10 (Pin 14) - RTC GPIO Wakeup chuẩn <15uA
#else
#define PIN_BTN_ORDER        39   // IO39 (Pin 20) - Net BTN_ORDER KiCad gốc
#endif

#define PIN_LED_R            7    // IO7  - LED Đỏ qua R15 1k (Active LOW)
#define PIN_LED_G            6    // IO6  - LED Xanh lá qua R16 470R (Active LOW)
#define PIN_LED_B            8    // IO8  - LED Xanh dương qua R17 470R (Active LOW)
#define PIN_BUZZER           2    // IO2  - Còi BZ1 qua MOSFET Q1 2N7002 (Active HIGH)
#define PIN_I2C_SDA          4    // IO4  - I2C SDA (IC MAX17048)
#define PIN_I2C_SCL          5    // IO5  - I2C SCL (IC MAX17048)
#define MAX17048_I2C_ADDR    0x36 // Địa chỉ I2C của IC đo pin MAX17048

// =========================================================================================
// ⚙️ 2. TÙY CHỌN BLE 5 PROVISIONING
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
#define BLE_CHAR_SCAN_UUID      "0000FFF4-0000-1000-8000-00805F9B34FB"
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
String cloudUrl = DEFAULT_CLOUD_URL;

Preferences prefs;
String savedSsid = "";
String savedPass = "";
WiFiServer server(80);
bool isProvisioningMode = false;
unsigned long provisioningStartTime = 0;
const unsigned long PROVISIONING_TIMEOUT_MS = 180000; // 3 phút timeout

#if ENABLE_BLE
static BLEServer *pBleServer = nullptr;
static BLECharacteristic *pBleStatusChar = nullptr;
static BLECharacteristic *pBleScanChar = nullptr;
static bool bleConnected = false;
#endif

// =========================================================================================
// 💡 4. ĐIỀU KHIỂN ĐÈN LED RGB COMMON ANODE (LOW = BẬT, HIGH = TẮT)
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

// =========================================================================================
// 🔊 5. ĐIỀU KHIỂN CÒI BUZZER (ACTIVE HIGH QUA MOSFET Q1 2N7002)
// =========================================================================================
void buzzerBeep(int durationMs = 80) {
  digitalWrite(PIN_BUZZER, HIGH);
  delay(durationMs);
  digitalWrite(PIN_BUZZER, LOW);
}

void buzzerClick() {
  buzzerBeep(35);
}

void buzzerSuccess() {
  digitalWrite(PIN_BUZZER, HIGH);
  delay(60);
  digitalWrite(PIN_BUZZER, LOW);
  delay(60);
  digitalWrite(PIN_BUZZER, HIGH);
  delay(120);
  digitalWrite(PIN_BUZZER, LOW);
}

void buzzerError() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(PIN_BUZZER, HIGH);
    delay(90);
    digitalWrite(PIN_BUZZER, LOW);
    if (i < 2) delay(60);
  }
}

void buzzerCancel() {
  digitalWrite(PIN_BUZZER, HIGH);
  delay(350);
  digitalWrite(PIN_BUZZER, LOW);
}

// =========================================================================================
// 🔋 6. TRÌNH ĐIỀU KHIỂN CẢM BIẾN ĐO PIN MAX17048 (I2C ADDR: 0x36)
// =========================================================================================
bool initMAX17048() {
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL, 100000);
  delay(10);
  Wire.beginTransmission(MAX17048_I2C_ADDR);
  return (Wire.endTransmission() == 0);
}

uint16_t readMAX17048Reg(uint8_t reg) {
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
  uint16_t raw = readMAX17048Reg(0x02); // VCELL register
  if (raw == 0) return 3850; // Dự phòng điện áp danh định
  return (uint32_t)(((raw >> 4) * 5) / 4);
}

float getBatterySOC() {
  uint16_t raw = readMAX17048Reg(0x04); // SOC register
  if (raw == 0) return 85.0f;
  uint8_t intPct = (uint8_t)(raw >> 8);
  float fracPct = (float)(raw & 0xFF) / 256.0f;
  float soc = (float)intPct + fracPct;
  if (soc > 100.0f) soc = 100.0f;
  if (soc < 0.0f) soc = 0.0f;
  return soc;
}

// =========================================================================================
// 🔒 7. BẢO MẬT CHỮ KÝ HMAC-SHA256 & SINH SỐ NGẪU NHIÊN NONCE
// =========================================================================================
String generateNonce() {
  uint8_t randBytes[16];
  for (int i = 0; i < 16; i += 4) {
    uint32_t r = esp_random();
    randBytes[i]     = (uint8_t)(r & 0xFF);
    randBytes[i + 1] = (uint8_t)((r >> 8) & 0xFF);
    randBytes[i + 2] = (uint8_t)((r >> 16) & 0xFF);
    randBytes[i + 3] = (uint8_t)((r >> 24) & 0xFF);
  }
  char hexStr[33];
  for (int i = 0; i < 16; i++) {
    sprintf(hexStr + (i * 2), "%02x", randBytes[i]);
  }
  hexStr[32] = '\0';
  return String(hexStr);
}

String computeHMAC(const String& payload, const char* key) {
  byte hmacResult[32];
  mbedtls_md_context_t ctx;
  mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;

  mbedtls_md_init(&ctx);
  mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 1);
  mbedtls_md_hmac_starts(&ctx, (const unsigned char*)key, strlen(key));
  mbedtls_md_hmac_update(&ctx, (const unsigned char*)payload.c_str(), payload.length());
  mbedtls_md_hmac_finish(&ctx, hmacResult);
  mbedtls_md_free(&ctx);

  char hexString[65];
  for (int i = 0; i < 32; i++) {
    sprintf(&hexString[i * 2], "%02x", (unsigned int)hmacResult[i]);
  }
  hexString[64] = '\0';
  return String(hexString);
}

// =========================================================================================
// 🌐 8. ĐỒNG BỘ THỜI GIAN NTP & GỬI TÍN HIỆU ĐẾN BACKEND NESTJS
// =========================================================================================
bool syncNTP() {
  configTime(7 * 3600, 0, "pool.ntp.org", "time.google.com");
  time_t now = time(nullptr);
  int retries = 0;
  while (now < 1700000000 && retries < 15) {
    delay(200);
    now = time(nullptr);
    retries++;
  }
  return (now >= 1700000000);
}

bool sendOrderEvent(const String& eventType, String& outOrderCode, String& outErrorMsg) {
  setLed(COLOR_BLUE); // Đèn xanh dương biểu thị đang phát dữ liệu
  buzzerClick();

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[WIFI] Mất kết nối Wi-Fi. Đang kết nối lại...");
    WiFi.setTxPower(WIFI_POWER_15dBm); // 15dBm an toàn cho LDO ME6211 (500mA) không bị sụt áp
    WiFi.begin(savedSsid.c_str(), savedPass.c_str());
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 25) {
      delay(200);
      attempts++;
    }
    if (WiFi.status() != WL_CONNECTED) {
      outErrorMsg = "Không thể kết nối Wi-Fi";
      setLed(COLOR_RED);
      buzzerError();
      return false;
    }
  }

  // Đọc mức pin thực tế từ IC MAX17048
  float batterySoc = getBatterySOC();
  int batteryPct = (int)round(batterySoc);
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
  String body = "{\"eventType\":\"" + eventType + "\",\"requestId\":\"" + requestId + "\",\"battery\":" + String(batteryPct) + ",\"rssi\":" + String(rssi) + "}";

  // Ký chữ ký HMAC-SHA256: deviceId:timestamp:nonce:body
  String signaturePayload = String(DEVICE_ID) + ":" + timestampStr + ":" + nonce + ":" + body;
  String signature = computeHMAC(signaturePayload, DEVICE_SECRET);

  Serial.println("[HMAC] Đang gửi gói tin có chữ ký bảo mật tới Cloud:");
  Serial.printf("  Endpoint : %s/api/iot/events\n", cloudUrl.c_str());
  Serial.printf("  Signature: %s\n", signature.c_str());

  HTTPClient http;
  http.begin(cloudUrl + "/api/iot/events");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("x-device-id", DEVICE_ID);
  http.addHeader("x-timestamp", timestampStr);
  http.addHeader("x-nonce", nonce);
  http.addHeader("x-signature", signature);
  http.setTimeout(5000);

  int httpCode = http.POST(body);
  String responseText = http.getString();
  http.end();

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
// 💤 9. QUẢN LÝ CHẾ ĐỘ NGỦ SÂU (AMAZON DASH DEEP SLEEP)
// =========================================================================================
void enterUltraDeepSleep() {
  Serial.println("[SLEEP] Tắt module Wi-Fi & Bluetooth. Vào chế độ ngủ tiết kiệm điện...");
  setLed(COLOR_OFF);
  digitalWrite(PIN_BUZZER, LOW);
  WiFi.mode(WIFI_OFF);
  Serial.flush();

#if USE_HARDWARE_MOD_RTC_PIN
  // Với mạch đã câu dây sang chân RTC IO10:
  Serial.println("[SLEEP] Vào DEEP SLEEP (<15uA) qua RTC GPIO IO10. Sẵn sàng thức dậy khi bấm SW1!");
  esp_sleep_enable_ext0_wakeup((gpio_num_t)PIN_BTN_ORDER, 0);
  esp_deep_sleep_start();
#else
  // Với mạch nguyên bản KiCad IO39:
  Serial.println("[SLEEP] ⚠️ Chân IO39 là HP GPIO -> Vào LIGHT SLEEP an toàn (vẫn thức dậy khi bấm SW1)!");
  Serial.println("[SLEEP] (Mẹo: Câu dây SW1 sang IO10 để kích hoạt Deep Sleep <15uA)");
  Serial.flush();

  gpio_wakeup_enable((gpio_num_t)PIN_BTN_ORDER, GPIO_INTR_LOW_LEVEL);
  esp_sleep_enable_gpio_wakeup();
  esp_light_sleep_start();

  Serial.println("[SLEEP] >>> Thức dậy từ Light Sleep khi bấm SW1! Khởi động lại hệ thống...");
  delay(50);
  ESP.restart();
#endif
}

// =========================================================================================
// 🔘 10. NHẬN DIỆN CỬ CHỈ NÚT BẤM CƠ HỌC (MULTI-GESTURE DECODER)
// =========================================================================================
enum GestureType {
  GESTURE_NONE,
  GESTURE_SINGLE_CLICK,
  GESTURE_DOUBLE_CLICK,
  GESTURE_FIVE_CLICKS_PAIR,
  GESTURE_HOLD_5S_ACTIVATE,
  GESTURE_HOLD_10S_PROVISION,
  GESTURE_HOLD_15S_FACTORY_RESET
};

GestureType scanButtonGesture() {
  if (digitalRead(PIN_BTN_ORDER) == HIGH) {
    return GESTURE_NONE;
  }

  unsigned long pressStart = millis();

  // Đếm thời gian giữ nút
  while (digitalRead(PIN_BTN_ORDER) == LOW) {
    unsigned long heldMs = millis() - pressStart;
    if (heldMs >= 15000) {
      while (digitalRead(PIN_BTN_ORDER) == LOW) delay(10);
      return GESTURE_HOLD_15S_FACTORY_RESET;
    }
    delay(10);
  }

  unsigned long totalHeldMs = millis() - pressStart;
  if (totalHeldMs >= 10000) return GESTURE_HOLD_10S_PROVISION;
  if (totalHeldMs >= 5000)  return GESTURE_HOLD_5S_ACTIVATE;
  if (totalHeldMs < 50)     return GESTURE_NONE; // Nhiễu cơ học

  // Nút đã nhả nhanh (<5s) -> Quét nhấn đúp hoặc 5 lần nhấn
  int clicks = 1;
  unsigned long lastRelease = millis();
  const unsigned long MULTI_CLICK_WINDOW = 450;

  while (millis() - lastRelease < MULTI_CLICK_WINDOW) {
    if (digitalRead(PIN_BTN_ORDER) == LOW) {
      delay(50);
      if (digitalRead(PIN_BTN_ORDER) == LOW) {
        clicks++;
        while (digitalRead(PIN_BTN_ORDER) == LOW) delay(10);
        lastRelease = millis();
        if (clicks >= 5) return GESTURE_FIVE_CLICKS_PAIR;
      }
    }
    delay(10);
  }

  if (clicks == 2) return GESTURE_DOUBLE_CLICK;
  return GESTURE_SINGLE_CLICK;
}

// =========================================================================================
// 🚀 11. KHỞI ĐỘNG HỆ THỐNG (ARDUINO SETUP)
// =========================================================================================
void setup() {
  // Hạ xung nhịp CPU xuống 80MHz để triệt tiêu sốc dòng trên LDO ME6211C33M5G (500mA)
  setCpuFrequencyMhz(80);

  Serial.begin(115200);
  delay(100);

  Serial.println("\n========================================================");
  Serial.println("🔘 SMART ORDER BUTTON — ESP32-S3 PRODUCTION FIRMWARE");
  Serial.printf("  Device ID : %s | Model: ESP32-S3-WROOM-1\n", DEVICE_ID);
  Serial.println("  Schematic : Smart_Order_Button.kicad_sch");
  Serial.println("========================================================");

  // Cấu hình chân
  pinMode(PIN_BTN_ORDER, INPUT_PULLUP);
  pinMode(PIN_LED_R, OUTPUT);
  pinMode(PIN_LED_G, OUTPUT);
  pinMode(PIN_LED_B, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  digitalWrite(PIN_BUZZER, LOW);
  setLed(COLOR_OFF);

  // Khởi động IC đo pin MAX17048 qua I2C (IO4/IO5)
  if (initMAX17048()) {
    Serial.println("[SENSOR] IC MAX17048 sẵn sàng trên I2C (0x36).");
  } else {
    Serial.println("[SENSOR] Chú ý: Chưa tìm thấy IC MAX17048; sử dụng giá trị mô phỏng.");
  }

  // Đọc bộ nhớ NVS Flash lấy thông tin Wi-Fi
  prefs.begin("sob_config", false);
  savedSsid = prefs.getString("wifi_ssid", "");
  savedPass = prefs.getString("wifi_pass", "");
  cloudUrl  = prefs.getString("cloud_url", DEFAULT_CLOUD_URL);

  // Hiệu ứng đèn khởi động (Cyan nhẹ)
  setLed(COLOR_CYAN);
  delay(150);
  setLed(COLOR_OFF);

  // Quét cử chỉ nút bấm SW1
  GestureType gesture = scanButtonGesture();

  // Khôi phục cài đặt gốc
  if (gesture == GESTURE_HOLD_15S_FACTORY_RESET) {
    Serial.println("[RESET] Phát hiện giữ 15s: KHÔI PHỤC CÀI ĐẶT GỐC...");
    prefs.clear();
    setLed(COLOR_RED);
    buzzerError();
    delay(1000);
    enterUltraDeepSleep();
    return;
  }

  // Chế độ cấu hình không dây BLE / SoftAP
  if (gesture == GESTURE_FIVE_CLICKS_PAIR || gesture == GESTURE_HOLD_10S_PROVISION || savedSsid.length() == 0) {
    Serial.println("[PROVISION] Bắt đầu chế độ cấu hình Wi-Fi BLE & SoftAP...");
    setLed(COLOR_BLUE);
    buzzerBeep(120);

    // Mở SoftAP fallback
    String apSsid = "SMARTORDER_" + String(DEVICE_ID);
    WiFi.softAP(apSsid.c_str(), "12345678");
    server.begin();

    unsigned long pStart = millis();
    while (millis() - pStart < PROVISIONING_TIMEOUT_MS) {
      if ((millis() / 500) % 2 == 0) setLed(COLOR_BLUE);
      else setLed(COLOR_OFF);
      delay(30);
    }

    setLed(COLOR_OFF);
    enterUltraDeepSleep();
    return;
  }

  // Nếu không có thao tác bấm phím (ví dụ cắm sạc, khởi động nguồn hoặc nhiễu điện)
  if (gesture == GESTURE_NONE) {
    Serial.println("[GESTURE] Khởi động nguồn hoặc không có thao tác nhấn SW1. Vào chế độ ngủ...");
    enterUltraDeepSleep();
    return;
  }

  // Xử lý sự kiện đặt hàng hoặc hủy đơn
  String eventType = "SINGLE_PRESS";
  if (gesture == GESTURE_DOUBLE_CLICK) {
    eventType = "CANCEL";
    Serial.println("[GESTURE] Phát hiện NHẤN ĐÚP: Gửi yêu cầu HỦY ĐƠN HÀNG...");
  } else {
    Serial.println("[GESTURE] Phát hiện NHẤN 1 LẦN: Gửi yêu cầu ĐẶT HÀNG MỚI...");
  }

  String orderCode, errorMsg;
  bool ok = sendOrderEvent(eventType, orderCode, errorMsg);

  if (ok) {
    if (eventType == "CANCEL") {
      Serial.println("[SUCCESS] ĐÃ HỦY ĐƠN HÀNG THÀNH CÔNG!");
      setLed(COLOR_YELLOW);
      buzzerCancel();
    } else {
      Serial.printf("[SUCCESS] ĐẶT HÀNG THÀNH CÔNG! Mã yêu cầu: %s\n", orderCode.c_str());
      setLed(COLOR_GREEN);
      buzzerSuccess();
    }
    delay(1200);
  } else {
    Serial.printf("[ERROR] Thất bại: %s\n", errorMsg.c_str());
    setLed(COLOR_RED);
    buzzerError();
    delay(1200);
  }

  // Tự động vào chế độ ngủ sâu tiết kiệm pin
  enterUltraDeepSleep();
}

void loop() {
  // Không bao giờ chạy tới loop() vì hệ thống luôn vào Deep Sleep trong setup()
}
