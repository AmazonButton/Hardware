# Quality Gates

All thresholds defined here correspond to `harness.config.yaml → quality_gates`.  
Override per-project by editing `harness.config.yaml`.

---

## Gate Overview

| Gate | Tool | Threshold | Blocks Merge? | Surface |
|---|---|---|---|---|
| TypeScript errors | `tsc --noEmit` | 0 errors | ✅ Yes | Frontend + Mobile |
| ESLint warnings | `eslint` | ≤ 60 warnings | ✅ Yes | Frontend + Mobile |
| Bundle size (gzip) | `size-limit` | < 500 KB | ⚠️ Warning | Admin Web |
| Lighthouse Performance | `lighthouse-ci` | ≥ 75 | ⚠️ Warning | Admin Web |
| Backend test coverage | JaCoCo | ≥ 60% | ⚠️ Warning | Backend |
| Backend critical CVEs | OWASP Dependency Check | 0 critical | ✅ Yes | Backend |
| Backend high CVEs | OWASP Dependency Check | 0 high | ⚠️ Warning | Backend |
| Mobile 64-bit ABI | bundletool / apktool | arm64+x86_64 | ✅ Yes | Mobile |
| Mobile 16KB page alignment | zipalign + llvm-objdump | aligned | ✅ Yes | Mobile |
| Mobile min SDK | Manifest check | ≥ 24 | ✅ Yes | Mobile |
| Mobile target SDK | Manifest check | 36 | ✅ Yes | Mobile |
| Conventional Commits | commitlint | all commits pass | ✅ Yes | All |
| PR size (lines) | Line count | ≤ 400 changed | ⚠️ Warning | All |
| PR size (files) | File count | ≤ 40 files | ⚠️ Warning | All |
| Failure memory (high-risk) | `check_failures.sh` | all sections present | ✅ Yes | All (high-risk) |
| Decision records format | format check | header present | ⚠️ Warning | All |
| Legal terms CI (E2E) | Maestro assertion | 0 forbidden terms | ✅ Yes | Mobile E2E |

---

## Gate Enforcement Points

### Pre-Push (Local — Optional)
- commitlint via husky `commit-msg` hook
- TypeScript + ESLint via `npm run validate`

### PR (CI — Required)
- All gates run on `pull_request → main`
- Critical gates block merge (CI failure = blocked PR)
- Warning gates emit `::warning::` in CI output but do not fail

### Post-Merge (Production validation)
- Health check after deploy: `curl https://api.example.com/actuator/health`
- Android ABI verification: `bundletool` on production AAB

---

## Escalation Path

When a gate warning is consistently triggered:

1. Record in harness backlog: `scripts/bin/harness-cli backlog add --title "..." --pain "..."`
2. Create a `chore/` branch to address the threshold violation
3. Discuss with team whether to tighten or loosen threshold

When a gate threshold is intentionally exceeded:

1. Document reason in commit body
2. Open a follow-up ticket within 24h
3. Set a deadline to fix (max 1 sprint)

---

## Adding a New Gate

1. Define the threshold in `harness.config.yaml → quality_gates`
2. Add CI step in `harness-engineering/workflows/reusable/harness-quality-gate.yml`
3. Document in this file
4. Add to `PR_REVIEW_CHECKLIST.md` for the relevant role

---

## Per-Surface Details

- **Frontend gates:** [`frontend-gates.md`](./frontend-gates.md)
- **Backend gates:** [`backend-gates.md`](./backend-gates.md)
- **Mobile gates:** [`mobile-gates.md`](./mobile-gates.md)
