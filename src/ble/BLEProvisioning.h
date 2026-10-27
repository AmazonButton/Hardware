#ifndef BLE_PROVISIONING_H
#define BLE_PROVISIONING_H

#include <Arduino.h>
#include "../../include/config.h"

class BLEProvisioning {
public:
  static void start(const String &deviceName, const String &deviceId);
  static void stop();
  static bool isActive();
  static bool isConnected();
  static bool checkConnectionChanged(bool &nowConnected);
  static void startAdvertising();
  static void scanAndPublishWiFiList(const String &deviceId);
  static void notifyEvent(const String &eventMsg);
};

#endif
