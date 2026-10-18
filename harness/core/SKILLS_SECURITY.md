# Skills Supply-Chain Security (v3)

> Mới trong v3. Hệ sinh thái Skills (`SKILL.md` — quy ước Anthropic + cộng đồng đang chuẩn
> hoá) phát triển rất nhanh trong 2026, kéo theo rủi ro cụ thể: skill là văn bản/markdown
> agent tự đọc và làm theo, nên skill độc hại = một dạng prompt injection đóng gói sẵn, có
> thể nằm im cho tới khi agent thực thi bước cuối trong quy trình skill đó.

## Nguyên tắc bắt buộc trước khi tin một skill (kể cả skill trong `skills/` của chính repo này)

1. **Đọc toàn bộ nội dung skill trước khi kích hoạt lần đầu** — không chỉ đọc tên/mô tả.
   Đặc biệt chú ý các bước cuối cùng của một quy trình nhiều bước (đây là nơi payload độc
   hại thường được giấu, sau khi các bước đầu đã "xây dựng lòng tin").
2. **Cảnh giác với ký tự Unicode ẩn** (zero-width, bidi override) trong file skill — có thể
   che giấu chỉ dẫn khác với những gì hiển thị khi đọc lướt. Dùng `cat -A`/hexdump nếu nghi
   ngờ, không chỉ đọc qua trình soạn thảo mặc định.
3. **Skill của bên thứ ba (không phải bạn/team bạn viết) = coi như code chưa audit.** Áp
   dụng review level tương đương một dependency mới trước khi thêm vào `skills/` hoặc cài
   qua plugin marketplace.
4. **Khi một skill "upstream" báo có bản cập nhật, review lại từ đầu** — đừng tự động
   update-and-trust. Một repo skill từng an toàn có thể bị compromise sau đó (kịch bản
   thực tế đã ghi nhận: một bước tưởng vô hại trong workflow bị lợi dụng để tải và chạy
   payload độc hại).
5. **Skill không nên có quyền vượt quá vai trò của nó.** Nếu 1 skill "hướng dẫn tạo ảnh
   GIF" lại yêu cầu chạy lệnh mạng/tải file thực thi — đó là dấu hiệu đáng ngờ, dừng lại.

## Áp dụng vào harness này

- Mọi skill mới thêm vào `skills/` phải qua ít nhất 1 role review (Security Auditor) trước
  khi merge, giống như code — ghi vào `templates/role-review-report.md` với `evidence_ref`
  là đường dẫn skill đã đọc.
- Nếu dùng plugin marketplace (bundle skill + subagent + hook + MCP server) — xem thêm
  `mcp/MCP_INTEGRATIONS.md § Plugin Marketplace` — áp dụng cùng mức độ soát xét cho TOÀN
  BỘ bundle, không chỉ phần skill.
- `.claude/agents/security-auditor.md` (subagent) tham chiếu trực tiếp tới file này —
  đảm bảo review skill mới nằm trong phạm vi trách nhiệm rõ ràng, không bị bỏ sót.

## Không phải rủi ro lý thuyết

Đây là các dạng tấn công đã được ghi nhận công khai trong hệ sinh thái skill/agent 2025–2026:
weaponized skill dùng để cài ransomware qua một bước tưởng như vô hại trong quy trình, và
chỉ dẫn ẩn bằng Unicode trong nội dung skill khiến agent làm điều khác với những gì người
review nhìn thấy. Không cần biết chi tiết từng vụ để áp dụng 5 nguyên tắc ở trên — nguyên
tắc chung là: **skill là bề mặt tấn công mới, đối xử với nó nghiêm túc như một dependency.**
