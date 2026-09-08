#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// =========================================================================
// HARDWARE PIN DEFINITIONS (ESP32-S3-WROOM-1)
// Fully mapped from KiCad Schematic: Smart_Order_Button.kicad_sch
// =========================================================================

// Button & RTC Wakeup Pin
// ⚠️ LƯU Ý PHẦN CỨNG ESP32-S3 QUAN TRỌNG:
// Trên vi điều khiển ESP32-S3, chỉ có các chân từ IO0 -> IO21 thuộc RTC Domain hỗ trợ Deep Sleep EXT0/EXT1.
// Chân IO39 (Pin 20) thuộc HP Domain, KHÔNG THỂ đánh thức từ Deep Sleep nếu dùng esp_sleep_enable_ext0_wakeup!
// - USE_HARDWARE_MOD_RTC_PIN = 0: Dùng chân IO39 gốc theo KiCad (Firmware tự động dùng Light Sleep để bấm nút vẫn thức dậy được 100%).
// - USE_HARDWARE_MOD_RTC_PIN = 1: Đã câu dây phần cứng SW1 sang chân RTC IO10 (Pin 14) -> Cho phép Deep Sleep chuẩn <15uA.
#define USE_HARDWARE_MOD_RTC_PIN    0             // Đổi thành 1 khi đã câu dây SW1 sang IO10

#if USE_HARDWARE_MOD_RTC_PIN
#define PIN_BUTTON                  GPIO_NUM_10   // IO10 (Pin 14) - RTC GPIO Wakeup chuẩn <15uA
#else
#define PIN_BUTTON                  GPIO_NUM_39   // IO39 (Pin 20) - Net BTN_ORDER KiCad gốc
#endif

// Tối ưu hóa nguồn để LDO ME6211C33M5G (500mA) không bị sụt áp Brownout khi phát Wi-Fi
#define ESP32_CPU_FREQ_MHZ          80            // 80MHz tiết kiệm điện năng và giảm xung dòng
#define WIFI_TX_POWER_SAFE          WIFI_POWER_15dBm // 15dBm giảm dòng đỉnh từ 480mA xuống ~220mA

// Status RGB LED D1 (Common Anode connected to +3.3V -> Active LOW)
#define PIN_LED_RED             7             // IO7  (Pin 7)  - Net LED_R via R15 (1k)
#define PIN_LED_GREEN           6             // IO6  (Pin 6)  - Net LED_G via R16 (470R)
#define PIN_LED_BLUE            8             // IO8  (Pin 12) - Net LED_B via R17 (470R)

// Piezo Buzzer BZ1
// Driven by Q1 2N7002 N-MOSFET via IO2 (Active HIGH, gate pull-down R5 10k)
#define PIN_BUZZER              2             // IO2  (Pin 38) - Net BUZZER_CTRL

// Battery Fuel Gauge IC U6 (MAX17048G_T10) via I2C Bus
#define PIN_I2C_SDA             4             // IO4  (Pin 4)  - Net I2C_SDA (Pull-up R9 4.7k)
#define PIN_I2C_SCL             5             // IO5  (Pin 5)  - Net I2C_SCL (Pull-up R3 4.7k)
#define MAX17048_I2C_ADDR       0x36          // 7-bit I2C Slave Address

// Native USB Type-C on ESP32-S3 (Protected by U2 USBLC6-2SC6)
// IO19 = USB_D- (USB_DN)
// IO20 = USB_D+ (USB_DP)

// =========================================================================
// GESTURE & TIMING CONSTANTS (Non-blocking & configurable from backend)
// =========================================================================
#define DEBOUNCE_MS                 50    // Debounce threshold
#define SHORT_PRESS_MAX_MS          800   // Short press (<800ms) -> Order
#define DOUBLE_PRESS_WINDOW_MS      1200  // Double press (<1200ms) -> Cancel
#define HOLD_5S_ACTIVATE_MS         5000  // Hold 5s -> Wake / Activate / Start Action
#define HOLD_10S_PROVISION_MS       10000 // Hold 10s -> WiFi Provisioning (BLE 5 + SoftAP)
#define HOLD_15S_FACTORY_RESET_MS   15000 // Hold 15s -> Factory Reset (NVS Clear)

#define WIFI_CONNECT_TIMEOUT_MS     5000  // Max 5s to connect Wi-Fi before sleep
#define WIFI_MAX_RETRIES            3     // Prevent battery drain loops
#define BLE_PROVISION_TIMEOUT_S     180   // 3 minutes BLE advertising timeout

// =========================================================================
// BLE 5 GATT SERVICE & CHARACTERISTICS UUIDs
// =========================================================================
#define BLE_SERVICE_UUID        "0000FFF0-0000-1000-8000-00805F9B34FB"
#define BLE_CHAR_INFO_UUID      "0000FFF1-0000-1000-8000-00805F9B34FB" // Read Device Info
#define BLE_CHAR_WIFI_UUID      "0000FFF2-0000-1000-8000-00805F9B34FB" // Write SSID + Pass
#define BLE_CHAR_STATUS_UUID    "0000FFF3-0000-1000-8000-00805F9B34FB" // Notify Status

// =========================================================================
// DEFAULT CLOUD BACKEND URL (Overrides from NVS)
// =========================================================================
#define DEFAULT_BACKEND_URL     "http://192.168.1.50:5000"

#endif // CONFIG_H
