# Golden Path: Mobile Client Archetype

## 1. Overview
The **Mobile Client Archetype** targets cross-platform native iOS & Android applications using Expo / React Native or Flutter, strictly enforcing mobile device compatibility, store policies, and performance guidelines.

---

## 2. Recommended Tech Stack
- **Framework**: Expo SDK (Managed Workflow) + React Native 0.83+
- **Language**: TypeScript 5.5+ (Strict Mode)
- **Styling**: NativeWind v4 (Tailwind for React Native) or StyleSheet
- **Navigation**: Expo Router (file-based routing)
- **State & Query**: TanStack Query v5 + Zustand (with AsyncStorage / MMKV persistence)
- **Testing**: Jest + React Native Testing Library (Unit) + Maestro (E2E)
- **Build & Release**: EAS Build (Cloud) + Fastlane / Gradle (Local prebuild)

---

## 3. Directory Layout

```
├── app/                  # Expo Router file-based screens
│   ├── (auth)/           # Authentication flows (login, register, forgot-password)
│   ├── (tabs)/           # Bottom tab navigation (home, scan, profile)
│   └── _layout.tsx       # Root layout, theme providers, error boundaries
├── components/           # Reusable UI primitives (buttons, inputs, cards)
├── hooks/                # Custom React hooks (useAuth, useCamera, useNetwork)
├── services/             # API client, offline sync, secure storage
├── e2e/                  # Maestro declarative YAML test flows
│   ├── login.yaml
│   └── main_flow.yaml
├── AGENTS.md             # Surface-specific agent instructions
└── app.json              # Expo application manifest configuration
```

---

## 4. Quality Gates Checklist

| Stage | Command | Threshold |
| :--- | :--- | :--- |
| **Type Check** | `npx tsc --noEmit` | 0 errors |
| **Lint** | `npm run lint` | 0 warnings |
| **Unit Tests** | `npm run test -- --ci` | 100% pass |
| **16 KB Page Alignment** | `llvm-objdump -p *.so \| grep LOAD` | 2**14 alignment (Android 15+) |
| **Mobile E2E** | `maestro test e2e/main_flow.yaml` | 100% pass on target emulator profile |
| **Local Release APK** | `./gradlew assembleRelease` | Clean build without ProGuard/R8 errors |
