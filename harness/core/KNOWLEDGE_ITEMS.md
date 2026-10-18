# Knowledge Items (KI) System

## 1. Overview
A **Knowledge Item (KI)** is a persistent, curated snapshot of repository-specific architectural patterns, gotchas, configuration recipes, and bug workarounds.

AI agents lose transient session history between conversations. KIs serve as the long-term memory buffer, preventing repetitive research and ensuring new agents follow established patterns immediately.

---

## 2. Directory Structure

KIs are stored in `<appDataDir>/knowledge/` or within the project at `.agents/knowledge/`:

```
.agents/knowledge/
├── KI-001-database-migrations/
│   ├── metadata.json
│   └── artifacts/
│       └── flyway_naming_and_locks.md
├── KI-002-mobile-16kb-alignment/
│   ├── metadata.json
│   └── artifacts/
│       └── ndk_linker_flags.md
└── index.json
```

### Format of `metadata.json`
```json
{
  "id": "KI-002",
  "title": "Android 16 KB Page Alignment Configuration",
  "summary": "Required NDK flags, AGP version, and zipalign commands to satisfy Google Play 16 KB page size policy.",
  "category": "mobile/android",
  "created_at": "2026-03-01T00:00:00Z",
  "updated_at": "2026-09-08T00:00:00Z",
  "tags": ["android", "16kb", "ndk", "google-play"],
  "sources": [
    "android/app/build.gradle",
    "plugins/with16KbPageSize.js"
  ]
}
```

---

## 3. When to Create or Consult a KI

### When to Consult KIs
- **Starting a Task**: Before planning an unfamiliar subsystem, check if an existing KI covers it.
- **Deceptively Simple Tasks**: "Add logging", "add migration", or "configure auth" usually have strict repo-specific conventions.
- **Debugging Rare Errors**: Check KIs for documented root causes of prior flaky tests or native crashes.

### When to Create a KI
- When resolving an undocumented platform issue that took > 30 minutes of investigation.
- When establishing a new architectural pattern (e.g., introducing a state machine or new caching layer).
- After writing a post-mortem or failure memory (`docs/failures/FAIL-*.md`).
