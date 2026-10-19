# Session Handoff Protocol

> Đặt file này tại `harness-engineering/core/SESSION_HANDOFF.md`.
> Mục tiêu: không để agent "mất trí nhớ" khi bị context-compaction, hết phiên, hoặc chuyển giao cho agent/người khác.

## 1. Khi nào phải ghi Handoff

Ghi một handoff artifact khi xảy ra BẤT KỲ điều nào:

- Sắp chạm ngưỡng token của phiên (theo `context_engineering.token_budget_max`), trước khi bị auto-compact.
- Task chưa hoàn thành nhưng phiên phải dừng (hết giờ, người dùng thoát, lỗi hệ thống).
- Chuẩn bị chuyển việc cho một agent/sub-agent khác (parallel agents, xem `PARALLEL_AGENTS.md`).
- Sau khi review bị BLOCK và cần một phiên mới để sửa.

## 2. Vị trí lưu

```
context/handoff/<story-id-or-task-slug>-<yyyyMMdd-HHmm>.md
```

Chỉ giữ handoff còn hiệu lực; khi task đóng (`story verify` PASS + trace `outcome: success`), archive file vào `context/handoff/_closed/`.

## 3. Cấu trúc bắt buộc

```markdown
# Handoff: <story-id / task slug>

## Trạng thái hiện tại
- Việc đã xong: ...
- Việc đang dở (mô tả chính xác điểm dừng, KHÔNG mô tả mơ hồ): ...
- File đã sửa (danh sách đường dẫn + tóm tắt thay đổi): ...

## Quyết định đã chốt (không lặp lại việc tranh luận)
- <quyết định 1> — lý do — link `docs/decisions/...` nếu có
- <quyết định 2> ...

## Giả định về thế giới bên ngoài phiên này
- Ví dụ: "API `/v1/scan` được giả định đã deploy ở staging", "biến môi trường X đã có sẵn"

## Lệnh xác minh còn phải chạy
- [ ] `<lệnh test/build cụ thể>`
- [ ] `<lệnh khác>`

## Rủi ro / điều KHÔNG được làm lại
- Ví dụ: "Đã thử cách A → gây regression ở US-014, đừng lặp lại, xem docs/failures/FAIL-021.md"

## Việc tiếp theo (cho phiên/agent kế tiếp)
1. ...
2. ...
```

## 4. Quy tắc đọc khi mở phiên mới

Agent mới nhận task **phải** đọc, theo thứ tự:
1. `context/handoff/<file mới nhất khớp story-id>.md` nếu tồn tại — đây là nguồn sự thật gần nhất, ưu tiên hơn suy luận lại từ đầu.
2. `scripts/bin/harness-cli query matrix` để xác nhận trạng thái durable khớp với handoff (nếu lệch → tin durable layer, ghi chú lệch vào trace).
3. Story packet + AGENTS.md liên quan như bình thường theo `CONTEXT_RULES.md`.

Agent **không được** giả định lại từ đầu nếu có handoff hợp lệ — việc đọc lại toàn bộ transcript cũ để "chắc ăn" là lãng phí token và đi ngược mục tiêu tối ưu của harness này.

## 5. Khác biệt với Compaction thông thường

| | Compaction (tự động, trong 1 phiên) | Handoff (thủ công/agent tự viết, giữa các phiên) |
|---|---|---|
| Ai tạo | Runtime của model/host | Agent, theo template cố định ở trên |
| Mất chi tiết | Có thể mất lệnh test đang fail, quyết định cụ thể | Không, vì là danh sách có cấu trúc, không phải tóm tắt tự do |
| Khi dùng | Giữa chừng 1 phiên dài, tự động | Cuối phiên, đầu phiên mới, hoặc bàn giao agent khác |
| Lưu ở đâu | Trong context window (tạm) | File durable trong repo (`context/handoff/`) |

## 6. Liên kết Default-FAIL Contract

Mọi mục trong "Lệnh xác minh còn phải chạy" mặc định ở trạng thái **FAIL/chưa xác minh** cho tới khi có bằng chứng (log/output) đính kèm trong trace tương ứng. Không được đánh dấu xong chỉ vì handoff nói "có vẻ đã chạy qua".
