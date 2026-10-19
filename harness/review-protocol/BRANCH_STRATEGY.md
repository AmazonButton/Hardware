# Branch Strategy

## Strategy: Trunk-Based Development (TBD)

Based on industry consensus (2025/2026), this framework uses **Trunk-Based Development**:
- One protected `main` branch — always in a releasable state.
- Short-lived feature branches: max **5 days** before merge (configurable in `harness.config.yaml`).
- Feature flags decouple deployment from release for incomplete work.

> **Why not GitFlow?** GitFlow is considered legacy for SaaS teams. Long-lived `develop` + `release/*` branches create merge hell and prevent daily CI/CD. See [industry reference](https://trunkbaseddevelopment.com).

---

## Branch Naming Convention

```
{type}/US-{story-id}-{short-description}
{type}/FAIL-{failure-id}-{short-description}
{type}/{short-description}  ← only for non-story work
```

### Allowed Types

| Type | Use for | Example |
|---|---|---|
| `feat` | New feature or behavior | `feat/US-042-barcode-scan-result-cache` |
| `fix` | Bug fix | `fix/US-043-null-pointer-scan-history` |
| `fix` | Failure memory fix | `fix/FAIL-007-jwt-refresh-race-condition` |
| `chore` | Build, deps, config | `chore/upgrade-spring-boot-3-5` |
| `docs` | Documentation only | `docs/US-044-update-api-contract` |
| `refactor` | Non-behavior refactoring | `refactor/extract-product-classifier` |
| `test` | Tests only, no behavior change | `test/US-042-add-edge-case-empty-barcode` |
| `perf` | Performance improvement | `perf/optimize-product-query-n-plus-1` |
| `harness` | Harness engineering changes | `harness/add-security-auditor-role` |

### Forbidden Branch Names
- `master` (use `main`)
- `develop` (not used in TBD)
- `release/*` (use tags instead)
- `hotfix/*` (use `fix/` prefix)
- Generic names: `my-branch`, `test`, `temp`, `wip`

---

## Protected Branches

| Branch | Protection | Who can push |
|---|---|---|
| `main` | Required: CI green + PR + role review PASS | No direct push |
| (all others) | None — must be short-lived | Anyone |

---

## Branch Lifecycle

```
1. Create branch from main (latest)
   git checkout main && git pull
   git checkout -b feat/US-042-barcode-cache

2. Implement (short sessions, commit often)
   [Session Review Protocol: implement → role review → PASS]

3. Push to remote
   git push -u origin feat/US-042-barcode-cache

4. Open PR → main
   - PR description: use template (see PR_REVIEW_CHECKLIST.md)
   - CI must pass
   - At least 1 human reviewer (for high-risk: 2)

5. Squash + Merge (preferred) or Merge Commit
   - Delete branch after merge

6. Tag release (when ready to ship)
   git tag v1.2.0 && git push origin v1.2.0
```

---

## Branch Age Policy

| Branch age | Action |
|---|---|
| < 3 days | Normal — keep working |
| 3–5 days | Warning — open PR or merge soon |
| > 5 days | Stale — must rebase from `main` and add comment explaining delay |
| > 10 days | Critical — escalate: either merge or close |

Rationale: Long-lived branches = integration risk = merge conflicts = CI failures.

---

## Release Tagging

Releases are triggered by semantic version tags:

```bash
# Patch release (bug fix)
git tag v1.2.1 && git push origin v1.2.1

# Minor release (new feature)
git tag v1.3.0 && git push origin v1.3.0

# Major release (breaking change)
git tag v2.0.0 && git push origin v2.0.0
```

Tag naming follows **Semantic Versioning**: `v{MAJOR}.{MINOR}.{PATCH}`.

GitHub Actions triggers on `v*.*.*` tags:
- Build Android APK (preview)
- Build Android AAB (production)
- Submit to Google Play (internal track)
- Create GitHub Release

---

## Feature Flags

For large features that cannot merge in < 5 days:

1. Use a feature flag to hide incomplete work:
   ```typescript
   // Mobile / Admin Web
   const FEATURE_SCAN_CACHE = __DEV__ || featureFlags.scanCache;
   ```

2. Merge the flagged code to `main` early.
3. Enable flag per-environment via config.
4. Remove flag (and `enabled: false` path) once fully rolled out.

**Zombie flag rule:** Every feature flag must have a removal ticket created at the time the flag is merged.

---

## Monorepo Considerations

For **monorepo** projects containing multiple packages or surfaces, branch strategy implications:

- CI uses path-based filtering (GitHub Actions `paths:`) to only run tests for affected packages.
- PRs should touch only one surface when possible (backend OR mobile OR web).
- Cross-surface PRs require all affected roles in the review cycle.
