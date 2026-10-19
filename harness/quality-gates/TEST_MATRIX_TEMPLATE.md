# Portable Test Matrix Template

## 1. Overview
The **Test Matrix** establishes required testing fidelity across all application layers and device profiles before code is accepted into `main` or promoted to production.

---

## 2. Multi-Tier Test Pyramid

```
                ▲
               / \
              /E2E\             (Maestro / Playwright)
             /-----\
            / Integ \           (Testcontainers / Supertest)
           /---------\
          / Component \         (React Testing Library)
         /-------------\
        /  Unit Tests   \       (JUnit 5, Vitest, Jest)
       /-----------------\
```

| Layer | Target | Tooling | Required Coverage |
| :--- | :--- | :--- | :--- |
| **Unit** | Pure functions, domain calculations, utilities | Vitest / Jest / JUnit | ≥ 80% line coverage |
| **Component** | UI render, user events, accessibility | Testing Library | Key user interactions |
| **Integration** | DB repositories, external service adapters | Testcontainers | 100% happy path + error handling |
| **E2E Smoke** | Critical user journeys (Auth, Core flow) | Playwright / Maestro | 100% pass on release candidate |

---

## 3. Platform & Device Test Matrix (Mobile & Web)

### Mobile Test Dimensions (Android / iOS)
| Category | Profile / Config | Verification Method |
| :--- | :--- | :--- |
| **Screen Sizes** | Phone (5.8"), Large Phone (6.7"), Tablet (10.1") | Flexbox dynamic insets check |
| **Android SDKs** | MinSdk (API 24), TargetSdk (API 36) | CI build & emulator run |
| **Page Size** | 4 KB legacy vs 16 KB modern (Android 15+) | `zipalign -c -P 16 4` + `getconf PAGE_SIZE` |
| **Locales** | Vietnamese (`vi-VN`), English (`en-US`) | Automated i18n snapshot test |
| **Network** | Fast WiFi vs Throttled 3G | Offline banner & retry verification |

### Web Test Dimensions (Desktop & Responsive)
| Browser / Viewport | Width x Height | Primary Check |
| :--- | :--- | :--- |
| **Desktop High-DPI** | 1920 x 1080 | Navigation bar, data table column layout |
| **Laptop Standard** | 1366 x 768 | Responsive container bounds |
| **Mobile Web** | 390 x 844 | Mobile drawer menu, touch targets ≥ 44px |
