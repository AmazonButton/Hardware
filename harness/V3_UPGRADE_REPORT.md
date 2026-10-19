# Harness Engineering Framework — Đánh giá & Nâng cấp v3.0.0

> Đánh giá dựa trên bundle `harness-engineering v0.2.0` bạn upload (đã tự audit 1 lần
> trước đó — xem `GAP_ANALYSIS_AND_ROADMAP.md`), đối chiếu với khảo sát xu hướng
> harness/agentic-engineering tính đến 09/2026: chuẩn AGENTS.md (đã được Linux Foundation
> Agentic AI Foundation công nhận cùng MCP), Claude Code subagents/hooks/skills 2026,
> hệ sinh thái plugin marketplace, và các báo cáo tấn công chuỗi cung ứng qua skill.

---

## 1. Việc đã làm trong v3 (tóm tắt điều hành)

| # | Vấn đề | Trạng thái trước v3 | Đã làm trong v3 |
|---|---|---|---|
| 1 | `harness-cli.exe` binary không có mã nguồn | Gap bảo mật ưu tiên #1, mới chỉ có checksum-verify (không chứng minh an toàn) | **Gỡ bỏ hoàn toàn**, thay bằng `scripts/harness_cli.py` (Python stdlib, đã test end-to-end) + wrapper tương thích ngược |
| 2 | `install-harness.sh` tải 1 binary KHÁC từ GitHub releases mặc định | Chưa từng được nêu trong gap analysis trước — phát hiện mới khi đọc kỹ script | `INSTALL_RUST_CLI` đổi mặc định `1`→`0`, opt-in thay vì opt-out |
| 3 | Không có adapter theo công nghệ cụ thể — hardcode Java Spring Boot + Expo ngay trong `HIERARCHICAL_AGENTS.md` | Mâu thuẫn trực tiếp với mục tiêu "1 harness cho mọi dự án" | `stack-packs/` — 15 file mới (frontend/mobile/backend/deploy), nạp theo `surfaces.<x>.stack` |
| 4 | Roles chỉ là prose, không thực thi được trực tiếp trong runtime agent hiện đại | Đọc bằng mắt người/agent tự diễn giải | `.claude/agents/*.md` — subagent thật, `tools:` field ENFORCE Fresh-Context Evaluator (read-only cho reviewer) thay vì chỉ dặn trong prose |
| 5 | Không có tầng enforcement rõ ràng (rule sống ở đâu?) | Toàn bộ dựa vào prose trong AGENTS.md/roles | `core/HOOKS_AND_ENFORCEMENT.md` — Rule Ownership Matrix + mapping sang hook |
| 6 | Không có hướng dẫn an toàn cho hệ sinh thái skill (đang phát triển rất nhanh, có tấn công thực tế) | Chưa có | `core/SKILLS_SECURITY.md` |
| 7 | Domain-pack loading vẫn thủ công (known-gap từ CHANGELOG v0.2.0) | Chưa tự động hoá | `harness_cli.py packs resolve` — 1 lệnh in ra chính xác context cần nạp |
| 8 | MCP integration doc dừng ở danh sách 2025 | Thiếu Chrome DevTools MCP, deploy-target MCP, plugin marketplace | Bổ sung + cảnh báo review bundle plugin |

**Không làm lại từ đầu** — như v0.2.0 đã tự đánh giá đúng, phần lõi (multi-role adversarial
review, hierarchical AGENTS.md, durable SQLite state, risk lanes, parallel agent isolation)
đã ngang chuẩn ngành. v3 là **vá gap cụ thể + thêm lớp linh hoạt theo stack**, đúng tinh
thần yêu cầu của bạn.

---

## 2. Đối chiếu xu hướng 2026 (nguồn tham khảo)

Các phát hiện dưới đây đến từ khảo sát trực tiếp tại thời điểm thực hiện (không phải trích
dẫn nguyên văn — diễn giải lại, tự kiểm tra thêm nếu cần dẫn nguồn chính thức):

- **AGENTS.md đã trở thành chuẩn liên-vendor**, được Linux Foundation Agentic AI Foundation
  bảo trợ cùng Model Context Protocol (Anthropic) và Goose. Nhiều harness (Zed Agent,
  OpenHands, DeepSeek Harness...) đều nói cùng "ngôn ngữ" AGENTS.md, hội tụ nhanh trong vài
  tháng gần đây. Khớp với cách HEF đã dùng AGENTS.md phân cấp — v3 làm rõ thêm quan hệ với
  `CLAUDE.md` (chỉ nên là `@AGENTS.md` import, không lặp nội dung).
- **Subagents + hooks là 2 cơ chế thực thi (enforcement) chính** trong Claude Code 2026,
  tách biệt rõ với "kiến thức ngữ cảnh" (Skills). Nguyên tắc phổ biến: rule cần enforce →
  Hook/permission; kiến thức ngữ cảnh → Skill; ranh giới uỷ quyền → Subagent; hướng dẫn
  luôn-bật → AGENTS.md ngắn. v3 áp dụng nguyên tắc này thành `core/HOOKS_AND_ENFORCEMENT.md`.
- **Plugin marketplace** (bundle skill+subagent+hook+MCP, cài 1 lệnh) đang nở rộ nhanh —
  nhiều repo cộng đồng cung cấp hàng chục đến hàng trăm subagent/skill đóng gói sẵn. Tiện
  nhưng tăng bề mặt tấn công; v3 thêm mục cảnh báo review trong `MCP_INTEGRATIONS.md`.
- **Tấn công thực tế qua skill đã được ghi nhận công khai** (skill "vô hại" bị lợi dụng cài
  ransomware qua 1 bước cuối trong quy trình nhiều bước; chỉ dẫn ẩn bằng Unicode trong nội
  dung skill). Đây là lý do `core/SKILLS_SECURITY.md` được thêm — framework trước đó hoàn
  toàn chưa đề cập rủi ro này dù đã có `skills/` từ v0.2.0.
- **Framework SDD khác (Spec Kit, OpenSpec, BMAD, harness-kit...) đều phân phối dưới dạng mã
  nguồn/npm package audit được, không binary đóng gói sẵn** — xác nhận lại đánh giá đã có
  trong `GAP_ANALYSIS_AND_ROADMAP.md` gốc, và là lý do chính khiến việc gỡ `harness-cli.exe`
  là ưu tiên #1 thay vì chỉ thêm checksum-verify.

---

## 3. Cách dùng harness này linh hoạt cho nhiều loại dự án (đúng yêu cầu ban đầu)

Với v3, quy trình cho 1 dự án MỚI (web, mobile, backend, hay kết hợp) là:

1. Chọn **Golden Path** gần nhất với topology dự án (`golden-paths/archetypes/*.md`) — vd
   `web-fullstack` nếu có cả frontend + backend, `mobile-only` nếu chỉ app di động.
2. Copy `harness.config.yaml.example` → `harness.config.yaml`, khai `surfaces.<x>.stack`
   cho từng surface thực sự có (không cần khai surface không dùng).
3. Chạy `./scripts/bin/harness-cli packs resolve` — CLI in ra chính xác:
   - `compliance/domain-packs/<domain>.md` nếu dự án thuộc domain đặc thù (fintech, y tế...)
   - `stack-packs/<nhóm>/<stack>.md` cho mỗi surface đang bật
4. Viết surface `AGENTS.md` theo khuôn mẫu trong `agent-instructions/HIERARCHICAL_AGENTS.md`
   — trỏ tới stack-pack, không copy-paste lại.
5. Nếu runtime là Claude Code/ACP-compatible: copy `.claude/agents/*.md` vào dự án — 7 role
   review đã sẵn sàng dùng như subagent thật, đúng quyền hạn theo Fresh-Context Evaluator.
6. Chạy vòng review chuẩn (`roles/ROLE_SYSTEM.md` Fan-Out/Fan-In) cho mọi thay đổi lane
   `normal`/`high-risk`.

Kết quả: cùng 1 bộ file harness-engineering, dùng được cho dự án React+Node, Vue+Go,
Flutter+FastAPI, hay CLI tool thuần Python — chỉ khác `harness.config.yaml` và stack-pack
nào được nạp, không cần sửa file harness nào khác.

---

## 4. Known gaps còn lại sau v3 (trung thực, không giấu)

Xem chi tiết trong `CHANGELOG.md § v3.0.0 § Known gaps`. Tóm tắt 3 điểm quan trọng nhất:

1. `scripts/install-harness.sh` là installer tự chứa (nội dung nhúng sẵn trong script, không
   copy từ cây thư mục nguồn) — **chưa được cập nhật để tự nhúng `stack-packs/`,
   `.claude/agents/`, và các file mới khác** khi cài vào 1 dự án hoàn toàn mới. Đây là công
   việc effort cao (viết lại phần lớn 1 script ~24KB) không nằm trong phạm vi lượt nâng cấp
   này. Giải pháp tạm: copy trực tiếp `harness-engineering/` (submodule hoặc copy nguyên
   thư mục) thay vì chạy installer, cho tới khi installer được viết lại.
2. `_load_yaml_minimal()` trong `harness_cli.py` là parser tối giản tự viết, không phải YAML
   engine đầy đủ — đủ cho các field phẳng hiện dùng, nhưng sẽ vỡ với YAML phức tạp hơn
   (anchors, multi-line block scalars). Nếu cần, thay bằng `pyyaml`.
3. Hook mapping trong `core/HOOKS_AND_ENFORCEMENT.md` là bản đồ khái niệm, không phải hook
   script chạy ngay — bề mặt hook API của agent runtime thay đổi theo phiên bản, cần tự
   nối dây theo tài liệu hiện hành tại thời điểm triển khai.

---

## 5. Đề xuất bước tiếp theo (nếu muốn đầu tư thêm, không bắt buộc)

- Viết lại `install-harness.sh` theo mô hình copy-from-source-tree thay vì nhúng nội dung,
  để mọi file mới trong repo harness tự động có mặt khi cài — giảm rủi ro "docs nói có
  nhưng installer không cài".
- Viết hook script thật (không chỉ skeleton) cho ít nhất 1 gate quan trọng nhất
  (`core/SANDBOX_POLICY.md` deny-list) để chứng minh mapping trong
  `HOOKS_AND_ENFORCEMENT.md` chạy được thật, không chỉ là tài liệu.
- Thêm CI job chạy `python3 scripts/harness_cli.py` qua 1 vòng smoke test (giống phần đã
  test thủ công ở đây) để tránh regress khi có người sửa script sau này.
