# Stack Pack: KiCad 8.x PCB & Schematic Design

> Nạp khi surface hardware sử dụng KiCad.

## 1. Công cụ & Định dạng Tệp
- **Phần mềm**: KiCad 8.x / 9.x.
- **Tệp nguồn chính**:
  - `Smart_Order_Button.kicad_sch`: Sơ đồ nguyên lý toàn diện (Schematic).
  - `Smart_Order_Button.kicad_pcb`: Bản vẽ bố trí linh kiện và đi dây mạch in (Layout).
  - `Smart_Order_Button.kicad_pro`: Tệp quản lý dự án KiCad.

## 2. Quy tắc Thiết kế Nguyên lý (Schematic Rules)
- Mọi chân IC chưa dùng phải có biểu tượng No Connect (`NC`).
- Tụ thoát nhiễu (Decoupling Capacitor) 100nF đặt gần nhất có thể với mỗi chân VDD của MCU.
- Kiểm tra Electrical Rules Check (ERC) phải đạt 0 lỗi và 0 cảnh báo xung đột chân nguồn.

## 3. Quy tắc Đi dây Mạch in (PCB Layout Rules)
- **Ăng-ten Wi-Fi/BLE**: Vùng cấm đi dây (Keep-out zone) dưới và xung quanh ăng-ten PCB của ESP32-S3. Không được để bất kỳ đường mạch hoặc mặt đồng (copper pour) nào trong vùng ăng-ten.
- **Đường nguồn & sạc**: Độ rộng đường mạch tối thiểu 0.5mm cho đường nguồn 3.3V và đường sạc pin LiPo để đảm bảo suy hao nội trở thấp.
- **Kiểm tra Design Rules Check (DRC)**: Bắt buộc 0 lỗi vi phạm khoảng cách (clearance) và 0 net chưa nối (unconnected nets).
