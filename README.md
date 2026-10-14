# ⚡ Smart Order Button — Tài Liệu Phần Cứng & Firmware ESP32-S3

Tài liệu kỹ thuật và mã nguồn firmware nhúng cho bo mạch nút bấm thông minh **Smart Order Button IoT**, thiết kế chuẩn xác 100% theo bản vẽ nguyên lý KiCad **`Smart_Order_Button.kicad_sch`** và layout PCB trên vi điều khiển **ESP32-S3-WROOM-1**, kết nối trực tiếp với hệ sinh thái **Backend NestJS Enterprise**.

---

## 📌 1. Bảng Tra Cứu Chân Phần Cứng (Hardware Pinout Mapping)

| Chân ESP32-S3 | Nhãn Net KiCad | Linh Kiện & Chức Năng | Nguyên Lý Hoạt Động |
| :--- | :--- | :--- | :--- |
| **IO39** (Pin 32) | `BTN_ORDER` | Nút bấm vật lý **SW1** | Nối giữa IO39 và GND. Cấu hình `INPUT_PULLUP`. Khi nhấn kéo xuống GND (**Active LOW**). Hỗ trợ đánh thức tức thì từ Light Sleep (<50µs) hoặc Deep Sleep (nếu câu sang RTC IO10). |
| **IO7** (Pin 7) | `LED_R` | Cực Catot LED Đỏ (qua trở R15 1kΩ) | Đèn LED RGB D1 là loại **Common Anode** nối nguồn +3.3V. Kéo xuống mức thấp để bật đèn (**Active LOW**). |
| **IO6** (Pin 6) | `LED_G` | Cực Catot LED Xanh Lá (qua trở R16 470Ω) | Kéo xuống mức thấp để bật đèn (**Active LOW**). |
| **IO8** (Pin 12) | `LED_B` | Cực Catot LED Xanh Dương (qua trở R17 470Ω) | Kéo xuống mức thấp để bật đèn (**Active LOW**). |
| **IO2** (Pin 38) | `BUZZER_CTRL`| Gate MOSFET **Q1 (2N7002)** kích còi **BZ1** | Kéo lên mức cao để kích hoạt MOSFET cấp nguồn cho còi phát âm thanh (**Active HIGH**). Có trở xả Gate R6 10kΩ xuống GND và tụ dập C10 100nF song song còi. |
| **IO4** (Pin 4) | `I2C_SDA` | Chân dữ liệu I2C IC đo pin **MAX17048 (U6)** | Bus I2C chuẩn, có điện trở treo R9 4.7kΩ lên nguồn +3.3V. |
| **IO5** (Pin 5) | `I2C_SCL` | Chân xung nhịp I2C IC đo pin **MAX17048 (U6)** | Bus I2C chuẩn, có điện trở treo R3 4.7kΩ lên nguồn +3.3V. |
| **IO20** (Pin 14) | `USB_D+` | Cổng Type-C Native qua IC chống tĩnh điện **U2 (USBLC6-2SC6)** | Nạp code firmware & Serial Monitor trực tiếp tốc độ cao qua USB CDC Native. |
| **IO19** (Pin 13) | `USB_D-` | Cổng Type-C Native qua IC chống tĩnh điện **U2 (USBLC6-2SC6)** | Truyền nhận dữ liệu USB. |

---

## 🔋 2. Quản Lý Nguồn & Đo Pin (Power Architecture)

1. **IC Sạc Li-Po**: `MCP73831T-2ACI_OT` (U3) nhận nguồn 5V từ cổng Type-C VBUS, sạc pin qua chân PROG (R11 2kΩ -> dòng sạc danh định ~500mA).
2. **IC Bảo Vệ Pin**: `DW01A` (U5) kết hợp Dual N-MOSFET `8205A` (Q2A/Q2B) bảo vệ pin chống quá sạc (Overcharge > 4.3V), chống quá xả (Overdischarge < 2.4V) và ngắn mạch.
3. **Hạ Áp LDO**: `ME6211C33M5G` (U4) chuyển điện áp pin Li-Po (3.3V - 4.2V) thành điện áp ổn định +3.3V với dòng tải lên tới 500mA cấp cho ESP32-S3. Firmware tự động giới hạn xung nhịp CPU 80MHz và công suất phát Wi-Fi 15dBm để triệt tiêu sụt áp (Brownout).
4. **IC Đo Dung Lượng Pin**: `MAX17048G_T10` (U6) đo điện áp tế bào pin (VCELL) và tính toán dung lượng State of Charge (SOC%) chính xác theo thuật toán ModelGauge, đọc dữ liệu qua I2C (địa chỉ 0x36).

---

## 🎮 3. Quy Ước Cử Chỉ & Phản Hồi LED / Còi Buzzer

| Cử Chỉ Nút Bấm SW1 | Màu Đèn LED RGB D1 | Âm Thanh Còi BZ1 | Ý Nghĩa Hệ Thống |
| :--- | :--- | :--- | :--- |
| **Khởi Động Nguồn** | 💎 Cyan (Xanh lơ) | 1 click nhẹ (40ms) | Khởi động ESP32-S3, đọc IC MAX17048 |
| **Nhấn 1 Lần (<800ms)** | 🔵 Xanh dương -> 🟢 Xanh lá | 2 bíp cao vui tai (Success Chime) | Đọc pin MAX17048, ký số HMAC và **TẠO ĐƠN HÀNG** |
| **Nhấn Đúp (<1.2s)** | 🟡 Vàng -> 🔴 Đỏ | 1 tiếng bíp dài (Cancel Tone 320ms) | Gửi `eventType: CANCEL` để **HỦY ĐƠN HÀNG** |
| **Nhấn 5 Lần / Giữ 10s** | 🔵 Nhấp nháy Xanh dương | 1 bíp dài (120ms) | Bật chế độ cấu hình Wi-Fi BLE 5 & SoftAP |
| **Nhấn Giữ 5 Giây** | 🟣 Tím (Purple) | 1 click nhẹ | Đồng bộ thông số cấu hình thiết bị (Sync Config) |
| **Nhấn Giữ 15 Giây** | 🔴 Đỏ (Red) | 3 bíp cảnh báo khẩn cấp | Khôi phục cài đặt gốc (Xóa trắng NVS Wi-Fi) |
| **Lỗi Mạng / Máy Chủ** | 🔴 Đỏ (Red) | 3 bíp gấp gáp | Cảnh báo mất kết nối hoặc Backend từ chối |

---

## 🚀 4. Chạy Thử Mạch Thực Nghiệm (Test Bench)

### Cách 1: File HTML Độc Lập (Chạy Ngay Trong Trình Duyệt)
- Mở trực tiếp file: [`test_bench/index.html`](file:///c:/Users/ADMIN/OneDrive/Desktop/Phancung/hardware/test_bench/index.html) bằng Chrome hoặc Edge.
- Trải nghiệm giao diện mô phỏng board mạch trực quan, bấm nút SW1 ảo, nghe còi thật qua Web Audio API, xem LED RGB chuyển màu và kiểm tra chữ ký HMAC-SHA256 sinh ra.

### Cách 2: Chạy Script Kiểm Thử Tự Động Bằng Node.js
Chạy câu lệnh sau trong terminal:
```bash
node test_bench/run_test.js
```
Kết quả hiển thị toàn bộ 10/10 bước kiểm thử đạt chuẩn (100% Pass) từ đo pin I2C, kích còi MOSFET, chuyển màu LED đến phòng thủ Replay Attack!
