#ifndef TFT_DISPLAY_MANAGER_H
#define TFT_DISPLAY_MANAGER_H

#include <Arduino.h>
#include <SPI.h>
#include "../../include/config.h"

class TftDisplayManager {
private:
  static bool s_initialized;

public:
  static void init();
  static bool isAvailable();
};

#endif // TFT_DISPLAY_MANAGER_H
