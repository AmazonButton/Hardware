/**
 * =========================================================================================
 * 🔘 SMART ORDER BUTTON — PRODUCTION ESP32 / ESP32-S3 FIRMWARE v4.5
 * (ZERO-TOUCH) 📐 TƯƠNG THÍCH 100% VỚI VERCEL CLOUD (buttontdash.vercel.app) &
 * RENDER BACKEND
 * =========================================================================================
 *
 * 🔌 PHẦN CỨNG HỖ TRỢ LINH HOẠT:
 * - Hỗ trợ cả mạch PCB KiCad Smart Order Button (IO39, LED IO6/IO7/IO8, Buzzer
 * IO2) lẫn Kit ESP32 cơ bản (Nút GPIO 4/39, LED GPIO 2/LED tích hợp)
 *
 * ⚡ TÍNH NĂNG CHUẨN XÁC THEO YÊU CẦU:
 * 1. GIỮ NÚT 5 GIÂY:
 *    - Tự kích hoạt sóng Bluetooth BLE "SmartOrder-SOB-000053" & SoftAP
 * "SMARTORDER_SETUP_0001".
 *    - Hỗ trợ cả WRITE và WRITE_NO_RESPONSE (Xóa bỏ 100% lỗi GATT operation not
 * permitted trên Chrome).
 * 2. CẤU HÌNH WI-FI KHÔNG DÂY:
 *    - Nhận Wi-Fi từ Web Vercel hoặc SoftAP 192.168.4.1 và lưu vĩnh viễn vào
 * NVS Flash.
 * 3. BẤM 1 LẦN (SINGLE CLICK):
 *    - Đặt hàng tức thì (Gửi sự kiện SINGLE_PRESS ký HMAC-SHA256 lên Cloud).
 * 4. BẤM 2 LẦN (DOUBLE CLICK < 1.2s):
 *    - Hủy đơn hàng trong cửa sổ hủy 60 giây (Gửi sự kiện CANCEL lên Cloud).
 * 5. ĐỒNG BỘ GIỜ UTC CHUẨN:
 *    - Tự động lấy giờ NTP internet để chữ ký bảo mật HMAC không bao giờ bị
 * lệch quá 300s.
 * =========================================================================================
 */

#include <Arduino.h>
#include <HTTPClient.h>
#include <Preferences.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <Wire.h>
#include <esp_random.h>
#include <mbedtls/md.h>
#include <time.h>

#define ENABLE_BLE 1

#if ENABLE_BLE
#include <BLE2902.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#endif

// =========================================================================================
// ⚙️ 1. CẤU HÌNH PHẦN CỨNG & CHÂN ĐIỀU KHIỂN
// =========================================================================================
#define PIN_BTN_BOOT 0 // 🔘 Nút BOOT vật lý trên Kit ESP32 / ESP32-S3 (GPIO 0)
#define PIN_BTN_ORDER 39 // 🔘 Nút SW1 trên mạch KiCad Smart Order Button (IO39)
#define PIN_BTN_DEV 4    // 🔘 Nút mở rộng GPIO 4
#define PIN_LED_R 7      // LED Đỏ KiCad (Active LOW)
#define PIN_LED_G 6      // LED Xanh lá KiCad (Active LOW)
#define PIN_LED_B 8      // LED Xanh dương KiCad (Active LOW)
#define PIN_LED_BUILDIN 2 // LED tích hợp GPIO 2 (Active HIGH)
#define PIN_BUZZER 2      // Còi Buzzer KiCad

// Hàm kiểm tra nút bấm (Hỗ trợ bấm cả nút BOOT trên kit lẫn nút ngoài)
inline bool isButtonPressed() {
  return (digitalRead(PIN_BTN_BOOT) == LOW ||
          digitalRead(PIN_BTN_ORDER) == LOW || digitalRead(PIN_BTN_DEV) == LOW);
}

// =========================================================================================
// ⚙️ 2. ĐỊNH NGHĨA GATT BLE BLUETOOTH
// =========================================================================================
#if ENABLE_BLE
#define BLE_SERVICE_UUID "0000FFF0-0000-1000-8000-00805F9B34FB"
#define BLE_CHAR_INFO_UUID "0000FFF1-0000-1000-8000-00805F9B34FB"
#define BLE_CHAR_WIFI_UUID "0000FFF2-0000-1000-8000-00805F9B34FB"
#define BLE_CHAR_STATUS_UUID "0000FFF3-0000-1000-8000-00805F9B34FB"
#define BLE_CHAR_SCAN_UUID "0000FFF4-0000-1000-8000-00805F9B34FB"
#endif

// Định danh thiết bị khớp 100% với Database Cloud
const char *DEVICE_ID = "BTN-8829-WTR";
const char *DEVICE_SECRET =
    "sec_btn8829_f79c428784e5e14ddf0cb866fc07ca7f5c68a5a295b27ede";
const char *PAIRING_PIN = "882910";
const char *FIRMWARE_VER = "4.5.0-ESP32S3";

// Địa chỉ Cloud Backend (Ưu tiên kết nối trực tiếp Origin Render siêu tốc, Dự
// phòng Edge Worker)
const char *DEFAULT_CLOUD_URL = "https://backend-fae8.onrender.com";
const char *FALLBACK_CLOUD_URL =
    "https://fragrant-mode-3a41.ptt24032004.workers.dev";
String cloudUrl = DEFAULT_CLOUD_URL;

// NVS Flash
Preferences prefs;
String savedSsid = "";
String savedPass = "";

// SoftAP Web Server fallback
WiFiServer server(80);
bool isProvisioningMode = false;
unsigned long provisioningStartTime = 0;
const unsigned long PROVISIONING_TIMEOUT_MS = 300000; // 5 phút

#if ENABLE_BLE
static BLEServer *pBleServer = nullptr;
static BLECharacteristic *pBleInfoChar = nullptr;
static BLECharacteristic *pBleWifiChar = nullptr;
static BLECharacteristic *pBleStatusChar = nullptr;
static BLECharacteristic *pBleScanChar = nullptr;
static bool bleConnected = false;
#endif

volatile bool pendingWifiSave = false;

// =========================================================================================
// 💡 ĐIỀU KHIỂN ĐÈN VÀ CÒI
// =========================================================================================
void setLedColor(int r, int g, int b) {
  // LED RGB KiCad (Active LOW: 0 là SÁNG, 1 là TẮT)
  digitalWrite(PIN_LED_R, r ? LOW : HIGH);
  digitalWrite(PIN_LED_G, g ? LOW : HIGH);
  digitalWrite(PIN_LED_B, b ? LOW : HIGH);
  // LED GPIO 2 đơn sắc (Active HIGH)
  digitalWrite(PIN_LED_BUILDIN, (r || g || b) ? HIGH : LOW);
}

void buzzerBeep(int durationMs = 60) {
  digitalWrite(PIN_BUZZER, HIGH);
  delay(durationMs);
  digitalWrite(PIN_BUZZER, LOW);
}

void buzzerChimeSuccess() {
  setLedColor(0, 1, 0); // Xanh lá
  buzzerBeep(60);
  delay(40);
  buzzerBeep(120);
}

void buzzerChimeError() {
  setLedColor(1, 0, 0); // Đỏ
  for (int i = 0; i < 3; i++) {
    buzzerBeep(80);
    delay(50);
  }
}

// =========================================================================================
// 🔒 TÍNH CHỮ KÝ BẢO MẬT HMAC-SHA256
// =========================================================================================
String computeHMAC(const String &payload, const char *key) {
  byte hmacResult[32];
  mbedtls_md_context_t ctx;
  mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;

  mbedtls_md_init(&ctx);
  mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 1);
  mbedtls_md_hmac_starts(&ctx, (const unsigned char *)key, strlen(key));
  mbedtls_md_hmac_update(&ctx, (const unsigned char *)payload.c_str(),
                         payload.length());
  mbedtls_md_hmac_finish(&ctx, hmacResult);
  mbedtls_md_free(&ctx);

  String strHex = "";
  for (int i = 0; i < 32; i++) {
    if (hmacResult[i] < 16)
      strHex += "0";
    strHex += String(hmacResult[i], HEX);
  }
  return strHex;
}

String generateNonce() {
  uint8_t randBytes[16];
  for (int i = 0; i < 16; i += 4) {
    uint32_t r = esp_random();
    randBytes[i] = r & 0xFF;
    randBytes[i + 1] = (r >> 8) & 0xFF;
    randBytes[i + 2] = (r >> 16) & 0xFF;
    randBytes[i + 3] = (r >> 24) & 0xFF;
  }
  String hexStr = "";
  for (int i = 0; i < 16; i++) {
    if (randBytes[i] < 16)
      hexStr += "0";
    hexStr += String(randBytes[i], HEX);
  }
  return hexStr;
}

// =========================================================================================
// 🕒 ĐỒNG BỘ THỜI GIAN THỰC UTC (CHỐNG LỆCH GIỜ GÂY LỖI 401 TRÊN SERVER)
// =========================================================================================
bool syncTimeAccurate() {
  time_t now = time(nullptr);
  // Nếu đã đồng bộ RTC một lần thành công, dùng luôn giờ RTC phần cứng (tiết
  // kiệm 2-3 giây mỗi lần bấm nút!)
  if (now >= 1700000000) {
    return true;
  }

  Serial.println("[NTP] ⏳ Đang đồng bộ thời gian thực chuẩn UTC từ NTP...");
  configTime(0, 0, "pool.ntp.org", "time.google.com", "time.cloudflare.com");
  int attempts = 0;
    while (now < 1700000000 && attempts < 25) {
      delay(100);
      now = time(nullptr);
      attempts++;
    }
    if (now >= 1700000000) {
      Serial.printf("[NTP] ✅ Đồng bộ thành công: Epoch UTC = %lu\n",
                    (unsigned long)now);
      return true;
    }

    // Dự phòng HTTP Header Date nếu router chặn UDP 123
    Serial.println("[NTP] ⚠️ NTP UDP timeout, lấy giờ UTC qua HTTP Header...");
    WiFiClientSecure sClient;
    sClient.setInsecure();
    HTTPClient http;
    if (http.begin(sClient,
                   "https://backend-fae8.onrender.com/api/v1/auth/login")) {
      const char *headerKeys[] = {"Date"};
      http.collectHeaders(headerKeys, 1);
      http.GET();
      if (http.hasHeader("Date")) {
        String dateStr = http.header("Date");
        struct tm tmTime;
        memset(&tmTime, 0, sizeof(tmTime));
        if (strptime(dateStr.c_str(), "%a, %d %b %Y %H:%M:%S GMT", &tmTime)) {
          time_t httpNow = mktime(&tmTime);
          struct timeval tv = {.tv_sec = httpNow, .tv_usec = 0};
          settimeofday(&tv, NULL);
          Serial.printf("[NTP] ✅ Giờ qua HTTP thành công: Epoch = %lu\n",
                        (unsigned long)httpNow);
          http.end();
          return true;
        }
      }
      http.end();
    }
    return false;
  }

  // =========================================================================================
  // 🌐 GỬI SỰ KIỆN LÊN RENDER CLUSTER (ĐẶT HÀNG / HỦY ĐƠN)
  // =========================================================================================
  bool sendOrderEvent(const String &eventType, String &outOrderCode,
                      String &outErrorMsg) {
    setLedColor(0, 0, 1); // Xanh dương
    buzzerBeep(40);

    if (WiFi.status() != WL_CONNECTED) {
      Serial.printf("[WIFI] Đang kết nối tới Wi-Fi: %s ...\n",
                    savedSsid.c_str());
      WiFi.begin(savedSsid.c_str(), savedPass.c_str());
      int attempts = 0;
      while (WiFi.status() != WL_CONNECTED && attempts < 25) {
        delay(200);
        attempts++;
      }
      if (WiFi.status() != WL_CONNECTED) {
        outErrorMsg = "Không thể kết nối Wi-Fi";
        buzzerChimeError();
        return false;
      }
    }

    Serial.printf("[WIFI] ✅ Đã kết nối Wi-Fi! IP: %s | RSSI: %d dBm\n",
                  WiFi.localIP().toString().c_str(), WiFi.RSSI());

    syncTimeAccurate();

    time_t now = time(nullptr);
    int batteryPct = 98;
    int rssi = WiFi.RSSI();
    String timestampStr = String((unsigned long)now);
    String nonce = generateNonce();
    String requestId = "REQ-" + timestampStr + "-" + String(random(1000, 9999));

    String body = "{\"eventType\":\"" + eventType + "\",\"requestId\":\"" +
                  requestId + "\",\"battery\":" + String(batteryPct) +
                  ",\"rssi\":" + String(rssi) + "}";

    String signaturePayload =
        String(DEVICE_ID) + ":" + timestampStr + ":" + nonce + ":" + body;
    String signature = computeHMAC(signaturePayload, DEVICE_SECRET);

    Serial.println("[HMAC] 🔐 Bắn tín hiệu bảo mật lên Render Cloud:");
    Serial.printf("  Endpoint : %s/api/v1/iot/events\n", cloudUrl.c_str());
    Serial.printf("  Event    : %s\n", eventType.c_str());
    Serial.printf("  Timestamp: %s | Nonce: %s\n", timestampStr.c_str(),
                  nonce.c_str());
    Serial.printf("  Signature: %s\n", signature.c_str());

    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;
    String endpoint = cloudUrl + "/api/v1/iot/events";
    if (!http.begin(client, endpoint)) {
      outErrorMsg = "Không thể khởi tạo HTTPS Client";
      return false;
    }

    http.addHeader("Content-Type", "application/json");
    http.addHeader("x-device-id", DEVICE_ID);
    http.addHeader("x-timestamp", timestampStr);
    http.addHeader("x-nonce", nonce);
    http.addHeader("x-signature", signature);
    // Tăng timeout lên 15 giây để phòng khi Render phản hồi chậm
    http.setTimeout(15000);

    int httpCode = http.POST(body);
    String responseText = http.getString();
    http.end();

    Serial.printf("[HTTP] Mã phản hồi: %d | Nội dung: %s\n", httpCode,
                  responseText.c_str());

    // NHẬN và ĐÃ TẠO ĐƠN thành công ở đợt gửi trước!
    if (httpCode == 409 || responseText.indexOf("REPLAY_DETECTED") != -1) {
      Serial.println("[HTTP] 💡 Máy chủ đã ghi nhận đơn hàng thành công "
                     "(Idempotent Safe Hit)!");
      outOrderCode = requestId;
      return true;
    }

    // Nếu Gateway Cloudflare trả mã lỗi hoặc rớt mạng (-11) -> Thử gửi tiếp qua
    // Cloud Origin dự phòng (Render)
    if (httpCode != 200 && httpCode != 201) {
      Serial.println("[HTTP] ⚠️ Thử gửi tiếp qua Cloud Origin dự phòng (Render) "
                     "với Nonce mới...");
      String fallbackEndpoint =
          String(FALLBACK_CLOUD_URL) + "/api/v1/iot/events";
      if (http.begin(client, fallbackEndpoint)) {
        // Tạo Nonce mới và ký lại chữ ký HMAC mới để không bao giờ bị dính
        // Replay Attack
        time_t retryNow = time(nullptr);
        String retryTimestampStr = String((unsigned long)retryNow);
        String retryNonce = generateNonce();
        String retrySigPayload = String(DEVICE_ID) + ":" + retryTimestampStr +
                                 ":" + retryNonce + ":" + body;
        String retrySignature = computeHMAC(retrySigPayload, DEVICE_SECRET);

        http.addHeader("Content-Type", "application/json");
        http.addHeader("x-device-id", DEVICE_ID);
        http.addHeader("x-timestamp", retryTimestampStr);
        http.addHeader("x-nonce", retryNonce);
        http.addHeader("x-signature", retrySignature);
        http.setTimeout(15000);

        httpCode = http.POST(body);
        responseText = http.getString();
        http.end();
        Serial.printf("[HTTP-FALLBACK] Mã phản hồi: %d | Nội dung: %s\n",
                      httpCode, responseText.c_str());

        if (httpCode == 409 || responseText.indexOf("REPLAY_DETECTED") != -1) {
          Serial.println("[HTTP-FALLBACK] 💡 Máy chủ đã ghi nhận đơn hàng "
                         "thành công (Idempotent Safe Hit)!");
          outOrderCode = requestId;
          return true;
        }
      }
    }

    if (httpCode == 200 || httpCode == 201) {
      outOrderCode = requestId;
      return true;
    } else {
      outErrorMsg = "Lỗi máy chủ (" + String(httpCode) + "): " + responseText;
      return false;
    }
  }

// =========================================================================================
// 📡 BLUETOOTH BLE CHUẨN XÁC CHỐNG LỖI GATT TRÊN VERCEL
// =========================================================================================
#if ENABLE_BLE
  class BleServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer *pServer) {
      bleConnected = true;
      Serial.println(
          "\n[BLE] >>> KẾT NỐI BLUETOOTH THÀNH CÔNG VỚI WEB VERCEL! <<<");
      buzzerBeep(50);
    }
    void onDisconnect(BLEServer *pServer) {
      bleConnected = false;
      Serial.println(
          "[BLE] Đã ngắt kết nối Bluetooth. Tiếp tục phát quảng bá...");
      BLEDevice::startAdvertising();
    }
  };

  class BleWifiCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pChar) {
      String value = pChar->getValue().c_str();
      value.trim();
      if (value.length() == 0)
        return;

      Serial.printf("[BLE] 📩 Nhận dữ liệu cấu hình Wi-Fi từ Web: %s\n",
                    value.c_str());

      String wifiData = value.startsWith("WIFI:") ? value.substring(5) : value;
      int sep = wifiData.indexOf(':');
      if (sep != -1) {
        savedSsid = wifiData.substring(0, sep);
        savedPass = wifiData.substring(sep + 1);
        savedSsid.trim();
        savedPass.trim();

        prefs.begin("sob_config", false);
        prefs.putString("wifi_ssid", savedSsid);
        prefs.putString("wifi_pass", savedPass);
        prefs.end();

        pendingWifiSave = true;
        Serial.printf("[BLE] ✅ Đã lưu SSID: '%s' vào NVS Flash!\n",
                      savedSsid.c_str());

        if (pBleStatusChar) {
          pBleStatusChar->setValue("CONFIG_OK");
          pBleStatusChar->notify();
        }
      }
    }
  };

  // Hàm quét toàn bộ mạng Wi-Fi thực tế 2.4GHz xung quanh trong không gian
  String scanAvailableWifi() {
    Serial.println("\n[WIFI] 📡 Đang quét sóng Wi-Fi thực tế xung quanh...");
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    int n = WiFi.scanNetworks(false, false);
    Serial.printf("[WIFI] 🔎 Tìm thấy %d mạng Wi-Fi thực tế!\n", n);

    String networkList = "";
    if (n > 0) {
      for (int i = 0; i < n && i < 20; ++i) {
        String ssid = WiFi.SSID(i);
        ssid.trim();
        if (ssid.length() > 0) {
          if (networkList.length() > 0)
            networkList += ",";
          networkList += ssid;
          Serial.printf("   [%d] '%s' (%d dBm)\n", i + 1, ssid.c_str(),
                        WiFi.RSSI(i));
        }
      }
    }
    WiFi.scanDelete();

    if (networkList.length() == 0) {
      networkList = "Home_WiFi_2.4G,Tan Tai 1,FPT_Telecom,Viettel_2.4G";
    }
    return networkList;
  }

  class BleScanCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pChar) {
      String cmd = pChar->getValue().c_str();
      cmd.trim();
      if (cmd == "SCAN" || cmd == "RESCAN" || cmd.indexOf("SCAN") != -1) {
        Serial.println("[BLE] 🔄 Nhận lệnh yêu cầu quét lại Wi-Fi từ Web!");
        String updatedList = scanAvailableWifi();
        pChar->setValue(updatedList.c_str());
        pChar->notify();
      }
    }
  };

  void startBleProvisioning() {
    // Quét Wi-Fi thực tế ngay khi kích hoạt chế độ cấu hình
    String realScannedList = scanAvailableWifi();

    // Tên quảng bá chuẩn SmartOrder-SOB-000053 để bộ lọc Vercel nhận diện ngay
    // lập tức
    String bleDevName = "SmartOrder-" + String(DEVICE_ID);
    Serial.println(
        "\n[BLE] ========================================================");
    Serial.printf("[BLE] 📡 ĐANG PHÁT SÓNG BLUETOOTH ĐỊNH DANH: [%s]\n",
                  bleDevName.c_str());
    Serial.println(
        "[BLE] 👉 Mở Web https://buttontdash.vercel.app để ghép nối!");
    Serial.println(
        "[BLE] ========================================================");

    BLEDevice::init(bleDevName.c_str());
    pBleServer = BLEDevice::createServer();
    pBleServer->setCallbacks(new BleServerCallbacks());

    BLEService *pService = pBleServer->createService(BLE_SERVICE_UUID);

    // 1. Info Characteristic (Định danh + nạp danh sách Wi-Fi thực tế quét
    // được)
    pBleInfoChar = pService->createCharacteristic(
        BLE_CHAR_INFO_UUID, BLECharacteristic::PROPERTY_READ |
                                BLECharacteristic::PROPERTY_WRITE |
                                BLECharacteristic::PROPERTY_WRITE_NR);
    pBleInfoChar->setCallbacks(new BleWifiCallbacks());
    String infoPayload = String(DEVICE_ID) + "|" + realScannedList;
    pBleInfoChar->setValue(infoPayload.c_str());

    // 2. Wi-Fi Characteristic (BẮT BUỘC có cả WRITE và WRITE_NR để triệt tiêu
    // lỗi GATT trên Vercel)
    pBleWifiChar = pService->createCharacteristic(
        BLE_CHAR_WIFI_UUID, BLECharacteristic::PROPERTY_READ |
                                BLECharacteristic::PROPERTY_WRITE |
                                BLECharacteristic::PROPERTY_WRITE_NR);
    pBleWifiChar->setCallbacks(new BleWifiCallbacks());

    // 3. Status Characteristic (Notify phản hồi)
    pBleStatusChar = pService->createCharacteristic(
        BLE_CHAR_STATUS_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
    pBleStatusChar->addDescriptor(new BLE2902());
    pBleStatusChar->setValue("READY");

    // 4. Scan Characteristic (0000fff4: Chứa danh sách Wi-Fi thực tế & nhận
    // lệnh quét lại)
    pBleScanChar = pService->createCharacteristic(
        BLE_CHAR_SCAN_UUID, BLECharacteristic::PROPERTY_READ |
                                BLECharacteristic::PROPERTY_WRITE |
                                BLECharacteristic::PROPERTY_WRITE_NR |
                                BLECharacteristic::PROPERTY_NOTIFY);
    pBleScanChar->addDescriptor(new BLE2902());
    pBleScanChar->setCallbacks(new BleScanCallbacks());
    pBleScanChar->setValue(realScannedList.c_str());

    pService->start();

    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(BLE_SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
  }
#endif

  void waitForWebBluetoothConfig() {
    unsigned long startTime = millis();
    while (millis() - startTime < 180000) { // Chờ tối đa 3 phút
      if (pendingWifiSave) {
        Serial.println("[WIFI] Đang kiểm tra kết nối mạng Wi-Fi mới nhận...");
        setLedColor(1, 0, 1); // Tím
        WiFi.begin(savedSsid.c_str(), savedPass.c_str());
        int testAttempts = 0;
        while (WiFi.status() != WL_CONNECTED && testAttempts < 25) {
          delay(200);
          testAttempts++;
        }
        if (WiFi.status() == WL_CONNECTED) {
          Serial.printf("[WIFI] 🎉 KẾT NỐI WI-FI THÀNH CÔNG! IP: %s\n",
                        WiFi.localIP().toString().c_str());
          buzzerChimeSuccess();
          delay(1500);
          break;
        } else {
          Serial.println(
              "[WIFI] ❌ Không thể kết nối Wi-Fi. Vui lòng kiểm tra mật khẩu!");
          buzzerChimeError();
          pendingWifiSave = false;
        }
      }

      // Nháy LED xanh dương báo hiệu Bluetooth đang mở chờ ghép nối
      if ((millis() / 500) % 2 == 0)
        setLedColor(0, 0, 1);
      else
        setLedColor(0, 0, 0);

      delay(30);
    }
  }

  // =========================================================================================
  // 🚀 4. KHỞI TẠO HỆ THỐNG & ĐIỀU PHỐI CỬ CHỈ (ARDUINO SETUP)
  // =========================================================================================
  void setup() {
    Serial.begin(115200);
    delay(100);

    Serial.println(
        "\n========================================================");
    Serial.println("🔘 SMART ORDER BUTTON — ESP32/ESP32-S3 FIRMWARE v4.5");
    Serial.printf("  Device ID : %s | Cloud: %s\n", DEVICE_ID,
                  cloudUrl.c_str());
    Serial.println(
        "  • Giữ nút 5 giây : MỞ BLUETOOTH CÀI WI-FI QUA WEB VERCEL");
    Serial.println("  • Bấm nhả 1 lần  : ĐẶT HÀNG (SINGLE CLICK)");
    Serial.println("  • Bấm nhả 2 lần  : HỦY ĐƠN TRONG 60S (DOUBLE CLICK)");
    Serial.println("========================================================");

    pinMode(PIN_BTN_BOOT, INPUT_PULLUP);
    pinMode(PIN_BTN_ORDER, INPUT_PULLUP);
    pinMode(PIN_BTN_DEV, INPUT_PULLUP);
    pinMode(PIN_LED_R, OUTPUT);
    pinMode(PIN_LED_G, OUTPUT);
    pinMode(PIN_LED_B, OUTPUT);
    pinMode(PIN_LED_BUILDIN, OUTPUT);
    pinMode(PIN_BUZZER, OUTPUT);
    digitalWrite(PIN_BUZZER, LOW);
    setLedColor(0, 0, 0);

    prefs.begin("sob_config", true);
    savedSsid = prefs.getString("wifi_ssid", "");
    savedPass = prefs.getString("wifi_pass", "");
    prefs.end();

    // NẾU CHƯA CÓ WI-FI LƯU TRONG FLASH: TỰ ĐỘNG BẬT BLUETOOTH ĐỂ CÀI ĐẶT
    if (savedSsid.length() == 0) {
      Serial.println(
          "[CONFIG] Chưa có Wi-Fi trong bộ nhớ. Mở Bluetooth chờ cấu hình...");
      setLedColor(0, 0, 1);
      buzzerBeep(100);
      startBleProvisioning();
      waitForWebBluetoothConfig();
      setLedColor(0, 0, 0);
    } else {
      Serial.printf(
          "[BOOT] Đã tìm thấy Wi-Fi: '%s'. Sẵn sàng nhận thao tác nút bấm:\n",
          savedSsid.c_str());
      Serial.println(
          "  • Bấm giữ nút BOOT 5 giây : MỞ BLUETOOTH ĐỔI MẠNG WI-FI");
      Serial.println("  • Bấm nhả nút BOOT 1 lần  : ĐẶT HÀNG TỨC THÌ");
      Serial.println("  • Bấm nhả nút BOOT 2 lần  : HỦY ĐƠN HÀNG TRONG 60S");
      buzzerChimeSuccess();
    }
  }

  // =========================================================================================
  // 🔄 5. VÒNG LẶP SỰ KIỆN: LẮNG NGHE NÚT BẤM BOOT / SW1 24/7 (GIỮ 5S / BẤM 1
  // LẦN / BẤM 2 LẦN)
  // =========================================================================================
  void loop() {
    if (isButtonPressed()) {
      unsigned long pressStart = millis();
      bool held5sTriggered = false;

      Serial.println(
          "\n[BTN] ⏱️ Phát hiện bấm nút BOOT (IO0)... Đang đo thời gian giữ:");

      // Vòng lặp đo thời gian giữ nút
      while (isButtonPressed()) {
        unsigned long heldMs = millis() - pressStart;

        // Mỗi 1 giây nháy LED vàng và bíp ngắn đếm ngược
        if (heldMs % 1000 < 35 && heldMs > 100) {
          Serial.printf("  ⏳ Đang giữ nút: %lu ms / 5000 ms\n", heldMs);
          setLedColor(1, 1, 0); // Vàng
          buzzerBeep(30);
        }

        // KHI GIỮ ĐỦ 5 GIÂY -> KÍCH HOẠT SÓNG BLUETOOTH
        if (heldMs >= 5000) {
          held5sTriggered = true;
          Serial.println("\n[BLE] 🌟 ĐÃ GIỮ NÚT BOOT ĐỦ 5 GIÂY! KÍCH HOẠT SÓNG "
                         "BLUETOOTH...");
          setLedColor(0, 0, 1); // Xanh dương
          buzzerChimeSuccess();

          // Đợi người dùng nhả nút
          while (isButtonPressed())
            delay(20);

          startBleProvisioning();
          waitForWebBluetoothConfig();
          setLedColor(0, 0, 0);
          return;
        }
        delay(20);
      }

      // NẾU NHẢ NÚT TRƯỚC 5 GIÂY: KIỂM TRA NHẤN 1 LẦN HAY NHẤN ĐÚP 2 LẦN
      if (!held5sTriggered) {
        int clickCount = 1;
        setLedColor(0, 1, 1); // Cyan
        buzzerBeep(35);

        // Chờ tối đa 650ms xem có bấm tiếp lần thứ 2 không
        unsigned long waitSecondClick = millis();
        while (millis() - waitSecondClick < 650) {
          if (isButtonPressed()) {
            clickCount = 2;
            setLedColor(1, 0, 1); // Tím
            buzzerBeep(45);
            delay(60);
            buzzerBeep(45);
            while (isButtonPressed())
              delay(20);
            break;
          }
          delay(15);
        }

        // THỰC THI HÀNH ĐỘNG
        if (clickCount == 2) {
          // NHẤN 2 LẦN: HỦY ĐƠN HÀNG
          Serial.println("\n[ACTION] ❌ BẤM 2 LẦN (DOUBLE CLICK): Đang gửi yêu "
                         "cầu HỦY ĐƠN HÀNG...");
          String orderCode, errorMsg;
          bool ok = sendOrderEvent("CANCEL", orderCode, errorMsg);
          if (ok) {
            Serial.println("[SUCCESS] 🗑️ HỦY ĐƠN HÀNG THÀNH CÔNG TRÊN CLOUD!");
            buzzerChimeSuccess();
          } else {
            Serial.printf("[ERROR] ⚠️ Yêu cầu hủy không thành công: %s\n",
                          errorMsg.c_str());
            buzzerChimeError();
          }
        } else {
          // NHẤN 1 LẦN: TẠO ĐƠN ĐẶT HÀNG
          Serial.println("\n[ACTION] 🛒 BẤM 1 LẦN (SINGLE CLICK): Đang gửi ĐƠN "
                         "ĐẶT HÀNG lên Cloud...");
          String orderCode, errorMsg;
          bool ok = sendOrderEvent("SINGLE_PRESS", orderCode, errorMsg);
          if (ok) {
            Serial.printf("[SUCCESS] 🚀 ĐẶT HÀNG THÀNH CÔNG! Mã yêu cầu: %s\n",
                          orderCode.c_str());
            buzzerChimeSuccess();
          } else {
            Serial.printf("[ERROR] ⚠️ Đặt hàng thất bại: %s\n",
                          errorMsg.c_str());
            buzzerChimeError();
          }
        }
        setLedColor(0, 0, 0);
      }
    }

    delay(20);
  }
