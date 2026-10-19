# Project Constitution — Anhem JaeBin – Ăn Chơi Điệu Nghệ

> **Nguyên tắc Bất Biến của Dự Án (Inviolables)**  
> Mọi tài liệu (Specs, Architecture, PRD), kế hoạch và mã nguồn bắt buộc phải nhất quán với Constitution này.  
> Nếu một giải pháp kỹ thuật mâu thuẫn với Constitution, đó là tín hiệu DỪNG LẠI để xin ý kiến của Developer.

---

## 1. Sứ Mệnh (Mission)
Xây dựng một tựa game ARPG 2D đường phố "Anhem JaeBin" có trải nghiệm chặt chém đã tay, cơ chế thanh Style "Ăn chơi điệu nghệ" kích thích dopamine, tích hợp hệ thống Web Portal phục vụ cộng đồng (Giftcode, Sự kiện, Nạp thẻ), do **1 Solo Developer** phát triển và vận hành bền vững với sự trợ lực tối đa từ AI Coding Agents.

---

## 2. Những Điều Tuyệt Đối Không Được Vi Phạm (Non-Negotiables)

1. **Không Bao Giờ Tin Tưởng Client (`Never Trust The Client`)**:
   - Tuyệt đối không tính toán thời gian và tài nguyên treo máy (AFK) dựa trên đồng hồ điện thoại của người chơi. Mọi tính toán offline phải được Server xác thực dựa trên `last_sync_timestamp` của cơ sở dữ liệu.
   - Không cộng Kim Cương hay vật phẩm trực tiếp từ Client; toàn bộ giao dịch thanh toán phải qua Server Webhook được ký điện tử (HMAC SHA256).
2. **Cơ Chế Bất Khả Lặp Bắt Buộc (`Idempotency Everywhere`)**:
   - Mọi API tiếp nhận mã Giftcode và xử lý Webhook nạp tiền (MoMo, VNPay, ZaloPay, Thẻ cào) phải có khóa Idempotency Key chống Double-spending và Replay Attack.
3. **Nói Không Với Hút Máu Cực Đoan (`Anti Hard Pay-to-Win`)**:
   - Không bán chỉ số độc quyền hoặc trang bị "thần thánh" chỉ người nạp tiền mới có.
   - Doanh thu tập trung $85\%$ vào Thẩm mỹ (Skin trang phục, Xe máy độ, Hiệu ứng chiêu thức) và Tiện ích tiết kiệm thời gian (Mở rộng rương đồ, Thẻ tháng, Battle Pass).
4. **Docs Là Nguồn Sự Thật (`Docs is Source of Truth`)**:
   - Không viết code khi chưa có file đặc tả tương ứng trong `docs/01-specs/FEAT-XXX.md`.
   - Nếu trong quá trình code phát sinh logic mới hoặc thay đổi công thức toán, bắt buộc cập nhật lại file spec trước khi đóng phiên làm việc.
5. **Tiêu Chuẩn Hiệu Năng Mobile 60 FPS (`Zero GC Spike in Update Loop`)**:
   - Cấm `new` đối tượng, cấm dùng `LINQ`, cấm nối chuỗi string trong các hàm `Update()` hoặc `FixedUpdate()` của Game Client.
   - Bắt buộc áp dụng **Object Pooling** cho quái, đạn, hiệu ứng hạt (VFX) và text hiển thị sát thương.

---

## 3. Những Điều Chủ Động Nằm Ngoài Phạm Vi (Explicitly Out of Scope)

1. **Tránh Phân Mảnh Microservices**: Backend được đóng gói theo dạng **Modular Monolith** duy nhất. Không dựng nhiều repo hay microservices phức tạp gây quá tải chi phí vận hành cho 1 lập trình viên.
2. **Tạm Hoãn Bản iOS Ở Giai Đoạn Khởi Đầu**: Tập trung 100% tài nguyên cho nền tảng Android (File APK / Google Play) và Web Portal. Bản iOS chỉ triển khai khi game đã có lượng người chơi ổn định và doanh thu thực tế.
3. **Không Làm Sàn Giao Dịch Tự Do Giữa Người Chơi (P2P Free Market)**: Tránh phát sinh gian lận, bot cày tiền và chợ đen trong giai đoạn MVP.

---

## 4. Quyền Hạn Ra Quyết Định (Decision Authority)

- **Quyết định tối cao**: Thuộc về **Solo Developer** của dự án.
- **Vai trò của AI Agent**: Đề xuất giải pháp, thực thi kiểm thử và viết code tuân thủ theo các Quality Gates. AI **tuyệt đối không tự ý nới lỏng** các rào chắn kỹ thuật hoặc chỉnh sửa Constitution mà không có sự chỉ đạo rõ ràng từ Developer.

---

## 5. Tham Chiếu Bắt Buộc
- Quy chuẩn AI hàng ngày: [AGENTS.md](file:///f:/project/repo-github/JaeBinPlatform/AGENTS.md)
- Cấu hình khung kỹ thuật: [harness.config.yaml](file:///f:/project/repo-github/JaeBinPlatform/harness-engineering/harness.config.yaml)
- Tầm nhìn 10 phút đầu & Định vị: [docs/00-vision/vision.md](file:///f:/project/repo-github/JaeBinPlatform/docs/00-vision/vision.md)
- Quy trình Solutioning Gate: [harness-engineering/discovery/README.md](file:///f:/project/repo-github/JaeBinPlatform/harness-engineering/discovery/README.md)
