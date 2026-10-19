# Domain Pack: FinTech

> Nạp thêm khi `project.domain: "fintech"`. Cộng thêm vào `compliance/COMPLIANCE_FRAMEWORK.md`.

## 1. Bối cảnh
Ví điện tử, cho vay, đầu tư, ngân hàng số, thanh toán. Chịu thêm ràng buộc từ luật chống rửa tiền (AML), bảo vệ dữ liệu tài chính, và quy định quảng cáo sản phẩm tài chính.

## 2. Thuật ngữ
- **Cấm dùng**: "Đảm bảo lợi nhuận", "Không rủi ro", "Lãi suất cố định trọn đời" (trừ khi hợp đồng thực sự cam kết), "Được nhà nước bảo lãnh" (trừ khi đúng sự thật).
- **Khuyến nghị**: "Lợi nhuận kỳ vọng, không đảm bảo", "Rủi ro tuỳ thuộc điều kiện thị trường", nêu rõ APR/phí thực tế.

## 3. Vai trò AI
- Không giả danh "Chuyên gia tài chính được cấp phép", "Cố vấn đầu tư".
- Mọi khuyến nghị đầu tư/vay phải có disclaimer "không phải lời khuyên tài chính cá nhân hoá".

## 4. Dữ liệu & KYC
- Dữ liệu định danh (CMND/CCCD, số tài khoản ngân hàng) phải mã hoá at-rest riêng biệt, giới hạn quyền truy cập theo role.
- Log giao dịch phải bất biến (append-only) để phục vụ audit/AML.
- Không log toàn bộ số thẻ/tài khoản ra file log thông thường (chỉ 4 số cuối).

## 5. Cổng kiểm tra tự động bổ sung
- CI chặn commit chứa pattern giống số thẻ/tài khoản thật trong test fixtures (dùng dữ liệu giả theo chuẩn test card của cổng thanh toán).
- Security Auditor review bắt buộc PASS cho mọi thay đổi liên quan luồng thanh toán/số dư (tự động rơi vào lane `high_risk`).
