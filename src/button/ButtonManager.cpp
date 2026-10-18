#include "ButtonManager.h"

uint8_t ButtonManager::s_variant = CURRENT_HARDWARE_VARIANT;

ButtonConfig ButtonManager::s_configs[3] = {
  { DEFAULT_BTN1_ACTION_TYPE, DEFAULT_BTN1_ITEM_ID, DEFAULT_BTN1_ITEM_NAME, DEFAULT_BTN1_QUANTITY, 0 },
  { DEFAULT_BTN2_ACTION_TYPE, DEFAULT_BTN2_ITEM_ID, DEFAULT_BTN2_ITEM_NAME, DEFAULT_BTN2_QUANTITY, 1 },
  { DEFAULT_BTN3_ACTION_TYPE, DEFAULT_BTN3_ITEM_ID, DEFAULT_BTN3_ITEM_NAME, DEFAULT_BTN3_QUANTITY, 2 }
};

void ButtonManager::resetDefaultButtonConfigs() {
  s_configs[0] = { DEFAULT_BTN1_ACTION_TYPE, DEFAULT_BTN1_ITEM_ID, DEFAULT_BTN1_ITEM_NAME, DEFAULT_BTN1_QUANTITY, 0 };
  s_configs[1] = { DEFAULT_BTN2_ACTION_TYPE, DEFAULT_BTN2_ITEM_ID, DEFAULT_BTN2_ITEM_NAME, DEFAULT_BTN2_QUANTITY, 1 };
  s_configs[2] = { DEFAULT_BTN3_ACTION_TYPE, DEFAULT_BTN3_ITEM_ID, DEFAULT_BTN3_ITEM_NAME, DEFAULT_BTN3_QUANTITY, 2 };
}

void ButtonManager::setButtonConfig(uint8_t buttonIndex, const ButtonConfig &config) {
  if (buttonIndex < 3) {
    s_configs[buttonIndex] = config;
  }
}

ButtonConfig ButtonManager::getButtonConfig(uint8_t buttonIndex) {
  if (buttonIndex < 3) {
    return s_configs[buttonIndex];
  }
  return s_configs[0];
}

void ButtonManager::init(uint8_t variant) {
  s_variant = variant;
  pinMode(PIN_BUTTON_SW1, INPUT_PULLUP);
  if (s_variant == HARDWARE_VARIANT_TRIPLE_BUTTON) {
    pinMode(PIN_BUTTON_SW2, INPUT_PULLUP);
    pinMode(PIN_BUTTON_SW3, INPUT_PULLUP);
  }
}

void ButtonManager::setVariant(uint8_t variant) {
  s_variant = variant;
  init(variant);
}

uint8_t ButtonManager::getVariant() {
  return s_variant;
}

bool ButtonManager::isPressed(gpio_num_t pin) {
  return (digitalRead(pin) == LOW);
}

bool ButtonManager::isPressed() {
  if (digitalRead(PIN_BUTTON_SW1) == LOW) return true;
  if (s_variant == HARDWARE_VARIANT_TRIPLE_BUTTON) {
    if (digitalRead(PIN_BUTTON_SW2) == LOW) return true;
    if (digitalRead(PIN_BUTTON_SW3) == LOW) return true;
  }
  return false;
}

ButtonGesture ButtonManager::detectGesture(bool wokenFromSleep) {
  if (s_variant == HARDWARE_VARIANT_TRIPLE_BUTTON) {
    return detectTripleButtonGesture(wokenFromSleep);
  } else {
    return detectSingleButtonGesture(wokenFromSleep);
  }
}

// =========================================================================
// BẢN 1 NÚT (SINGLE-BUTTON MULTI-GESTURE TRÊN SW1)
// =========================================================================
ButtonGesture ButtonManager::detectSingleButtonGesture(bool wokenFromSleep) {
  bool initiallyLow = (digitalRead(PIN_BUTTON_SW1) == LOW);

  // Nếu không thức dậy từ sleep và nút không được nhấn -> Không có thao tác
  if (!initiallyLow && !wokenFromSleep) {
    return GESTURE_NONE;
  }

  unsigned long pressStart = millis();

  // 1. Nếu nút đang được đè giữ (LOW): Đo chính xác thời lượng giữ
  if (initiallyLow) {
    while (digitalRead(PIN_BUTTON_SW1) == LOW) {
      unsigned long heldMs = millis() - pressStart;

      // Giữ >= 15 giây -> Khôi phục cài đặt gốc
      if (heldMs >= HOLD_15S_FACTORY_RESET_MS) {
        while (digitalRead(PIN_BUTTON_SW1) == LOW) delay(10);
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
  }

  // 2. Chờ phát hiện nhấn đúp hoặc 5 lần nhấn (Multi-click window)
  int clickCount = 1;
  unsigned long lastReleaseTime = millis();
  const unsigned long MULTI_CLICK_WINDOW_MS = 450;

  while (millis() - lastReleaseTime < MULTI_CLICK_WINDOW_MS) {
    if (digitalRead(PIN_BUTTON_SW1) == LOW) {
      delay(DEBOUNCE_MS);
      if (digitalRead(PIN_BUTTON_SW1) == LOW) {
        clickCount++;
        // Chờ nhả phím
        while (digitalRead(PIN_BUTTON_SW1) == LOW) {
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

// =========================================================================
// BẢN 3 NÚT (TRIPLE-BUTTON DEDICATED BUTTONS: SW1, SW2, SW3)
// =========================================================================
ButtonGesture ButtonManager::detectTripleButtonGesture(bool wokenFromSleep) {
  bool sw1 = (digitalRead(PIN_BUTTON_SW1) == LOW);
  bool sw2 = (digitalRead(PIN_BUTTON_SW2) == LOW);
  bool sw3 = (digitalRead(PIN_BUTTON_SW3) == LOW);

  if (!sw1 && !sw2 && !sw3 && !wokenFromSleep) {
    return GESTURE_NONE;
  }

  delay(DEBOUNCE_MS);
  sw1 = (digitalRead(PIN_BUTTON_SW1) == LOW);
  sw2 = (digitalRead(PIN_BUTTON_SW2) == LOW);
  sw3 = (digitalRead(PIN_BUTTON_SW3) == LOW);

  unsigned long pressStart = millis();

  // COMBO: Giữ đồng thời SW1 + SW3 trong 5s -> Factory Reset
  if (sw1 && sw3) {
    while (digitalRead(PIN_BUTTON_SW1) == LOW && digitalRead(PIN_BUTTON_SW3) == LOW) {
      if (millis() - pressStart >= 5000) {
        while (isPressed()) delay(10);
        return GESTURE_ALL_HOLD_RESET;
      }
      delay(10);
    }
  }

  // NÚT SW1: ĐẶT HÀNG (ORDER)
  if (sw1) {
    while (digitalRead(PIN_BUTTON_SW1) == LOW) {
      unsigned long heldMs = millis() - pressStart;
      if (heldMs >= HOLD_15S_FACTORY_RESET_MS) {
        while (digitalRead(PIN_BUTTON_SW1) == LOW) delay(10);
        return GESTURE_HOLD_15S_FACTORY_RESET;
      }
      if (heldMs >= HOLD_10S_PROVISION_MS) {
        while (digitalRead(PIN_BUTTON_SW1) == LOW) delay(10);
        return GESTURE_HOLD_10S_PROVISION;
      }
      delay(10);
    }
    return GESTURE_BTN1_ORDER;
  }

  // NÚT SW2: GỌI NHÂN VIÊN / THANH TOÁN (BILL / CALL STAFF)
  if (sw2) {
    while (digitalRead(PIN_BUTTON_SW2) == LOW) {
      unsigned long heldMs = millis() - pressStart;
      if (heldMs >= HOLD_5S_ACTIVATE_MS) {
        while (digitalRead(PIN_BUTTON_SW2) == LOW) delay(10);
        return GESTURE_BTN2_HOLD_CONFIG;
      }
      delay(10);
    }
    return GESTURE_BTN2_BILL;
  }

  // NÚT SW3: HỦY ĐƠN HÀNG (CANCEL ORDER / SOS)
  if (sw3) {
    while (digitalRead(PIN_BUTTON_SW3) == LOW) {
      unsigned long heldMs = millis() - pressStart;
      if (heldMs >= HOLD_15S_FACTORY_RESET_MS) {
        while (digitalRead(PIN_BUTTON_SW3) == LOW) delay(10);
        return GESTURE_HOLD_15S_FACTORY_RESET;
      }
      delay(10);
    }
    return GESTURE_BTN3_CANCEL;
  }

  return GESTURE_NONE;
}
