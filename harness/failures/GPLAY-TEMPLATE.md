# Google Play Policy Failure Memory Template

Use this template specifically for Google Play rejection, policy warning, or store listing compliance issues.
File naming: `FAIL-GPLAY-NNN-short-description.md`.

---

# [FAIL-GPLAY-NNN] — Title

## Summary

Brief summary of Play Store rejection or policy warning.

## Severity

`high_risk` (Google Play policy failures are always `high_risk`)

## Surface

`mobile`

## Google Play Context

- **Policy section:** (e.g., Families Policy, Data Safety, Subscription Policy)
- **Play Console Issue ID / Link:**
- **App Version / Build Number:**
- **Rejection Date:** YYYY-MM-DD

## Root Cause

Technical or policy root cause details (e.g., missing privacy policy link, improper permission declaration, absolute safety claim wording).

## Resolution & Fix

Summary of changes made to source code, metadata, or store listing.

## Prevention

- [ ] Maestro E2E assertion added for compliance check
- [ ] Data safety declaration form updated in Play Console
- [ ] Failure memory recorded via harness-cli
- [ ] Regression test verified in CI
