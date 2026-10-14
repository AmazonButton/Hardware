#include "MAX17048.h"

MAX17048::MAX17048() : _initialized(false) {}

bool MAX17048::begin(uint8_t sdaPin, uint8_t sclPin) {
  Wire.begin(sdaPin, sclPin, 100000); // 100kHz standard I2C
  delay(10);

  // Probe MAX17048 on address 0x36
  Wire.beginTransmission(I2C_ADDR);
  if (Wire.endTransmission() == 0) {
    _initialized = true;
    return true;
  }

  _initialized = false;
  return false;
}

bool MAX17048::isConnected() {
  Wire.beginTransmission(I2C_ADDR);
  return (Wire.endTransmission() == 0);
}

uint16_t MAX17048::readRegister16(uint8_t reg) {
  Wire.beginTransmission(I2C_ADDR);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) {
    return 0;
  }

  if (Wire.requestFrom((uint8_t)I2C_ADDR, (uint8_t)2) == 2) {
    uint8_t msb = Wire.read();
    uint8_t lsb = Wire.read();
    return (uint16_t)((msb << 8) | lsb);
  }

  return 0;
}

void MAX17048::writeRegister16(uint8_t reg, uint16_t value) {
  Wire.beginTransmission(I2C_ADDR);
  Wire.write(reg);
  Wire.write((uint8_t)(value >> 8));
  Wire.write((uint8_t)(value & 0xFF));
  Wire.endTransmission();
}

uint32_t MAX17048::getVoltageMv() {
  // Register 0x02 is VCELL (16-bit)
  // 12-bit ADC value in top 12 bits: resolution is 1.25mV per 16 LSBs
  // VCELL (mV) = (raw >> 4) * 1.25
  uint16_t raw = readRegister16(0x02);
  if (raw == 0) {
    return 3700; // Fallback nominal LiPo voltage
  }
  uint32_t vcellMv = (uint32_t)(((raw >> 4) * 5) / 4);
  return vcellMv;
}

float MAX17048::getVoltage() {
  return (float)getVoltageMv() / 1000.0f;
}

float MAX17048::getSOC() {
  // Register 0x04 is SOC (16-bit)
  // MSB is integer percentage (0-100), LSB is fraction (1/256)
  uint16_t raw = readRegister16(0x04);
  if (raw == 0) {
    return 80.0f; // Fallback nominal percentage
  }
  uint8_t intPct = (uint8_t)(raw >> 8);
  float fracPct = (float)(raw & 0xFF) / 256.0f;
  float soc = (float)intPct + fracPct;
  if (soc > 100.0f) soc = 100.0f;
  if (soc < 0.0f) soc = 0.0f;
  return soc;
}

void MAX17048::reset() {
  writeRegister16(0xFE, 0x5400); // Reset command
}
