# Domain Pack: IoT Hardware & Embedded Firmware

> Nạp thêm khi `project.domain: "iot-hardware"`. Áp dụng kết hợp với `compliance/COMPLIANCE_FRAMEWORK.md`.

## 1. Bối cảnh
Thiết bị phần cứng IoT thông minh chạy vi điều khiển ESP32-S3, tích hợp mạch nguồn pin LiPo (sạc TP4065), quản lý chế độ tiết kiệm năng lượng (Light Sleep / Deep Sleep), giao tiếp RF (Wi-Fi 2.4GHz 802.11 b/g/n, BLE 5.0 Mesh/Provisioning), MQTT client và ngoại vi người dùng (Nút bấm cơ học, LED RGB, Còi Buzzer).

## 2. Nguyên tắc an toàn điện & phần cứng (Hardware Safety)
- **Bảo vệ ESD**: Tất cả các đường tín hiệu nút bấm (SW1, SW2, SW3) và đầu nối tiếp xúc tay người dùng phải có mạch bảo vệ chống phóng tĩnh điện hoặc điện trở kéo / tụ lọc chống nảy (debounce RC).
- **Quản lý nguồn sạc pin**: Mạch sạc TP4065 phải có giới hạn dòng sạc phù hợp dung lượng pin LiPo (mặc định R_PROG đặt dòng sạc an toàn <= 0.5C). Không bao giờ vượt quá ngưỡng điện áp sạc 4.2V.
- **Phân tách nguồn & tiếp địa (Ground Planes)**: Đường công suất RF và nguồn xung LDO phải có ground plane vững chắc, via tản nhiệt (thermal vias) và tụ bypass 100nF đặt sát chân VDD của ESP32-S3.

## 3. Nguyên tắc an toàn Firmware & Chống Treo (Firmware Reliability)
- **Watchdog Timer (WDT)**: Luôn kích hoạt Task Watchdog Timer (TWDT) trên FreeRTOS core. Bất kỳ vòng lặp xử lý mạng hoặc chờ ngoại vi nào đều phải yield (`vTaskDelay` hoặc `delay`) để tránh kích hoạt Panic WDT.
- **Quản lý trạng thái NVS & Re-entrant**: Không ghi flash NVS liên tục trong vòng lặp chính. Chỉ lưu cấu hình khi có thay đổi (WiFi credentials, hardware variant mode, order sequence).
- **Wakeup an toàn sau Sleep**: Sau khi thức dậy từ Light Sleep hoặc Deep Sleep qua GPIO Interrupt (IO39, IO40, IO41), phải thực hiện lọc chống rung (debounce >= 50ms) trước khi kích hoạt State Machine để loại bỏ xung nhiễu sườn điện áp.

## 4. Bảo mật truyền thông (Security & Privacy)
- **BLE Provisioning an toàn**: Không truyền WiFi SSID / Password ở dạng Cleartext qua BLE mà không có handshake mã hóa hoặc cơ chế Session Key (Curve25519 / AES-GCM theo chuẩn ESP-IDF BLE Prov).
- **MQTT TLS**: Khuyến nghị dùng MQTTS (Port 8883) với chứng chỉ x509 hoặc xác thực Token khi xuất xưởng ra môi trường production.

## 5. Cổng kiểm tra chất lượng tự động (CI / Automated Gates)
- Bộ kiểm thử tự động Test Bench (`node hardware/test_bench/run_test.js`) phải vượt qua 100% kịch bản (14/14 test cases) trước khi commit.
- Mọi macro GPIO trong `config.h` phải ánh xạ chính xác với schematic KiCad (`Smart_Order_Button.kicad_sch`).
