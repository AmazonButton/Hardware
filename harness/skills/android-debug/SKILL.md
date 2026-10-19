---
name: android-debug
description: Portable debugging workflow for Android emulators (BlueStacks, Android Studio AVD), ADB logcat inspection, native crash symbolication, and 16 KB page size verification. Activates during mobile Android debugging or native crash investigations.
---

# Android Debug & Emulation Skill

## Overview
This skill provides a standardized, repeatable procedure for debugging Android mobile applications across emulators (BlueStacks 5, Android Studio AVD) and physical devices using ADB.

## Prerequisites & Path Resolution
1. **ADB Tooling**:
   - Standard Android SDK: `%LOCALAPPDATA%\Android\Sdk\platform-tools\adb.exe`
   - BlueStacks 5: `C:\Program Files\BlueStacks_nxt\HD-Adb.exe`
2. **Device Discovery**:
   ```powershell
   & $AdbPath devices -l
   ```
   Default emulator serial: `emulator-5554` or `127.0.0.1:5555`.

## Standard Diagnostic Workflow

### 1. Connection & State Verification
```powershell
# Verify ADB connection
& $AdbPath get-state

# Check Android API level
& $AdbPath shell getprop ro.build.version.sdk

# Check CPU Architecture (arm64-v8a vs x86_64)
& $AdbPath shell getprop ro.product.cpu.abi
```

### 2. 16 KB Memory Page Size Verification (Android 15+)
Google Play requires 16 KB page size compatibility:
```powershell
# Query page size (4096 vs 16384)
& $AdbPath shell getconf PAGE_SIZE
```
If targeting 16 KB test images:
- Verify that ELF shared libraries (`.so`) are 16 KB aligned:
  ```bash
  llvm-objdump -p lib/*.so | grep -E "LOAD|align"
  ```

### 3. Logcat Filtering Patterns
Avoid noisy logs by filtering to the app package or React Native/Crash tags:

```powershell
# Clear old logs
& $AdbPath logcat -c

# Stream filtered logs (replace com.example.app with target package)
& $AdbPath logcat -v time | Select-String "com.example.app|AndroidRuntime|ReactNativeJS|FATAL EXCEPTION"
```

### 4. Native & JS Crash Diagnosis
- **React Native JS Crash**: Look for `ReactNativeJS` tag or redbox error screen.
- **Native SIGSEGV / Crash**:
  ```powershell
  & $AdbPath logcat -d -b crash
  ```
  Look for stack trace containing `backtrace:`, `#00 pc`, and library name (e.g., `libreactnativejni.so`).

### 5. Local Release APK Sideload & Test Loop
To verify production release builds locally before cloud EAS / Play Store submission:
```powershell
# Install release APK
& $AdbPath install -r -d ./path/to/app-release.apk

# Launch main activity
& $AdbPath shell monkey -p com.example.app -c android.intent.category.LAUNCHER 1

# Monitor crash for 10 seconds
Start-Sleep -Seconds 10
& $AdbPath logcat -d -b crash
```
