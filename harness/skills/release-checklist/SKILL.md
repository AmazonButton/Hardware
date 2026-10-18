---
name: release-checklist
description: Operational runbook for verifying pre-release gates, CI/CD status, regression runs, rollback safety, and compliance checks before tagging a production release. Activates when executing or verifying production deployments.
---

# Release Checklist Skill

## Overview
This skill operationalizes the verification steps outlined in `RELEASE_PROCESS.md` and `PRE_RELEASE_CHECKLIST.md`. It acts as the final gatekeeper for the Devops Engineer and Orchestrator before releasing code to production.

---

## 5-Step Verification Runbook

### Step 1: Clean Working Tree & Head Verification
```bash
# Check git status
git status --porcelain

# Verify commit history on main
git log -n 5 --oneline
```
Requirement: Working tree must be 100% clean. Head commit must have passed all CI checks.

### Step 2: Quality Gates Check
Run the platform-specific validation scripts:
```bash
# For backend
./mvnw clean test -q

# For web frontend
npm run validate # typecheck + lint + test

# For mobile
npm run validate
```

### Step 3: Migration & Backward Compatibility
- Ensure database migrations (Flyway, Prisma, Liquibase) are purely additive:
  - No dropped columns in current release.
  - New nullable fields or default values.
  - Old code running against new database schema does not throw SQL syntax errors.

### Step 4: Health Check & Actuator Smoke Test
Before routing user traffic to the new release:
```bash
# Probe staging/health endpoint
curl -s -f https://api.staging.example.com/actuator/health | grep '"status":"UP"'
```

### Step 5: Rollback Readiness
- Confirm artifact rollback tag / commit hash is known and recorded.
- Database rollback plan verified (or forward-fix strategy documented).
- Rollback execution command verified:
  ```bash
  # Example container redeploy of previous SHA
  docker pull registry.example.com/api:sha-previous
  docker stop api-current && docker run -d registry.example.com/api:sha-previous
  ```
