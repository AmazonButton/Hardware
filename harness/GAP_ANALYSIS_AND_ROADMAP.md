# Đánh giá & Lộ trình Nâng cấp — Harness Engineering Framework (HEF)

> Đánh giá dựa trên bộ `harness-engineering/` bạn upload (README, core/, roles/, golden-paths/, quality-gates/, review-protocol/, mcp/, scripts/…) đối chiếu với các framework harness/agentic-engineering đang trend nhất hiện nay (BMAD-METHOD, GitHub Spec Kit, OpenSpec, chuẩn AGENTS.md, và nghiên cứu harness dài-hạn của Anthropic Engineering).

---

## 0. Cảnh báo bảo mật cần xử lý NGAY

Trong `scripts/bin/harness-cli.exe` là một **binary Windows (PE32+) đã biên dịch sẵn, ~2.7MB**, không kèm mã nguồn Rust trong bundle này. Đây là rủi ro chuỗi cung ứng (supply-chain): một binary lạ được thực thi bởi mọi agent/CI trong mọi dự án dùng harness này.

Khuyến nghị bắt buộc trước khi rollout diện rộng:
- Không chạy `.exe` này trong môi trường CHƯA xác minh nguồn gốc.
- Thêm mã nguồn Rust của `harness-cli` vào repo (không chỉ binary), build bằng CI, publish checksum (SHA-256) kèm mỗi release.
- Ký (code-sign) binary hoặc tối thiểu commit hash vào `CHANGELOG` để mọi project instance có thể verify trước khi `chmod +x`.
- Thêm `harness-cli.exe`, `harness-cli` vào danh sách file cần SCA/AV scan trong `security-scan.yml`.

Đây là gap nghiêm trọng nhất so với mọi framework SDD/harness hiện nay (Spec Kit, OpenSpec, BMAD) — tất cả đều phân phối dưới dạng mã nguồn/npm package có thể audit, không phải binary đóng gói sẵn.

---

## 1. Điểm mạnh hiện có (đã ngang hoặc vượt chuẩn 2026)

| Hạng mục | Hiện trạng HEF | Đối chiếu xu hướng |
|---|---|---|
| Multi-role adversarial review | 7 roles (Orchestrator → 5 chuyên gia → Devil's Advocate) Fan-out/Fan-in | Khớp với "Adversarial Review" của BMAD Method và "reviewer agent phải tìm ra lỗi, 0 lỗi → halt" |
| Hierarchical AGENTS.md | Root <80 dòng, mỗi surface có AGENTS.md riêng, tránh primacy bias | Đúng chuẩn AGENTS.md 2026 (spec trung lập, không khoá vendor) |
| Context Engineering 3-tier | Tier 1 rules / Tier 2 repo map / Tier 3 file slice theo yêu cầu | Tương đồng kiến trúc Aider/SWE-Agent |
| Durable state (SQLite + CLI) | Story, decision, trace, backlog đều có bản ghi durable, query được | Vượt trội — hầu hết framework (Spec Kit, OpenSpec) chỉ lưu markdown, không có lớp truy vấn |
| Golden Paths theo archetype | 4 archetype (web/mobile/api/mobile-fullstack) với stack, CI, AGENTS.md riêng | Tương đương "preset" của Spec Kit (từ 2 → 12 preset cộng đồng) |
| Risk lanes (tiny/normal/high-risk) | Gate leo thang theo rủi ro, cần xác nhận người cho high-risk | Tương đồng "gates trước/sau mỗi phase" của Gangsta Agents |
| Parallel agent orchestration | Surface isolation, branch/agent, advisory file lock | Đúng nguyên tắc multi-agent orchestration hiện đại |
| Skills library | `skills/*/SKILL.md` tái sử dụng | Đúng chuẩn SKILL.md đang được Anthropic + cộng đồng chuẩn hoá |

**Kết luận:** đây không phải một bộ khung sơ khai — nó đã đạt mức trưởng thành ngang các framework SDD hàng đầu hiện nay (H2/H3 theo thang HARNESS_MATURITY của chính bạn). Phần việc còn lại là **vá các khoảng trống cụ thể**, không phải xây lại.

---

## 2. Các khoảng trống (gap) so với xu hướng mới nhất

### 2.1 Không có cơ chế "Context Reset / Session Handoff" cho phiên dài
Nghiên cứu harness dài-hạn mới nhất (Anthropic Engineering, và các dự án cộng đồng như `agent-handoff`, `harness-kit`) chỉ ra: **compaction tự động là chỗ agent "chết"** — sau khi bị tóm tắt, agent bắt đầu mâu thuẫn với quyết định trước đó, hoặc mất chi tiết quan trọng (lệnh test đang fail, quyết định kiến trúc, file đang dở). Giải pháp phổ biến nhất: agent tự viết một **handoff artifact** trước khi hết context/kết thúc phiên, phiên sau đọc file đó thay vì dựa vào compaction.

HEF hiện có thư mục `context/` **rỗng** và `CONTEXT_ENGINEERING.md` chỉ nói về ngân sách token trong 1 lượt, **chưa có quy trình xử lý khi hết context giữa chừng hoặc giữa 2 phiên**.

→ **Đề xuất:** thêm `core/SESSION_HANDOFF.md` (đã soạn sẵn bên dưới) + quy ước ghi file `context/handoff/<session-id>.md` mỗi khi: (a) sắp chạm ngưỡng token, (b) kết thúc phiên chưa xong task, (c) trước khi chuyển agent khác tiếp nhận.

### 2.2 Role review có nguy cơ "tự chấm điểm mình" (self-grading bias)
Nghiên cứu 2026 ghi nhận: agent được yêu cầu tự đánh giá công việc mình vừa làm có xu hướng khen ngợi dù chất lượng kém. HEF đã có Devil's Advocate (tốt), nhưng không có yêu cầu rõ ràng rằng **role đánh giá phải chạy trên context "sạch"** (chưa từng thấy quá trình implement) và **không có quyền Write/Edit**.

→ **Đề xuất:** bổ sung vào `roles/ROLE_SYSTEM.md` nguyên tắc "Fresh-Context Evaluator": mọi role review (đặc biệt QA/Security/Devil's Advocate) nhận diff + story packet, KHÔNG nhận transcript quá trình code; và cấu hình tool permission cho các role này ở chế độ read-only (không có Write/Edit) khi orchestration cho phép (Claude Code sub-agent, Agent SDK).

### 2.3 Tiêu chí "Done" đang là default-PASS ngầm định
Hiện `story verify` chạy lệnh và ghi PASS/FAIL — tốt — nhưng các tiêu chí định tính (a11y, UX, security review) trong role review không có ràng buộc "phải có bằng chứng mới được PASS". Xu hướng mới (harness-kit, Anthropic long-running harness) dùng **Default-FAIL contract**: mọi tiêu chí khởi tạo ở trạng thái FAIL, chỉ chuyển PASS khi có evidence (log, screenshot, output lệnh) đính kèm.

→ **Đề xuất:** thêm cột `evidence_ref` bắt buộc trong `templates/role-review-report.md` và `templates/validation-report.md`; role không được tick PASS nếu thiếu `evidence_ref`.

### 2.4 Bộ config mẫu KHÔNG generic — đang là config thật của 1 dự án
`harness.config.yaml` hiện tại chứa dữ liệu thật của dự án "dico-barcode/DICO Scan" (Java Spring Boot + Expo + React admin, forbidden_terms tiếng Việt về y tế...). `INSTALL.md` lại tham chiếu tới `harness.config.yaml.example` — file này **không tồn tại** trong bundle. Điều này mâu thuẫn trực tiếp với mục tiêu "chuẩn linh hoạt cho mọi loại dự án" của bạn: người dùng mới sẽ vô tình copy nguyên cấu hình và compliance rules của dự án y tế cũ.

→ **Đề xuất:** tách thành 2 file — `harness.config.yaml.example` (generic, placeholder, đã soạn sẵn bên dưới) và giữ bản hiện tại làm `examples/dico-scan.harness.config.yaml` (case study tham khảo).

### 2.5 `compliance/LEGAL_COMPLIANCE.md` bị khoá cứng vào domain y tế
File này là "Universal Compliance Guardrails" nhưng nội dung 100% viết cho app sức khoẻ/quét mã vạch (thuật ngữ cấm về y tế, GDPR/Nghị định 13). Với dự án web thương mại điện tử, backend nội bộ, hay app không liên quan sức khoẻ, file này không áp dụng được và có thể gây nhiễu agent.

→ **Đề xuất:** tách 2 lớp — `compliance/COMPLIANCE_FRAMEWORK.md` (nguyên tắc chung: không tuyên bố tuyệt đối, không giả danh chuyên gia được cấp phép, minh bạch dữ liệu — áp dụng mọi ngành) và `compliance/domain-packs/health-wellness.md`, `compliance/domain-packs/fintech.md`, `compliance/domain-packs/ecommerce.md`... nạp theo `project.domain` trong config.

### 2.6 Chưa có "Pre-flight Checklist" trước khi chạy agent không giám sát dài
Với các phiên agent chạy nhiều giờ không người giám sát (điều bạn cần cho CI/CD tự động), thực hành tốt nhất hiện nay là một checklist go/no-go trước khi thả agent chạy (quyền hạn, giới hạn ngân sách, lệnh bị cấm, điều kiện dừng khẩn).

→ **Đề xuất:** thêm `core/PREFLIGHT_CHECKLIST.md` — 7 điểm: (1) lane/risk đã phân loại, (2) phạm vi file/surface đã khoanh, (3) lệnh bash bị cấm đã cấu hình, (4) ngân sách token/thời gian tối đa, (5) điều kiện dừng khẩn (kill-switch), (6) ai review kết quả, (7) rollback plan.

### 2.7 Chưa có tầng "Boxed Bash" / permission model cho lệnh shell
`bash_tool`/CLI hiện không có allow/ask/deny list rõ ràng cho lệnh nguy hiểm (`rm -rf`, `curl | sh`, ghi vào `~/.ssh`, push force lên `main`...). Các harness hiện đại (Claude Code, Codex) đều có tầng permission 3 mức cho shell execution.

→ **Đề xuất:** thêm `core/SANDBOX_POLICY.md` định nghĩa allow/ask/deny theo lane (tiny=ask mọi lệnh ghi, normal=allow trong surface đã khoanh, high-risk=deny mọi lệnh network/production trừ khi có xác nhận người).

### 2.8 Token/hiệu năng: có ngân sách nhưng chưa có đo lường thực tế
`context_engineering.token_budget_max: 16000` là ngưỡng tĩnh, nhưng không có cơ chế đo token thực tế tiêu thụ theo role/task để tối ưu dần (đúng tinh thần "Growth Rule" mà bạn đã có cho friction).

→ **Đề xuất:** thêm cột `tokens_used`, `cost_usd_est` vào bảng `traces` (schema hiện có ở `scripts/schema/001-init.sql`) và một view `query cost` trong CLI để orchestrator tự học "role nào tốn token nhất trên loại task nào" → điều chỉnh việc có bật đủ 7 role hay rút gọn cho lane `tiny`.

### 2.9 Golden Paths thiếu 2 archetype phổ biến
Hiện có 4 archetype (web-fullstack, mobile-only, api-backend, mobile-fullstack) nhưng thiếu: `cli-tool` (đã nhắc trong config type nhưng chưa có archetype file), và `data-pipeline` / `background-worker` (batch job, queue consumer) — rất phổ biến trong CI/CD & backend hiện nay.

---

## 3. Ưu tiên hoá (Quick win → Đầu tư lớn)

| Ưu tiên | Hạng mục | Công sức | Tác động |
|---|---|---|---|
| 🔴 Ngay | 0. Audit/nguồn hoá `harness-cli.exe` | Trung bình | Bảo mật chuỗi cung ứng |
| 🔴 Ngay | 2.4 Tách `harness.config.yaml.example` generic | Thấp | Đúng mục tiêu "chuẩn mọi dự án" |
| 🟠 Tuần này | 2.1 `SESSION_HANDOFF.md` + `context/handoff/` | Trung bình | Giảm mất context giữa phiên dài |
| 🟠 Tuần này | 2.7 `SANDBOX_POLICY.md` | Thấp | An toàn khi chạy agent tự động |
| 🟡 Sau đó | 2.2 Fresh-context evaluator rule | Trung bình | Giảm thiên vị tự chấm điểm |
| 🟡 Sau đó | 2.3 Default-FAIL + evidence_ref | Trung bình | Tăng độ tin cậy review |
| 🟢 Dài hạn | 2.5 Tách compliance theo domain-pack | Cao | Mở rộng sang ngành khác |
| 🟢 Dài hạn | 2.8 Đo token/cost thực tế | Cao (cần đổi schema) | Tối ưu chi phí lâu dài |
| 🟢 Dài hạn | 2.9 Thêm archetype cli-tool, data-pipeline | Trung bình | Bao phủ nhiều loại dự án hơn |

---

## 4. Nguồn tham khảo (xu hướng GitHub/ngành, thời điểm 2026)

- Anthropic Engineering — "Harness design for long-running application development" (context reset vs compaction, fresh-context evaluator, planner/generator/evaluator split)
- GitHub Spec Kit, OpenSpec, BMAD Method, Gangsta Agents — so sánh SDD framework (HackerNoon, DEV Community, "Spec-Driven" repo, tháng 2–5/2026)
- AGENTS.md — chuẩn "README cho agent", trung lập vendor (morphllm.com, 2026)
- `awesome-agent-harness` (mahonzhan) — 6 thành phần nền tảng của harness: Filesystem/Git, Boxed Bash, Context Compaction, Lifecycle Hooks, Search, Multi-Agent Orchestration
- `agent-handoff` (zzyyfff), `harness-kit` (aimasteracc) — mẫu handoff artifact, pre-flight checklist, default-FAIL contract
- Mastra Blog — "Anatomy of a harness: building a coding agent that can run for hours" (observational memory, compaction thất thoát chi tiết)

*(Ghi chú: đây là các nguồn công khai để đối chiếu xu hướng, không phải trích dẫn nguyên văn — bạn nên tự kiểm tra lại số liệu/sao chép nếu cần dẫn nguồn chính thức.)*
