# Discovery & Planning Phase (v4 — mới)

> **Vì sao thêm phần này**: khảo sát các harness/spec-driven framework đang dẫn đầu GitHub
> trend 2026 (GitHub Spec Kit ~90k+ sao, BMAD-METHOD ~37k+ sao, OpenSpec) cho thấy điểm
> chung mà `harness-engineering` v3 CHƯA có: một pha **Discovery → Planning → Solutioning
> Gate** tường minh, xảy ra TRƯỚC khi story packet/implementation bắt đầu. v3 trở về trước
> giả định story packet đã tồn tại sẵn (`core/HARNESS.md`: Human intent → Feature intake →
> Story packet) — bỏ qua toàn bộ phần "biến 1 ý tưởng mơ hồ thành PRD/Architecture/UX spec
> đã được thẩm định", vốn là phần dễ sai nhất khi làm việc với sáng kiến mới/lớn.

## Mental model mới (thay thế đoạn đầu `core/HARNESS.md`)

```text
Human intent
     |
     v
Intake classification (harness_cli.py intake)  --- input_type = new_spec | new_initiative?
     |                                               NO → đi thẳng "Feature intake" cũ
     v YES
Planning Track selection (Quick Flow / Standard / Enterprise)
     |
     v
Discovery: Project Brief  (roles/product-analyst.md)
     |
     v
Planning: PRD  (roles/product-analyst.md)  [+ UX Spec song song nếu có UI — roles/ux-designer.md]
     |
     v
Solutioning: Architecture doc  (roles/architect.md)
     |
     v
Solutioning Gate  ── verdict: PASS / CONCERNS / FAIL  (harness_cli.py gate decide)
     |                                    |
     | PASS                              | FAIL/CONCERNS nghiêm trọng
     v                                    v
Story packets (core/HARNESS.md cũ tiếp tục từ đây)      Quay lại Planning, không tạo story
```

## Planning Track — scale-adaptive (tái dùng risk lane, không tạo khái niệm mới)

| Track | Map với risk lane | Khi nào dùng | Artifact bắt buộc |
|---|---|---|---|
| `quick_flow` | `tiny` | Bug fix, thay đổi nhỏ, phạm vi đã rõ | Không cần — bỏ qua toàn bộ Discovery/Planning, vào thẳng story packet |
| `standard` | `normal` | Feature/sản phẩm mới, phạm vi vừa | Project Brief + PRD (+ UX Spec nếu có UI) + Architecture |
| `enterprise` | `high_risk` | Sáng kiến lớn, nhiều surface, compliance | Standard + Security review sớm (Security Auditor đọc PRD trước khi Architecture chốt) + Architecture Decision Records cho mọi lựa chọn lớn |

> Track này KHÔNG thay thế risk lane hiện có (`core/HARNESS.md`) — nó là bước xảy ra
> **trước** khi risk lane áp dụng cho từng story cụ thể được tách ra từ initiative.

## Khi nào BỎ QUA phase này (quan trọng — đừng biến mọi task thành BMAD-style overhead)

- `intake --type change_request` hoặc `maintenance` → **luôn bỏ qua**, đi thẳng story packet
  như v3 (đây vẫn là phần lớn công việc ngày-qua-ngày).
- `intake --type new_spec`/`new_initiative` nhưng phạm vi nhỏ, rõ ràng → track `quick_flow`,
  vẫn bỏ qua Discovery/Planning artifact, chỉ ghi tech-spec ngắn trực tiếp trong story packet.
- Chỉ dùng track `standard`/`enterprise` khi thực sự có sự mơ hồ cần làm rõ TRƯỚC khi code —
  đây là bài học rút ra từ chính điểm yếu bị phê bình ở BMAD (overhead nặng cho task nhỏ).

## Cách chạy (agent hoặc người)

```bash
# 1. Intake xác định đây là new_initiative
./scripts/bin/harness-cli intake --type new_initiative --summary "Checkout redesign" --lane normal

# 2. Tạo gate record, chọn track
./scripts/bin/harness-cli gate add --summary "Checkout redesign" --track standard

# 3. Kích hoạt role tương ứng để viết từng artifact (xem discovery/templates/)
#    - roles/product-analyst.md  -> discovery/briefs/checkout.md, discovery/prds/checkout.md
#    - roles/ux-designer.md      -> discovery/ux-specs/checkout.md (nếu có UI)
#    - roles/architect.md        -> discovery/architecture/checkout.md

# 4. Solutioning Gate — CHÍNH role Architect (không phải người viết PRD) ra quyết định,
#    tương tự Fresh-Context Evaluator principle áp dụng cho review code (roles/ROLE_SYSTEM.md)
./scripts/bin/harness-cli gate decide 1 --verdict pass --reviewed-by architect \
  --story-ids '["US-101","US-102"]'

# 5. Chỉ khi verdict = pass, mới tạo story packet và đi vào vòng review chuẩn (roles 7 vai cũ)
./scripts/bin/harness-cli story add --id US-101 --title "..." --lane normal
```

## Artifact templates

- `discovery/templates/PROJECT_BRIEF.md` — vấn đề, đối tượng, mục tiêu đo được, phạm vi/ngoài phạm vi.
- `discovery/templates/PRD.md` — yêu cầu chức năng + phi chức năng, user stories cấp cao.
- `discovery/templates/ARCHITECTURE.md` — tech stack, data model, source tree, ADR liên kết,
  liệt kê chính xác `stack-packs/` nào sẽ dùng cho từng surface (nối `discovery` với
  `stack-packs/` — xem `stack-packs/README.md`).
- `discovery/templates/UX_SPEC.md` — luồng người dùng, wireframe mô tả bằng text/ASCII,
  design tokens tham chiếu (không tạo lại — trỏ tới design system hiện có nếu có).
- `discovery/templates/SOLUTIONING_GATE_REPORT.md` — checklist PASS/CONCERNS/FAIL, theo
  đúng khuôn Default-FAIL + `evidence_ref` như `templates/role-review-report.md`.

## Liên hệ với `CONSTITUTION.md` (mới, ở root)

`CONSTITUTION.md` là tập nguyên tắc bất biến (ít thay đổi) của dự án — tương đương
`constitution.md` trong Spec Kit hoặc `project-context.md` trong BMAD. MỌI artifact trong
`discovery/` phải nhất quán với `CONSTITUTION.md`; nếu PRD/Architecture mâu thuẫn với
constitution, đó là lý do hợp lệ để Solutioning Gate trả verdict `fail`.
