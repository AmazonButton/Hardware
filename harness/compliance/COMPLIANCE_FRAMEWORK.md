# Compliance Framework (Universal)

## 1. Scope & Intent

Mọi phần mềm — dù có thuộc ngành quản lý chặt (y tế, tài chính, thương mại điện tử) hay không — đều có rủi ro pháp lý/uy tín khi AI agent sinh nội dung UI, đưa ra khuyến nghị, hoặc xử lý dữ liệu người dùng. Tài liệu này định nghĩa các **guardrail phổ quát**, áp dụng cho MỌI dự án bất kể domain.

Nếu dự án thuộc một ngành có yêu cầu riêng (y tế, tài chính, thương mại điện tử...), nạp thêm domain-pack tương ứng tại `compliance/domain-packs/<domain>.md` (chọn qua `harness.config.yaml → compliance.domain_pack`). Domain-pack chỉ được **thêm** ràng buộc, không được nới lỏng nguyên tắc ở đây.

---

## 2. Nguyên tắc phổ quát (áp dụng mọi dự án)

### Rule 1 — Không tuyên bố tuyệt đối
- Tránh các khẳng định tuyệt đối mà hệ thống không thể đảm bảo 100% ("hoàn toàn an toàn", "không bao giờ lỗi", "đảm bảo lợi nhuận", "chắc chắn đúng").
- Dùng ngôn ngữ có giới hạn xác suất/điều kiện: "chưa phát hiện rủi ro trong dữ liệu hiện có", "phù hợp trong hầu hết trường hợp", "kết quả có thể thay đổi tuỳ ngữ cảnh".
- Lý do: tuyên bố tuyệt đối có thể cấu thành quảng cáo sai sự thật/gây hiểu nhầm theo luật bảo vệ người tiêu dùng ở nhiều quốc gia.

### Rule 2 — Không giả danh vai trò được cấp phép
- AI không được tự xưng là "Bác sĩ", "Luật sư", "Chuyên gia tài chính được cấp phép", hoặc bất kỳ vai trò yêu cầu chứng chỉ hành nghề nào, trừ khi hệ thống thực sự tích hợp con người có chứng chỉ đó.
- Dùng nomenclature trung tính: "Trợ lý AI", "Hệ thống phân tích tự động".

### Rule 3 — Minh bạch giới hạn của AI
- Mọi kết quả do AI sinh ra có tác động tới quyết định của người dùng (sức khoẻ, tài chính, pháp lý, an toàn) phải có disclaimer rõ ràng về bản chất xác suất/tham khảo của kết quả.

### Rule 4 — Bảo vệ dữ liệu người dùng (nguyên tắc tối thiểu, không thay thế luật áp dụng: GDPR, Nghị định 13/2023/NĐ-CP, CCPA...)
- Có checkpoint đồng ý rõ ràng (explicit consent) trước khi thu thập dữ liệu nhạy cảm (sức khoẻ, tài chính, sinh trắc học, vị trí chính xác).
- Cung cấp cơ chế: xuất dữ liệu, xoá tài khoản/dữ liệu trong thời hạn luật định hoặc cam kết của sản phẩm.
- Mã hoá khi truyền (TLS 1.2+) và khi lưu trữ (AES-256 hoặc tương đương) đối với dữ liệu nhạy cảm.

### Rule 5 — Không thao túng hành vi người dùng (dark patterns)
- Không thiết kế luồng đăng ký/huỷ, thanh toán, hoặc consent theo hướng gây khó/đánh lừa người dùng.

---

## 3. Cổng kiểm tra tự động (Automated Compliance Gates)

1. **Linter/grep CI**: chặn từ khoá bị cấm xuất hiện trong bundle UI. Danh sách từ khoá lấy từ `harness.config.yaml → compliance.forbidden_terms` (không hardcode trong doc này):
   ```bash
   # ví dụ pattern chung, danh sách thật load từ config
   ! rg -if <(yq '.compliance.forbidden_terms[]' harness.config.yaml) [frontend-dir]/ [mobile-dir]/
   ```
2. **E2E assertion**: nếu `compliance.e2e_legal_assertion: true`, test E2E (Maestro/Playwright) phải xác nhận disclaimer bắt buộc hiện diện trên các màn hình có kết quả do AI sinh ra.
3. **Review role**: Security Auditor / QA Engineer role phải kiểm tra Rule 1–5 như một phần checklist review cho lane `normal` trở lên khi `compliance.legal_rules_enabled: true`.

## 4. Domain Packs hiện có

| Domain pack | Dùng khi | File |
|---|---|---|
| `generic` | Không thuộc ngành đặc thù nào | (chỉ dùng framework này, không cần file thêm) |
| `health-wellness` | App sức khoẻ, dinh dưỡng, quét sản phẩm, thiết bị y tế | `domain-packs/health-wellness.md` |
| `fintech` | App tài chính, ví điện tử, cho vay, đầu tư | `domain-packs/fintech.md` |
| `ecommerce` | Sàn TMĐT, marketplace, thanh toán tiêu dùng | `domain-packs/ecommerce.md` |

Muốn thêm domain mới: copy 1 file trong `domain-packs/` làm mẫu, chỉ viết phần khác biệt so với `COMPLIANCE_FRAMEWORK.md`.
