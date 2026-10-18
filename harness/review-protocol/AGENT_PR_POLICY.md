# Agent Pull Request & Merge Policy

## 1. Scope & Purpose
AI agents generate code rapidly, but without automated governance, agent PRs risk introducing hidden regressions, breaking API contracts, or violating store policies.

This policy defines the **automated and human gates required before any agent-generated PR may be merged into `main`**.

---

## 2. Agent PR Mandatory Requirements

Every Pull Request authored or co-authored by an AI agent MUST include:

1. **Structured PR Description**:
   - **Lane Classification**: `bug:tiny`, `bug:normal`, `bug:high_risk`, or `feature:normal`, `feature:high_risk`.
   - **Summary of Changes**: Bullet points of files created, modified, or deleted.
   - **Impact Map & Blast Radius**: Downstream APIs, database columns, or mobile screens affected.
   - **Verification Evidence**: Terminal output, test coverage metrics, or emulator screenshots attached.
2. **Commit Convention Compliance**:
   - Every commit must satisfy the regex in `commitlint.config.js` (`feat(scope): ...`, `fix(scope): ...`).
3. **Session Review Sign-Off**:
   - Include the output of the multi-role review:
     - QA Engineer sign-off
     - Security Auditor sign-off (for high-risk lanes)
     - Devil's Advocate critique & resolution

---

## 3. Automated CI Enforcement Matrix

```
Agent Opens PR
      │
      ▼
[Stage 1: Static Checks] ──────► Linter, TypeCheck, CommitLint, 16KB ELF Alignment
      │
      ▼
[Stage 2: Test Suites] ────────► Unit Tests, Integration Tests (Testcontainers), JaCoCo
      │
      ▼
[Stage 3: Policy Gates] ───────► Failure Memory Check, Legal Copy grep ("An toàn" ban)
      │
      ▼
[Stage 4: Human Sign-Off] ─────► Required for High-Risk (Auth, Payment, DB Schema)
```

## 4. Forbidden Autonomous Actions for Agents
An AI agent is STRICTLY PROHIBITED from autonomously:
1. Merging PRs into `main` without human approval if classified as `high_risk`.
2. Running `git push --force` or modifying git tags on remote branches.
3. Committing or modifying secrets (`.env`, `.pem`, `.jks`, keystores).
4. Disabling existing test assertions or deleting failing test files to make CI turn green.
