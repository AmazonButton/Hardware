#include "PowerManager.h"
#include <Wire.h>

static MAX17048 s_gauge;

void PowerManager::init() {
  // Hạ tần số CPU xuống 80MHz để tránh sốc dòng trên IC hạ áp LDO ME6211 (500mA)
  setCpuFrequencyMhz(ESP32_CPU_FREQ_MHZ);

  // Initialize MAX17048 I2C Fuel Gauge on IO4 (SDA) and IO5 (SCL)
  bool gaugeOk = s_gauge.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  if (gaugeOk) {
    Serial.println("[POWER] MAX17048 Fuel Gauge detected on I2C (addr 0x36).");
  } else {
    Serial.println("[POWER] Note: MAX17048 I2C not responding; fallback to nominal estimation.");
  }

  // Đảm bảo chân nút bấm có trở kéo lên
  pinMode(PIN_BUTTON, INPUT_PULLUP);

#if USE_HARDWARE_MOD_RTC_PIN
  // Với mạch đã câu dây SW1 sang IO10 (RTC GPIO): hỗ trợ Deep Sleep ext0 chuẩn <15uA
  esp_sleep_enable_ext0_wakeup(PIN_BUTTON, 0);
  Serial.println("[POWER] Configured RTC Wakeup on IO10 (Deep Sleep Mode ready).");
#else
  // Với mạch nguyên bản KiCad (IO39 thuộc HP domain):
  // Cấu hình GPIO wakeup tương thích an toàn trên ESP32-S3
  gpio_wakeup_enable(PIN_BUTTON, GPIO_INTR_LOW_LEVEL);
  esp_sleep_enable_gpio_wakeup();
  Serial.println("[POWER] Configured HP GPIO Wakeup on IO39 (Light Sleep Fallback ready).");
#endif
}

void PowerManager::enterDeepSleep() {
  Serial.println("[POWER] Shutting down peripherals. Entering low-power sleep...");
  Serial.flush();

#if USE_HARDWARE_MOD_RTC_PIN
  Serial.println("[POWER] Entering DEEP SLEEP (<15uA) via RTC IO10. Waiting for SW1 press...");
  esp_sleep_enable_ext0_wakeup(PIN_BUTTON, 0);
  esp_deep_sleep_start();
#else
  Serial.println("[POWER] ⚠️ Chân IO39 không thuộc RTC: Chuyển sang LIGHT SLEEP an toàn...");
  Serial.println("[POWER] (Khi bấm SW1 mạch sẽ thức dậy ngay lập tức mà không bị treo chết mạch!)");
  Serial.println("[POWER] (Mẹo: Câu 1 sợi dây từ SW1 sang IO10 để kích hoạt Deep Sleep <15uA chuẩn)");
  Serial.flush();

  gpio_wakeup_enable(PIN_BUTTON, GPIO_INTR_LOW_LEVEL);
  esp_sleep_enable_gpio_wakeup();
  esp_light_sleep_start();

  // Khi người dùng bấm nút SW1 (kéo IO39 xuống LOW), ESP32-S3 thức dậy tại đây
  Serial.println("[POWER] >>> Thức dậy từ Light Sleep qua nút SW1 (IO39)! Khởi động lại chu trình...");
  delay(50);
  ESP.restart();
#endif
}

esp_sleep_wakeup_cause_t PowerManager::getWakeupCause() {
  return esp_sleep_get_wakeup_cause();
}

uint32_t PowerManager::readBatteryMillivolts() {
  if (s_gauge.isConnected()) {
    return s_gauge.getVoltageMv();
  }
  return 3850; // Nominal fallback (3.85V)
}

uint8_t PowerManager::calculateBatteryPercentage(uint32_t millivolts) {
  if (s_gauge.isConnected()) {
    float soc = s_gauge.getSOC();
    return (uint8_t)constrain((int)round(soc), 0, 100);
  }

  // Fallback estimation from millivolts
  if (millivolts == 0) millivolts = 3850;
  if (millivolts >= 4200) return 100;
  if (millivolts <= 3300) return 0;
  return (uint8_t)(((millivolts - 3300) * 100) / (4200 - 3300));
}

MAX17048& PowerManager::getGauge() {
  return s_gauge;
}
