# Harness Evolution Protocol

## 1. Principle: The Self-Hardening Harness
A static software engineering harness degrades over time as new dependencies, platform API deprecations, and edge cases emerge.

The **Harness Evolution Protocol** defines how HEF learns from every production incident, CI failure, and agent misbehavior to permanently prevent recurrence.

---

## 2. The Evolution Feedback Loop

```
Production Incident / Regressed Bug
                 │
                 ▼
[Step 1: Failure Memory Recording] ──► Create docs/failures/FAIL-NNN.md
                 │
                 ▼
[Step 2: Root Cause Extraction]    ──► Identify: Process gap, missing lint rule, or missing test
                 │
                 ▼
[Step 3: Harness Patch Injection]  ──► Add CI check, update AGENTS.md rule, or update Quality Gate
                 │
                 ▼
[Step 4: Regression Test Lock]     ──► Write automated test preventing that specific regression
                 │
                 ▼
[Step 5: Verification & Merge]     ──► Verify harness-cli check_failures succeeds
```

---

## 3. Harness Patch Categories

| Failure Type | Required Harness Upgrade | Example File Modified |
| :--- | :--- | :--- |
| **Mobile Policy Rejection** | Add pre-build verification gate | `quality-gates/mobile-gates.md`, `with16KbPageSize.js` |
| **Legal/Copy Compliance Violation** | Add CI regex search rule | `.github/workflows/reusable/harness-quality-gate.yml` |
| **Silent API Contract Break** | Add client-server contract test | `golden-paths/archetypes/mobile-fullstack.md` |
| **Agent Hallucination Loop** | Add Context Budget rule or Skill | `core/CONTEXT_ENGINEERING.md`, `skills/` |
| **Drifting Secrets / Configs** | Update drift verification script | `scripts/checks/check_caddyfile_drift.sh` |

---

## 4. Quarterly Harness Review Cycle
1. **Audit Maturity**: Run `harness-cli query matrix` to score current maturity against `HARNESS_MATURITY.md`.
2. **Review Failure Memories**: Triage all `docs/failures/` from the last quarter; ensure every prevention item has an active, passing automated gate.
3. **Update Skills & Roles**: Prune outdated instructions; update library versions (e.g., React Native, Spring Boot, JDK).
