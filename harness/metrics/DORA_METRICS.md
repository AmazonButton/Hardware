# DORA Metrics Tracking

## What Are DORA Metrics?

The **DORA (DevOps Research and Assessment)** four key metrics measure software delivery performance.  
They are the industry standard for evaluating engineering effectiveness.

| Metric | Measures | Elite Benchmark |
|---|---|---|
| **Deployment Frequency** | How often code deploys to production | On-demand (multiple/day) |
| **Lead Time for Changes** | Commit → production (clock time) | < 1 hour |
| **Change Failure Rate** | % of deployments causing incidents | < 5% |
| **MTTR (Mean Time to Recovery)** | Time to recover from a failure | < 1 hour |

---

## Configured Targets

From `harness.config.yaml → dora.targets`:

| Metric | Current Target | Industry Elite |
|---|---|---|
| Deployment Frequency | Weekly | Daily/Multiple per day |
| Lead Time for Changes | < 72 hours | < 1 hour |
| Change Failure Rate | < 15% | < 5% |
| MTTR | < 24 hours | < 1 hour |

**Current classification: Medium performer** (based on targets)  
**Goal: High performer** by moving to weekly+ deploys and < 48h lead time.

---

## How to Track

### Manual Tracking (Current)

Record deployments in a simple table. Update after each deploy to production.

### Deployment Log

| Date | Version | Lead Time | Incident? | MTTR |
|---|---|---|---|---|
| 2026-09-08 | v1.x.x | ~48h | No | N/A |
| (add rows) | | | | |

**Lead Time calculation:**
```
Lead Time = merge-to-main timestamp → production deploy timestamp
```

**Change Failure Rate calculation (monthly):**
```
CFR = (deployments that caused incidents / total deployments) × 100
```

**MTTR calculation:**
```
MTTR = (incident-detected timestamp → service-restored timestamp)
```

### Automated Tracking (Future)

When harness reaches H4+, implement automated DORA tracking:

1. **GitHub Actions post-deploy hook** records deployment timestamp to harness DB
2. **Incident tag on PR** (`incident: yes`) triggers MTTR tracking
3. **Weekly DORA report** via `harness-cli query dora`

---

## DORA Improvement Path

### Deployment Frequency (Currently: Weekly → Target: 2–3×/week)

**Blockers:**
- Manual BlueStacks debug step (required before EAS build)
- Manual Play Console promotion (internal → production)
- No feature flags yet (forces large batch releases)

**Actions:**
1. Add feature flags for mobile → enables smaller deploys
2. Automate BlueStacks smoke test in CI (headless Android emulator)
3. Set a deployment cadence target: **minimum 2× per week**

### Lead Time (Currently: ~72h → Target: < 48h)

**Blockers:**
- EAS cloud build queue wait (~30-60 min)
- Manual review bottleneck for normal lane
- No automated stacked PR strategy

**Actions:**
1. Start multi-role review earlier in story cycle (not after full implementation)
2. Use short-lived branches (≤ 5 days per BRANCH_STRATEGY.md)
3. Explore EAS priority builds for time-sensitive fixes

### Change Failure Rate (Unknown → Target: < 15%)

**Current gap:** CFR not tracked yet.

**Actions:**
1. Add `incident:` label to PRs that caused production issues
2. Track via `docs/failures/` — count FAIL-NNN files per month
3. Start reporting CFR monthly in team retrospective

### MTTR (Unknown → Target: < 24h)

**Current gap:** MTTR not tracked.

**Actions:**
1. Document recovery time in all `docs/failures/FAIL-NNN-*.md` files (Timeline section)
2. Add rollback instructions to all deploy scripts
3. Require rollback plan in high-risk PR descriptions

---

## Monthly DORA Review

**Schedule:** First Monday of each month.

**Agenda:**
1. Count deployments to production this month.
2. Calculate average lead time (commit → deploy for key stories).
3. Count incidents (deployments that caused issues).
4. Review FAIL-NNN files for MTTR data.
5. Compare to targets. Identify top blocker to improve.

**Output:** Update this table:

| Month | Deploy Freq | Avg Lead Time | CFR | Avg MTTR | Classification |
|---|---|---|---|---|---|
| 2026-09 | TBD | TBD | TBD | TBD | TBD |

---

## DORA Classification

| Level | Deploy Freq | Lead Time | CFR | MTTR |
|---|---|---|---|---|
| **Elite** | Multiple/day | < 1 hour | < 5% | < 1 hour |
| **High** | 1/day – 1/week | 1 day – 1 week | 5–10% | < 1 day |
| **Medium** | 1/week – 1/month | 1 week – 1 month | 10–15% | 1–7 days |
| **Low** | < 1/month | > 1 month | > 15% | > 7 days |
