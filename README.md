# ⚡ Smart Order Button — Tài Liệu Phần Cứng & Firmware ESP32-S3 (Bản 1 Nút & 3 Nút)

Tài liệu kỹ thuật và mã nguồn firmware nhúng cho bo mạch nút bấm thông minh **Smart Order Button IoT**, thiết kế chuẩn xác 100% theo sơ đồ nguyên lý KiCad **`Smart_Order_Button.kicad_sch`** và layout PCB trên vi điều khiển **ESP32-S3-WROOM-1**, kết nối trực tiếp với hệ sinh thái **Backend NestJS Enterprise**.

---

## 🎛️ 1. Hỗ Trợ 2 Phiên Bản Phần Cứng (Dual Hardware Variants)

Hệ thống hỗ trợ linh hoạt cả 2 phiên bản phần cứng thông qua macro `#define CURRENT_HARDWARE_VARIANT` trong [`include/config.h`](file:///c:/Users/ADMIN/OneDrive/Desktop/Phancung/hardware/include/config.h) và giao diện trực quan trên Test Bench:

| Tiêu Chí So Sánh | 🔘 Bản 1 Nút (Single Button) | 🔘🔘🔘 Bản 3 Nút (Triple Button - KiCad v4.5) |
| :--- | :--- | :--- |
| **Phần Cứng Nút Bấm** | Chỉ sử dụng duy nhất nút **SW1** | Trang bị đủ 3 nút vật lý: **SW1, SW2, SW3** |
| **Chân GPIO** | `IO39` (Pin 32) | `IO39` (SW1), `IO40` (SW2), `IO41` (SW3) |
| **Nghiệp Vụ Nút** | **Đa cử chỉ (Multi-gesture)**:<br>- Nhấn 1 lần: Đặt hàng<br>- Nhấn đúp: Hủy đơn<br>- Nhấn 5 lần / Giữ 10s: Cấu hình BLE<br>- Giữ 5s: Kích hoạt / Đồng bộ<br>- Giữ 15s: Khôi phục cài đặt gốc | **Nút chuyên dụng (Dedicated Buttons)**:<br>- **SW1 (IO39)**: Tạo đơn hàng tức thì (ORDER)<br>- **SW2 (IO40)**: Yêu cầu thanh toán / Gọi phục vụ (BILL)<br>- **SW3 (IO41)**: Hủy đơn hàng (CANCEL)<br>- Giữ SW2 5s: Vào chế độ BLE Wi-Fi<br>- Giữ đồng thời SW1 + SW3: Khôi phục cài đặt gốc |
| **Ưu Điểm Nổi Bật** | Thiết kế vỏ hộp siêu nhỏ gọn, tiết kiệm chi phí | Thao tác 1 chạm tức thì, không bị nhầm lẫn cử chỉ nhấn đúp, phù hợp nhà hàng chuyên nghiệp |
| **Cấu Hình Firmware** | `#define CURRENT_HARDWARE_VARIANT HARDWARE_VARIANT_SINGLE_BUTTON` | `#define CURRENT_HARDWARE_VARIANT HARDWARE_VARIANT_TRIPLE_BUTTON` |

---

## 📌 2. Bảng Tra Cứu Chân Phần Cứng (Hardware Pinout Mapping)

| Chân ESP32-S3 | Nhãn Net KiCad | Linh Kiện & Chức Năng | Nguyên Lý Hoạt Động |
| :--- | :--- | :--- | :--- |
| **IO39** (Pin 32) | `BTN_ORDER / BTN_SW1` | Nút bấm vật lý **SW1** | Nối giữa IO39 và GND. Cấu hình `INPUT_PULLUP`. Khi nhấn kéo xuống GND (**Active LOW**). Đánh thức tức thì từ Light Sleep (<50µs) hoặc Deep Sleep (nếu mod câu dây sang RTC IO10). |
| **IO40** (Pin 33) | `BTN_SW2` | Nút bấm vật lý **SW2** | Nối giữa IO40 và GND. Cấu hình `INPUT_PULLUP`. Chức năng **Yêu cầu thanh toán / Gọi nhân viên** (**Active LOW**). |
| **IO41** (Pin 34) | `BTN_SW3` | Nút bấm vật lý **SW3** | Nối giữa IO41 và GND. Cấu hình `INPUT_PULLUP`. Chức năng **Hủy đơn hàng / Trợ giúp SOS** (**Active LOW**). |
| **IO7** (Pin 7) | `LED_R` | Cực Catot LED Đỏ (qua trở R15 1kΩ) | Đèn LED RGB D1 là loại **Common Anode** nối nguồn +3.3V. Kéo xuống mức thấp để bật đèn (**Active LOW**). |
| **IO6** (Pin 6) | `LED_G` | Cực Catot LED Xanh Lá (qua trở R16 470Ω) | Kéo xuống mức thấp để bật đèn (**Active LOW**). |
| **IO8** (Pin 12) | `LED_B` | Cực Catot LED Xanh Dương (qua trở R17 470Ω) | Kéo xuống mức thấp để bật đèn (**Active LOW**). |
| **IO2** (Pin 38) | `BUZZER_CTRL`| Gate MOSFET **Q1 (2N7002)** kích còi **BZ1** | Kéo lên mức cao để kích hoạt MOSFET cấp nguồn cho còi phát âm thanh (**Active HIGH**). Có trở xả Gate R6 10kΩ xuống GND và tụ dập C10 100nF song song còi. |
| **IO4** (Pin 4) | `I2C_SDA` | Chân dữ liệu I2C IC đo pin **MAX17048 (U6)** | Bus I2C chuẩn, có điện trở treo R9 4.7kΩ lên nguồn +3.3V. |
| **IO5** (Pin 5) | `I2C_SCL` | Chân xung nhịp I2C IC đo pin **MAX17048 (U6)** | Bus I2C chuẩn, có điện trở treo R3 4.7kΩ lên nguồn +3.3V. |
| **IO20** (Pin 14) | `USB_D+` | Cổng Type-C Native qua IC chống tĩnh điện **U2 (USBLC6-2SC6)** | Nạp code firmware & Serial Monitor trực tiếp tốc độ cao qua USB CDC Native. |
| **IO19** (Pin 13) | `USB_D-` | Cổng Type-C Native qua IC chống tĩnh điện **U2 (USBLC6-2SC6)** | Truyền nhận dữ liệu USB. |

---

## 🔋 3. Quản Lý Nguồn & Đo Pin (Power Architecture)

1. **IC Sạc Li-Po**: `MCP73831T-2ACI_OT` (U3) nhận nguồn 5V từ cổng Type-C VBUS, sạc pin qua chân PROG (R11 2kΩ -> dòng sạc danh định ~500mA).
2. **IC Bảo Vệ Pin**: `DW01A` (U5) kết hợp Dual N-MOSFET `8205A` (Q2A/Q2B) bảo vệ pin chống quá sạc (Overcharge > 4.3V), chống quá xả (Overdischarge < 2.4V) và ngắn mạch.
3. **Hạ Áp LDO**: `ME6211C33M5G` (U4) chuyển điện áp pin Li-Po (3.3V - 4.2V) thành điện áp ổn định +3.3V với dòng tải lên tới 500mA cấp cho ESP32-S3. Firmware tự động giới hạn xung nhịp CPU 80MHz và công suất phát Wi-Fi 15dBm để triệt tiêu sụt áp (Brownout).
4. **IC Đo Dung Lượng Pin**: `MAX17048G_T10` (U6) đo điện áp tế bào pin (VCELL) và tính toán dung lượng State of Charge (SOC%) chính xác theo thuật toán ModelGauge, đọc dữ liệu qua I2C (địa chỉ 0x36).

---

## 🎮 4. Quy Ước Cử Chỉ & Phản Hồi LED / Còi Buzzer

| Thao Tác / Sự Kiện | Bản 1 Nút (SW1) | Bản 3 Nút (SW1/SW2/SW3) | Màu LED RGB D1 | Âm Thanh Còi BZ1 |
| :--- | :--- | :--- | :--- | :--- |
| **Đặt hàng (ORDER)** | Nhấn 1 lần (<800ms) | Bấm nút **SW1** | 🔵 Xanh dương -> 🟢 Xanh lá | 2 tiếng bíp cao (Success Chime: 2.7kHz -> 3.5kHz) |
| **Thanh toán (BILL)** | Nhấn giữ 5s (Activate) | Bấm nút **SW2** | 💎 Cyan (Xanh lơ) | 2 bíp thanh thoát (Bill Chime: 2.4kHz -> 2.8kHz) |
| **Hủy đơn (CANCEL)** | Nhấn đúp (<1.2s) | Bấm nút **SW3** | 🟡 Vàng -> 🔴 Đỏ | 1 tiếng bíp trầm dài (Cancel Tone: 1.8kHz 320ms) |
| **Cấu hình Wi-Fi BLE** | Nhấn 5 lần / Giữ 10s | Nhấn giữ **SW2** 5 giây | 🔵 Nhấp nháy Xanh dương | 1 bíp dài (120ms) |
| **Reset cài đặt gốc** | Nhấn giữ 15 giây | Giữ đồng thời **SW1 + SW3** | 🔴 Nhấp nháy Đỏ 5 lần | 3 tiếng bíp báo động |
| **Khởi động nguồn** | Cắm pin / USB | Cắm pin / USB | 💎 Cyan nhẹ | 1 click nhẹ (80ms 3.0kHz) |

---

## 🚀 5. Chạy Thử Mạch Thực Nghiệm (Test Bench)

### Cách 1: File HTML Độc Lập (Chạy Ngay Trong Trình Duyệt)
- Mở trực tiếp file: [`test_bench/index.html`](file:///c:/Users/ADMIN/OneDrive/Desktop/Phancung/hardware/test_bench/index.html) bằng Chrome hoặc Edge.
- Trải nghiệm chuyển đổi mượt mà giữa **Bản 1 Nút** và **Bản 3 Nút**.
- Xem bản vẽ **Layout Bo Mạch KiCad Thực Tế (PCB Canvas)** 2D tương tác trực quan, click trực tiếp lên từng linh kiện trên mạch in!
- **Giao diện Web Bluetooth Bản 1 Nút Riêng Biệt**: Mở [`test_bench/fe_bluetooth_1nut.html`](file:///c:/Users/ADMIN/OneDrive/Desktop/Phancung/hardware/test_bench/fe_bluetooth_1nut.html) (hoặc [`web_bluetooth_tool.html`](file:///c:/Users/ADMIN/OneDrive/Desktop/Phancung/hardware/web_bluetooth_tool.html)) để kết nối trực tiếp mạch thật qua Web Bluetooth API, chọn tài khoản Wi-Fi có sẵn và đồng bộ nút bấm!

### Cách 2: Chạy Script Kiểm Thử Tự Động Bằng Node.js
Chạy câu lệnh sau trong terminal:
```bash
node test_bench/run_test.js
```
Kết quả hiển thị toàn bộ 16/16 bước kiểm thử đạt chuẩn (100% Pass) cho cả 2 phiên bản phần cứng!

---

## 🤝 6. Đóng Gói & Hướng Dẫn Đóng Góp (Contributors & Packaging)
Chi tiết về cấu trúc đóng gói mô-đun hóa, cổng kiểm thử chất lượng phần cứng bắt buộc và quy chuẩn gửi Pull Request (PR) được mô tả đầy đủ tại:
👉 **[CONTRIBUTING.md](file:///c:/Users/ADMIN/OneDrive/Desktop/Phancung/hardware/CONTRIBUTING.md)**.
Mọi đóng góp từ cộng đồng đều được chào đón và đánh giá tự động qua bộ kiểm thử `run_test.js`!

