# Hierarchical AGENTS.md Architecture

## AGENTS.md vs CLAUDE.md (v3 — chuẩn ngành 2026)

`AGENTS.md` là chuẩn **harness-trung lập** (không khoá vendor) được cộng đồng và nhiều nhà
cung cấp agent hội tụ về, giờ được xem là "README cho agent". Nếu dự án của bạn cũng có
`CLAUDE.md` (Claude Code đọc riêng), quy ước v3:

- `AGENTS.md` là **nguồn sự thật duy nhất**. Viết rule vào đây.
- `CLAUDE.md` chỉ còn là `@AGENTS.md` import + phần THỰC SỰ chỉ áp dụng cho Claude Code
  (vd cấu hình subagent cụ thể, hook path). KHÔNG lặp lại nội dung đã có trong AGENTS.md —
  nếu thấy mình đang viết "nhớ đồng bộ 2 file này", đó là dấu hiệu đang tách sai.
- Không commit secrets vào 1 trong 2 file — cả 2 đều tracked trong git.

```markdown
# CLAUDE.md
@AGENTS.md

## Claude Code-only
- Subagents: xem .claude/agents/*.md
- Hooks: xem core/HOOKS_AND_ENFORCEMENT.md
```

## Why Hierarchical?

> **2026 Industry Finding**: A single long root `AGENTS.md` causes **primacy bias** — AI agents disproportionately weight content at the top, effectively ignoring rules at the bottom. Files over 100-150 lines are measurably less effective.

**Solution**: A hierarchy where:
- Root `AGENTS.md` = global rules only (< 80 lines)  
- Surface `AGENTS.md` = surface-specific rules in their directory

---

## Structure

```
/AGENTS.md                        ← global: governance, project compliance, lane policy
/[backend-dir]/AGENTS.md          ← backend: API, DB, migration, test commands
/[mobile-dir]/AGENTS.md           ← mobile: Mobile client, emulator, release pipeline
/[frontend-dir]/AGENTS.md         ← web/admin: Frontend UI, components, design tokens
/infrastructure/AGENTS.md         ← infra: Docker, Cloud, reverse proxy, secrets
/harness-engineering/AGENTS.md    ← harness: how to evolve the harness itself
```

---

## Root AGENTS.md — Content Rules

Keep root `AGENTS.md` to **< 80 lines**. It should contain ONLY:

1. **Legal/compliance rules** (critical, project-wide)  
2. **Harness reference** (where to find framework docs)  
3. **Lane classification** (tiny/normal/high-risk triggers)  
4. **Surface command table** (which directory = which commands)
5. **Bug classification** labels

> Do NOT put stack-specific rules in root AGENTS.md.  
> Do NOT duplicate content already in `harness-engineering/` docs.  
> Link to harness docs; don't inline them.

---

## Surface AGENTS.md — Template

Each surface directory should have an `AGENTS.md` with:

```markdown
# [Surface Name] Agent Instructions

## Stack
[Framework, version, key dependencies]

## Commands
| Action | Command |
|---|---|
| Test | [test command] |
| Build | [build command] |
| Dev | [dev server command] |

## Surface-Specific Rules
[3-10 rules that only apply to this surface]

## Quality Gates
[Link to harness-engineering/quality-gates/[surface]-gates.md]

## Read These First (surface-specific docs)
- [key architecture doc for this surface]
- [key decision records relevant to this surface]
```

---

## Implementation Templates

> **v3 change**: các ví dụ dưới đây trước là hardcode 1 stack cụ thể (Java Spring Boot,
> Expo, React) như thể là "chuẩn" — sai với mục tiêu 1 harness dùng được cho mọi stack.
> Từ v3, phần "Stack" và "Rules" của mỗi surface AGENTS.md nên **trỏ tới** file tương ứng
> trong `stack-packs/` (xem `stack-packs/README.md`) thay vì liệt kê lại. Ví dụ dưới đây
> giữ lại 1 bộ khung rút gọn để minh hoạ hình dạng file, không phải để copy nguyên si.

### Surface AGENTS.md — ví dụ rút gọn (trỏ sang stack-pack)

```markdown
# Backend Agent Instructions

## Stack
Xem: harness-engineering/stack-packs/backend/<node|python|go|java>.md
(điền đúng file khớp với harness.config.yaml → surfaces.backend.stack)

## Commands
Xem bảng lệnh trong stack-pack tương ứng ở trên.

## Surface-Specific Rules (CHỈ những rule đặc thù dự án này, KHÔNG lặp lại stack-pack)
1. <rule riêng của dự án, vd: "mọi endpoint public phải qua API Gateway nội bộ">
2. <rule riêng khác>

## Quality Gates
See: harness-engineering/quality-gates/backend-gates.md

## Surface Docs
- docs/ARCHITECTURE.md
- docs/decisions/ (read before changing API shape)
```

Áp dụng cùng khuôn mẫu cho `mobile/AGENTS.md`, `frontend/AGENTS.md`, `infrastructure/AGENTS.md`
— chỉ đổi `stack-packs/<nhóm>/` tương ứng. Xem `stack-packs/README.md` cho danh mục đầy đủ
15 stack hiện có (frontend: React/Vue/Svelte · mobile: RN/Flutter/native iOS/native Android
· backend: Node/Python/Go/Java · deploy: Docker-K8s/Serverless/Static-Edge/Mobile-store).

---

## Migration Guide (Existing Projects)

If you have one long root `AGENTS.md`:

1. **Audit** — Categorize each rule by surface (backend/mobile/admin/infra/global)
2. **Global rules** — Keep in root AGENTS.md (legal, harness block, lane policy, commands table)
3. **Surface rules** — Move to `[surface-dir]/AGENTS.md`
4. **Cross-reference** — Root AGENTS.md links to surface-specific ones:
   ```markdown
   ## Surface-Specific Instructions
   See the AGENTS.md in each surface directory:
   - Backend: [backend-dir]/AGENTS.md
   - Mobile: [mobile-dir]/AGENTS.md
   - Web: [frontend-dir]/AGENTS.md
   ```
5. **Verify length** — Root AGENTS.md should be < 80 lines after migration

---

## Primacy Bias Mitigation

Beyond hierarchical structure, also apply these rules to all AGENTS.md files:

| Rule | Why |
|---|---|
| Put CRITICAL rules first | Primacy bias works in your favor |
| Use headers to chunk content | Helps model parse sections |
| Use tables for commands | Denser = less context window |
| Link to docs, don't inline | Avoids overwhelming context |
| Enforce via linter, not prose | Prose rules are weaker than CI gates |
| Max 50-80 lines per file | Above this = measurable degradation |
