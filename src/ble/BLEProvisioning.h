#ifndef BLE_PROVISIONING_H
#define BLE_PROVISIONING_H

#include <Arduino.h>
#include "config.h"

class BLEProvisioning {
public:
  static void start(const String &deviceName, const String &deviceId);
  static void stop();
  static bool isConnected();
  static bool checkConnectionChanged(bool &nowConnected);
  static void startAdvertising();
};

#endif
