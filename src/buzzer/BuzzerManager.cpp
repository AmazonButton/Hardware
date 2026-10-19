#include "BuzzerManager.h"

static void playTone(uint32_t freq, uint32_t durationMs) {
  // Hỗ trợ cả còi thụ động (Passive Piezo) phát đúng tần số resonant
  // lẫn còi chủ động (Active Buzzer) phát ra âm lượng to và rõ ràng
  tone(PIN_BUZZER, freq, durationMs);
  delay(durationMs);
  noTone(PIN_BUZZER);
  digitalWrite(PIN_BUZZER, LOW); // Đảm bảo Gate Q1 2N7002 về 0V (dòng tiêu thụ tĩnh = 0)
}

void BuzzerManager::init() {
  pinMode(PIN_BUZZER, OUTPUT);
  digitalWrite(PIN_BUZZER, LOW);
}

void BuzzerManager::beepBoot() {
  // 1 tiếng bíp ngắn 80ms (3.0kHz) khi khởi động nguồn
  playTone(3000, 80);
}

void BuzzerManager::beepSuccess() {
  // 2 tiếng bíp vui tai (Success Chime: 2.7kHz -> 3.5kHz)
  playTone(2700, 70);
  delay(60);
  playTone(3500, 90);
}

void BuzzerManager::beepError() {
  // 3 tiếng bíp cảnh báo gấp gáp (2.2kHz)
  for (int i = 0; i < 3; i++) {
    playTone(2200, 100);
    if (i < 2) delay(60);
  }
}

void BuzzerManager::beepCancel() {
  // 1 tiếng bíp trầm dài (Cancel Tone: 1.8kHz 320ms)
  playTone(1800, 320);
}

void BuzzerManager::beepBill() {
  // 2 tiếng bíp nhẹ nhàng thanh thoát (2.4kHz -> 2.8kHz) cho chức năng Gọi nhân viên / Thanh toán
  playTone(2400, 70);
  delay(50);
  playTone(2800, 80);
}
