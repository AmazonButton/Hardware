# Harness Engineering Framework (HEF) — Glossary

This document defines standardized terms used across all HEF documentation, configuration files, and role prompts.

---

### A
- **ACI (Agent-Computer Interface)**: The set of specialized tools (ripgrep, file slicers, AST parsers, linter feedback) designed specifically for LLM agents to interact with a software project without context bloat.
- **Advisory Locking**: A soft-locking file mechanism preventing concurrent agents from modifying overlapping code paths simultaneously.
- **Archetype**: A reference architectural blueprint for a specific category of project (e.g., `web-fullstack`, `mobile-only`, `api-backend`).

### B
- **Blast Radius**: The full downstream surface of an application (consumers, database tables, tests, clients) that could be broken or impacted by a proposed change.

### D
- **Devil's Advocate**: A mandatory adversarial review role whose sole objective is finding hidden edge cases, failure modes, race conditions, and compliance risks before code is approved.
- **DORA Metrics**: DevOps Research and Assessment metrics (Deployment Frequency, Lead Time for Changes, Change Failure Rate, Time to Restore Service).

### F
- **Failure Memory**: A post-incident markdown record (`FAIL-NNN.md`) capturing root cause, timeline, and an automated prevention mechanism to immunize the codebase from repeat errors.

### G
- **Golden Path**: A pre-architected, pre-tested development pattern that provides developers and agents with the fastest, safest path to production.

### H
- **HEF (Harness Engineering Framework)**: The comprehensive engineering governance system uniting agent roles, quality gates, automated testing, and CI/CD pipelines.

### I
- **Impact Map**: A structured audit of symbols, APIs, and dependencies analyzed prior to executing changes in normal or high-risk lanes.

### K
- **Knowledge Item (KI)**: A persistent, structured documentation artifact capturing repo-specific configurations, workarounds, or patterns to prevent repetitive agent exploration.

### L
- **Lane Classification**: Triaging tasks by risk level (`bug:tiny`, `bug:normal`, `bug:high_risk`) to dictate the rigor of testing and role reviews required.

### M
- **MCP (Model Context Protocol)**: An open protocol enabling AI models to interact with local databases, browser drivers, and development tools via standardized RPC.
- **Microagent / Skill**: A modular set of prompt instructions and execution scripts activated on demand based on specific task contexts.

### Q
- **Quality Gate**: An automated or procedural checkpoint (e.g., 0 lint warnings, ≥ 80% coverage, 16KB ELF alignment) that must pass before promotion to the next stage.
