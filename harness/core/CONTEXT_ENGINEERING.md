# Context Engineering Protocol

## 1. The Core Problem: Context Bloat & Primacy Bias
When AI agents work on software repositories, providing the entire codebase as raw text causes:
1. **Context Window Exhaustion**: Costly API calls and latency spikes.
2. **Primacy & Recency Bias**: Agents fixate on the start or end of large prompts and ignore middle instructions.
3. **Hallucinated Edits**: Large token dumps reduce attention precision, leading to broken syntax and ghost symbols.

---

## 2. The 3-Tier Context Architecture (Inspired by Aider & SWE-Agent)

```
┌─────────────────────────────────────────────────────────┐
│ Tier 1: System & Invariant Rules (AGENTS.md, Roles)     │ ~1,000 - 2,000 tokens
├─────────────────────────────────────────────────────────┤
│ Tier 2: Architectural Map & AST Signatures (Repo Map)   │ ~2,000 - 4,000 tokens
├─────────────────────────────────────────────────────────┤
│ Tier 3: Just-In-Time Active Files (Read on demand)      │ ~4,000 - 8,000 tokens
└─────────────────────────────────────────────────────────┘
```

### Tier 1: Invariant System Rules
- Keep surface-specific rules localized in directory `AGENTS.md`.
- Never put full implementation guides in root rules; link out to skills (`.agents/skills/`).

### Tier 2: AST / Repo Map
- Do not read entire files just to find where a method lives.
- Use tree-sitter or ripgrep (`grep_search`) to inspect function signatures, interface contracts, and exported symbols.
- Cache symbol locations in `IMPACT_MAP.md`.

### Tier 3: Targeted File Slices
- Use slice viewing (`StartLine` to `EndLine`) when inspecting large files (> 300 lines).
- Never view more than 800 lines in a single call unless performing an end-to-end audit.

---

## 3. Token Budgeting Guidelines

| Task Phase | Max Recommended Context | Primary Tools |
| :--- | :--- | :--- |
| **Intake & Exploration** | ≤ 6,000 tokens | `list_dir`, `grep_search`, `view_file` (headers only) |
| **Planning & Impact Map** | ≤ 10,000 tokens | Read target types, DTOs, schemas |
| **Code Implementation** | ≤ 16,000 tokens | `replace_file_content`, `multi_replace_file_content` |
| **Verification & Review** | ≤ 8,000 tokens | Run tests via terminal, parse only error outputs |

---

## 4. Context Pruning Rules
1. **Error Output Truncation**: When a test fails with 1,000 lines of stack traces, extract only the first relevant exception, failing file, and line number.
2. **Binary & Asset Exclusion**: Never read images, binaries, bundled `.min.js`, or lockfiles directly.
3. **Session Compaction Protocol**: When context approaches compaction limits, summarize past steps into atomic decision logs (`task.md`) before continuing.
