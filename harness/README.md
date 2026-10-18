# 🔧 Harness Engineering Framework (HEF) — v4.0.0

> A portable, multi-project engineering harness with multi-role agent review,
> branch governance, quality gates, DORA metrics, and per-stack adapters — ready to plug into any project.

---

## What Is This?

The **Harness Engineering Framework (HEF)** is a structured operational system that allows AI agents and humans to collaborate safely on production-grade software.

> **The App is what users touch. The Harness is what agents touch.**

This framework is portable: copy or submodule it into any project and configure it via `harness.config.yaml`. Since v3, the harness separates **process** (roles, gates, lanes — same for every project) from **technology** (`stack-packs/` — React vs Vue, Node vs Go, Docker vs Serverless…), so the same harness works across web, mobile, backend, and deploy targets without hardcoding one stack.

---

## Quick Start

```bash
# 0. Copy config mẫu generic thành config thật của dự án (đừng dùng examples/ trực tiếp)
cp harness.config.yaml.example harness.config.yaml
# → sửa project.*, surfaces.* (kể cả surfaces.<x>.stack — xem stack-packs/README.md), compliance.domain_pack

# 1. Initialize durable database (source-available CLI, no binary — see scripts/bin/SECURITY_NOTE.md)
python3 scripts/harness_cli.py init
# hoặc, giữ nguyên các lệnh cũ trong docs (wrapper tương thích ngược):
./scripts/bin/harness-cli init

# 2. Xem context nào cần nạp cho dự án này (domain-pack + stack-packs)
./scripts/bin/harness-cli packs resolve

# 3. Check project status
./scripts/bin/harness-cli query matrix
```

See [`INSTALL.md`](./INSTALL.md) for full installation guide for new projects.

---

## Framework Map

```
harness-engineering/
│
├── GAP_ANALYSIS_V4_TREND_COMPARISON.md  Đánh giá khắt khe v4 vs Spec Kit/BMAD/OpenSpec (đọc trước)
├── V3_UPGRADE_REPORT.md  Đánh giá v3 (lịch sử)
├── GAP_ANALYSIS_AND_ROADMAP.md  Audit v0.2.0 gốc (lịch sử)
├── CONSTITUTION.md       NEW v4 — nguyên tắc bất biến của dự án (điền khi khởi tạo)
├── discovery/            NEW v4 — Discovery/Planning/Solutioning Gate TRƯỚC khi có code
│                         (Project Brief -> PRD -> UX Spec -> Architecture -> Gate)
├── .claude/agents/       7 role dưới dạng Claude Code subagent thật (tools/model scoped)
├── agent-instructions/   Hierarchical AGENTS.md guide, quan hệ AGENTS.md ↔ CLAUDE.md
├── compliance/
│   ├── COMPLIANCE_FRAMEWORK.md   Universal guardrails (mọi domain)
│   ├── LEGAL_COMPLIANCE.md       (deprecated — pointer sang cấu trúc mới)
│   └── domain-packs/             health-wellness, fintech, ecommerce — nạp theo project.domain
├── context/              Session handoff artifacts (xem core/SESSION_HANDOFF.md)
├── core/                 Core policies — HARNESS, CONTEXT_ENGINEERING, IMPACT_MAP,
│                         PARALLEL_AGENTS, SESSION_HANDOFF, SANDBOX_POLICY,
│                         PREFLIGHT_CHECKLIST, COST_TRACKING, HOOKS_AND_ENFORCEMENT (mới),
│                         SKILLS_SECURITY (mới)
├── examples/             Case study config thật (vd. dico-scan.harness.config.yaml)
├── failures/             Failure Memory protocol & post-mortem templates
├── golden-paths/         Blueprints (topology): web-fullstack, mobile-only, api-backend,
│                         mobile-fullstack, cli-tool, data-pipeline — kết hợp với stack-packs/
├── mcp/                  Model Context Protocol configs, plugin marketplace guidance
├── metrics/              DORA metrics + benchmark evaluation protocol
├── quality-gates/        Automated quality thresholds & multi-device test matrix
├── release/              Portable release process & pre-release checklist
├── review-protocol/      Session review, merge policy, agent PR policy & checklists
├── roles/                7 review roles (post-code) + 3 planning roles NEW v4
│                         (product-analyst, architect, ux-designer) — see .claude/agents/
│                         for runnable subagent versions + Fresh-Context Evaluator principle
├── scripts/
│   ├── harness_cli.py    Source-available CLI (Python stdlib) — replaces the old binary
│   └── bin/               Thin wrapper scripts (harness-cli / harness-cli.ps1) + SECURITY_NOTE.md
├── skills/                Portable agent skills library (android-debug, UI design, tables)
├── stack-packs/           NEW in v3 — per-technology adapters (frontend/mobile/backend/deploy)
├── templates/             Story packets, decisions, review reports (evidence_ref bắt buộc)
└── workflows/             Reusable GitHub Actions CI/CD workflows
```

> v4 vá gap lớn nhất khi đối chiếu khắt khe với GitHub trend (Spec Kit ~90k★, BMAD-METHOD
> ~37k★): thêm pha Discovery/Planning/Solutioning Gate trước code, thứ mà v3 hoàn toàn
> chưa có. Xem [`GAP_ANALYSIS_V4_TREND_COMPARISON.md`](./GAP_ANALYSIS_V4_TREND_COMPARISON.md)
> cho bảng chấm điểm đối đầu (không tô hồng) và việc còn thiếu.

---

## Core Concept: Multi-Role Agent Review

In a single session, the agent cycles through **7 specialist roles** before any push/merge is allowed:

```
[Orchestrator] → classifies risk, dispatches roles
    ├── [Frontend Engineer]   → React, TypeScript, a11y, bundle
    ├── [Backend Engineer]    → API, DB queries, concurrency, contracts
    ├── [DevOps Engineer]     → Docker, CI, secrets, infra drift
    ├── [Security Auditor]    → OWASP, JWT, SQL injection, data safety
    └── [QA Engineer]         → test coverage, edge cases, regression
              ↓
    [Devil's Advocate]        → adversarial pass: tries to break all findings
              ↓
    [VERDICT: PASS or BLOCK]
```

See [`roles/ROLE_SYSTEM.md`](./roles/ROLE_SYSTEM.md) and [`review-protocol/SESSION_REVIEW_PROTOCOL.md`](./review-protocol/SESSION_REVIEW_PROTOCOL.md).

---

## Harness Maturity Level

| Level | Description | Status |
|---|---|---|
| H0 | No harness | ✅ Passed |
| H1 | Policy + templates | ✅ Achieved |
| H2 | Durable state + observability | ✅ Achieved |
| H3 | Active observability + evolution | 🟡 Partial |
| H4 | Automated verification | 🟡 Partial |
| H5 | Self-improving harness | ❌ Target |

---

## Risk Lanes

| Lane | Use when | CI enforcement |
|---|---|---|
| `tiny` | Docs, copy, narrow edits | Tests pass |
| `normal` | Story-sized behavior change | Story packet + verify_command |
| `high-risk` | Auth, billing, data safety, Play policy | Failure memory + Detailed trace + regression test |

---

## Key Commands

```bash
# Classify intake
scripts/bin/harness-cli intake --type change-request --summary "..." --lane normal

# Add/update story
scripts/bin/harness-cli story add --id US-042 --title "..." --lane normal
scripts/bin/harness-cli story verify US-042

# Record decision
scripts/bin/harness-cli decision add --id 0012-new-decision --title "..." --doc docs/decisions/0012.md

# Record trace
scripts/bin/harness-cli trace --summary "..." --outcome success

# Check matrix
scripts/bin/harness-cli query matrix
scripts/bin/harness-cli query friction
```

---

## Project Configuration

Customize the harness for your project in [`harness.config.yaml`](./harness.config.yaml).

---

## License

MIT — free to use in any project.
