# Hooks & Enforcement Layer (v3)

> Mới trong v3. Trả lời câu hỏi: "quy tắc này nên sống ở đâu?" — vì viết quy tắc vào prose
> (`AGENTS.md`) là hình thức **yếu nhất** để enforce một rule; harness này giờ có 4 tầng rõ
> ràng, xếp từ yếu → mạnh.

## Rule Ownership Matrix

| Loại rule | Sống ở đâu | Vì sao |
|---|---|---|
| Luôn-đúng, ngắn, cần agent nhớ mọi lúc | `AGENTS.md` (root, < 80 dòng) | Đọc mỗi phiên, không cần enforce tự động |
| Kiến thức theo ngữ cảnh (stack cụ thể, domain cụ thể) | `stack-packs/*.md`, `compliance/domain-packs/*.md` | Nạp theo nhu cầu, không làm phình context mọi lúc |
| Ranh giới uỷ quyền (ai được làm gì, review qua ai) | `.claude/agents/*.md` (subagent `tools:` field) | Subagent chỉ có công cụ được khai — không dựa vào "agent tự giác" |
| Bắt buộc kỹ thuật, có thể kiểm chứng bằng exit code | **Hooks** (`PreToolUse`, `PostToolUse`, `SubagentStop`) hoặc CI gate | Prose có thể bị agent "quên"; hook thì không — nó chặn hành động trước khi xảy ra |

> Nguyên tắc chọn tầng (rút gọn từ khảo sát ngành 2026): nếu rule PHẢI được enforce, dùng
> Hook/permission. Nếu là kiến thức ngữ cảnh, dùng Skill/stack-pack. Nếu là ranh giới uỷ
> quyền, dùng Subagent. Nếu là hướng dẫn luôn-bật, để ngắn gọn trong AGENTS.md.

---

## Mapping cụ thể sang Claude Code hooks

Nếu agent runtime là Claude Code (hoặc tương thích Agent Client Protocol), các gate của
harness này có thể được thực thi bằng hooks thay vì chỉ dựa vào role review đọc prose:

| Harness gate | Hook event gợi ý | Hành vi |
|---|---|---|
| `core/SANDBOX_POLICY.md` (allow/ask/deny lệnh shell theo lane) | `PreToolUse` trên `Bash` | Chặn (exit code ≠ 0) lệnh nằm trong deny-list của lane hiện tại trước khi nó chạy |
| Role review phải chạy read-only (Fresh-Context Evaluator) | Đã enforce qua `tools:` trong `.claude/agents/*.md` — không cần hook riêng | Subagent không có `Write`/`Edit` trong danh sách tools thì không thể sửa file, bất kể prompt nói gì |
| Devil's Advocate bắt buộc chạy trước khi merge (lane normal/high-risk) | `SubagentStop` trên các subagent reviewer + kiểm tra trong CI trước merge | Nếu chưa có trace `devil_advocate` verdict cho story này → CI fail |
| `templates/role-review-report.md` Default-FAIL + `evidence_ref` bắt buộc | `PostToolUse` sau khi role ghi report, hoặc CI lint | Script kiểm tra report không có dòng "PASS" nào thiếu `evidence_ref` liền kề |
| `core/PREFLIGHT_CHECKLIST.md` trước phiên agent không giám sát | `SessionStart` (hoặc bước đầu CI job) | In ra 7 điểm checklist, yêu cầu xác nhận trước khi tiếp tục nếu chạy tương tác |
| Ghi trace mọi task | `Stop` (cuối phiên) | Gợi ý agent gọi `scripts/bin/harness-cli trace ...` trước khi kết thúc phiên nếu chưa ghi |

Ví dụ skeleton hook (`.claude/hooks/pretooluse-bash-sandbox.sh`, tham số nhận qua stdin JSON
theo chuẩn hook của Claude Code — xem tài liệu hooks chính thức để biết format input/exit
code chính xác tại thời điểm bạn tích hợp, vì bề mặt hook đổi theo phiên bản công cụ):

```bash
#!/usr/bin/env bash
# Đọc lane hiện tại (từ file trạng thái phiên hoặc biến môi trường HARNESS_LANE),
# đối chiếu lệnh sắp chạy với core/SANDBOX_POLICY.md deny-list cho lane đó.
# Exit non-zero để chặn lệnh; in lý do ra stderr để agent thấy và tự sửa hướng đi.
```

> Đây là **bản đồ** để bạn tự nối dây, không phải hook đã hoàn thiện sẵn sàng chạy — bề mặt
> hook API thay đổi theo từng phiên bản Claude Code/harness runtime. Kiểm tra tài liệu hooks
> hiện hành của công cụ bạn dùng trước khi triển khai production.

---

## Skills vs Subagents vs Hooks — khi nào dùng cái nào (tóm tắt)

- **Skill** (`skills/*/SKILL.md`) = dạy agent CÁCH làm 1 việc cụ thể, nạp khi liên quan.
  Không enforce gì — agent vẫn có thể bỏ qua nếu không thấy skill liên quan.
- **Subagent** (`.claude/agents/*.md`) = TÁCH ngữ cảnh + giới hạn quyền công cụ cho 1 vai
  trò. Dùng khi cần cô lập review khỏi bias, hoặc chạy song song (fan-out).
- **Hook** = CHẶN/CHO PHÉP hành động dựa trên điều kiện, chạy độc lập với việc model "có
  nhớ luật hay không". Dùng cho bất cứ điều gì nếu vi phạm sẽ gây hậu quả thật (production
  data, secrets, force-push).

Xem thêm: `core/SKILLS_SECURITY.md` (rủi ro khi cài skill của bên thứ ba).
