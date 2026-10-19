# Release Process (Portable Template)

This document defines the **generic release mechanics** for projects using HEF.  
Customize each section for your project by filling in the `[PROJECT]` placeholders.

---

## Overview

```
Development → main branch → Production Deploy → Version Tag → Store/Registry Publish
```

Every surface (backend, frontend-admin, mobile) has its own release path.  
They are **independent** — you can release backend without releasing mobile.

---

## Branch Strategy for Releases

| Branch | Purpose | Deploy trigger |
|---|---|---|
| `main` | Production-ready | Auto-deploy backend on push |
| `feat/*`, `fix/*` | Development | CI only (no deploy) |
| `v*.*.*` tags | Version releases | Triggers store/registry publish |

---

## Backend Release

### Automated (Recommended)

Push to `main` triggers:
1. CI builds and tests the application
2. Packages with production Dockerfile
3. Deploys to server via SSH/SCP or container registry
4. Health check verification

**Manual fallback:**
```bash
# [PROJECT]: replace with your deploy script path
infrastructure/scripts/deploy_backend.ps1    # Windows
bash infrastructure/scripts/deploy_backend.sh  # macOS/Linux
```

### Health Check (required after every deploy)
```bash
# [PROJECT]: replace with your health endpoint URL
curl --fail https://api.[YOUR-DOMAIN]/actuator/health
# Expected: {"status":"UP"}
```

### Rollback
No automated rollback. To roll back:
1. Re-run deploy against previous commit/artifact
2. Or SSH to server and restart previous container with known-good image tag
3. Document in `docs/failures/FAIL-NNN.md` if production incident

---

## Frontend / Admin Web Release

### Automated Build
```bash
cd [admin-web-dir] && npm run build
# Output: dist/
```

### Deploy Options
- **CDN/S3**: Upload `dist/` to S3 + CloudFront invalidation
- **Static hosting**: Copy `dist/` to server webroot
- **Vercel/Netlify**: Auto-deploy on main push (if configured)

```bash
# [PROJECT]: replace with your deploy script
infrastructure/scripts/deploy_admin.ps1     # Windows
bash infrastructure/scripts/deploy_admin.sh # macOS/Linux
```

---

## Mobile Release (Android / Google Play)

> See [`PRE_RELEASE_CHECKLIST.md`](./PRE_RELEASE_CHECKLIST.md) **before** running any production build.

### Stage Pipeline (MANDATORY ORDER — do not skip steps)

```
[1] Validate         → npm run validate (typecheck + lint + tests)
[2] Local APK Debug  → Build Release APK + install on emulator
[3] Manual QA        → Smoke test all critical flows on emulator
[4] Cloud AAB Build  → EAS production build
[5] Store Submit     → EAS submit to internal track
[6] Store Verify     → Confirm in Play Console, review release
```

### Step 1: Validate
```bash
cd [mobile-dir] && npm run validate
```

### Step 2: Local Debug APK
```bash
# [PROJECT]: customize emulator/device command
cd [mobile-dir]/android && ./gradlew assembleRelease
# Install on emulator (customize path)
adb install app/build/outputs/apk/release/app-release.apk
```

### Step 3: Manual QA Smoke Test

Before any production build, verify on emulator:
- [ ] App opens without crash
- [ ] Primary user flow works end-to-end (customize per app)
- [ ] Auth flow: login + logout works
- [ ] No forbidden terms in AI/generated content (if applicable)
- [ ] Critical error scenarios show user-friendly messages

### Step 4: Cloud Production Build
```bash
# [PROJECT]: set platform and profile
eas build --platform android --profile production --non-interactive
```

### Step 5: Submit to Store
```bash
eas submit --platform android --profile production --latest --non-interactive
```

### Step 6: Play Console Verification
1. Open Play Console → [App] → Testing → Internal Testing
2. Confirm new build appears (versionCode incremented)
3. Click "Review release" → check content rating, data safety
4. Promote to higher tracks only after QA sign-off

### Versioning

| Config | Rule |
|---|---|
| `versionCode` | Auto-incremented by EAS on each production build |
| `versionName` | Update in `app.json` before major releases |
| Git tag | Push `v1.2.3` tag after successful Play Console submission |

```bash
git tag v1.2.3 && git push origin v1.2.3
```

---

## Version Tagging Convention

```bash
# Patch (bug fix, small improvement)
git tag v1.2.1

# Minor (new feature, backward compatible)
git tag v1.3.0

# Major (breaking change, major redesign)
git tag v2.0.0

git push origin --tags
```

Tags trigger (if configured in CI):
- GitHub Release creation with changelog
- Android APK preview build (sideload distribution)
- Production AAB build + Play Store submission

---

## Release Verification

After each release:

| Surface | Verification command |
|---|---|
| Backend | `curl https://api.[DOMAIN]/actuator/health` |
| Admin Web | Open `https://admin.[DOMAIN]` in browser |
| Mobile | Check EAS dashboard + Play Console internal track |

---

## Rollback Decision Tree

```
Production incident detected
       ↓
Is it reproducible? → No → Monitor + document in docs/failures/
       ↓ Yes
Is it code (not data)? → Yes → Re-deploy previous version
       ↓ No
Is it data corruption? → Yes → Emergency DB backup restore + notify team
       ↓ No
Is it infra? → Restart service / failover region
       ↓
Document MTTR in docs/failures/FAIL-NNN.md
```

---

## One-Time Setup (Human Operator Required)

Before automated releases work, a human must configure:

| Secret | Where | Purpose |
|---|---|---|
| `AWS_EC2_SSH_KEY` | GitHub Secrets | Backend SSH deploy |
| `GOOGLE_PLAY_SERVICE_ACCOUNT_JSON` | GitHub Secrets | Play Store submit |
| `EXPO_TOKEN` | GitHub Secrets | EAS build trigger |

Document all secrets in `.env.example` with placeholder values.
