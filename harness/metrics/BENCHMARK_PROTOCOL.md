# Benchmark Protocol

## Purpose

This document defines how to benchmark the harness itself — measuring whether changes to the harness improve or regress agent behavior.

Benchmarking is required for:
- Claiming a maturity level advance (e.g., H3 → H4)
- Evaluating new role definitions or protocol changes
- Comparing "before vs after" a harness improvement

---

## Benchmark Dimensions

| Dimension | Metric | Tool |
|---|---|---|
| Lane accuracy | % of tasks classified into correct lane | Manual review |
| Trace quality | Average trace score (0–3) | `harness-cli score-trace` |
| Harness compliance | % of tasks that followed full protocol | Manual review |
| Friction captured | % of friction events recorded | `harness-cli query friction` |
| Role review coverage | % of normal/high-risk tasks with full role cycle | Manual review |
| Devil's Advocate activation | % of normal/high-risk tasks with DA pass | Manual review |

---

## Benchmark Suite (6 Tasks)

Run the following 6 representative tasks against the harness and score each:

| # | Task | Lane | Expected | Tests |
|---|---|---|---|---|
| T1 | Fix typo in UI copy | tiny | No role review, quick trace | CI pass |
| T2 | Add new REST endpoint | normal | Story + role review + trace | Unit tests pass |
| T3 | Modify JWT auth flow | high-risk | All 7 roles + failure memory + decision | Full test suite |
| T4 | Update AGENTS.md instruction | tiny | Direct patch + trace | N/A |
| T5 | Add Flyway migration + service logic | normal | Story + Backend + QA roles + trace | Integration test pass |
| T6 | Change user data access scope | high-risk | All roles + Security BLOCK scenario | Security test |

---

## Scoring Rubric

### Lane Accuracy (per task)
- 1 point: correct lane chosen
- 0 points: wrong lane

### Trace Quality (per task, 0–3)
Score using `scripts/bin/harness-cli score-trace --id <id>`:

| Score | Meaning |
|---|---|
| 3.0 | Full fields, story linked, friction captured |
| 2.0–2.9 | Most fields present, minor gaps |
| 1.0–1.9 | Partial trace — story missing or thin fields |
| 0–0.9 | Inadequate trace |

### Harness Compliance (per task)
- 1 point: intake recorded + story created (if needed) + trace recorded + backlog updated (if friction)
- 0.5 points: most steps followed, 1 step missed
- 0 points: significant steps skipped

### Role Review Coverage (for normal/high-risk tasks only)
- 1 point: full role cycle run (correct roles per lane)
- 0.5 points: most roles, missed 1 non-critical role
- 0 points: role review skipped or only 1 role used

---

## Benchmark Scores → Maturity Level

| Level | Lane Accuracy | Avg Trace Quality | Compliance | Role Coverage |
|---|---|---|---|---|
| H1 | ≥ 4/6 | ≥ 1.0 | ≥ 40% | N/A |
| H2 | ≥ 5/6 | ≥ 2.0 | ≥ 75% | N/A |
| H3 | 6/6 | ≥ 2.3 | ≥ 85% | N/A |
| H4 | 6/6 | ≥ 2.5 | ≥ 90% | ≥ 80% |
| H5 | 6/6 | ≥ 2.8 | ≥ 95% | ≥ 95% |

---

## Running a Benchmark

```bash
# 1. Ensure harness DB is initialized
scripts/bin/harness-cli init

# 2. Run the 6 benchmark tasks (manual or scripted)
# For each task, follow the Session Review Protocol

# 3. After all tasks, score traces
scripts/bin/harness-cli query traces --limit 6
scripts/bin/harness-cli score-trace --id <id>  # per trace

# 4. Query friction captured
scripts/bin/harness-cli query friction

# 5. Calculate scores against rubric above

# 6. Record benchmark results in this file (table below)
```

---

## Benchmark History

| Date | H-Level | Lane Acc | Avg Trace | Compliance | Role Coverage | Notes |
|---|---|---|---|---|---|---|
| (initial run) | H3 partial | TBD | TBD | TBD | TBD | Baseline before HEF |
| (post-HEF) | TBD | TBD | TBD | TBD | TBD | After harness-engineering/ added |

---

## Harness Evolution Loop

When benchmark shows a regression or gap:

1. Identify which dimension regressed (lane accuracy? trace quality?)
2. Trace back to which harness file caused the gap (CONTEXT_RULES? ROLE_SYSTEM?)
3. Propose fix in a `harness/` branch
4. Re-run benchmark on the 6 affected tasks
5. Record outcome: `scripts/bin/harness-cli backlog add --title "..." --pain "..." --outcome "..."`
6. Update `HARNESS_MATURITY.md` if a level changed
