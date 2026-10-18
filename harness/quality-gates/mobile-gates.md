# Mobile Quality Gates

Applies to: Mobile Client application (configured via `surfaces.mobile` in `harness.config.yaml`)

---

## Gate 1: TypeScript + Lint + Tests (Validate)

**Tool:** `npm run validate` (typecheck + lint + unit tests)  
**Threshold:** 0 errors, ≤ 60 lint warnings, all tests pass  
**Blocks merge:** Yes  

```bash
cd [mobile-dir]
npm run validate
# Equivalent to: npx tsc --noEmit && npm run lint && npm run test
```

---

## Gate 2: 64-bit Architecture Enforcement

**Tool:** `bundletool` / `apktool` / `readelf`  
**Threshold:** APK/AAB contains ONLY `arm64-v8a` and `x86_64` ABI slices  
**Blocks merge:** Yes — Google Play requires 64-bit  

```bash
# Check ABI in APK
cd [mobile-dir]/android
./gradlew assembleRelease

# List native libs in APK
unzip -l app/build/outputs/apk/release/app-release.apk | grep "\.so"
# Expected: only arm64-v8a/ and x86_64/ paths
# Forbidden: armeabi-v7a/ or x86/
```

`build.gradle` enforcement:
```groovy
android {
  defaultConfig {
    ndk {
      abiFilters 'arm64-v8a', 'x86_64'
    }
  }
}
```

---

## Gate 3: 16 KB Page Size Alignment

**Tool:** `zipalign`, `llvm-objdump`, `bundletool PAGE_ALIGNMENT_16K`  
**Threshold:** All native `.so` files aligned to 16 KB  
**Blocks merge:** Yes — required for Android 15+ (API 35+)  

```bash
# Check APK alignment
zipalign -v -c -P 16 4 app-release.apk

# Check individual .so alignment
llvm-objdump -p libapp.so | grep LOAD
# Expected: 2**14 alignment (= 16384 = 16KB)

# Gradle properties must include:
# android.use16kPageSize=true
```

`gradle.properties` requirement:
```properties
android.use16kPageSize=true
```

`build.gradle` linker flags (in `plugins/with16KbPageSize.js`):
```js
config.android.linkerFlags = [
  '-Wl,-z,max-page-size=16384',
  '-Wl,-z,common-page-size=16384'
];
```

---

## Gate 4: SDK Version Compliance

**Tool:** Manifest check (automated in CI)  
**Threshold:** `minSdkVersion=24`, `targetSdkVersion=36`  
**Blocks merge:** Yes  

```bash
# Check from AndroidManifest.xml
grep -A5 "uses-sdk" android/app/src/main/AndroidManifest.xml

# Or from build.gradle
grep -E "minSdkVersion|targetSdkVersion" android/app/build.gradle
```

---

## Gate 5: APK Size Limit

**Tool:** Manual check after `assembleRelease`  
**Threshold:** < 75 MB APK warning, < 100 MB hard warning  
**Blocks merge:** Warning only  

```bash
ls -lh android/app/build/outputs/apk/release/app-release.apk
```

Optimize if over limit:
- Enable R8 / ProGuard code shrinking
- Enable WebP compression for assets
- Remove unused native libs

---

## Gate 6: AAB (Production) — Bundletool Verification

**Tool:** `bundletool`  
**Threshold:** Valid AAB, correct ABI splits, 16KB page alignment  
**Blocks merge:** Yes (production only)  

```bash
# Build AAB
cd [mobile-dir] && npx expo prebuild --clean
cd android && ./gradlew bundleRelease

# Verify AAB
bundletool validate --bundle=app/build/outputs/bundle/release/app-release.aab
```

---

## Gate 7: Emulator / Local Smoke Test (Pre-Production)

**Tool:** Local emulator or debug script  
**Threshold:** App installs, opens, and core user journey works end-to-end  
**Blocks merge:** Yes — mandatory before production build  

```powershell
# Run local emulator verification script (e.g. bluestacks / AVD)
[local-emulator-debug-script]
```

Manual smoke test checklist after install:
- [ ] App opens cleanly without crash
- [ ] Required runtime permissions flow works (Camera, Location, etc.)
- [ ] Core user journeys execute successfully end-to-end
- [ ] Compliance disclaimers visible on AI/data screens
- [ ] Settings / Profile / Navigation flows work without lag or white screen

---

## Gate 8: Google Play Policy Compliance

Manual check before every production submission:

- [ ] Data Safety form reflects new data types collected (if any)
- [ ] Age rating matches current content
- [ ] Country availability unchanged (unless intentional)
- [ ] 64-bit ABI verified
- [ ] `targetSdkVersion=36` (Android 16)
- [ ] App does NOT claim health expertise ("Chuyên gia", "Bác sĩ")
- [ ] Privacy Policy URL valid and accessible

---

## Mobile Release Pipeline Order

Mandatory sequence before Google Play submission (from AGENTS.md):

```
[1] npm run validate              — typecheck + lint + unit tests
[2] bluestacks_debug.ps1          — APK Release + BlueStacks install
[3] Manual smoke test on emulator / test device
[4] Production cloud / local build
[5] Store submit / upload
[6] Check Store Console
```

---

## Commands Summary

```bash
# Development validation
cd [mobile-dir] && npm run validate

# Local APK build + emulator debug
[local-emulator-debug-script]

# Production build command
[mobile-production-build-command]

# Submit to App Store / Google Play
[mobile-store-submit-command]
```
