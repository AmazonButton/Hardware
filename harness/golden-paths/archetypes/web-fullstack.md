# Golden Path: Web Fullstack Archetype

## 1. Overview
The **Web Fullstack Archetype** is designed for modern, high-performance web applications featuring server-side rendering (SSR), dynamic data dashboards, and robust API endpoints.

---

## 2. Recommended Tech Stack
- **Frontend Framework**: Next.js (App Router) or Vite + React 19
- **Language**: TypeScript 5.5+ (Strict Mode)
- **Styling**: Tailwind CSS + shadcn/ui
- **State Management & Data Fetching**: TanStack Query v5 + Zustand
- **Backend / ORM**: Node.js / Hono / Express OR Go / Gin; Prisma / Drizzle / pg
- **Database**: PostgreSQL 16
- **Testing**: Vitest / Jest (Unit) + Playwright (E2E)
- **Package Manager**: pnpm

---

## 3. Directory Layout

```
├── app/                  # Next.js App Router or src/routes
│   ├── (auth)/           # Route group: login, register
│   ├── (dashboard)/      # Route group: main application layout
│   └── api/              # Route handlers / REST endpoints
├── components/
│   ├── ui/               # shadcn/ui primitive design tokens
│   ├── common/           # Shared layouts, headers, footers
│   └── forms/            # React Hook Form + Zod validated forms
├── lib/
│   ├── api/              # Axios/Fetch clients with interceptors
│   ├── db/               # ORM clients, connection pools
│   └── utils.ts          # Common formatters & helpers
├── tests/
│   ├── unit/             # Component & utility unit tests
│   └── e2e/              # Playwright user flows
└── AGENTS.md             # Surface-specific agent instructions
```

---

## 4. Quality Gates Checklist

| Stage | Command | Threshold |
| :--- | :--- | :--- |
| **Type Check** | `pnpm tsc --noEmit` | 0 errors |
| **Lint** | `pnpm eslint . --max-warnings=0` | 0 warnings |
| **Unit Tests** | `pnpm vitest run --coverage` | ≥ 80% line coverage on core business logic |
| **E2E Tests** | `pnpm playwright test` | 100% pass on critical auth & checkout flows |
| **Production Build** | `pnpm build` | Bundle size within budget, 0 build errors |
