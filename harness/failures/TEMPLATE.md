# Failure Memory Template

Use this template for every production bug or high-risk failure. File naming:
`FAIL-NNN-short-description.md` (e.g., `FAIL-001-otp-bypass.md`).

Record the failure with `scripts/bin/harness-cli backlog add` as well.

---

# [FAIL-NNN] — Title

## Summary

One-line description of what failed and its user-visible impact.

## Severity

<!-- One of: tiny | normal | high_risk -->

## Surface

<!-- One of: mobile | backend | admin | landing | infra -->

## Root Cause

What actually caused the failure. Be specific — name the file, function,
config, or external dependency.

## Timeline

- **Discovered:** YYYY-MM-DD — how (user report, monitoring, CI, agent)
- **Fixed:** YYYY-MM-DD — PR link

## Prevention

Every prevention item must be a concrete, verifiable action — not a vague
promise. Check boxes as they are completed.

- [ ] Regression test added: `<test command>`
- [ ] Harness check added: `<check description>`
- [ ] CI gate updated: `<workflow change>`
- [ ] Story updated: `<story id>` with proof columns

## Google Play Context (if applicable)

Fill this section only for failures related to Google Play policy,
monetization, data safety, or store listing.

- **Policy violated:** (link to Play policy page)
- **Console reference:** (link to Play Console issue/email)
- **Data Safety form impact:** yes / no — what changed
- **Rejection history:** dates and reasons

## Evidence

- PR: (link)
- Trace ID: (harness trace id)
- Logs / screenshots: (links or inline)
- Related stories: (story ids)
