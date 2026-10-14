#include "ButtonManager.h"

void ButtonManager::init() {
  pinMode(PIN_BUTTON, INPUT_PULLUP);
}

bool ButtonManager::isPressed() {
  return (digitalRead(PIN_BUTTON) == LOW);
}

ButtonGesture ButtonManager::detectGesture(bool wokenFromSleep) {
  bool initiallyLow = (digitalRead(PIN_BUTTON) == LOW);

  // Nếu không thức dậy từ sleep và nút không được nhấn -> Không có thao tác
  if (!initiallyLow && !wokenFromSleep) {
    return GESTURE_NONE;
  }

  unsigned long pressStart = millis();

  // 1. Nếu nút đang được đè giữ (LOW): Đo chính xác thời lượng giữ
  if (initiallyLow) {
    while (digitalRead(PIN_BUTTON) == LOW) {
      unsigned long heldMs = millis() - pressStart;

      // Giữ >= 15 giây -> Khôi phục cài đặt gốc
      if (heldMs >= HOLD_15S_FACTORY_RESET_MS) {
        while (digitalRead(PIN_BUTTON) == LOW) delay(10);
        return GESTURE_HOLD_15S_FACTORY_RESET;
      }
      delay(10);
    }

    unsigned long totalHeldMs = millis() - pressStart;

    // Giữ từ 10s đến 15s -> Cấu hình Wi-Fi BLE 5 & SoftAP
    if (totalHeldMs >= HOLD_10S_PROVISION_MS) {
      return GESTURE_HOLD_10S_PROVISION;
    }

    // Giữ từ 5s đến 10s -> Kích hoạt / Đồng bộ tham số cấu hình
    if (totalHeldMs >= HOLD_5S_ACTIVATE_MS) {
      return GESTURE_HOLD_5S_ACTIVATE;
    }

    // Nếu không phải giữ lâu thì đây là 1 lần nhấn hợp lệ (<5s)
  }

  // 2. Chờ phát hiện nhấn đúp hoặc 5 lần nhấn (Multi-click window)
  int clickCount = 1;
  unsigned long lastReleaseTime = millis();
  const unsigned long MULTI_CLICK_WINDOW_MS = 450; // Khoảng thời gian chờ click tiếp theo

  while (millis() - lastReleaseTime < MULTI_CLICK_WINDOW_MS) {
    if (digitalRead(PIN_BUTTON) == LOW) {
      delay(DEBOUNCE_MS);
      if (digitalRead(PIN_BUTTON) == LOW) {
        clickCount++;
        // Chờ nhả phím
        while (digitalRead(PIN_BUTTON) == LOW) {
          delay(10);
        }
        lastReleaseTime = millis();

        // 5 lần bấm nhanh liên tiếp -> Bật cấu hình nhanh BLE
        if (clickCount >= 5) {
          return GESTURE_FIVE_CLICKS_PAIR;
        }
      }
    }
    delay(10);
  }

  if (clickCount == 2) {
    return GESTURE_DOUBLE_PRESS;
  }

  return GESTURE_SHORT_PRESS;
}
