#include "PowerManager.h"
#include <WiFi.h>
#include <esp_wifi.h>
#include "../button/ButtonManager.h"

static MAX17048 s_gauge;

void PowerManager::init() {
  // Hạ tần số CPU xuống 80MHz để tránh sốc dòng trên IC hạ áp LDO
  setCpuFrequencyMhz(ESP32_CPU_FREQ_MHZ);

  // Khởi tạo các chân nút bấm có điện trở kéo lên nội bộ
  pinMode(PIN_BUTTON_SW1, INPUT_PULLUP);
#if CURRENT_HARDWARE_VARIANT == HARDWARE_VARIANT_TRIPLE_BUTTON
  pinMode(PIN_BUTTON_SW2, INPUT_PULLUP);
  pinMode(PIN_BUTTON_SW3, INPUT_PULLUP);
#endif

#ifdef PIN_BATTERY_ADC
  pinMode(PIN_BATTERY_ADC, INPUT);
  analogReadResolution(12);
#endif

  // Initialize MAX17048 I2C Fuel Gauge on I2C Bus
  bool gaugeOk = s_gauge.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  if (gaugeOk) {
    Serial.println("[POWER] MAX17048 Fuel Gauge detected on I2C (addr 0x36).");
  } else {
    Serial.println("[POWER] MAX17048 not detected; using direct ADC for Pin 18650/LiPo (3.2V - 4.2V).");
  }
}

void PowerManager::enterSleep() {
  // Tắt hoàn toàn ngoại vi sóng vô tuyến Wi-Fi để tiết kiệm năng lượng
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  esp_wifi_stop();
  Serial.println("[POWER] Wi-Fi stopped. Entering ultra low power sleep, waiting for button events...");
  Serial.flush();

#if USE_HARDWARE_MOD_RTC_PIN
  // Với mạch đã câu dây SW1 sang IO10 (RTC GPIO): hỗ trợ Deep Sleep ext0 chuẩn <15uA
  Serial.println("[POWER] Entering DEEP SLEEP (<15uA) via RTC IO10...");
  Serial.flush();
  esp_sleep_enable_ext0_wakeup(PIN_BUTTON_SW1, 0);
  esp_deep_sleep_start();
#else
  // Với mạch nguyên bản:
  // Dùng Light Sleep: tiêu thụ điện cực thấp (~300uA), đánh thức ngay lập tức trong <50us
  gpio_wakeup_enable(PIN_BUTTON_SW1, GPIO_INTR_LOW_LEVEL);
  if (ButtonManager::getVariant() == HARDWARE_VARIANT_TRIPLE_BUTTON) {
    gpio_wakeup_enable(PIN_BUTTON_SW2, GPIO_INTR_LOW_LEVEL);
    gpio_wakeup_enable(PIN_BUTTON_SW3, GPIO_INTR_LOW_LEVEL);
  }
  esp_sleep_enable_gpio_wakeup();

  esp_light_sleep_start();

  // Khi bấm nút kéo chân xuống LOW, CPU tiếp tục thực thi ngay lập tức tại đây!
  Serial.println("[POWER] >>> Woke up instantly from Light Sleep via Button press!");
#endif
}

esp_sleep_wakeup_cause_t PowerManager::getWakeupCause() {
  return esp_sleep_get_wakeup_cause();
}

uint32_t PowerManager::readBatteryMillivolts() {
  if (s_gauge.isConnected()) {
    return s_gauge.getVoltageMv();
  }

#ifdef PIN_BATTERY_ADC
  uint32_t rawMv = analogReadMilliVolts(PIN_BATTERY_ADC);
  if (rawMv > 50) {
    uint32_t battMv = (uint32_t)(rawMv * BATTERY_ADC_DIVIDER_RATIO);
    return constrain(battMv, (uint32_t)2500, (uint32_t)4500);
  }
#endif

  return 3850; // Nominal fallback (3.85V)
}

uint8_t PowerManager::calculateBatteryPercentage(uint32_t millivolts) {
  if (s_gauge.isConnected()) {
    float soc = s_gauge.getSOC();
    return (uint8_t)constrain((int)round(soc), 0, 100);
  }

  // Fallback estimation for 18650 / LiPo cell (3.2V to 4.2V)
  if (millivolts == 0) millivolts = readBatteryMillivolts();
  if (millivolts >= BATTERY_MAX_MV) return 100;
  if (millivolts <= BATTERY_MIN_MV) return 0;
  return (uint8_t)(((millivolts - BATTERY_MIN_MV) * 100) / (BATTERY_MAX_MV - BATTERY_MIN_MV));
}

MAX17048& PowerManager::getGauge() {
  return s_gauge;
}
