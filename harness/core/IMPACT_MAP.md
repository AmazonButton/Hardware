# Blast Radius & Impact Mapping Protocol

## 1. Concept: Blast Radius Analysis
Before an AI agent or engineer modifies an existing file, it must assess the **Blast Radius** — the set of downstream components, API contracts, tests, and dependencies that could break as a result of the change.

```
[Target File / Symbol Edit]
         │
         ├──► Direct Dependents (Imports, Injections)
         │           │
         │           └──► Downstream APIs & Contracts
         │                       │
         │                       └──► Client Apps (Mobile, Web)
         │
         └──► Unit & Integration Test Suites
```

---

## 2. Impact Mapping Checklist

Before editing files in **Normal** or **High-Risk** lanes:

### 1. Symbol Usages
- Use ripgrep / AST search to locate all usages of renamed, deleted, or changed function signatures:
  ```bash
  # Search all references to methodName
  rg "methodName\(" --type ts --type java
  ```

### 2. Contract Integrity (API / DTO / Database)
- Did you alter a REST/gRPC response schema?
  - If YES: Check client apps (mobile app, web admin). Are they resilient to missing or renamed keys?
- Did you alter a Database column name or type?
  - If YES: Ensure zero downtime via 2-phase migration (Add new column -> Dual write -> Read new -> Deprecate old).

### 3. State & Cache Invalidation
- Does this change alter cached values (Redis, LocalStorage, React Query cache keys)?
- Does the cache key need to be versioned or invalidated?

### 4. Test Coverage Verification
- Identify which existing unit/integration tests cover this component.
- If no tests exist, write a characterization test *before* refactoring.

---

## 3. Impact Severity Levels

| Severity | Scope | Required Pre-Flight Gate |
| :--- | :--- | :--- |
| **Low (L0)** | Isolated UI style, internal helper, private method | Local linter + unit test |
| **Medium (L1)** | Shared service, public component, domain entity | Local integration tests + Devil's Advocate review |
| **High (L2)** | Auth flow, database schema, payment/billing, API contract | High-risk story packet + Staging smoke test + Orchestrator approval |
