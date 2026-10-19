# Merge Policy

## Core Rule

**No code merges to `main` without:**
1. CI pipeline green (all required jobs pass)
2. Session Review Protocol completed (for normal/high-risk)
3. Devil's Advocate CONFIRM_PASS or ESCALATE_WARNING (not OVERRIDE_BLOCK)
4. Conventional Commit format on all commits in the branch

---

## Merge Decision Matrix

| Lane | CI Required | Role Review | Human Approval | Failure Memory |
|---|---|---|---|---|
| `tiny` | ✅ Yes | ❌ No | ❌ Not required | ❌ No |
| `normal` | ✅ Yes | ✅ Yes (≥4 roles PASS) | 🔶 1 human recommended | ❌ No |
| `high-risk` | ✅ Yes | ✅ Yes (≥6 roles PASS) | ✅ 1 human required | ✅ Yes (FAIL-NNN file) |

---

## Push Policy (Before Opening PR)

An agent may push to a feature branch when:
- [ ] Session Review Protocol completed
- [ ] All BLOCK findings resolved
- [ ] Devil's Advocate CONFIRM_PASS or ESCALATE_WARNING
- [ ] `git status --short` shows only intentional changes
- [ ] Commit message passes Conventional Commit format
- [ ] `.env` or `*.pem` or `*.jks` NOT accidentally staged

**Hard blocks on push:**
- Devil's Advocate OVERRIDE_BLOCK — must fix before push
- Security Auditor CRITICAL finding not resolved
- Backend Engineer CRITICAL finding (SQL injection, N+1 on hot path) not resolved

---

## PR Requirements

### PR Title Format
Must match Conventional Commit format:
```
<type>(<scope>): <short description>

Examples:
feat(backend): add barcode scan result caching
fix(mobile): resolve null pointer in scan history screen
chore(deps): upgrade Spring Boot to 3.5.0
harness: add security auditor role definition
```

### PR Description Template
```markdown
## Summary
Brief description of what changed and why.

## Story
- Harness story: US-XXX
- Lane: normal | high-risk | tiny

## Role Review Summary
| Role | Status |
|---|---|
| Frontend Engineer | ✅ PASS |
| Backend Engineer | ✅ PASS |
| DevOps Engineer | ✅ PASS |
| Security Auditor | ✅ PASS |
| QA Engineer | ✅ PASS |
| Devil's Advocate | ✅ CONFIRM_PASS |

## Testing Evidence
- `./mvnw test` → PASS (67 tests)
- `npx tsc --noEmit` → 0 errors
- Story verify: `scripts/bin/harness-cli story verify US-XXX` → PASS

## Failure Memory
<!-- Only for high-risk PRs -->
- [ ] docs/failures/FAIL-NNN-description.md created

## Decision Records
<!-- If architecture changed -->
- [ ] docs/decisions/NNNN-title.md created
```

---

## Merge Methods

| Method | When to use |
|---|---|
| **Squash and Merge** | Default for all feature branches — clean linear history |
| **Merge Commit** | When preserving individual commits matters (multi-commit story) |
| **Rebase and Merge** | When branch is clean and rebased on latest main |

**Never use:** Fast-forward-only on protected branches (hides merge context).

---

## Auto-Merge Policy

Auto-merge (merging when CI passes without human click) is allowed ONLY for:
- `tiny` lane with 0 risk flags
- `chore/deps` PRs with dependency bot + CI green + 0 critical CVEs

Not allowed for:
- `normal` or `high-risk` lanes
- Any PR touching auth, payments, health data, or public API shape

---

## Post-Merge Actions

After merge to `main`:
1. Delete the feature branch (automated via GitHub settings)
2. Verify deployment (if auto-deploy configured): check health endpoint
3. Update story status: `scripts/bin/harness-cli story update --id US-XXX --status merged`
4. Record final trace: `scripts/bin/harness-cli trace --summary "..." --outcome success`

---

## Emergency Hotfix Policy

For production incidents requiring immediate fix:
1. Create `fix/FAIL-NNN-short-description` branch from `main`
2. Run abbreviated review: **Security Auditor + Devil's Advocate only** (minimum)
3. CI must pass
4. 1 human approval required even for tiny fixes
5. Create `docs/failures/FAIL-NNN-description.md` within 24h of merge
6. Schedule full post-mortem within 1 week
