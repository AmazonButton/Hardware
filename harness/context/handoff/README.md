# context/handoff/

Thư mục này lưu các **Session Handoff artifacts** — xem quy trình đầy đủ tại
[`../../core/SESSION_HANDOFF.md`](../../core/SESSION_HANDOFF.md).

- File đang hiệu lực: `context/handoff/<story-id-or-task-slug>-<yyyyMMdd-HHmm>.md`
- File đã đóng (task hoàn tất): chuyển vào `context/handoff/_closed/`
- Thư mục này (và `_closed/`) nên nằm trong `.gitignore` NẾU dự án coi đây là trạng thái
  tạm thời của agent; nếu muốn giữ lịch sử bàn giao như tài liệu audit, bỏ khỏi
  `.gitignore` và commit bình thường — tự quyết theo nhu cầu observability của dự án.
