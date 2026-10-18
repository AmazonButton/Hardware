# Commit Convention

## Standard: Conventional Commits

All commits in this project follow the [Conventional Commits](https://www.conventionalcommits.org/) specification.

---

## Format

```
<type>(<scope>): <short description>

[optional body]

[optional footer(s)]
```

### Short Description Rules
- Imperative mood: "add", "fix", "update" — not "added", "fixed"
- Lowercase after colon
- No period at the end
- Max 72 characters total (type + scope + description)

---

## Types

| Type | Use for | Triggers |
|---|---|---|
| `feat` | New feature or behavior | Minor version bump |
| `fix` | Bug fix | Patch version bump |
| `docs` | Documentation only | No version bump |
| `style` | Formatting, whitespace (no behavior change) | No version bump |
| `refactor` | Code restructure (no behavior change) | No version bump |
| `perf` | Performance improvement | Patch version bump |
| `test` | Adding or updating tests | No version bump |
| `build` | Build system, dependencies | No version bump |
| `ci` | CI/CD configuration | No version bump |
| `chore` | Maintenance tasks, non-code | No version bump |
| `harness` | Harness engineering changes | No version bump |
| `revert` | Reverting a previous commit | Patch version bump |

### BREAKING CHANGE

Breaking changes must include `BREAKING CHANGE:` in the footer:

```
feat(backend)!: change product scan API response shape

BREAKING CHANGE: `rating` field renamed to `suitabilityScore` in all scan endpoints.
Mobile and admin clients must be updated before this is deployed.
```

---

## Recommended Scopes

Configured in `harness.config.yaml → commits.scopes`:

| Scope | Surface |
|---|---|
| `backend` | Java Spring Boot API |
| `mobile` | Expo React Native app |
| `admin` | Admin web (React 19 + Vite) |
| `infra` | Infrastructure, Docker, AWS |
| `harness` | Harness engineering |
| `docs` | Documentation |
| `deps` | Dependency updates |
| `security` | Security-related changes |
| `legal` | Legal/compliance |

---

## Examples

```bash
# Feature
feat(backend): add barcode scan result caching with Redis

# Bug fix with scope
fix(mobile): resolve null pointer in scan history list screen

# Failure memory fix
fix(backend): resolve JWT refresh race condition (FAIL-007)

# Harness change
harness: add Devil's Advocate role to review pipeline

# Breaking change
feat(backend)!: rename product rating API field

BREAKING CHANGE: `rating` renamed to `suitabilityScore`.

# Dependency update
chore(deps): upgrade Spring Boot from 3.4 to 3.5

# Documentation
docs(harness): update session review protocol for multi-role pipeline

# CI change
ci: add security scan to harness quality gate workflow

# Test only
test(backend): add edge case for empty barcode scan result

# Performance
perf(backend): optimize product query with @EntityGraph to eliminate N+1
```

---

## Commit Body Guidelines

For `normal` and `high-risk` commits, add a body explaining WHY:

```
fix(backend): prevent N+1 query in product scan history

Previously, `ScanHistoryService.getUserHistory()` iterated over each scan
record and lazily loaded the associated Product entity — triggering one query
per record. On users with 100+ scans this caused 100+ DB roundtrips.

Fixed by adding @EntityGraph on the repository query to eagerly join Product
in a single query. Verified with Hibernate statistics (log level DEBUG).

Story: US-043
Lane: normal
```

---

## Enforcement

### CI Enforcement (GitHub Actions)

Enforced via `harness-engineering/workflows/reusable/harness-quality-gate.yml`:

```yaml
- name: Validate commit messages
  uses: wagoid/commitlint-github-action@v5
  with:
    configFile: harness-engineering/commit-convention/commitlint.config.js
```

### Local Enforcement (Optional — Recommended)

Install pre-commit hook:

```bash
# Install commitlint
npm install --save-dev @commitlint/cli @commitlint/config-conventional

# Install husky
npm install --save-dev husky
npx husky init

# Add commit-msg hook
echo 'npx commitlint --edit $1 --config harness-engineering/commit-convention/commitlint.config.js' > .husky/commit-msg
chmod +x .husky/commit-msg
```

---

## Relationship to Harness Stories

Commit messages should reference story IDs in the body or footer:

```
Story: US-042
Lane: normal
```

This allows tracing commits → stories → traces in the harness.
