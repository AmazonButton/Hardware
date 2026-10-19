# Domain Pack: Health & Wellness

> Nạp thêm file này khi `harness.config.yaml → project.domain: "health-wellness"`.
> Các nguyên tắc dưới đây CỘNG THÊM vào `compliance/COMPLIANCE_FRAMEWORK.md`, không thay thế.

## 1. Bối cảnh
Áp dụng cho app sức khoẻ, dinh dưỡng, quét thành phần sản phẩm, thiết bị đeo, theo dõi chỉ số cơ thể... Chịu ràng buộc bổ sung từ luật bảo vệ người tiêu dùng, quy định quảng cáo y tế, và chính sách review của Apple/Google cho app sức khoẻ.

## 2. Thuật ngữ

- **Cấm dùng**: "100% An toàn", "Hoàn toàn an toàn", "An toàn tuyệt đối", "Nguy hiểm" (như phán quyết dứt khoát), "Độc hại" (khi chưa có kiểm nghiệm cụ thể).
- **Khuyến nghị dùng thay thế**: "Chưa phát hiện rủi ro trong dữ liệu hiện có", "Có thành phần cần lưu ý", "Khả năng phù hợp cao", "Phù hợp với hầu hết trường hợp thông thường".
- **Lý do**: ứng dụng không thể đảm bảo khả năng dung nạp sinh học/hoá học của từng cá nhân; tuyên bố tuyệt đối có thể cấu thành quảng cáo gây hiểu lầm.

## 3. Vai trò AI
- Không được thể hiện AI như "Bác sĩ", "Chuyên gia y tế", "Cố vấn y tế".
- Dùng: "Trợ lý AI", "Hệ thống AI phân tích".

## 4. Disclaimer bắt buộc
Mọi phân tích/đánh giá/khuyến nghị do AI sinh ra phải kèm disclaimer rõ ràng, đại ý: kết quả mang tính xác suất/tham khảo, không thay thế tư vấn y tế chuyên môn, khuyến khích người dùng kiểm tra nhãn sản phẩm gốc hoặc hỏi ý kiến người có chuyên môn.

## 5. Dữ liệu nhạy cảm (GDPR / Nghị định 13/2023/NĐ-CP hoặc luật tương đương tại thị trường vận hành)
- Cần checkpoint đồng ý rõ ràng (`acceptedSensitiveData = true`) trước khi thu thập dị ứng, tình trạng bệnh lý, chỉ số sinh trắc học.
- Cung cấp: xuất dữ liệu; xoá tài khoản/dữ liệu trong khung thời gian đã cam kết (vd. 72 giờ kể từ yêu cầu).
- Mã hoá khi truyền (TLS 1.3) và khi lưu (AES-256).

## 6. Cổng kiểm tra tự động bổ sung
```bash
# CI: chặn từ khoá cấm trong bundle UI
! rg -i "hoàn toàn an toàn|an toàn tuyệt đối" [mobile-dir]/ [frontend-dir]/
```
E2E (Maestro/Playwright) phải assert disclaimer y tế xuất hiện trên mọi màn hình kết quả AI.

## 7. Danh sách `forbidden_terms` mẫu cho `harness.config.yaml`
```yaml
compliance:
  domain_pack: "health-wellness"
  forbidden_terms:
    - "An toàn tuyệt đối"
    - "Hoàn toàn an toàn"
    - "Nguy hiểm"
    - "Chuyên gia"
    - "Bác sĩ"
    - "Cố vấn y tế"
  e2e_legal_assertion: true
```
