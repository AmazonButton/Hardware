# Architecture Rules

This file captures the universal architecture rules that apply across projects using HEF.  
Project-specific architecture decisions go in `docs/decisions/` (using the template).

## Universal Rules (All Projects)

### 1. Layering Contract

```
domain
  <- application
      <- infrastructure
          <- interface
              <- app surfaces
```

Inner layers must not depend on outer layers.

| Layer | May depend on | Must not depend on |
|---|---|---|
| domain | nothing external except tiny pure utilities | framework, database, UI, provider |
| application | domain | framework, UI, provider, database concrete clients |
| infrastructure | domain, application | interface controllers or UI |
| interface | all backend layers | UI state or platform shell assumptions |
| app surfaces | API contracts + app-facing clients | domain internals directly |

### 2. Parse-First Boundary Rule

All unknown data must be parsed at boundaries before entering inner code.

**Boundaries include:**
- HTTP request bodies, params, query strings
- Session payloads and identity claims
- Environment variables
- Database rows from external clients
- Platform shell payloads
- Deep links, tokens, signed URLs
- Provider webhooks, events, async payloads

**Target flow:**
```
unknown input → parser → typed DTO or command → application use case → domain object
```

### 3. Command/Query Separation

- **Commands** mutate state and own audit side effects.
- **Queries** read state and format for consumers.
- Shared domain rules live in domain/application, not controllers.

### 4. Observability Contract

Every server must emit one canonical JSON log line per request:

```json
{
  "timestamp": "...",
  "level": "INFO",
  "request_id": "...",
  "user_id": "...",
  "action": "...",
  "duration_ms": 42,
  "status_code": 200,
  "message": "..."
}
```

Audit logs are product records. Application logs are operational records.  
Do not use one as a substitute for the other.

### 5. Dependency Rule for CI

When a new external dependency is added:

1. Check for known CVEs (OWASP Dependency Check).
2. Verify license compatibility.
3. Document rationale in commit message body.
4. Add to dependency audit list if it touches security/auth/payment.

### 6. Configuration & Secrets

- Secrets MUST come from environment variables or a secrets manager.
- Never hardcode API keys, passwords, or tokens.
- Every new secret must be added to `.env.example` (with placeholder value).
- See `harness.config.yaml: compliance.*` for project-specific rules.

### 7. API Contract Stability

- Public API shape changes are **high-risk** (require a decision record).
- Additive changes (new optional fields) are **normal** lane.
- Removing or renaming fields is **high-risk** and requires a migration plan.

---

## Stack-Specific Rules

### Java / Spring Boot

- Use `@Validated` on request DTOs with bean validation constraints.
- Repository layer returns `Optional<T>` — never `null`.
- Exceptions flow through `@ControllerAdvice` — no raw `try/catch` in controllers.
- Database migrations use Flyway — never modify existing migration files.
- N+1 queries are a high-risk bug — use `@EntityGraph` or projection queries.

### React / TypeScript

- Strict TypeScript (`strict: true`) — no `any` without documented suppression comment.
- Components are pure — no direct API calls; use React Query / TanStack Query.
- State management: local state first, then Zustand; no Redux unless justified.
- No inline styles — use design system tokens only.
- Every interactive element must have an accessible label (aria-label or visible label).

### React Native / Expo

- Use `expo-constants` for environment variables — not `process.env` directly.
- Navigation with Expo Router — no raw React Navigation unless Expo Router doesn't support the case.
- Images: SVG vectors or WebP downsampled — never PNG above 100KB.
- Hardcoded pixel heights are **blocked** — use `dp`/`sp` equivalent flexbox.
- All strings must use `useTranslation()` — no hardcoded user-facing text.

### Android / Mobile

- `targetSdkVersion` = 36 (Android 16)
- `minSdkVersion` = 24 (Android 7.0+)
- 64-bit architectures only: `arm64-v8a`, `x86_64`
- 16 KB page alignment required
- See `harness-engineering/quality-gates/mobile-gates.md` for full checklist.

---

## Discovery Before Shape

Before proposing implementation, identify:

- **Product surfaces** — browser, mobile, desktop, CLI, API, worker, or service.
- **Runtime stack** — language, framework, database, queues, providers, hosting.
- **Core domains** — product concepts that deserve stable names and contracts.
- **Boundary inputs** — user input, API requests, webhooks, jobs, files, credentials.
- **Validation ladder** — smallest checks that can prove the selected stack.

Record stack choices in `docs/decisions/` when they meaningfully constrain future work.
