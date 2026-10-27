#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// =========================================================================
// ⚡ TARGET HARDWARE BOARD SELECTION
// =========================================================================
#define BOARD_SMART_ORDER_KICAD_V1      1 // Mạch Smart Order Button KiCad V1 (3 nút/1 nút, LED RGB, Buzzer)
#define BOARD_CHATBOT_V2_ESP32S3        2 // Mạch CHATBOT V2 2025 (ESP32-S3, HMI_UART, TFT_SPI, Button, Pin 18650, MIC)

// ⚡ LỰA CHỌN BO MẠCH MẶC ĐỊNH:
#ifndef CURRENT_BOARD_TARGET
#define CURRENT_BOARD_TARGET            BOARD_CHATBOT_V2_ESP32S3
#endif

// =========================================================================
// HARDWARE VARIANT SELECTION (BẢN 1 NÚT vs BẢN 3 NÚT)
// =========================================================================
#define HARDWARE_VARIANT_SINGLE_BUTTON  1 // Bản 1 nút: Đa cử chỉ (Single/Double/Hold trên nút bấm)
#define HARDWARE_VARIANT_TRIPLE_BUTTON  3 // Bản 3 nút: Cấu hình tự do từng nút (Món hàng / Thanh toán / Dịch vụ)

#ifndef CURRENT_HARDWARE_VARIANT
#define CURRENT_HARDWARE_VARIANT        HARDWARE_VARIANT_SINGLE_BUTTON
#endif

// =========================================================================
// CONFIGURABLE BUTTON ACTIONS & MENU ITEM MAPPING (CẤU HÌNH TỪNG NÚT BẤM / MÓN HÀNG)
// =========================================================================
enum ButtonActionType {
  ACTION_ORDER_DEFAULT = 0,    // Đặt món tổng quát (ORDER)
  ACTION_ORDER_ITEM,           // Đặt 1 món hàng cụ thể (Ví dụ: Bia Heineken, Cà phê muối, Khăn lạnh...)
  ACTION_REQUEST_BILL,         // Yêu cầu thanh toán / Tính tiền (BILL)
  ACTION_CALL_STAFF,           // Gọi nhân viên phục vụ / Trợ giúp (CALL STAFF)
  ACTION_CANCEL_ORDER,         // Hủy đơn hàng gần nhất (CANCEL)
  ACTION_CUSTOM_EVENT          // Sự kiện tùy biến theo yêu cầu kinh doanh
};

struct ButtonConfig {
  ButtonActionType actionType;
  char itemId[24];             // Mã SKU/ID món hàng (VD: "ITEM_BEER_01", "ITEM_COFFEE_02")
  char itemName[48];           // Tên món hàng hiển thị (VD: "Bia Heineken", "Cà phê muối")
  uint8_t quantity;            // Số lượng đặt mặc định (1, 2...)
  uint8_t ledColorIndex;       // Màu LED chỉ định phản hồi
};

// Cấu hình mặc định cho các nút bấm
#define DEFAULT_BTN1_ACTION_TYPE        ACTION_ORDER_ITEM
#define DEFAULT_BTN1_ITEM_ID            "ITEM_BEER_01"
#define DEFAULT_BTN1_ITEM_NAME          "Bia Heineken"
#define DEFAULT_BTN1_QUANTITY           1

#define DEFAULT_BTN2_ACTION_TYPE        ACTION_REQUEST_BILL
#define DEFAULT_BTN2_ITEM_ID            "SVC_BILL_01"
#define DEFAULT_BTN2_ITEM_NAME          "Yêu cầu Thanh toán"
#define DEFAULT_BTN2_QUANTITY           1

#define DEFAULT_BTN3_ACTION_TYPE        ACTION_CANCEL_ORDER
#define DEFAULT_BTN3_ITEM_ID            "SVC_CANCEL_01"
#define DEFAULT_BTN3_ITEM_NAME          "Hủy Đơn Hàng"
#define DEFAULT_BTN3_QUANTITY           1

// =========================================================================
// HARDWARE PIN DEFINITIONS (ESP32-S3-WROOM-1)
// =========================================================================

#if CURRENT_BOARD_TARGET == BOARD_CHATBOT_V2_ESP32S3
// =========================================================================
// 🤖 1. CẤU HÌNH BO MẠCH: CHATBOT V2_2025 (ESP32-S3)
// =========================================================================

// Nút Bấm Người Dùng (Ô đỏ 'button' trên đỉnh module ESP32-S3):
// Mặc định hỗ trợ chân IO0 (BOOT button) hoặc IO39
#define PIN_BUTTON_SW1                  GPIO_NUM_0    // Chân nút bấm chính (Active LOW, PULLUP)
#define PIN_BUTTON_SW2                  GPIO_NUM_40   // Nút mở rộng nếu có
#define PIN_BUTTON_SW3                  GPIO_NUM_41   // Nút mở rộng nếu có
#define PIN_BUTTON                      PIN_BUTTON_SW1
#define USE_HARDWARE_MOD_RTC_PIN        0

// Cổng HMI_UART (Jack 4-pin trắng trên bên trái: 5V, TX, RX, GND)
#define ENABLE_HMI_UART                 1
#define HMI_UART_PORT_NUM               1             // Hardware Serial1
#define HMI_UART_TX_PIN                 17            // IO17 TX -> RX màn hình HMI
#define HMI_UART_RX_PIN                 18            // IO18 RX <- TX màn hình HMI
#define HMI_UART_BAUD                   115200        // Tốc độ truyền thông HMI Nextion/DWIN/Serial

// Cổng TFT_SPI (Jack 8-pin trắng góc dưới bên phải: VCC, GND, CS, DC, RST, MOSI, SCK, BLK)
// ⚠️ Chú ý: Cắm pin vào B- B+ trước rồi mới bật công tắc cấp nguồn TFT SPI để bảo vệ tải!
#define ENABLE_TFT_SPI                  1
#define PIN_TFT_SCK                     12            // SPI SCK
#define PIN_TFT_MOSI                    11            // SPI MOSI
#define PIN_TFT_MISO                    13            // SPI MISO
#define PIN_TFT_CS                      10            // SPI CS
#define PIN_TFT_DC                      9             // SPI Data/Command
#define PIN_TFT_RST                     14            // SPI Reset
#define PIN_TFT_BLK                     48            // Backlight control (IO48)

// Cổng I2C2 (Header góc trên bên phải, có trở kéo R8, R9 4.7kΩ)
#define PIN_I2C_SDA                     4             // IO4 SDA
#define PIN_I2C_SCL                     5             // IO5 SCL
#define MAX17048_I2C_ADDR               0x36

// Cổng Nguồn Pin 18650 / LiPo (B- B+ 3.2V - 4.2VDC) & Cổng sạc 5VDC
#define PIN_BATTERY_ADC                 1             // IO1 (ADC1_CH0) đo qua phân áp điện trở
#define BATTERY_MIN_MV                  3200          // 3.2V: Cạn pin (0%)
#define BATTERY_MAX_MV                  4200          // 4.2V: Đầy pin (100%)
#define BATTERY_ADC_DIVIDER_RATIO       2.0f          // Cầu phân áp 1:2 (2x 100kΩ)

// Cổng MIC (Jack 3/4-pin trắng góc trên bên trái)
#define PIN_MIC_WS                      15            // I2S Word Select
#define PIN_MIC_SCK                     16            // I2S Clock
#define PIN_MIC_SD                      7             // I2S Serial Data

// LED & Buzzer ảo/ngoại vi tương thích hệ thống
#define PIN_LED_RED                     7
#define PIN_LED_GREEN                   6
#define PIN_LED_BLUE                    8
#define PIN_BUZZER                      2
#define BUZZER_FREQ_DEFAULT             2700

#else
// =========================================================================
// 📐 2. CẤU HÌNH BO MẠCH: SMART ORDER BUTTON KICAD V1
// =========================================================================

#define USE_HARDWARE_MOD_RTC_PIN        0
#if USE_HARDWARE_MOD_RTC_PIN
#define PIN_BUTTON_SW1                  GPIO_NUM_10   // IO10 (RTC GPIO Wakeup Deep Sleep <15uA)
#else
#define PIN_BUTTON_SW1                  GPIO_NUM_39   // IO39 (HP Domain Light Sleep)
#endif
#define PIN_BUTTON_SW2                  GPIO_NUM_40   // IO40
#define PIN_BUTTON_SW3                  GPIO_NUM_41   // IO41
#define PIN_BUTTON                      PIN_BUTTON_SW1

#define ENABLE_HMI_UART                 0
#define ENABLE_TFT_SPI                  0

#define PIN_LED_RED                     7             // IO7
#define PIN_LED_GREEN                   6             // IO6
#define PIN_LED_BLUE                    8             // IO8
#define PIN_BUZZER                      2             // IO2
#define BUZZER_FREQ_DEFAULT             2700

#define PIN_I2C_SDA                     4             // IO4
#define PIN_I2C_SCL                     5             // IO5
#define MAX17048_I2C_ADDR               0x36

#define PIN_BATTERY_ADC                 1
#define BATTERY_MIN_MV                  3200
#define BATTERY_MAX_MV                  4200
#define BATTERY_ADC_DIVIDER_RATIO       2.0f

#endif

// =========================================================================
// CẤU HÌNH NĂNG LƯỢNG TIẾT KIỆM CHO ESP32-S3
// =========================================================================
#define ESP32_CPU_FREQ_MHZ              80            // 80MHz tiết kiệm điện năng và giảm xung dòng đỉnh
#define WIFI_TX_POWER_SAFE              WIFI_POWER_15dBm // 15dBm giảm dòng đỉnh từ 480mA xuống ~220mA

// =========================================================================
// GESTURE & TIMING CONSTANTS (Non-blocking & configurable from backend)
// =========================================================================
#define DEBOUNCE_MS                     30    // Ngưỡng lọc rung phím (debounce)
#define SHORT_PRESS_MAX_MS              800   // Nhấn nhanh (<800ms) -> Tạo đơn hàng (Order)
#define DOUBLE_PRESS_WINDOW_MS          500   // Khoảng cách tối đa giữa 2 lần nhấn đúp
#define HOLD_5S_PROVISION_MS            5000  // Nhấn giữ 5s nút BOOT / SW1 -> Mở địa chỉ Web & Bluetooth (BLE) cấu hình Wi-Fi
#define HOLD_5S_ACTIVATE_MS             5000  // Tương thích ngược (5s -> Cấu hình Wi-Fi & BLE)
#define HOLD_10S_PROVISION_MS           5000  // Giữ 5s vào ngay chế độ cấu hình Wi-Fi (BLE 5 + Web Captive Portal)
#define HOLD_15S_FACTORY_RESET_MS       15000 // Nhấn giữ 15s -> Xóa sạch bộ nhớ NVS Flash

#define WIFI_CONNECT_TIMEOUT_MS         6000  // Thời gian tối đa kết nối Wi-Fi trước khi ngủ
#define WIFI_MAX_RETRIES                3     // Số lần thử kết nối tránh treo cạn kiệt pin
#define BLE_PROVISION_TIMEOUT_S         180   // 3 phút phát BLE quảng bá cấu hình

// =========================================================================
// BLE 5 GATT SERVICE & CHARACTERISTICS UUIDs
// =========================================================================
#define BLE_SERVICE_UUID                "0000FFF0-0000-1000-8000-00805F9B34FB"
#define BLE_CHAR_INFO_UUID              "0000FFF1-0000-1000-8000-00805F9B34FB" // Read Device Info
#define BLE_CHAR_WIFI_UUID              "0000FFF2-0000-1000-8000-00805F9B34FB" // Write SSID + Pass
#define BLE_CHAR_STATUS_UUID            "0000FFF3-0000-1000-8000-00805F9B34FB" // Notify Status
#define BLE_CHAR_LED_UUID               "0000FFF4-0000-1000-8000-00805F9B34FB" // Write LED Color / Mode

// =========================================================================
// DEFAULT CLOUD BACKEND URL (Được ghi đè từ NVS Flash)
// =========================================================================
#define DEFAULT_BACKEND_URL             "http://192.168.1.50:5000"

#endif // CONFIG_H
