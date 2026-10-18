# Changelog

## v4.0.0 — Discovery/Planning Phase (vá gap lớn nhất so với GitHub trend)

Xem đánh giá đầy đủ trong [`GAP_ANALYSIS_V4_TREND_COMPARISON.md`](./GAP_ANALYSIS_V4_TREND_COMPARISON.md).

Đối chiếu khắt khe với Spec Kit (~90k★), BMAD-METHOD (~37k★), OpenSpec: harness-engineering
v3.0.0 mạnh nhất ngành ở phần review-sau-code (7-role fan-out/fan-in) nhưng **hoàn toàn
không có** pha Discovery/Planning/Architecture trước code mà 2 framework dẫn đầu kia đều
coi là trung tâm. v4 vá đúng gap này.

### Added
- `discovery/` — pha mới: Project Brief → PRD → (UX Spec song song) → Architecture →
  Solutioning Gate, xảy ra TRƯỚC story packet cho intake `new_spec`/`new_initiative` track
  `standard`/`enterprise`. Track `quick_flow` (= lane `tiny`) bỏ qua toàn bộ, giữ đúng tinh
  thần "đừng biến việc nhỏ thành overhead" học từ điểm yếu bị phê bình ở BMAD.
- `CONSTITUTION.md` (root) — nguyên tắc bất biến của dự án, tương đương `constitution.md`
  (Spec Kit) / `project-context.md` (BMAD). Mọi artifact Discovery/Planning phải nhất quán.
- 3 role Planning mới: `roles/product-analyst.md`, `roles/architect.md`, `roles/ux-designer.md`
  (+ subagent tương ứng trong `.claude/agents/`) — khác nhóm 7 role review cũ: SINH artifact
  thay vì chỉ review diff.
- `scripts/schema/004-solutioning-gate.sql` + `harness_cli.py gate add/decide` +
  `query gates` — đã test end-to-end. Solutioning Gate là checkpoint hình thức
  PASS/CONCERNS/FAIL, Default-FAIL + evidence_ref, giữ nguyên tắc Fresh-Context Evaluator
  (Architect ra verdict, không phải Product Analyst tự chấm PRD của chính mình).
- `.claude/commands/harness-{brief,prd,architecture,ux,solutioning-gate}.md` — slash-command
  layer, bắt kịp UX của `/speckit.*` (Spec Kit) và `/pm`, `/architect` (BMAD).
- `GAP_ANALYSIS_V4_TREND_COMPARISON.md` — bảng chấm điểm đối đầu Spec Kit/BMAD/OpenSpec,
  nêu rõ cả điểm mạnh lẫn điểm còn thua, không tô hồng.
- `core/HARNESS.md` mental model cập nhật — Discovery/Planning rẽ nhánh trước "Story packet",
  không ảnh hưởng luồng cũ cho change_request/maintenance.
- `roles/ROLE_SYSTEM.md` — thêm bảng "3 Planning Roles" bên cạnh "7 Roles" review cũ.

### Known gaps sau v4 (chi tiết trong GAP_ANALYSIS_V4_TREND_COMPARISON.md § 3)
- Chưa có `init` 1 lệnh tạo dự án mới từ đầu (wizard sinh CONSTITUTION.md/config lần đầu)
  như `npx specify init`/`npx bmad-method install` — gap UX lớn nhất còn lại.
- Solutioning Gate chưa có CI check thật chặn story packet nếu gate chưa `pass` — mới dừng
  ở tài liệu + CLI, dựa vào agent tự giác.
- `install-harness.sh` vẫn chưa nhúng tự động `discovery/`, `CONSTITUTION.md`,
  `.claude/commands/`, 3 role mới khi cài dự án mới (known gap kế thừa từ v3).
- Chưa test qua 1 dự án ví dụ thật với luồng Discovery/Planning mới.

---

## v3.0.0 — Flexibility + Supply-Chain Hardening Pass (2026)

Xem đánh giá đầy đủ trong [`V3_UPGRADE_REPORT.md`](./V3_UPGRADE_REPORT.md).

### 🔴 Breaking / Security
- **`scripts/bin/harness-cli.exe` (binary Rust không có mã nguồn) đã bị GỠ BỎ.** Thay thế
  bằng `scripts/harness_cli.py` — pure Python stdlib, source-available, đã test end-to-end
  (init/intake/story/decision/trace/trace-cost/query/packs resolve). `scripts/bin/harness-cli`
  và `.ps1` giờ là wrapper mỏng gọi Python script, giữ nguyên interface cho mọi lệnh đã có
  trong docs — không cần sửa lời gọi lệnh ở nơi khác.
- `scripts/install-harness.sh`: `INSTALL_RUST_CLI` đổi mặc định từ `1` → `0`. Trước đây
  installer TẢI một binary khác từ GitHub releases của 1 repo ngoài theo mặc định — rủi ro
  chuỗi cung ứng còn nặng hơn cả binary bundle sẵn. Giờ là opt-in, tắt mặc định.
- `scripts/checks/check_cli_checksum.sh` viết lại: không còn "gate bắt buộc trước khi chạy
  binary" (vì giờ là source), chỉ còn là tripwire tuỳ chọn phát hiện sửa đổi cục bộ.

### Added — Flexibility across stacks (yêu cầu chính của v3)
- `stack-packs/` — 15 adapter mới: frontend (React/Vue/Svelte), mobile (React Native/
  Flutter/native iOS/native Android), backend (Node/Python/Go/Java), deploy (Docker+K8s/
  Serverless/Static-Edge/Mobile-store-release). Nạp theo `surfaces.<x>.stack` trong config,
  y hệt cơ chế `compliance/domain-packs/*.md` theo `project.domain`. `stack-packs/_TEMPLATE.md`
  cho stack chưa có sẵn.
- `scripts/harness_cli.py packs resolve` — đọc `harness.config.yaml`, in ra chính xác
  domain-pack + stack-pack nào cần nạp cho phiên làm việc hiện tại (giải quyết known-gap
  "chưa tự động hoá việc load domain-packs" từ CHANGELOG v0.2.0).
- `golden-paths/README.md § 1.5` — làm rõ Golden Path (topology) tách khỏi Stack Pack
  (công nghệ cụ thể): 1 archetype giờ dùng được với bất kỳ tổ hợp stack-pack nào.
- `agent-instructions/HIERARCHICAL_AGENTS.md` — bỏ các template hardcode 1 stack cụ thể
  (Java Spring Boot, Expo, React) từng mâu thuẫn với mục tiêu "harness cho mọi dự án";
  thay bằng khuôn mẫu trỏ sang `stack-packs/`. Thêm phần AGENTS.md ↔ CLAUDE.md (chuẩn
  ngành 2026: CLAUDE.md chỉ còn là `@AGENTS.md` import + phần riêng của Claude Code).

### Added — Native agent runtime integration
- `.claude/agents/*.md` — 7 role trong `roles/*.md` giờ có bản subagent thật (YAML
  frontmatter: `tools`, `model`) dùng trực tiếp được với Claude Code/ACP-compatible harness.
  Reviewer roles bị khoá tools read-only theo đúng Fresh-Context Evaluator principle đã có
  trong `roles/ROLE_SYSTEM.md` — giờ được ENFORCE bằng cấu hình, không chỉ nằm trong prose.
- `core/HOOKS_AND_ENFORCEMENT.md` — Rule Ownership Matrix (rule nào sống ở AGENTS.md vs
  Skill vs Subagent vs Hook) + mapping cụ thể từng gate của harness sang hook event.
- `core/SKILLS_SECURITY.md` — checklist an toàn chuỗi cung ứng cho skill bên thứ ba, dựa
  trên các dạng tấn công đã ghi nhận trong hệ sinh thái skill 2025–2026.
- `mcp/MCP_INTEGRATIONS.md` — bổ sung Chrome DevTools MCP, deploy-target MCP, issue-tracker
  MCP, và mục Plugin Marketplace mới (bundle skill+subagent+hook+MCP, kèm cảnh báo review).

### Changed
- `harness.config.yaml` và `.example`: thêm field `harness_framework_version: "3.0.0"`.

### Known gaps chưa xử lý trong v3 (theo dõi tiếp)
- `scripts/install-harness.sh` là installer tự chứa (nội dung nhúng, không copy từ source
  tree) — chưa cập nhật để tự động nhúng `stack-packs/`, `.claude/agents/`, các file mới
  khác khi cài vào 1 dự án hoàn toàn mới từ đầu. Tạm thời: copy thủ công các thư mục mới
  này sau khi chạy installer, hoặc submodule/copy nguyên `harness-engineering/` thay vì
  dùng installer cho tới khi installer được cập nhật.
- `scripts/harness_cli.py` dùng 1 YAML reader tối giản tự viết (không phụ thuộc PyYAML) —
  đủ cho các field phẳng mà `packs resolve` cần, nhưng KHÔNG phải parser YAML đầy đủ. Nếu
  `harness.config.yaml` dùng cấu trúc YAML phức tạp hơn (anchors, multi-line strings), cài
  thêm `pyyaml` và thay thế `_load_yaml_minimal()`.
- Hook skeleton trong `core/HOOKS_AND_ENFORCEMENT.md` là bản đồ, chưa phải hook script chạy
  được ngay — cần nối dây theo đúng hook API hiện hành của runtime bạn dùng.

---

## v0.2.0 — 2026 Upgrade Pass (Gap Analysis & Hardening)

Xem chi tiết đầy đủ trong [`GAP_ANALYSIS_AND_ROADMAP.md`](./GAP_ANALYSIS_AND_ROADMAP.md).

### Added
- `core/SESSION_HANDOFF.md` — protocol bàn giao ngữ cảnh giữa các phiên/agent, giải quyết
  vấn đề mất chi tiết khi context bị compact hoặc phiên bị ngắt giữa chừng.
- `core/SANDBOX_POLICY.md` — bảng allow/ask/deny cho lệnh shell theo risk lane.
- `core/PREFLIGHT_CHECKLIST.md` — 7 điểm go/no-go trước khi chạy agent không giám sát.
- `core/COST_TRACKING.md` + `scripts/schema/003-cost-tracking.sql` — đo token/cost theo
  từng role để tối ưu dần việc có nên kích hoạt đủ 7 role hay không cho từng lane.
- `compliance/COMPLIANCE_FRAMEWORK.md` — nguyên tắc pháp lý/compliance phổ quát, tách
  khỏi domain y tế.
- `compliance/domain-packs/{health-wellness,fintech,ecommerce}.md` — mở rộng theo ngành,
  nạp thêm qua `harness.config.yaml → compliance.domain_pack`.
- `golden-paths/archetypes/{cli-tool,data-pipeline}.md` — 2 archetype mới.
- `harness.config.yaml.example` — bản config generic thật sự, không dính dữ liệu 1 dự án.
- `examples/dico-scan.harness.config.yaml` — case study (là file `harness.config.yaml`
  gốc trước khi tách).
- `scripts/bin/SECURITY_NOTE.md` + `scripts/checks/check_cli_checksum.sh` — cảnh báo và
  công cụ verify checksum cho `harness-cli` binary (chưa có mã nguồn kèm theo).
- `roles/ROLE_SYSTEM.md § Fresh-Context Evaluator Principle` — giảm thiên vị tự chấm
  điểm, yêu cầu Default-FAIL contract.
- `templates/role-review-report.md`, `templates/validation-report.md` — thêm cột/field
  `Evidence Ref` bắt buộc.
- `context/handoff/` — thư mục hoạt động thật cho session handoff (trước đây `context/`
  hoàn toàn rỗng).

### Changed
- `harness.config.yaml` ở gốc nay là bản generic (giống `.example`), KHÔNG còn là config
  thật của dự án "dico-barcode" — tránh dự án mới vô tình kế thừa cấu hình/compliance
  của dự án khác.
- `compliance/LEGAL_COMPLIANCE.md` nay chỉ còn là pointer sang cấu trúc mới (không xoá
  hẳn để không gãy các tham chiếu cũ).

### Known gaps chưa xử lý trong lần này (xem roadmap, mục 2.9 và phần "dài hạn")
- `harness-cli` vẫn là binary, chưa có mã nguồn Rust trong repo — checksum chỉ phát
  hiện thay đổi, không chứng minh an toàn tuyệt đối.
- Lệnh CLI mới được nhắc tới trong tài liệu (`trace-cost add`, `query cost`) **chưa
  được binary hiện tại hỗ trợ** — cần cập nhật mã nguồn CLI thật sự. Cho tới lúc đó,
  ghi tạm dữ liệu cost vào `trace.notes` dạng JSON.
- Chưa tự động hoá việc load `domain-packs/*.md` vào context theo `project.domain` —
  hiện là quy ước thủ công (agent tự đọc file tương ứng khi thấy config), chưa có
  script enforce.
