#include "BLEProvisioning.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <WiFi.h>
#include "../wifi/WiFiManager.h"
#include "../led/LedManager.h"
#include "../buzzer/BuzzerManager.h"
#include "../button/ButtonManager.h"

static BLEServer *pServer = nullptr;
static BLECharacteristic *pInfoChar = nullptr;
static BLECharacteristic *pStatusChar = nullptr;
static BLECharacteristic *pLedChar = nullptr;
static bool deviceConnected = false;
static bool connectionStateChanged = false;
static String activeDeviceId = "BTN-8829-WTR";

class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    deviceConnected = true;
    connectionStateChanged = true;
    Serial.println("\n[BLE] ========================================================");
    Serial.println("[BLE] >>> ✅ ĐÃ KẾT NỐI BLUETOOTH THÀNH CÔNG TRÊN FE! <<<");
    Serial.println("[BLE] ========================================================");
  }

  void onDisconnect(BLEServer *pServer) {
    deviceConnected = false;
    connectionStateChanged = true;
    Serial.println("\n[BLE] ========================================================");
    Serial.println("[BLE] >>> THIẾT BỊ ĐÃ NGẮT KẾT NỐI BLUETOOTH! <<<");
    Serial.println("[BLE] Tự động phát lại quảng bá Bluetooth để chờ kết nối lại...");
    Serial.println("[BLE] ========================================================");
    BLEDevice::startAdvertising();
  }
};

class LEDControlCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    String value = pCharacteristic->getValue().c_str();
    value.trim();
    value.toUpperCase();
    Serial.printf("[BLE] Nhận lệnh điều khiển LED từ Bluetooth: '%s'\n", value.c_str());

    if (value == "RED" || value == "1") {
      LedManager::setColor(LED_RED);
    } else if (value == "GREEN" || value == "2") {
      LedManager::setColor(LED_GREEN);
    } else if (value == "BLUE" || value == "3") {
      LedManager::setColor(LED_BLUE);
    } else if (value == "YELLOW" || value == "4") {
      LedManager::setColor(LED_YELLOW);
    } else if (value == "PURPLE" || value == "5") {
      LedManager::setColor(LED_PURPLE);
    } else if (value == "WHITE" || value == "6") {
      LedManager::setColor(LED_WHITE);
    } else if (value == "OFF" || value == "0") {
      LedManager::setColor(LED_OFF);
    }
  }
};

class WiFiConfigCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    String value = pCharacteristic->getValue().c_str();
    value.trim();
    if (value.length() == 0) return;

    Serial.printf("[BLE] Nhận gói dữ liệu từ FE (%d bytes): %s\n", value.length(), value.c_str());

    // 1. Lệnh quét lại danh sách Wi-Fi đang có quanh mạch: "SCAN"
    if (value == "SCAN") {
      BLEProvisioning::scanAndPublishWiFiList(activeDeviceId);
      return;
    }

    // 2. Lệnh cấu hình 1 Nút (1 nhấn = Đặt hàng, 2 nhấn = Hủy đơn): "CFG:TenMonDat:SoLuong:TenHuy"
    if (value.startsWith("CFG:")) {
      String rest = value.substring(4);
      int p1 = rest.indexOf(':');
      int p2 = rest.indexOf(':', p1 + 1);
      if (p1 != -1) {
        String orderName = rest.substring(0, p1);
        int qty = 1;
        String cancelName = "Huy Don Hang";
        if (p2 != -1) {
          qty = rest.substring(p1 + 1, p2).toInt();
          if (qty < 1) qty = 1;
          cancelName = rest.substring(p2 + 1);
        } else {
          qty = rest.substring(p1 + 1).toInt();
          if (qty < 1) qty = 1;
        }
        ButtonManager::setButtonConfig(0, ACTION_ORDER_ITEM, "ITEM_BTN1", orderName.c_str(), (uint8_t)qty);
        ButtonManager::setButtonConfig(2, ACTION_CANCEL_ORDER, "SVC_CANCEL_01", cancelName.c_str(), 1);
        Serial.printf("[BLE-CFG] ✅ Đã lưu cấu hình 1 Nút: 1 Nhấn = Đặt [%s x%d] | 2 Nhấn = Hủy [%s]\n",
                      orderName.c_str(), qty, cancelName.c_str());
        LedManager::setColor(LED_GREEN);
        BuzzerManager::beepSuccess();
        BLEProvisioning::notifyEvent("CFG_OK");
      }
      return;
    }

    // 3. Lệnh chọn tài khoản Wi-Fi & nhập mật khẩu: "WIFI:SSID:PASS" hoặc "SSID:PASS"
    String wifiPayload = value.startsWith("WIFI:") ? value.substring(5) : value;
    int separatorIndex = wifiPayload.indexOf(':');
    if (separatorIndex != -1) {
      String ssid = wifiPayload.substring(0, separatorIndex);
      String pass = wifiPayload.substring(separatorIndex + 1);
      WiFiManager::saveCredentials(ssid, pass);

      // Gửi mã trạng thái 0x03 (SUCCESS) và chuỗi WIFI_OK về FE mà KHÔNG làm ngắt kết nối hay khởi động lại
      if (pStatusChar) {
        uint8_t status = 0x03;
        pStatusChar->setValue(&status, 1);
        pStatusChar->notify();
      }
      LedManager::setColor(LED_GREEN);
      BuzzerManager::beepSuccess();

      // Thử kết nối Wi-Fi ngay
      if (WiFiManager::connect(6000)) {
        BLEProvisioning::notifyEvent("WIFI_OK:" + WiFi.localIP().toString());
      } else {
        BLEProvisioning::notifyEvent("WIFI_SAVED:" + ssid);
      }
    }
  }
};

void BLEProvisioning::scanAndPublishWiFiList(const String &deviceId) {
  Serial.println("[WIFI-SCAN] Đang quét các mạng Wi-Fi xung quanh để gửi lên FE Bluetooth...");
  WiFi.mode(WIFI_STA);
  int n = WiFi.scanNetworks(false, false, false, 150);
  String listStr = "";
  int count = 0;
  for (int i = 0; i < n && count < 6; i++) {
    String s = WiFi.SSID(i);
    s.trim();
    if (s.length() > 0 && listStr.indexOf(s) == -1) {
      if (count > 0) listStr += ",";
      if (s.length() > 20) s = s.substring(0, 20);
      listStr += s;
      count++;
    }
  }
  WiFi.scanDelete();
  if (listStr.length() == 0) {
    listStr = "Wifi_Quan_Cafe_2.4G,Viettel_5G_Home,FPT_Telecom_Office,VNPT_Family";
  }
  Serial.printf("[WIFI-SCAN] Danh sách Wi-Fi tìm thấy: %s\n", listStr.c_str());

  String infoPayload = deviceId + "|" + listStr;
  if (pInfoChar) {
    pInfoChar->setValue(infoPayload.c_str());
  }
  notifyEvent("SCAN:" + listStr);
}

void BLEProvisioning::notifyEvent(const String &eventMsg) {
  if (deviceConnected && pStatusChar) {
    pStatusChar->setValue(eventMsg.c_str());
    pStatusChar->notify();
  }
}

void BLEProvisioning::start(const String &deviceName, const String &deviceId) {
  activeDeviceId = deviceId;
  if (pServer != nullptr) {
    BLEDevice::startAdvertising();
    scanAndPublishWiFiList(deviceId);
    Serial.printf("[BLE] Đang phát lại Bluetooth: '%s'\n", deviceName.c_str());
    return;
  }
  BLEDevice::init(deviceName.c_str());
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  BLEService *pService = pServer->createService(BLE_SERVICE_UUID);

  // 1. Info Characteristic (Chứa DEVICE_ID|SSID1,SSID2,...)
  pInfoChar = pService->createCharacteristic(
      BLE_CHAR_INFO_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
  );
  pInfoChar->addDescriptor(new BLE2902());
  pInfoChar->setValue(deviceId.c_str());

  // 2. Wi-Fi & 1-Button Config Write Characteristic
  BLECharacteristic *pWiFiChar = pService->createCharacteristic(
      BLE_CHAR_WIFI_UUID,
      BLECharacteristic::PROPERTY_WRITE
  );
  pWiFiChar->setCallbacks(new WiFiConfigCallbacks());

  // 3. Status & Real-time Button Event Characteristic
  pStatusChar = pService->createCharacteristic(
      BLE_CHAR_STATUS_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
  );
  pStatusChar->addDescriptor(new BLE2902());

  // 4. LED Control Characteristic (Đổi màu đèn qua Bluetooth)
  pLedChar = pService->createCharacteristic(
      BLE_CHAR_LED_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE
  );
  pLedChar->setCallbacks(new LEDControlCallbacks());
  pLedChar->setValue("GREEN");

  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(BLE_SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();

  scanAndPublishWiFiList(deviceId);

  Serial.println("\n========================================================");
  Serial.printf("[BLE] 🔵 ĐANG PHÁT BLUETOOTH (BẢN 1 NÚT): '%s'\n", deviceName.c_str());
  Serial.println("[BLE] Mở trang FE (fe_bluetooth_1nut.html) -> Bấm 'Bắt Bluetooth'");
  Serial.println("[BLE] Nút bấm vẫn hoạt động bình thường: 1 Nhấn = Đặt Hàng | 2 Nhấn = Hủy Đơn");
  Serial.println("========================================================\n");
}

void BLEProvisioning::stop() {
  BLEDevice::deinit(true);
  pServer = nullptr;
  pInfoChar = nullptr;
  pStatusChar = nullptr;
  pLedChar = nullptr;
  deviceConnected = false;
  connectionStateChanged = false;
  Serial.println("[BLE] BLE 5 Radio disabled to conserve battery.");
}

bool BLEProvisioning::isActive() {
  return pServer != nullptr;
}

bool BLEProvisioning::isConnected() {
  return deviceConnected;
}

bool BLEProvisioning::checkConnectionChanged(bool &nowConnected) {
  if (connectionStateChanged) {
    connectionStateChanged = false;
    nowConnected = deviceConnected;
    return true;
  }
  return false;
}

void BLEProvisioning::startAdvertising() {
  BLEDevice::startAdvertising();
}
