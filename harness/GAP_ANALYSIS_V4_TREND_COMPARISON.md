# Đánh giá khắc khe v4 — Đối chiếu với GitHub Trend (Spec Kit, BMAD-METHOD, OpenSpec)

> Yêu cầu: "phân tích khắt khe" + "đi tiên phong". Tài liệu này KHÔNG tô hồng — nêu thẳng
> những gì harness-engineering còn thua các framework hàng đầu, việc gì v4 đã vá, và việc
> gì CHƯA vá dù đã biết.

## 1. Bảng chấm điểm đối đầu (thang 0-5, đánh giá tại thời điểm v3.0.0, trước khi vá v4)

| Tiêu chí | Spec Kit (~90k★) | BMAD-METHOD (~37k★) | OpenSpec | harness-engineering v3.0.0 |
|---|---|---|---|---|
| Pha Discovery/Planning trước code (PRD/Architecture/UX) | 5 — 4 pha rõ ràng: Constitution→Specify→Plan→Tasks→Implement | 5 — Analyst→PM→Architect→UX→Solutioning Gate | 3 — chỉ delta-spec, không PRD/Architecture đầy đủ | **0 — hoàn toàn không có, giả định story packet đã tồn tại sẵn** |
| Scale-adaptive planning depth theo độ phức tạp | 4 (qua checklist/clarify) | 5 — 3 track: Quick Flow / Standard / Enterprise | 2 | **0** |
| Multi-role adversarial review SAU khi có code | 3 (checklist quality gate) | 4 (QA + Test Architect) | 2 | **5 — đây là điểm mạnh nhất của harness-engineering, vượt cả 3 framework trên** |
| Constitution/immutable-principles document | 5 (`constitution.md`, bất biến) | 4 (`project-context.md`) | 3 | **0 → v4: `CONSTITUTION.md`** |
| Native slash-command / CLI init 1 lệnh | 5 (`npx specify init`, `/speckit.*`) | 5 (`npx bmad-method install`, `/pm`, `/architect`...) | 4 | **1 — có CLI (Python) nhưng không có slash-command layer, không có `init` tạo project mới từ đầu bằng 1 lệnh** |
| Đa dự án / đa stack (không khoá 1 công nghệ) | 5 (agent-agnostic, tool-agnostic) | 4 (game dev + software dev modules riêng) | 4 | **2 → v3: `stack-packs/` đã vá phần lớn, nhưng vẫn thiếu "init tạo dự án mới"** |
| Supply-chain / bảo mật CLI | Không rõ công khai risk tương đương | Cài qua npx — dependency chain của npm | N/A | **v0.2.0: 1/5 (binary không nguồn) → v3: 5/5 (Python source-available)** |
| Governance / audit trail cho thay đổi lớn | 3 | 4 (artifact-driven, dùng làm tài liệu onboarding) | **5 — đây là điểm mạnh riêng của OpenSpec, thiết kế riêng cho compliance** | 3 (`compliance/` framework có, nhưng không gắn với 1 quy trình approval hình thức) |
| Cộng đồng / độ trưởng thành (số sao, số fork, người dùng thực) | 5 (90k★, 8k+ fork) | 5 (37k★) | 3 | **0 — đây là bộ nội bộ, chưa public, không có dữ liệu cộng đồng để so** |
| Context engineering (document sharding, tránh phình context) | 3 | 5 (`*shard-doc` command, chia PRD lớn thành story-level) | 3 | 4 (`core/CONTEXT_ENGINEERING.md`, `core/SESSION_HANDOFF.md` đã khá tốt) |

**Kết luận trung thực**: trước v4, harness-engineering v3.0.0 vượt trội ở phần **review sau
khi có code** (7-role adversarial fan-out/fan-in là thứ CHƯA framework nào ở trên làm sâu
bằng) và vừa vá xong phần **linh hoạt đa stack + bảo mật CLI**. Nhưng **thua toàn diện** ở
phần đầu SDLC — Discovery/Planning/Architecture — vốn là phần BMAD và Spec Kit đầu tư nhiều
nhất và là lý do chính khiến 2 framework đó dẫn đầu trend. Nói "harness-engineering là bộ
quy trình fullstack hoàn chỉnh" mà thiếu hẳn pha này là **không đúng sự thật** — đây là lý
do v4 ưu tiên vá đúng điểm này thay vì làm thêm việc khác.

---

## 2. Việc đã vá trong v4

| # | Gap | Đã làm |
|---|---|---|
| 1 | Không có Discovery/Planning/Architecture trước code | `discovery/` mới: README + 5 template (Brief/PRD/Architecture/UX Spec/Gate Report) |
| 2 | Không có Constitution bất biến | `CONSTITUTION.md` mới ở root |
| 3 | Không có Solutioning Gate hình thức (PASS/CONCERNS/FAIL) | `scripts/schema/004-solutioning-gate.sql` + `harness_cli.py gate add/decide` + `query gates` — đã test |
| 4 | Không có scale-adaptive planning track | Track `quick_flow`/`standard`/`enterprise` — **tái dùng risk lane hiện có**, không tạo khái niệm song song gây rối |
| 5 | Không có role cho Planning (chỉ có role review code) | 3 role mới: `product-analyst`, `architect`, `ux-designer` (+ subagent `.claude/agents/`) |
| 6 | Không có slash-command layer như Spec Kit/BMAD | `.claude/commands/harness-brief.md`, `harness-prd.md`, `harness-architecture.md`, `harness-ux.md`, `harness-solutioning-gate.md` |
| 7 | Mental model (`core/HARNESS.md`) bỏ qua hoàn toàn pha planning | Sơ đồ cập nhật: Intake → (rẽ nhánh) Discovery/Planning/Gate → Story packet |

---

## 3. CHƯA vá trong v4 (nói thẳng, không giấu)

1. **Không có `init` 1 lệnh tạo dự án mới từ đầu** như `npx specify init` hay
   `npx bmad-method install`. `scripts/install-harness.sh` cài harness VÀO 1 dự án đã tồn
   tại, nhưng không có wizard hỏi-đáp để sinh `CONSTITUTION.md`/`harness.config.yaml` lần
   đầu. Đây là gap UX lớn nhất còn lại — cần 1 lượt riêng để làm đúng (không phải việc nhỏ).
2. **Solutioning Gate hiện là bản đồ tài liệu + CLI, chưa có CI gate thật** chặn story
   packet được tạo nếu gate chưa `pass` — hiện dựa vào agent tự giác đọc `discovery/README.md`.
   Nên bổ sung 1 hook/CI check đọc `harness.db` để chặn thật (xem
   `core/HOOKS_AND_ENFORCEMENT.md` — nguyên tắc đã có, chưa nối dây cho gate cụ thể này).
3. **Không có document sharding tự động** như BMAD's `*shard-doc` — PRD/Architecture lớn
   vẫn phải tự tay hoặc tự agent chia nhỏ khi đưa vào context, không có tool hỗ trợ.
4. **Chưa có test thực tế trên 1 dự án thật** (BMAD/Spec Kit có hàng nghìn người dùng thực
   trải nghiệm và báo lỗi liên tục) — mọi thứ ở đây mới dừng ở "logic đúng + smoke test CLI
   pass", chưa được stress-test qua sử dụng thật dài hạn.
5. **`install-harness.sh` vẫn chưa được viết lại** để tự động nhúng `discovery/`,
   `CONSTITUTION.md`, `.claude/commands/`, 3 role mới khi cài vào dự án mới — known gap đã
   nêu từ v3, vẫn tồn tại.

---

## 4. Vì sao KHÔNG copy nguyên BMAD/Spec Kit thay vì tự vá

Cân nhắc thẳng thắn: có thể đề xuất "bỏ harness này, dùng BMAD hoặc Spec Kit". Nhưng:
- BMAD/Spec Kit **không có** cơ chế review-sau-code sâu bằng 7-role fan-out/fan-in +
  Fresh-Context Evaluator + Default-FAIL evidence_ref của harness-engineering — đây là phần
  giá trị nhất của bộ này, không nên đánh đổi.
- harness-engineering có `compliance/domain-packs/`, `core/SANDBOX_POLICY.md`,
  `core/COST_TRACKING.md` — các mối quan tâm vận hành thực tế (chi phí, sandbox, compliance
  theo domain) mà Spec Kit/BMAD không native hỗ trợ.
- Việc đúng là **học cấu trúc pha Planning từ BMAD/Spec Kit** (đã làm ở v4) trong khi **giữ
  nguyên phần review mạnh hơn** — không phải thay thế toàn bộ.

---

## 5. Đề xuất ưu tiên cho lượt tiếp theo (nếu tiếp tục đầu tư)

1. Viết `scripts/init-project.py` (hoặc mở rộng `install-harness.sh`) — wizard hỏi-đáp sinh
   `CONSTITUTION.md` + `harness.config.yaml` lần đầu, tương đương trải nghiệm `specify init`.
2. Nối 1 CI check thật cho Solutioning Gate (không chỉ tài liệu) — ví dụ: job CI đọc
   `harness.db`, fail nếu có story packet nào không truy vết được về 1 gate `pass` (khi
   track không phải `quick_flow`).
3. Test toàn bộ luồng v4 trên ít nhất 1 dự án ví dụ thật trong `examples/` (hiện chỉ có
   `dico-scan` — chưa chạy qua pha Discovery/Planning mới).
