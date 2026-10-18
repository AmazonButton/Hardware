#include "BLEProvisioning.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "../wifi/WiFiManager.h"
#include "../led/LedManager.h"

static BLEServer *pServer = nullptr;
static BLECharacteristic *pStatusChar = nullptr;
static BLECharacteristic *pLedChar = nullptr;
static bool deviceConnected = false;
static bool connectionStateChanged = false;

class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    deviceConnected = true;
    connectionStateChanged = true;
    Serial.println("\n[BLE] ========================================================");
    Serial.println("[BLE] >>> THIẾT BỊ ĐÃ KẾT NỐI BLUETOOTH THÀNH CÔNG! <<<");
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
    if (value.length() > 0) {
      Serial.printf("[BLE] Received Wi-Fi Provisioning Packet (%d bytes)\n", value.length());
      // Expected format: "SSID:PASSWORD"
      int separatorIndex = value.indexOf(':');
      if (separatorIndex != -1) {
        String ssid = value.substring(0, separatorIndex);
        String pass = value.substring(separatorIndex + 1);
        WiFiManager::saveCredentials(ssid, pass);

        // Notify client status: 0x03 (SUCCESS)
        if (pStatusChar) {
          uint8_t status = 0x03;
          pStatusChar->setValue(&status, 1);
          pStatusChar->notify();
        }

        delay(1000);
        ESP.restart();
      }
    }
  }
};

void BLEProvisioning::start(const String &deviceName, const String &deviceId) {
  if (pServer != nullptr) {
    Serial.println("[BLE] BLE đã được khởi tạo và đang hoạt động.");
    return;
  }
  BLEDevice::init(deviceName.c_str());
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  BLEService *pService = pServer->createService(BLE_SERVICE_UUID);

  // 1. Info Characteristic
  BLECharacteristic *pInfoChar = pService->createCharacteristic(
      BLE_CHAR_INFO_UUID,
      BLECharacteristic::PROPERTY_READ
  );
  pInfoChar->setValue(deviceId.c_str());

  // 2. Wi-Fi Config Write Characteristic
  BLECharacteristic *pWiFiChar = pService->createCharacteristic(
      BLE_CHAR_WIFI_UUID,
      BLECharacteristic::PROPERTY_WRITE
  );
  pWiFiChar->setCallbacks(new WiFiConfigCallbacks());

  // 3. Provisioning Status Characteristic
  pStatusChar = pService->createCharacteristic(
      BLE_CHAR_STATUS_UUID,
      BLECharacteristic::PROPERTY_NOTIFY
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
  pAdvertising->setMinPreferred(0x06); // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();

  Serial.println("\n========================================================");
  Serial.printf("[BLE] ĐANG PHÁT QUẢNG BÁ BLUETOOTH: '%s'\n", deviceName.c_str());
  Serial.println("[BLE] Đèn LED Xanh dương nhấp nháy: Đang chờ điện thoại kết nối Bluetooth...");
  Serial.println("[BLE] Khi điện thoại bắt được kết nối -> Đèn sẽ tự động đổi màu XANH LÁ!");
  Serial.println("========================================================\n");
}

void BLEProvisioning::stop() {
  BLEDevice::deinit(true);
  pServer = nullptr;
  pStatusChar = nullptr;
  pLedChar = nullptr;
  deviceConnected = false;
  connectionStateChanged = false;
  Serial.println("[BLE] BLE 5 Radio disabled to conserve battery.");
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
