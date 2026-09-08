#ifndef MAX17048_H
#define MAX17048_H

#include <Arduino.h>
#include <Wire.h>

class MAX17048 {
public:
  MAX17048();

  // Initialize I2C communication on IO4 (SDA) and IO5 (SCL)
  bool begin(uint8_t sdaPin = 4, uint8_t sclPin = 5);

  // Read single-cell battery voltage in millivolts (e.g. 3700 mV = 3.7V)
  uint32_t getVoltageMv();

  // Read battery voltage in Volts (e.g. 4.15 V)
  float getVoltage();

  // Read State of Charge in percentage (0.0% to 100.0%)
  float getSOC();

  // Check if MAX17048 is responding on I2C bus (0x36)
  bool isConnected();

  // Reset IC configuration
  void reset();

private:
  uint16_t readRegister16(uint8_t reg);
  void writeRegister16(uint8_t reg, uint16_t value);
  bool _initialized;
  static const uint8_t I2C_ADDR = 0x36;
};

#endif // MAX17048_H
