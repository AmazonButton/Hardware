# Golden Path: Mobile Fullstack Monorepo Archetype

## 1. Overview
The **Mobile Fullstack Archetype** coordinates multi-surface systems containing:
1. **Mobile Application**: Native iOS/Android app for end users (e.g., consumer scanning, checkout).
2. **Admin Web Dashboard**: Back-office operational console for administrative staff.
3. **Backend API Service**: Centralized REST/GraphQL backend handling business rules, auth, and persistence.
4. **Infrastructure & Shared Tools**: Unified deployment scripts, Caddy/Nginx reverse proxy, and CI/CD pipelines.

---

## 2. Directory Layout

```
├── [app-name]-backend/         # Backend API (Spring Boot / Go / NestJS)
│   ├── src/
│   ├── pom.xml (or package.json)
│   └── AGENTS.md               # Backend-specific instructions
├── [app-name]-mobile/          # Mobile Client (Expo / React Native)
│   ├── app/
│   ├── package.json
│   └── AGENTS.md               # Mobile-specific instructions
├── [app-name]-admin-web/       # Admin Dashboard (React + Vite + shadcn)
│   ├── src/
│   ├── package.json
│   └── AGENTS.md               # Admin web-specific instructions
├── infrastructure/             # Cloud, Docker, Reverse Proxy, Scripts
│   ├── docker-compose.yml
│   └── scripts/                # Deployment and smoke test scripts
├── harness-engineering/        # HEF core, quality gates, review roles
└── AGENTS.md                   # Root-level architectural rules & surface index
```

---

## 3. Surface Dependency Rules
1. **Backend First**: Never modify client API consumption before the corresponding backend contract is finalized, merged to staging, or mocked via MSW / WireMock.
2. **Backward Compatibility**: Mobile users do not update apps simultaneously. The backend MUST support `N-1` and `N-2` mobile app versions. Never rename or delete API fields without a deprecation window.
3. **Unified Release Pipeline Order**:
   ```
   Backend Service Staging Deploy
                │
                ▼
   Admin Web Production Deploy (Immediate rollback capable)
                │
                ▼
   Mobile Client EAS / App Store Build (Long review window: 24-72h)
   ```
