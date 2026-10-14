#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// =========================================================================
// HARDWARE PIN DEFINITIONS (ESP32-S3-WROOM-1)
// Khớp 100% Sơ đồ nguyên lý KiCad: Smart_Order_Button.kicad_sch & PCB Layout
// =========================================================================

// Button SW1 (Net BTN_ORDER)
// ⚠️ ĐẶC TÍNH PHẦN CỨNG ESP32-S3 QUAN TRỌNG:
// - Trên module ESP32-S3-WROOM-1, chân IO39 nằm ở Pin 32 (Net BTN_ORDER nối SW1 xuống GND).
// - Do IO39 thuộc High-Performance (HP) Domain, không nằm trong RTC Domain (IO0-IO21),
//   ESP32-S3 hỗ trợ đánh thức từ LIGHT SLEEP qua IO39 cực kỳ nhanh (<50us), không cần reboot!
// - USE_HARDWARE_MOD_RTC_PIN = 0 (MẶC ĐỊNH): Chạy nguyên bản mạch KiCad/PCB qua IO39 (Light Sleep).
// - USE_HARDWARE_MOD_RTC_PIN = 1: Đã câu dây phần cứng SW1 sang chân RTC IO10 (Pin 18) -> Cho phép Deep Sleep <15uA.
#define USE_HARDWARE_MOD_RTC_PIN    0             // Đặt 1 nếu đã mod câu dây sang IO10

#if USE_HARDWARE_MOD_RTC_PIN
#define PIN_BUTTON                  GPIO_NUM_10   // IO10 (Pin 18) - RTC GPIO Wakeup Deep Sleep <15uA
#else
#define PIN_BUTTON                  GPIO_NUM_39   // IO39 (Pin 32) - Net BTN_ORDER KiCad/PCB gốc (Light Sleep)
#endif

// Tối ưu hóa nguồn để LDO ME6211C33M5G (500mA) không bị sụt áp Brownout khi phát Wi-Fi
#define ESP32_CPU_FREQ_MHZ          80            // 80MHz tiết kiệm điện năng và giảm xung dòng đỉnh
#define WIFI_TX_POWER_SAFE          WIFI_POWER_15dBm // 15dBm giảm dòng đỉnh từ 480mA xuống ~220mA

// Status RGB LED D1 (Common Anode nối nguồn +3.3V -> Active LOW)
#define PIN_LED_RED                 7             // IO7  (Pin 7)  - Net LED_R via R15 (1kΩ)
#define PIN_LED_GREEN               6             // IO6  (Pin 6)  - Net LED_G via R16 (470Ω)
#define PIN_LED_BLUE                8             // IO8  (Pin 12) - Net LED_B via R17 (470Ω)

// Piezo / Magnetic Buzzer BZ1
// Kích qua MOSFET N-Channel Q1 2N7002 qua chân IO2 (Active HIGH, gate pull-down R6 10kΩ)
#define PIN_BUZZER                  2             // IO2  (Pin 38) - Net BUZZER_CTRL
#define BUZZER_FREQ_DEFAULT         2700          // Tần số âm báo 2.7kHz (chuẩn resonant piezo/active)

// Battery Fuel Gauge IC U6 (MAX17048G_T10) via I2C Bus
#define PIN_I2C_SDA                 4             // IO4  (Pin 4)  - Net I2C_SDA (Trở treo R9 4.7kΩ)
#define PIN_I2C_SCL                 5             // IO5  (Pin 5)  - Net I2C_SCL (Trở treo R3 4.7kΩ)
#define MAX17048_I2C_ADDR           0x36          // Địa chỉ I2C 7-bit mặc định của MAX17048

// Native USB Type-C trên ESP32-S3 (Bảo vệ qua U2 USBLC6-2SC6)
// IO19 = USB_D- (Pin 13, USB_DN)
// IO20 = USB_D+ (Pin 14, USB_DP)

// =========================================================================
// GESTURE & TIMING CONSTANTS (Non-blocking & configurable from backend)
// =========================================================================
#define DEBOUNCE_MS                 30    // Ngưỡng lọc rung phím (debounce)
#define SHORT_PRESS_MAX_MS          800   // Nhấn nhanh (<800ms) -> Tạo đơn hàng (Order)
#define DOUBLE_PRESS_WINDOW_MS      500   // Khoảng cách tối đa giữa 2 lần nhấn đúp
#define HOLD_5S_ACTIVATE_MS         5000  // Nhấn giữ 5s -> Kích hoạt / Đồng bộ cấu hình
#define HOLD_10S_PROVISION_MS       10000 // Nhấn giữ 10s -> Cấu hình Wi-Fi (BLE 5 + SoftAP)
#define HOLD_15S_FACTORY_RESET_MS   15000 // Nhấn giữ 15s -> Xóa sạch bộ nhớ NVS Flash

#define WIFI_CONNECT_TIMEOUT_MS     6000  // Thời gian tối đa kết nối Wi-Fi trước khi ngủ
#define WIFI_MAX_RETRIES            3     // Số lần thử kết nối tránh treo cạn kiệt pin
#define BLE_PROVISION_TIMEOUT_S     180   // 3 phút phát BLE quảng bá cấu hình

// =========================================================================
// BLE 5 GATT SERVICE & CHARACTERISTICS UUIDs
// =========================================================================
#define BLE_SERVICE_UUID            "0000FFF0-0000-1000-8000-00805F9B34FB"
#define BLE_CHAR_INFO_UUID          "0000FFF1-0000-1000-8000-00805F9B34FB" // Read Device Info
#define BLE_CHAR_WIFI_UUID          "0000FFF2-0000-1000-8000-00805F9B34FB" // Write SSID + Pass
#define BLE_CHAR_STATUS_UUID        "0000FFF3-0000-1000-8000-00805F9B34FB" // Notify Status

// =========================================================================
// DEFAULT CLOUD BACKEND URL (Được ghi đè từ NVS Flash)
// =========================================================================
#define DEFAULT_BACKEND_URL         "http://192.168.1.50:5000"

#endif // CONFIG_H
