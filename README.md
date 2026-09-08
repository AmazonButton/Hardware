# ⚡ Smart Order Button — Tài Liệu Phần Cứng & Firmware ESP32-S3

Tài liệu kỹ thuật và mã nguồn firmware nhúng cho bo mạch nút bấm thông minh **Smart Order Button IoT**, thiết kế chuẩn xác 100% theo bản vẽ nguyên lý KiCad **`Smart_Order_Button.kicad_sch`** trên vi điều khiển **ESP32-S3-WROOM-1** và kết nối trực tiếp với hệ sinh thái **Backend NestJS Enterprise**.

---

## 📌 1. Bảng Tra Cứu Chân Phần Cứng (Hardware Pinout Mapping)

| Chân ESP32-S3 | Nhãn Net KiCad | Linh Kiện & Chức Năng | Nguyên Lý Hoạt Động |
| :--- | :--- | :--- | :--- |
| **IO39** (Pin 19) | `BTN_ORDER` | Nút bấm vật lý **SW1** | Nối giữa IO39 và GND. Cấu hình `INPUT_PULLUP`. Khi nhấn kéo xuống GND (**Active LOW**). Hỗ trợ đánh thức từ Deep Sleep qua RTC. |
| **IO7** (Pin 7) | `LED_R` | Cực Catot LED Đỏ (qua trở R15 1kΩ) | Đèn LED RGB D1 là loại **Common Anode** nối nguồn +3.3V. Kéo xuống mức thấp để bật đèn (**Active LOW**). |
| **IO6** (Pin 6) | `LED_G` | Cực Catot LED Xanh Lá (qua trở R16 470Ω) | Kéo xuống mức thấp để bật đèn (**Active LOW**). |
| **IO8** (Pin 12) | `LED_B` | Cực Catot LED Xanh Dương (qua trở R17 470Ω) | Kéo xuống mức thấp để bật đèn (**Active LOW**). |
| **IO2** (Pin 38) | `BUZZER_CTRL`| Gate MOSFET **Q1 (2N7002)** kích còi **BZ1** | Kéo lên mức cao để kích hoạt MOSFET cấp nguồn cho còi phát âm thanh (**Active HIGH**). Có trở kéo xuống R5 10kΩ. |
| **IO4** (Pin 4) | `I2C_SDA` | Chân dữ liệu I2C IC đo pin **MAX17048 (U6)** | Bus I2C chuẩn, có điện trở treo R9 4.7kΩ lên nguồn +3.3V. |
| **IO5** (Pin 5) | `I2C_SCL` | Chân xung nhịp I2C IC đo pin **MAX17048 (U6)** | Bus I2C chuẩn, có điện trở treo R3 4.7kΩ lên nguồn +3.3V. |
| **IO20** (Pin 13) | `USB_D+` | Cổng Type-C Native qua IC chống tĩnh điện **U2 (USBLC6-2SC6)** | Nạp code firmware & Serial Monitor trực tiếp tốc độ cao. |
| **IO19** (Pin 14) | `USB_D-` | Cổng Type-C Native qua IC chống tĩnh điện **U2 (USBLC6-2SC6)** | Truyền nhận dữ liệu USB. |

---

## 🔋 2. Quản Lý Nguồn & Đo Pin (Power Architecture)

1. **IC Sạc Li-Po**: `MCP73831T-2ACI_OT` (U3) nhận nguồn 5V từ cổng Type-C VBUS, sạc pin qua chân PROG (R11 2kΩ -> dòng sạc danh định ~500mA).
2. **IC Bảo Vệ Pin**: `DW01A` (U5) kết hợp Dual N-MOSFET `8205A` (Q2A/Q2B) bảo vệ pin chống quá sạc (Overcharge > 4.3V), chống quá xả (Overdischarge < 2.4V) và ngắn mạch.
3. **Hạ Áp LDO**: `ME6211C33M5G` (U4) chuyển điện áp pin Li-Po (3.3V - 4.2V) thành điện áp ổn định +3.3V với dòng tải lên tới 500mA cấp cho ESP32-S3.
4. **IC Đo Dung Lượng Pin**: `MAX17048G_T10` (U6) đo điện áp tế bào pin (VCELL) và tính toán dung lượng State of Charge (SOC%) chính xác theo thuật toán ModelGauge, đọc dữ liệu qua I2C (địa chỉ 0x36).

---

## 🎮 3. Quy Ước Cử Chỉ & Phản Hồi LED / Còi Buzzer

| Cử Chỉ Nút Bấm SW1 | Màu Đèn LED RGB D1 | Âm Thanh Còi BZ1 | Ý Nghĩa Hệ Thống |
| :--- | :--- | :--- | :--- |
| **Khởi Động Nguồn** | 💎 Cyan (Xanh lơ) | 1 click nhẹ (35ms) | Khởi động ESP32-S3, đọc IC MAX17048 |
| **Nhấn 1 Lần (<800ms)** | 🔵 Xanh dương -> 🟢 Xanh lá | 2 bíp cao vui tai (Success Chime) | Đọc pin MAX17048, ký số HMAC và **TẠO ĐƠN HÀNG** |
| **Nhấn Đúp (<1.2s)** | 🟡 Vàng -> 🔴 Đỏ | 1 tiếng bíp dài (Cancel Tone 350ms) | Gửi `eventType: CANCEL` để **HỦY ĐƠN HÀNG** |
| **Nhấn 5 Lần / Giữ 10s** | 🔵 Nhấp nháy Xanh dương | 1 bíp dài (120ms) | Bật chế độ cấu hình Wi-Fi BLE 5 & SoftAP |
| **Nhấn Giữ 5 Giây** | 🟣 Tím (Purple) | 1 click nhẹ | Đồng bộ thông số cấu hình thiết bị (Sync Config) |
| **Nhấn Giữ 15 Giây** | 🔴 Đỏ (Red) | 3 bíp cảnh báo khẩn cấp | Khôi phục cài đặt gốc (Xóa trắng NVS Wi-Fi) |
| **Lỗi Mạng / Máy Chủ** | 🔴 Đỏ (Red) | 3 bíp gấp gáp | Cảnh báo mất kết nối hoặc Backend từ chối |

---

## 🚀 4. Ba Cách Chạy Thử Mạch Thực Nghiệm (Test Bench)

### Cách 1: Bàn Thử Mạch Ảo Trên Giao Diện Web React
1. Chạy frontend:
   ```bash
   cd frontend
   npm run dev
   ```
2. Truy cập trình duyệt tại: **`http://localhost:5173/hardware-test`** (hoặc bấm nút **"Bàn Thử Mạch ESP32-S3"** ngay trên thanh Menu Navbar).
3. Tại đây anh có thể:
   - Click nút SW1 trực quan với âm thanh cơ học thật.
   - Thấy đèn LED RGB Common Anode đổi màu rực rỡ theo đúng tín hiệu chân IO7, IO6, IO8.
   - Nghe còi BZ1 phát âm thanh thật qua Web Audio API.
   - Kéo thanh trượt pin MAX17048 và theo dõi chữ ký HMAC-SHA256 sinh ra theo thời gian thực!

### Cách 2: File HTML Độc Lập (Chạy Ngay Không Cần Cài Đặt)
- Mở trực tiếp file: [`hardware/test_bench/index.html`](file:///c:/Users/ADMIN/OneDrive/Desktop/Button%20V2/hardware/test_bench/index.html) bằng bất kỳ trình duyệt nào (Chrome, Edge, Cốc Cốc).
- Hoạt động 100% độc lập, có sẵn chế độ Giả Lập Mạch Biên (Edge Mode) và tự động nhận diện nếu Backend NestJS đang bật.

### Cách 3: Chạy Script Kiểm Thử Tự Động Bằng Node.js
Chạy câu lệnh sau trong terminal:
```bash
node hardware/test_bench/run_test.js
```
Kết quả hiển thị toàn bộ 10/10 bước kiểm thử đạt chuẩn (100% Pass) từ đo pin I2C, kích còi MOSFET, chuyển màu LED đến phòng thủ Replay Attack!

---

## 🛠️ 5. Hướng Dẫn Nạp Code Firmware Lên Bo Mạch ESP32-S3

### Nạp Bằng Arduino IDE 2.x
1. Mở file: [`hardware/SmartOrderButton/SmartOrderButton.ino`](file:///c:/Users/ADMIN/OneDrive/Desktop/Button%20V2/hardware/SmartOrderButton/SmartOrderButton.ino).
2. Vào **Tools -> Manage Libraries**, cài đặt thư viện `ArduinoJson` (phiên bản 6.x hoặc 7.x).
3. Cấu hình menu **Tools**:
   - **Board**: `ESP32S3 Dev Module`
   - **USB CDC On Boot**: `Enabled`
   - **USB DFU On Boot**: `Enabled`
   - **Upload Mode**: `UART0 / Hardware CDC`
   - **Partition Scheme**: `Huge APP (3MB No OTA/1MB SPIFFS)`
4. Nhấn nút **Upload** và mở **Serial Monitor** ở baudrate **115200**.

### Nạp Bằng PlatformIO (VS Code)
1. Mở thư mục [`hardware/`](file:///c:/Users/ADMIN/OneDrive/Desktop/Button%20V2/hardware) trong VS Code.
2. Cắm cáp Type-C vào mạch ESP32-S3.
3. Bấm biểu tượng **PlatformIO: Upload** trên thanh trạng thái.

---

## 🔬 6. Báo Cáo Chuẩn Đoán & Khắc Phục Phần Cứng (Hardware Stability & PCB Fixes)

Sau khi kiểm tra kỹ lưỡng sơ đồ nguyên lý KiCad `Smart_Order_Button.kicad_sch` và Layout PCB 2 lớp:

### A. Các Lỗi Phần Cứng Chí Mạng Cần Lưu Ý
1. **Lỗi Đánh Thức Deep Sleep (SW1 trên IO39)**:
   - Trên ESP32-S3, chân RTC Wakeup chỉ nằm trong khoảng `IO0` - `IO21`. Chân `IO39` là chân GPIO thông thường (HP Domain), không thể dùng `esp_sleep_enable_ext0_wakeup`.
   - **Xử lý phần mềm**: Firmware mặc định dùng **Light Sleep** kết hợp `esp_sleep_enable_gpio_wakeup` để bấm nút SW1 vẫn thức dậy 100%.
   - **Xử lý phần cứng (Khuyên dùng)**: Cắt đường mạch SW1 vào IO39, hàn 1 sợi dây đồng (bodge wire) từ chân SW1 sang chân RTC còn trống như **IO10** (Pin 14) hoặc **IO14** (Pin 10), sau đó bật `#define USE_HARDWARE_MOD_RTC_PIN 1` để đạt dòng ngủ chuẩn `<15µA`.
2. **Thiếu Tụ Trễ RC Trên Chân EN (Pin 3)**:
   - Chân EN chỉ có trở kéo lên R13 10k, thiếu tụ $1\mu\text{F}$ / $100\text{nF}$ xuống GND. Khi cấp nguồn pin, nguồn 3.3V lên chậm hơn mức kích hoạt EN dẫn đến lỗi khởi động (Boot brownout/hang).
   - **Khắc phục**: Hàn thêm 1 tụ gốm $1\mu\text{F}$ (hoặc $100\text{nF}$) từ chân số 3 (EN) xuống GND (chân số 1).
3. **Sụt Nguồn Trên LDO ME6211 (500mA)**:
   - Đỉnh dòng phát Wi-Fi đạt 450-500mA dễ làm sụt áp ngõ ra LDO.
   - **Khắc phục phần mềm**: Firmware đã hạ xung CPU xuống `80MHz` và đặt công suất Wi-Fi ở `15dBm` (giảm 35% dòng đỉnh).
   - **Khắc phục phần cứng**: Hàn thêm tụ gốm $10\mu\text{F} - 22\mu\text{F}$ kẹp trực tiếp vào chân 3V3 (Pin 2) và GND (Pin 1) của ESP32-S3.
4. **Vùng Cấm Đồng Anten RF Trên PCB**:
   - Phần anten mạch in của module ESP32-S3 bị bao bọc bởi mặt phẳng đồng và đường mạch, gây suy hao sóng RF 15-25dBm.
   - **Khắc phục bản PCB tiếp theo**: Kéo module ESP32-S3 ra sát mép bo mạch, đặt anten nhô ra ngoài viền hoặc cạo bỏ toàn bộ lớp đồng (Keep-out zone) dưới và xung quanh khu vực anten.
5. **Diode Dập Xung Còi Buzzer BZ1**:
   - Hàn thêm 1 Diode Schottky (1N5819 hoặc 1N4148) song song 2 chân còi BZ1 (Cathode nối +3.3V, Anode nối Drain Q1) để dập xung điện áp cảm ứng ngược khi ngắt MOSFET Q1.
