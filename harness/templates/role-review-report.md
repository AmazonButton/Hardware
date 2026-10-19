# Role Review Report Template

Use this template to document the complete multi-role review for a story or significant change.  
File naming: `docs/stories/{story-id}-role-review.md`

---

# Role Review Report: {STORY-ID}

**Date:** YYYY-MM-DD  
**Story:** {story-id} — {story-title}  
**Lane:** tiny | normal | high-risk  
**Agent:** {agent-name-or-session-id}  
**Final Verdict:** ✅ PASS | ❌ BLOCK | 🔄 IN PROGRESS

---

> **Default-FAIL contract**: mỗi hàng dưới đây khởi tạo ở trạng thái chưa đạt.
> Không được ghi ✅ PASS nếu cột `Evidence Ref` trống. Xem
> `roles/ROLE_SYSTEM.md § Fresh-Context Evaluator Principle`.

## Role Review Summary

| Role | Status | Critical | High | Medium | Evidence Ref |
|---|---|---|---|---|---|
| [0] Orchestrator | ✅ PASS | 0 | 0 | 0 | intake id / trace id |
| [1] Frontend Engineer | ✅ PASS | 0 | 0 | 0 | link log/output |
| [2] Backend Engineer | ✅ PASS | 0 | 0 | 0 | link log/output |
| [3] DevOps Engineer | ✅ PASS | 0 | 0 | 0 | link log/output |
| [4] Security Auditor | ✅ PASS | 0 | 0 | 0 | link log/output |
| [5] QA Engineer | ✅ PASS | 0 | 0 | 0 | link log/output |
| [6] Devil's Advocate | ✅ CONFIRM_PASS | — | — | — | link log/output |

---

## Orchestrator — Pre-Implementation

**Lane classification:** normal  
**Reason:** {explain why this lane}  
**Risk flags triggered:** {list flags or "None"}  
**Surfaces affected:** backend | mobile | admin | all  
**Roles activated:** {list}  
**Intake command:**
```bash
scripts/bin/harness-cli intake --type change-request --summary "..." --lane normal
```

---

## [1] Frontend Engineer Verdict

**Status:** PASS | BLOCK | WARNING  
**Confidence:** HIGH  

### Findings
| Severity | Finding | File | Line |
|---|---|---|---|
| — | No findings | — | — |

### Evidence
- TypeScript: `npx tsc --noEmit` → {result}
- ESLint: `npx eslint src` → {N warnings}
- Tests: `npm run test` → {result}

---

## [2] Backend Engineer Verdict

**Status:** PASS | BLOCK | WARNING  
**Confidence:** HIGH  

### Findings
| Severity | Finding | File | Line |
|---|---|---|---|
| — | No findings | — | — |

### Evidence
- Unit tests: `./mvnw test` → {result}
- N+1 check: {notes}
- Migration check: {Flyway status}

---

## [3] DevOps Engineer Verdict

**Status:** PASS | BLOCK | WARNING  
**Confidence:** HIGH  

### Findings
| Severity | Finding | File | Line |
|---|---|---|---|
| — | No findings | — | — |

### Evidence
- Workflow YAML: {valid/invalid}
- Secrets documented: {yes/no}
- Drift check: {result}

---

## [4] Security Auditor Verdict

**Status:** PASS | BLOCK | WARNING  
**OWASP Flags:** {list or "None"}  

### Findings
| Severity | Finding | File | Line |
|---|---|---|---|
| — | No findings | — | — |

### Privacy/Compliance
- Health data consent gate: {intact/MISSING/N/A}
- AI output legal terms: {clean/VIOLATION}
- PII in logs: {none found/FOUND}

---

## [5] QA Engineer Verdict

**Status:** PASS | BLOCK | WARNING  
**Regression Risk:** LOW | MEDIUM | HIGH  

### Coverage
| Surface | Coverage | Threshold | Status |
|---|---|---|---|
| Backend | {%} | 60% | ✅ PASS |

### Edge Cases
- Empty input: {tested/MISSING/N/A}
- Null handling: {tested/MISSING/N/A}
- Unauthorized path: {tested/MISSING/N/A}

### Story Verification
```bash
scripts/bin/harness-cli story verify {story-id}
# Result: {PASS/FAIL}
```

---

## [6] Devil's Advocate Verdict

**Status:** CONFIRM_PASS | ESCALATE_WARNING | OVERRIDE_BLOCK  
**Challenges attempted:** {N}  
**New issues found:** {K}  

### Assumption Audit
| Assumption | Challenge | Result |
|---|---|---|
| {assumption} | {challenge} | ✅ Handled |

### Failure Mode Analysis
| Code Path | Failure Mode | Handled? |
|---|---|---|
| {path} | {failure} | ✅ Yes |

### 3am Production Scenario
{Description of most likely production failure and recoverability}

---

## Final Verdict

```
╔══════════════════════════════════════════════════════╗
║  ✅ REVIEW COMPLETE — PASS                           ║
║  All required roles: PASS                            ║
║  Devil's Advocate: CONFIRM_PASS                      ║
║  Push authorized for branch: {branch-name}           ║
╚══════════════════════════════════════════════════════╝
```

---

## Post-Review Actions

```bash
# Update story status
scripts/bin/harness-cli story update --id {story-id} --status implemented

# Record trace
scripts/bin/harness-cli trace \
  --summary "Implemented {feature}. All roles PASS. Devils Advocate CONFIRM_PASS." \
  --outcome success \
  --story {story-id}
```
