# Parallel Agents Orchestration Guide

## 1. Overview
When multiple AI agents or automated background subagents work simultaneously within the same codebase, they risk colliding on shared files, producing git merge conflicts, and invalidating each other's test baselines.

This guide defines how to partition work and maintain isolation across concurrent agent executions.

---

## 2. Work Partitioning Principles

```
                    [Orchestrator Agent]
                             │
            ┌────────────────┴────────────────┐
            ▼                                 ▼
   [Agent A: Backend Surface]        [Agent B: Web Admin Surface]
   (Branch: feat/api-payments)       (Branch: feat/admin-refunds)
            │                                 │
   Scoped: [backend-dir]/            Scoped: [frontend-dir]/
```

### 1. Surface Isolation
- Agents MUST be assigned non-overlapping file scopes whenever possible.
- Agent A operates exclusively in `[backend-dir]/`.
- Agent B operates exclusively in `[frontend-dir]/`.
- Cross-cutting files (`harness-engineering/`, root `package.json`, root `AGENTS.md`) require a single orchestrating agent.

### 2. Branch Per Agent Task
- Never run two parallel agents on the same local git working tree or branch.
- Each agent executes on its dedicated feature branch (`agent/<feature-name>-<id>`).

### 3. File Lock Mechanism (Advisory Locking)
Before starting multi-file modifications in complex modules, agents can create an advisory lockfile:
```json
// .agents/locks/auth-module.lock
{
  "locked_by": "Agent-Backend-01",
  "files": [
    "src/main/java/com/example/api/security/*",
    "src/main/resources/application.yml"
  ],
  "started_at": "2026-09-08T01:00:00Z",
  "expires_at": "2026-09-08T02:00:00Z"
}
```

---

## 3. Conflict Resolution Protocol
1. **Rebase, Never Force-Push**: The second agent to finish must rebase onto `main` and execute its validation suite against the newly merged changes.
2. **Contract Re-verification**: If Agent A modified an API DTO, Agent B's client integration tests must pass against Agent A's compiled schema before merging.
