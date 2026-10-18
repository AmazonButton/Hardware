# PR Review Checklist

Quick checklists for each role during a PR review.  
Use this when reviewing another engineer's PR (human or agent).

---

## Universal (All PRs)

- [ ] PR title follows Conventional Commit format
- [ ] PR description includes story ID and lane
- [ ] Branch name follows naming convention
- [ ] Branch is ≤ 5 days old (no stale branches)
- [ ] PR size: ≤ 400 lines changed, ≤ 40 files (warning if exceeded)
- [ ] No `.env`, `*.pem`, `*.jks`, or secrets accidentally staged
- [ ] CI is green (or failure is documented with explanation)

---

## Frontend Checklist

- [ ] TypeScript: `npx tsc --noEmit` → 0 errors
- [ ] ESLint: ≤ 60 warnings
- [ ] No `any` type without justification comment
- [ ] No hardcoded user-facing strings (uses `useTranslation()`)
- [ ] All interactive elements have accessibility labels
- [ ] No hardcoded pixel heights in React Native
- [ ] Legal: no "An toàn", "Chuyên gia", "Bác sĩ" in UI copy
- [ ] New components have corresponding tests

---

## Backend Checklist

- [ ] `./mvnw test` → all pass
- [ ] New endpoints have `@Valid` on request DTO
- [ ] No N+1 patterns in the diff (associations, loops)
- [ ] New queries have appropriate index coverage
- [ ] Flyway migration: new file, no edits to existing
- [ ] No hardcoded secrets in `application.properties`
- [ ] Health endpoint still returns 200: `curl /actuator/health`

---

## DevOps Checklist

- [ ] No secrets hardcoded in workflow YAML
- [ ] Third-party actions pinned to version or SHA
- [ ] Deploy only triggers on `main` push (not on PR)
- [ ] Health check step present after deploy
- [ ] Cleanup step (`if: always()`) for SSH keys / service accounts
- [ ] New `.env.example` entry for any new secret
- [ ] Secrets handover updated if secret changed

---

## Security Checklist

- [ ] No IDOR — user can only access their own data
- [ ] All new endpoints have authorization check
- [ ] SQL: no string concatenation in queries
- [ ] JWT expiry configured on new tokens
- [ ] Health/allergy data gated behind consent flag
- [ ] NPM audit / OWASP check: 0 critical CVEs
- [ ] No PII in log output

---

## QA Checklist

- [ ] New behavior → new tests (not just happy path)
- [ ] Modified behavior → existing tests updated
- [ ] Coverage meets or exceeds threshold (60% backend)
- [ ] Story verification: `harness-cli story verify <id>` → PASS
- [ ] Edge cases: empty, null, max boundary covered
- [ ] For mobile: Maestro flow for the changed screen exists or updated

---

## Devil's Advocate Pass

After all other reviewers:
- [ ] No cross-role interaction gaps (Frontend assumes backend response that can change?)
- [ ] No silent assumptions ("This will never be null")
- [ ] Failure modes analyzed: what breaks at 3am?
- [ ] Deployment assumptions checked (migration ran? env var exists?)
- [ ] Verdict: CONFIRM_PASS | ESCALATE_WARNING | OVERRIDE_BLOCK

---

## High-Risk PR Extra

- [ ] `docs/failures/FAIL-NNN-*.md` created (if triggered by a production failure)
- [ ] `docs/decisions/NNNN-*.md` created (if architecture changed)
- [ ] 1 human approval present
- [ ] `scripts/bin/harness-cli decision add` run for new decision
