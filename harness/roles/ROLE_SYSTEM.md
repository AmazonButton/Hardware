# Multi-Role Agent Review System

## Philosophy

> **One agent reviewing its own code = reasoning bias.**  
> The solution: a Fan-Out/Fan-In pipeline where **specialist roles** independently evaluate the same change, then an adversarial agent challenges all findings.

This system implements **Context Isolation** — each role operates as if it has never seen the implementation phase. Roles evaluate outcomes, not intentions.

---

## The 7 Roles (Implementation Review — sau khi có code)

| # | Role | File | Focus |
|---|---|---|---|
| 0 | **Orchestrator** | `orchestrator.md` | Classify, dispatch, synthesize |
| 1 | **Frontend Engineer** | `frontend-engineer.md` | React/RN, TypeScript, a11y, UX |
| 2 | **Backend Engineer** | `backend-engineer.md` | API, DB, business logic, contracts |
| 3 | **DevOps Engineer** | `devops-engineer.md` | CI/CD, Docker, infra, secrets |
| 4 | **Security Auditor** | `security-auditor.md` | OWASP, auth, data safety |
| 5 | **QA Engineer** | `qa-engineer.md` | Test coverage, edge cases |
| 6 | **Devil's Advocate** | `devil-advocate.md` | Adversarial: break all findings |

## 3 Planning Roles (mới v4 — TRƯỚC khi có code)

> Xem `discovery/README.md` cho toàn bộ luồng. Khác nhóm 7 role ở trên: các role này SINH
> RA artifact (Write), không chỉ review diff (Read-only). Kích hoạt cho track
> `standard`/`enterprise`, bỏ qua ở track `quick_flow`.

| # | Role | File | Focus |
|---|---|---|---|
| P1 | **Product Analyst** | `product-analyst.md` | Project Brief, PRD — vấn đề & mục tiêu đo được |
| P2 | **Architect** | `architect.md` | Architecture doc + Solutioning Gate verdict cuối |
| P3 | **UX Designer** | `ux-designer.md` | UX Spec — chỉ khi có bề mặt UI |

---

## Fan-Out / Fan-In Architecture

```
┌─────────────────────────────────────────────────────────┐
│                     PR / Code Change                     │
│               diff + story packet + context              │
└─────────────────────────┬───────────────────────────────┘
                          │
                          ▼
              ┌───────────────────────┐
              │   [0] ORCHESTRATOR    │
              │  • Reads: diff, AGENTS│
              │    .md, story, arch   │
              │  • Classifies: lane,  │
              │    surfaces, risk     │
              │  • Dispatches roles   │
              └───┬───────────────────┘
                  │ FAN-OUT (parallel evaluation)
    ┌─────────────┼──────────────────────────────────┐
    │             │              │                    │
    ▼             ▼              ▼                    ▼
┌────────┐  ┌─────────┐  ┌──────────┐  ┌────────────────────┐
│[1] FE  │  │[2] BE   │  │[3] DevOps│  │[4] Security Auditor│
│Engineer│  │Engineer │  │Engineer  │  │                    │
└────┬───┘  └────┬────┘  └────┬─────┘  └────────┬───────────┘
     │           │             │                  │
     └─────┬─────┴─────────────┴──────────────────┘
           │ FAN-IN (aggregate findings)
           ▼
      ┌──────────┐
      │[5] QA    │
      │Engineer  │ ← reviews ALL changes + test coverage
      └────┬─────┘
           │
           ▼
    ┌──────────────────┐
    │ [6] DEVIL'S      │
    │ ADVOCATE         │ ← adversarial: tries to refute all PASS verdicts
    └────────┬─────────┘
             │
             ▼
    ┌────────────────────────────────────┐
    │ FINAL VERDICT (by Orchestrator)    │
    │                                    │
    │  PASS  → Push + Merge allowed      │
    │  BLOCK → List specific failures    │
    └────────────────────────────────────┘
```

---

## Role Activation Rules

Based on `harness.config.yaml → lanes`:

| Lane | Required Roles | Min PASS |
|---|---|---|
| `tiny` | None (skip role cycle) | N/A |
| `normal` | Orchestrator + relevant surface roles + Devil's Advocate | 4 of 7 |
| `high-risk` | All 7 roles mandatory | 6 of 7 |

### Surface-to-Role Mapping

| Surface Changed | Required Roles |
|---|---|
| Frontend only (admin-web) | [0] + [1] + [4] + [5] + [6] |
| Backend only (Spring Boot) | [0] + [2] + [4] + [5] + [6] |
| Mobile only (React Native) | [0] + [1] + [3] + [4] + [5] + [6] |
| CI/CD / Infra | [0] + [3] + [4] + [6] |
| All surfaces | All 7 roles |
| Harness/docs only | [0] + [6] (lightweight) |

---

## Role Verdict Format

Each role produces a structured verdict block:

```
## [ROLE NAME] Verdict

**Status:** PASS | BLOCK | WARNING
**Confidence:** HIGH | MEDIUM | LOW
**Surface Reviewed:** backend / frontend / mobile / infra / all

### Findings

| Severity | Finding | File | Line |
|---|---|---|---|
| CRITICAL | SQL query lacks index on user_id | UserRepository.java | 42 |
| HIGH | Missing input validation on email field | ... | ... |
| MEDIUM | Component missing aria-label | ... | ... |

### Evidence
- Command run: `./mvnw test` → Exit 0
- Coverage: 67% (above 60% threshold)
- N+1 check: No N+1 patterns detected in this diff

### Recommendation
[Specific action required if BLOCK, or validation notes if PASS]
```

---

## Agent Session Role-Switch Protocol

When an agent is in a **single session** (implementing + reviewing):

1. Declare role switch explicitly:
   ```
   ═══════════════════════════════════════════════
   🔄 ROLE SWITCH: [Frontend Engineer]
   Context isolation active — ignoring implementation intent
   ═══════════════════════════════════════════════
   ```
2. Read the code as an **external reviewer** — do not use knowledge of why you made the change.
3. Produce a full verdict block per role.
4. Switch back to Orchestrator to synthesize.
5. Final verdict before any `git push`.

See [`review-protocol/SESSION_REVIEW_PROTOCOL.md`](../review-protocol/SESSION_REVIEW_PROTOCOL.md) for the full protocol.

---

## Convergence Loop

If Devil's Advocate finds a CRITICAL issue:

1. **BLOCK** is issued.
2. Agent returns to implementation mode to fix.
3. **All roles re-review** only the changed section (delta review).
4. Devil's Advocate re-runs.
5. Max 3 iterations — if still failing after 3 loops, escalate to human.

---

## Fresh-Context Evaluator Principle

> Nguyên tắc bổ sung (2026): agent tự đánh giá công việc mình vừa làm có xu hướng
> thiên vị tích cực (self-grading bias) — kể cả khi chất lượng thực tế kém.

Để giảm thiên vị này:

1. Mọi role review (đặc biệt **QA Engineer**, **Security Auditor**, **Devil's Advocate**)
   nên nhận **diff + story packet + AGENTS.md liên quan**, KHÔNG nhận nguyên transcript
   quá trình implement (không thấy các lần thử-sai, không thấy agent "tự thuyết phục"
   rằng cách làm của nó đúng).
2. Khi orchestration layer hỗ trợ (Claude Agent SDK, Claude Code sub-agent), các role
   review nên chạy với quyền công cụ **read-only** (không có Write/Edit) — role review
   không sửa code, nó chỉ ra vấn đề để Orchestrator/agent implement xử lý.
3. **Default-FAIL contract**: mọi tiêu chí trong bảng Role Review Summary khởi tạo ở
   trạng thái chưa đạt. Một role chỉ được ghi ✅ PASS khi có `evidence_ref` cụ thể
   (đường dẫn log, output lệnh, screenshot) — xem trường `evidence_ref` bắt buộc trong
   `templates/role-review-report.md`. Không có bằng chứng = không được PASS, kể cả khi
   role "cảm thấy" thay đổi có vẻ ổn.
4. Nếu hạ tầng agent không hỗ trợ tách context (chạy trong 1 phiên chat đơn), tối thiểu
   yêu cầu agent **nêu lại rõ ràng** nó đang chuyển sang "chế độ đánh giá độc lập" và chỉ
   dựa vào diff cuối cùng + kết quả lệnh test thực tế, không dựa vào ký ức về quá trình
   đã tự tin thế nào lúc code.

## Role Files

Read the full instructions for each role before activating it:

- [`orchestrator.md`](./orchestrator.md)
- [`frontend-engineer.md`](./frontend-engineer.md)
- [`backend-engineer.md`](./backend-engineer.md)
- [`devops-engineer.md`](./devops-engineer.md)
- [`security-auditor.md`](./security-auditor.md)
- [`qa-engineer.md`](./qa-engineer.md)
- [`devil-advocate.md`](./devil-advocate.md)
