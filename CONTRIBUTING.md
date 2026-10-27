# 🤝 Hướng Dẫn Đóng Góp & Đóng Gói Dự Án (Contributor & Packaging Guide)

Chào mừng bạn đến với dự án phần cứng **Smart Order Button IoT (ESP32-S3)**. Tài liệu này hướng dẫn các nhà phát triển và cộng tác viên (Contributors) cách thiết lập môi trường, đóng gói dự án, tuân thủ các cổng kiểm thử chất lượng phần cứng và quy trình gửi Pull Request (PR).

---

## 📦 1. Cấu Trúc Dự Án & Đóng Gói (Package Structure)

Dự án được tổ chức theo cấu trúc mô-đun hóa cao độ:

```text
hardware/
├── SmartOrderButton/         # Firmware chính nạp qua Arduino IDE (SmartOrderButton.ino)
├── include/                  # Cấu hình phần cứng toàn cục (config.h)
├── src/                      # Mô-đun mã nguồn C++ FreeRTOS / Arduino
│   ├── api/                  # Giao tiếp HTTP/REST Client & HMAC-SHA256
│   ├── ble/                  # BLE GATT Server, Wi-Fi Provisioning & Live Notify
│   ├── button/               # Xử lý cử chỉ nút (1 Nút đa cử chỉ & 3 Nút độc lập)
│   ├── buzzer/               # Điều khiển còi Piezo resonant 2.7kHz qua MOSFET Q1
│   ├── core/                 # StateMachine điều khiển trạng thái hệ thống
│   ├── display/              # Trình điều khiển màn hình TFT SPI ST7789
│   ├── hmi/                  # Giao tiếp UART HMI màn hình phụ
│   ├── led/                  # Điều khiển LED RGB Common Anode (Active LOW)
│   ├── power/                # Quản lý sạc MCP73831 & I2C Fuel Gauge MAX17048
│   └── wifi/                 # Quản lý kết nối Wi-Fi & NVS Preferences
├── test_bench/               # Phòng lab mô phỏng mạch 3D & kiểm thử tự động
│   ├── index.html            # Lab mô phỏng mạch điện & PCB Canvas KiCad
│   ├── fe_bluetooth_1nut.html# Giao diện Web Bluetooth bắt mạch thật & cấu hình Wi-Fi
│   └── run_test.js           # Bộ test tự động 16 bước Node.js (Quality Gate)
├── Smart_Order_Button.kicad_*# Bản vẽ nguyên lý & layout mạch in KiCad 8.x
├── CONTRIBUTING.md           # Hướng dẫn đóng góp cho Contributor
└── README.md                 # Tài liệu kỹ thuật phần cứng & hướng dẫn sử dụng
```

---

## 🛠️ 2. Hướng Dẫn Thiết Lập Cho Contributor

### Yêu Cầu Môi Trường
- **Arduino IDE 2.x** hoặc **PlatformIO (VS Code)**.
- **ESP32 Board Package**: phiên bản `>= 2.0.14` hoặc `3.x` (hỗ trợ ESP32-S3).
- **Node.js**: phiên bản `>= 18.x` (để chạy bộ kiểm thử tự động `run_test.js`).
- **Trình duyệt**: Google Chrome hoặc Microsoft Edge (hỗ trợ Web Bluetooth API).

### Cấu Hình Bo Mạch Trong Arduino IDE
- **Board**: `ESP32S3 Dev Module`
- **USB CDC On Boot**: `Enabled`
- **Flash Size**: `8MB` (hoặc `4MB`)
- **Partition Scheme**: `Default 4MB with spiffs (1.2MB APP/1.5MB SPIFFS)` hoặc `Huge APP (3MB No OTA)`
- **CPU Frequency**: `80MHz` (khuyến nghị cho dòng tải LDO ME6211)
- **Upload Mode**: `UART0 / Hardware CDC`

---

## 🧪 3. Cổng Kiểm Thử Bắt Buộc Trước Khi Commit (Quality Gate)

Mọi thay đổi liên quan đến firmware, cấu hình chân GPIO hoặc logic nút bấm **bắt buộc phải vượt qua 100% các bước kiểm thử tự động**:

```bash
# Di chuyển vào thư mục test bench và chạy kiểm thử
node test_bench/run_test.js
```

Yêu cầu đầu ra:
```text
================================================================
📊 KẾT QUẢ KIỂM THỬ MẠCH: 16/16 BƯỚC ĐẠT CHUẨN (100%)
================================================================
```

---

## ⚙️ 4. Quy Chuẩn Phần Cứng & Biến Thể (Hardware Variant Rules)

Dự án hỗ trợ 2 biến thể phần cứng thông qua macro trong `include/config.h`:
1. **Bản 1 Nút (`HARDWARE_VARIANT_SINGLE_BUTTON`)**:
   - Chân đọc: `PIN_BUTTON_SW1` (IO39) và nút BOOT (IO0).
   - Nhấn 1 lần: Đặt hàng (`ORDER`).
   - Nhấn 2 lần: Hủy đơn (`CANCEL_ORDER`).
   - Giữ 5 giây: Phát quảng bá Bluetooth (`SOB-BTN-8829-WTR`) để kết nối với Web Bluetooth FE.
2. **Bản 3 Nút (`HARDWARE_VARIANT_TRIPLE_BUTTON`)**:
   - SW1 (IO39): Đặt hàng (`ORDER`).
   - SW2 (IO40): Gọi thanh toán (`REQUEST_BILL`) / Gọi phục vụ (`CALL_STAFF`).
   - SW3 (IO41): Hủy đơn (`CANCEL_ORDER`).

> [!IMPORTANT]
> Tuyệt đối không thay đổi sơ đồ chân IO đã được định nghĩa chuẩn theo KiCad schematic (`Smart_Order_Button.kicad_sch`), đặc biệt là chân sạc pin, đường I2C đo pin MAX17048 (IO4/IO5) và các chân LED Active LOW.

---

## 🚀 5. Quy Trình Gửi Đóng Góp (Pull Request Workflow)

1. **Fork** repository về tài khoản cá nhân.
2. Tạo nhánh tính năng mới theo quy ước:
   ```bash
   git checkout -b feat/ten-tinh-nang-moi
   # hoặc fix/ten-loi-can-sua
   ```
3. Tuân thủ quy ước Commit Message (Conventional Commits):
   - `feat(...)`: Thêm tính năng mới (BLE, HMI, cảm biến, v.v.)
   - `fix(...)`: Sửa lỗi phần cứng, chống dội phím, rò rỉ bộ nhớ
   - `docs(...)`: Cập nhật tài liệu kỹ thuật, sơ đồ chân
   - `refactor(...)`: Tối ưu mã nguồn, giảm dung lượng bộ nhớ Flash
4. Chạy kiểm thử xác thực:
   ```bash
   node test_bench/run_test.js
   ```
5. Push nhánh lên GitHub và tạo **Pull Request (PR)** kèm mô tả chi tiết thay đổi và ảnh chụp kiểm thử thành công.

---

## 👥 6. Danh Sách Đóng Góp & Tác Quyền (Contributors & Credits)

- **Smart Order Button IoT Engineering Team**
- **Hardware Architecture & Firmware**: ESP32-S3 IoT Core Team
- **Harness Framework Integration**: HEF v4.0.0
