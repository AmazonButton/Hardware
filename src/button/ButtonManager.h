#ifndef BUTTON_MANAGER_H
#define BUTTON_MANAGER_H

#include <Arduino.h>
#include "config.h"

enum ButtonGesture {
  GESTURE_NONE = 0,

  // === BẢN 1 NÚT (SINGLE-BUTTON MULTI-GESTURE TRÊN SW1) ===
  GESTURE_SHORT_PRESS,              // SW1 nhấn nhanh (<800ms) -> Tạo đơn hàng (ORDER)
  GESTURE_DOUBLE_PRESS,             // SW1 nhấn đúp (<500ms) -> Hủy đơn hàng (CANCEL)
  GESTURE_FIVE_CLICKS_PAIR,         // SW1 nhấn 5 lần -> Ghép đôi BLE / SoftAP
  GESTURE_HOLD_5S_ACTIVATE,         // SW1 nhấn giữ 5s -> Đồng bộ / Kích hoạt
  GESTURE_HOLD_10S_PROVISION,       // SW1 nhấn giữ 10s -> Cấu hình Wi-Fi BLE 5
  GESTURE_HOLD_15S_FACTORY_RESET,   // SW1 nhấn giữ 15s -> Xóa sạch Flash NVS

  // === BẢN 3 NÚT (TRIPLE-BUTTON DEDICATED BUTTONS) ===
  GESTURE_BTN1_ORDER,               // Nút SW1 (IO39): Đặt hàng tức thì (ORDER)
  GESTURE_BTN2_BILL,                // Nút SW2 (IO40): Yêu cầu thanh toán / Gọi nhân viên (BILL / CALL STAFF)
  GESTURE_BTN3_CANCEL,              // Nút SW3 (IO41): Hủy đơn hàng (CANCEL)
  GESTURE_BTN2_HOLD_CONFIG,         // Nút SW2 giữ 5s -> Cấu hình BLE Wi-Fi
  GESTURE_ALL_HOLD_RESET            // SW1 + SW3 giữ 5s -> Khôi phục cài đặt gốc
};

class ButtonManager {
private:
  static uint8_t s_variant;
  static ButtonConfig s_configs[3];

public:
  static void init(uint8_t variant = CURRENT_HARDWARE_VARIANT);
  static void setVariant(uint8_t variant);
  static uint8_t getVariant();

  static void setButtonConfig(uint8_t buttonIndex, const ButtonConfig &config);
  static ButtonConfig getButtonConfig(uint8_t buttonIndex);
  static void resetDefaultButtonConfigs();

  static bool isPressed();
  static bool isPressed(gpio_num_t pin);
  
  static ButtonGesture detectGesture(bool wokenFromSleep = false);

private:
  static ButtonGesture detectSingleButtonGesture(bool wokenFromSleep);
  static ButtonGesture detectTripleButtonGesture(bool wokenFromSleep);
};

#endif
