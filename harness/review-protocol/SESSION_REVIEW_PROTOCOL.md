# Session Review Protocol

## Purpose

This protocol defines how an AI agent conducts a **complete implementation + multi-role review** within a single session.  
It prevents reasoning bias by enforcing context isolation between implementation and review phases.

---

## The Problem This Solves

An agent that implements code and then reviews it in the same reasoning chain suffers from:

1. **Confirmation bias** — it knows why it made the choice and defends it unconsciously.
2. **Intent blindness** — it reviews the intention, not the actual code.
3. **Edge case blindness** — it skips cases it "knows" are handled.

**Solution:** Declare explicit role switches with context isolation headers.

---

## Full Session Protocol

### Phase 0: Orchestrator — Pre-Implementation Gate

```
══════════════════════════════════════════════════════════
🎯 ORCHESTRATOR — PRE-IMPLEMENTATION GATE
══════════════════════════════════════════════════════════
```

1. Classify the request using `harness-engineering/core/FEATURE_INTAKE.md`
2. Determine lane: `tiny` | `normal` | `high-risk`
3. Identify affected surfaces
4. Select active roles (per `harness.config.yaml → lanes`)
5. Record intake: `scripts/bin/harness-cli intake --type <type> --summary "<text>" --lane <lane>`
6. Output intake summary

**For `tiny` tasks:** Skip Phase 1–7. Implement directly, run quick checks, record trace.

---

### Phase 1: Implementation

```
══════════════════════════════════════════════════════════
🛠️ IMPLEMENTATION MODE — Active
[story: US-XXX | lane: normal]
══════════════════════════════════════════════════════════
```

- Implement the change.
- Run quick self-check (typecheck, lint).
- **Do NOT start review while still in implementation mode.**
- When implementation is complete, explicitly close:

```
══════════════════════════════════════════════════════════
🛠️ IMPLEMENTATION COMPLETE
Files changed: [list key files]
Commands run: [e.g., ./mvnw test → PASS]
══════════════════════════════════════════════════════════
```

---

### Phase 2: Role Review Cycle

**Context Isolation Declaration** (mandatory before EACH role switch):

```
══════════════════════════════════════════════════════════
🔄 ROLE SWITCH: [Role Name]
Context isolation: ACTIVE
I am now evaluating this code as [Role], not as the implementer.
I will assess the outcome, not the intention.
══════════════════════════════════════════════════════════
```

#### 2.1 — Orchestrator: Dispatch

```
══════════════════════════════════════════════════════════
🎯 ORCHESTRATOR — POST-IMPLEMENTATION DISPATCH
Lane: normal
Roles to activate: [1] Frontend, [2] Backend, [4] Security, [5] QA, [6] Devil's Advocate
Files in diff: [list]
══════════════════════════════════════════════════════════
```

#### 2.2 — Frontend Engineer Review (if frontend touched)

```
══════════════════════════════════════════════════════════
🔄 ROLE SWITCH: [1] Frontend Engineer
══════════════════════════════════════════════════════════
```

→ Load `harness-engineering/roles/frontend-engineer.md`  
→ Run checklist  
→ Output verdict block

#### 2.3 — Backend Engineer Review (if backend touched)

```
══════════════════════════════════════════════════════════
🔄 ROLE SWITCH: [2] Backend Engineer
══════════════════════════════════════════════════════════
```

→ Load `harness-engineering/roles/backend-engineer.md`  
→ Run checklist  
→ Output verdict block

#### 2.4 — DevOps Engineer Review (if CI/infra touched)

```
══════════════════════════════════════════════════════════
🔄 ROLE SWITCH: [3] DevOps Engineer
══════════════════════════════════════════════════════════
```

→ Load `harness-engineering/roles/devops-engineer.md`  
→ Run checklist  
→ Output verdict block

#### 2.5 — Security Auditor Review

```
══════════════════════════════════════════════════════════
🔄 ROLE SWITCH: [4] Security Auditor
══════════════════════════════════════════════════════════
```

→ Load `harness-engineering/roles/security-auditor.md`  
→ Run checklist  
→ Output verdict block

#### 2.6 — QA Engineer Review

```
══════════════════════════════════════════════════════════
🔄 ROLE SWITCH: [5] QA Engineer
══════════════════════════════════════════════════════════
```

→ Load `harness-engineering/roles/qa-engineer.md`  
→ Run checklist + coverage check  
→ Output verdict block

#### 2.7 — Devil's Advocate (ALWAYS LAST)

```
══════════════════════════════════════════════════════════
🔄 ROLE SWITCH: [6] Devil's Advocate
Context isolation: MAXIMUM
I am ignoring all implementation intent. I am trying to break this.
══════════════════════════════════════════════════════════
```

→ Load `harness-engineering/roles/devil-advocate.md`  
→ Attempt to break all PASS verdicts  
→ Output verdict

---

### Phase 3: Orchestrator — Synthesis & Verdict

```
══════════════════════════════════════════════════════════
🎯 ORCHESTRATOR — FINAL VERDICT SYNTHESIS
══════════════════════════════════════════════════════════
```

Aggregate all role verdicts into the summary table.  
Apply merge policy from `harness-engineering/review-protocol/MERGE_POLICY.md`.

**If PASS:**
```
╔══════════════════════════════════════════════════════╗
║  ✅ REVIEW COMPLETE — PASS                           ║
║  All required roles: PASS                            ║
║  Devil's Advocate: CONFIRM_PASS                      ║
║  Push authorized for branch: feat/US-XXX-...         ║
║  Merge policy: standard (CI must pass)               ║
╚══════════════════════════════════════════════════════╝
```

**If BLOCK:**
```
╔══════════════════════════════════════════════════════╗
║  ❌ REVIEW BLOCKED                                   ║
║  Required fixes before push:                        ║
║  1. [Backend] N+1 query: UserRepository.java:42     ║
║  2. [Devil's Advocate] No rollback for migration    ║
║  Return to: IMPLEMENTATION MODE                     ║
╚══════════════════════════════════════════════════════╝
```

---

### Phase 4: Trace Recording

After PASS verdict:

```bash
scripts/bin/harness-cli story update --id US-XXX --status implemented
scripts/bin/harness-cli story verify US-XXX
scripts/bin/harness-cli trace \
  --summary "Implemented [feature]. All 5 roles passed. Devil's Advocate CONFIRM_PASS." \
  --outcome success \
  --story US-XXX
```

---

## Convergence Loop (when BLOCK is issued)

```
BLOCK issued
    ↓
Return to IMPLEMENTATION MODE
Fix all CRITICAL issues
    ↓
DELTA REVIEW: Re-run only affected roles on changed sections
    ↓
Devil's Advocate re-runs
    ↓
If still BLOCK after 3 iterations → Escalate to human
```

Max 3 convergence loops per task. If loop 3 still fails:
- Record the blocker in `docs/failures/` using `docs/failures/TEMPLATE.md`
- Record harness friction: `scripts/bin/harness-cli backlog add --title "..." --pain "..."`
- Escalate to human with full evidence

---

## Quick Reference Card

```
Session phases:
  [0] Orchestrator: Intake gate
  [1] Implementation
  [2.1] Orchestrator: Dispatch
  [2.2] Frontend Engineer
  [2.3] Backend Engineer
  [2.4] DevOps Engineer
  [2.5] Security Auditor
  [2.6] QA Engineer
  [2.7] Devil's Advocate ← ALWAYS LAST
  [3] Orchestrator: Final verdict
  [4] Trace recording

Role switch header (mandatory):
  ══════════════════════════════
  🔄 ROLE SWITCH: [Role Name]
  Context isolation: ACTIVE
  ══════════════════════════════

PASS authorization:
  ✅ REVIEW COMPLETE — PASS → push authorized

BLOCK:
  ❌ REVIEW BLOCKED → list findings → return to implementation
```
