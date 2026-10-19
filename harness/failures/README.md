# Failure Memory — How To Use

This directory contains templates for documenting production failures.  
Every high-risk production incident MUST have a failure memory file.

---

## Why Failure Memory?

1. **Prevents recurrence** — agents and humans can read past failures before touching the same code.
2. **Measures MTTR** — timestamp data feeds DORA metrics.
3. **Enforced by CI** — `scripts/checks/check_failures.sh` validates required sections.

---

## When to Create a Failure Memory File

| Trigger | Required |
|---|---|
| Production crash or outage | ✅ Yes |
| Data integrity issue | ✅ Yes |
| Security incident | ✅ Yes |
| Failed production deploy | ✅ Yes |
| Google Play policy violation | ✅ Yes (use GPLAY-TEMPLATE.md) |
| High-risk bug (`bug:high_risk` label) | ✅ Yes |
| Normal bug | ❌ No |

---

## Naming Convention

```
FAIL-{NNN}-{short-description}.md
FAIL-GPLAY-{NNN}-{short-description}.md   (Google Play specific)

Examples:
FAIL-001-google-signin-developer-error.md
FAIL-GPLAY-016-auto-refund-missing-service-account.md
```

NNN = sequential number, padded to 3 digits.

---

## Templates

- [`TEMPLATE.md`](./TEMPLATE.md) — General failure template
- [`GPLAY-TEMPLATE.md`](./GPLAY-TEMPLATE.md) — Google Play specific failures

---

## Required Sections (Enforced by CI)

Every failure file MUST contain these sections (CI checks via `grep`):

```
## Summary
## Severity
## Surface
## Root Cause
## Timeline
## Prevention
## Evidence
```

For GPLAY failures, also required:
```
## Google Play Context
```

---

## Prevention Section Rules

Every Prevention item must be **concrete and verifiable**, not vague:

| ❌ Vague (not acceptable) | ✅ Concrete (required) |
|---|---|
| "Be more careful next time" | "Add CI test: verify JWT expiry > 0 before deploy" |
| "Review code better" | "Add BlueStacks smoke test step to release pipeline" |
| "Don't forget to check" | "Add `check_failures.sh` to CI — blocks merge if FAIL-NNN missing Prevention section" |

---

## CLI Integration

```bash
# After creating a failure file, record in harness backlog
scripts/bin/harness-cli backlog add \
  --title "FAIL-NNN: [short description]" \
  --pain "[what went wrong]" \
  --risk high-risk

# When prevention actions are implemented
scripts/bin/harness-cli backlog update --id <id> \
  --status implemented \
  --outcome "[what was done to prevent recurrence]"
```

---

## Index

<!-- Append entries here as failures are documented -->
| ID | Date | Surface | Summary | Status |
|---|---|---|---|---|
| (start adding FAIL-NNN rows) | | | | |
