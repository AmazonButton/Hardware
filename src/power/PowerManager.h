#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include <Arduino.h>
#include <esp_sleep.h>
#include "../../include/config.h"
#include "MAX17048.h"

class PowerManager {
public:
  static void init();
  static void enterSleep();
  static esp_sleep_wakeup_cause_t getWakeupCause();
  static uint32_t readBatteryMillivolts();
  static uint8_t calculateBatteryPercentage(uint32_t millivolts = 0);
  static MAX17048& getGauge();
};

#endif // POWER_MANAGER_H
