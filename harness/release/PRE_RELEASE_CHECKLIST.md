# Pre-Release Checklist

This checklist must be completed before any **production release**.  
Customize sections marked `[PROJECT]` for your project.

---

## Backend Pre-Release

### Code Quality
- [ ] `./mvnw test` → all tests pass (0 failures)
- [ ] `./mvnw package -DskipTests` → builds successfully
- [ ] No OWASP Critical CVEs: `./mvnw org.owasp:dependency-check-maven:check`
- [ ] No secrets hardcoded: grep scan clean
- [ ] All Flyway migrations sequential and non-conflicting
- [ ] API breaking changes documented and backward-compatible or versioned

### Configuration
- [ ] `.env.example` has all new environment variables documented
- [ ] Production `.env` / secrets manager updated (human operator)
- [ ] Docker image version pinned (not `:latest`)
- [ ] Health endpoint returns 200 in staging: `curl /actuator/health`

### Security
- [ ] Auth endpoints rate-limited
- [ ] CORS only allows expected origins
- [ ] No PII in log output
- [ ] JWT expiry configured

---

## Admin Web Pre-Release

### Code Quality
- [ ] `npx tsc --noEmit` → 0 errors
- [ ] `npx eslint src` → 0 errors, ≤ 60 warnings
- [ ] `npm run test` → all pass
- [ ] `npm run build` → successful (no build errors)
- [ ] Bundle size gzip < 500 KB

### UX
- [ ] All interactive elements have keyboard focus states
- [ ] Empty states implemented for all data tables
- [ ] Error states implemented (with retry)
- [ ] Loading states implemented (skeleton, not spinner)
- [ ] [PROJECT]: Legal copy reviewed — no forbidden terms

---

## Mobile (Android / Google Play) Pre-Release

### Technical
- [ ] `npm run validate` → all pass (typecheck + lint + tests)
- [ ] `versionCode` will auto-increment on EAS production build
- [ ] `versionName` in `app.json` updated if major/minor bump
- [ ] Local Release APK installed and smoke-tested on emulator
- [ ] No native crashes in logcat during smoke test
- [ ] 64-bit ABI only: `arm64-v8a`, `x86_64` (no 32-bit)
- [ ] 16 KB page alignment verified (if applicable)
- [ ] `minSdkVersion` ≥ required minimum
- [ ] `targetSdkVersion` = current target

### Smoke Test (Manual — on emulator or device)
- [ ] App launches without crash
- [ ] [PROJECT]: Core user flow 1 (customize per app)
- [ ] [PROJECT]: Core user flow 2
- [ ] Auth: login + logout works
- [ ] Error states shown for network failures (not crash)
- [ ] [PROJECT]: No forbidden terms in AI/generated content (legal compliance)

### Play Store Compliance
- [ ] Data Safety form reflects all data types collected in this release
- [ ] Privacy Policy URL accessible and up to date
- [ ] Content rating unchanged (or updated in Play Console)
- [ ] App does not claim capabilities beyond what it actually does
- [ ] [PROJECT]: No forbidden AI claims (if health/wellness app)

### Store Listing Assets (First Release / Major Changes)
- [ ] App icon: 512×512 PNG
- [ ] Feature graphic: 1024×500 PNG
- [ ] Phone screenshots: minimum 2, recommended 4-8
- [ ] Short description: ≤ 80 characters
- [ ] Full description: ≤ 4000 characters
- [ ] [PROJECT]: IAP products created in Play Console (if applicable)

---

## Infrastructure Pre-Release

- [ ] Server has enough disk space for new deployment
- [ ] Database backup taken before major releases
- [ ] Rollback plan documented
- [ ] On-call engineer notified for high-risk deployments
- [ ] Monitoring/alerts configured for new endpoints

---

## Sign-Off

| Role | Name | Date | Status |
|---|---|---|---|
| Developer | | | ☐ Approved |
| QA | | | ☐ Approved |
| DevOps | | | ☐ Approved |

For agent-only releases (no humans):
- Role review PASS verdict from all required roles = sign-off
- Devil's Advocate CONFIRM_PASS required
