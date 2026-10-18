# Stack Pack: ESP32-S3 Firmware (C++ / Arduino / FreeRTOS)

> Nạp khi surface firmware sử dụng ESP32-S3.

## 1. Môi trường & Build Toolchain
- **Vi điều khiển**: ESP32-S3-WROOM-1 (Dual-core Xtensa LX7 @ 240MHz, 16MB Flash, 8MB PSRAM).
- **Framework**: Arduino-ESP32 Core v2.x / v3.x hoặc ESP-IDF v5.x.
- **Tổ chức thư mục**:
  - `hardware/include/`: Chứa các file định nghĩa header chung (`config.h`).
  - `hardware/src/`: Mã nguồn module C++ hướng đối tượng (`core/`, `button/`, `led/`, `buzzer/`, `power/`, `ble/`).
  - `hardware/SmartOrderButton/`: Arduino IDE all-in-one bundle tương thích nạp trực tiếp.

## 2. Tiêu chuẩn Mã nguồn (Code Standards)
- **Header Guards**: Tất cả các tệp `.h` bắt buộc có `#ifndef FOO_H` / `#define FOO_H` / `#endif` hoặc `#pragma once`.
- **Phân tách phần cứng qua Macro Config**: Không hardcode số chân GPIO trong logic nghiệp vụ. Luôn dùng các macro định nghĩa trong `config.h` (ví dụ: `BUTTON_ORDER_PIN`, `BUTTON_BILL_PIN`, `BUTTON_CANCEL_PIN`, `LED_R_PIN`, v.v.).
- **Hỗ trợ Đa biến thể (Hardware Variants)**:
  - `HARDWARE_VARIANT_SINGLE_BUTTON (1)`: 1 nút cơ khí duy nhất giải mã 3 hành vi (Nhấn đơn, Nhấn đúp, Nhấn giữ).
  - `HARDWARE_VARIANT_TRIPLE_BUTTON (3)`: 3 nút riêng biệt cho Đặt món, Tính tiền, Hủy đơn.

## 3. Quản lý Năng lượng & Luồng (Power & Concurrency)
- Dùng FreeRTOS Mutex hoặc Semaphore khi tài nguyên dùng chung giữa nhiều Task hoặc ISR.
- Cấu hình chân GPIO Wakeup cho Light Sleep / Deep Sleep:
  ```cpp
  gpio_wakeup_enable((gpio_num_t)BUTTON_ORDER_PIN, GPIO_INTR_LOW_LEVEL);
  esp_sleep_enable_gpio_wakeup();
  ```
- Dòng điện mục tiêu:
  - Active (WiFi/BLE TX): < 95mA
  - Light Sleep: < 2.5mA
  - Deep Sleep: < 15µA

## 4. Kiểm thử & Xác minh (Verification)
- Kiểm tra biên dịch không có lỗi và không có cảnh báo kiểu dữ liệu.
- Chạy kiểm thử mô phỏng logic qua Test Bench (`node hardware/test_bench/run_test.js`).
