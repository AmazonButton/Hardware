# Frontend Quality Gates

Applies to: Web Frontend and Mobile Client applications (configured via `surfaces` in `harness.config.yaml`)

---

## Gate 1: TypeScript Strict Mode

**Tool:** `tsc --noEmit`  
**Threshold:** 0 errors  
**Blocks merge:** Yes  

```bash
# Web Frontend
cd [frontend-dir] && npx tsc --noEmit

# Mobile Client
cd [mobile-dir] && npx tsc --noEmit
```

Configuration must include `strict: true` in `tsconfig.json`:
```json
{
  "compilerOptions": {
    "strict": true,
    "noImplicitAny": true,
    "strictNullChecks": true
  }
}
```

---

## Gate 2: ESLint

**Tool:** ESLint  
**Threshold:** ≤ 60 warnings (0 errors)  
**Blocks merge:** Yes (errors), Warning (>60 warnings)  

```bash
# Web Frontend
cd [frontend-dir] && npx eslint src --ext .ts,.tsx --max-warnings 60

# Mobile Client
cd [mobile-dir] && npx eslint src --ext .ts,.tsx --max-warnings 60
```
```

Required ESLint rules (must be `error` not `warn`):
- `@typescript-eslint/no-explicit-any`
- `react-hooks/exhaustive-deps`
- `no-console` (or configured to `warn`)
- `no-debugger`

---

## Gate 3: Bundle Size (Admin Web)

**Tool:** `size-limit` or manual Vite build analysis  
**Threshold:** Gzipped bundle < 500 KB for initial load  
**Blocks merge:** Warning only  

```bash
cd [frontend-dir] && npm run build
# Check dist/ folder sizes
du -sh dist/assets/*.js | sort -rh | head -5
```

Add `size-limit` config to `package.json`:
```json
{
  "size-limit": [
    {
      "path": "dist/assets/index-*.js",
      "limit": "500 KB",
      "gzip": true
    }
  ]
}
```

---

## Gate 4: Lighthouse CI (Admin Web)

**Tool:** `@lhci/cli` (Lighthouse CI)  
**Threshold:** Performance ≥ 75, Accessibility ≥ 90  
**Blocks merge:** Warning only  

```bash
# Run locally
npx @lhci/cli autorun --config=lighthouserc.json
```

`lighthouserc.json`:
```json
{
  "ci": {
    "assert": {
      "assertions": {
        "categories:performance": ["warn", {"minScore": 0.75}],
        "categories:accessibility": ["warn", {"minScore": 0.90}],
        "categories:best-practices": ["warn", {"minScore": 0.80}]
      }
    }
  }
}
```

---

## Gate 5: Accessibility (a11y)

**Tool:** `axe-core` via React Testing Library or `@axe-core/react`  
**Threshold:** 0 CRITICAL or SERIOUS violations  
**Blocks merge:** Yes (critical/serious), Warning (moderate)  

```bash
# Run a11y tests
cd [frontend-dir] && npm run test -- --run
# Look for: "Found X accessibility violations"
```

---

## Gate 6: UI Compliance Assertions (When Configured)

**Tool:** E2E Test Assertion (Maestro / Playwright)  
**Threshold:** 0 occurrences of forbidden or misleading terms on AI / evaluation screens  
**Blocks merge:** Yes  

```yaml
# In E2E flow (e.g. Maestro / Playwright):
- assertVisible:
    id: "result-screen"
- assertNotVisible:
    text: "[prohibited-term-1]"
- assertNotVisible:
    text: "[prohibited-term-2]"
```

---

## Commands Summary

```bash
# Full frontend validation (Web)
cd [frontend-dir]
npx tsc --noEmit              # TypeScript
npm run test -- --run          # Unit tests
npm run build                  # Build check

# Full frontend validation (Mobile)
cd [mobile-dir]
npx tsc --noEmit               # TypeScript
npm run lint                   # Linter
npm run test                   # Unit tests
```
