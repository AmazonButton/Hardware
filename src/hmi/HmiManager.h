#ifndef HMI_MANAGER_H
#define HMI_MANAGER_H

#include <Arduino.h>
#include "../../include/config.h"

class HmiManager {
private:
  static bool s_initialized;
  static HardwareSerial* s_serial;

public:
  static void init();
  static bool isAvailable();

  // Các phương thức hiển thị trạng thái hệ thống lên màn hình HMI UART
  static void showBoot(const String& deviceId, const String& model);
  static void showBleAdvertising(const String& bleName, const String& deviceId);
  static void showBleConnected();
  static void showWifiConnecting(int attempt, int maxAttempts);
  static void showWifiConnected(const String& ssid, const String& ipStr, int8_t rssi, uint8_t batteryPct);
  static void showWifiError();

  static void showOrderSending(const String& itemName, uint8_t quantity);
  static void showOrderSuccess(const String& orderNumber, const String& itemName, uint8_t quantity);
  static void showBillRequest();
  static void showBillSuccess();
  static void showCancelOrder();
  static void showCancelSuccess();

  static void showBatteryStatus(uint32_t voltageMv, uint8_t percentage);
  static void showError(const String& message);

  // Gửi lệnh thô chuẩn Nextion (kết thúc 0xFF 0xFF 0xFF) hoặc chuỗi text
  static void sendNextionCommand(const String& cmd);
  static void sendText(const String& text);
};

#endif // HMI_MANAGER_H
