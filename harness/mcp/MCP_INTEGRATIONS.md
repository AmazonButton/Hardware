# Model Context Protocol (MCP) Integration Guide

## 1. Overview
The **Model Context Protocol (MCP)** provides open standard connections between AI developer assistants and local developer tooling, databases, browser engines, and testing frameworks.

Instead of writing bespoke ad-hoc scripts, AI agents use standardized MCP tool calls to inspect schema definitions, execute database queries in sandboxes, manipulate web browsers, and run mobile test assertions.

---

## 2. Configured MCP Servers

### 1. PostgreSQL (`@modelcontextprotocol/server-postgres`)
- **Use Case**: Allows the Backend Engineer and QA Engineer roles to inspect live/staging schemas, run read-only EXPLAIN queries, and verify Flyway migration outputs.
- **Security Rule**: Read-only credentials by default. Never expose production database write credentials to agent MCP configs.

### 2. Filesystem (`@modelcontextprotocol/server-filesystem`)
- **Use Case**: Granular file browsing, workspace indexing, and directory structure exploration outside immediate context windows.

### 3. GitHub (`@modelcontextprotocol/server-github`)
- **Use Case**: Checking open pull requests, reading CI action run failures, inspecting release tags, and updating issue discussions.

### 4. Puppeteer / Playwright (`@modelcontextprotocol/server-puppeteer`)
- **Use Case**: Headless browser automation for verifying responsive web layout, DOM rendering, taking screenshots, and testing accessibility.

### 5. Maestro Mobile (`maestro-mcp`)
- **Use Case**: Direct agent execution of declarative mobile test flows on Android emulators (BlueStacks/AVD) with automatic screenshot assertions.

---

### 6. Chrome DevTools / Browser MCP (`chrome-devtools-mcp` or equivalent)
- **Use Case**: Frontend Engineer role inspects live console errors, network waterfalls, and
  performance traces directly instead of guessing from source alone. Complements Playwright
  for cases needing real browser devtools data, not just DOM assertions.

### 7. Deploy-target MCP servers (Vercel / Cloudflare / your cloud provider)
- **Use Case**: DevOps Engineer role reads deployment status, environment variables (names
  only, never values) and recent deploy logs when reviewing `stack-packs/deploy/*.md`
  changes. Configure read-only scopes; never grant an MCP server production write access by
  default.

### 8. Issue tracker MCP (Linear / Jira / GitHub Issues — pick one)
- **Use Case**: Orchestrator role links intake/story records to the team's actual backlog
  instead of only `scripts/harness_cli.py` records, when a project already runs a tracker.

> Đừng bật tất cả server ở trên cho mọi dự án. Chỉ nạp server cần thiết cho surfaces đang
> `enabled: true` trong `harness.config.yaml` — mỗi MCP server thêm vào là thêm bề mặt tấn
> công + context overhead.

---

## 3.5 Plugin Marketplace (mới 2026)

Nhiều agent runtime (Claude Code và tương thích ACP) hiện hỗ trợ **plugin** — 1 bundle đóng
gói sẵn skill + subagent + hook + MCP server config, cài bằng 1 lệnh. Đây là cách nhanh nhất
để đưa 1 vai trò harness (vd Security Auditor) vào 1 dự án mới, nhưng cũng là bề mặt tấn công
lớn hơn 1 skill đơn lẻ vì nó bundle nhiều thứ cùng lúc.

**Trước khi cài 1 plugin bên thứ ba**: áp dụng nguyên tắc trong `core/SKILLS_SECURITY.md`
cho TOÀN BỘ bundle (đọc hết skill + hook + MCP config bên trong, không chỉ đọc mô tả
plugin). Ưu tiên plugin do chính team maintain hoặc từ nguồn có review công khai.

---

## 4. How to Connect in Your IDE / Agent Environment
- **Claude Code / Cursor / Antigravity / other ACP-compatible harness**: Reference `harness-engineering/mcp/mcp.config.json` in your local IDE config or pass via command line flags.
- **Environment Variables**: Ensure sensitive keys (e.g., `GITHUB_PERSONAL_ACCESS_TOKEN`, `DB_PASSWORD`) are sourced from local environment files, NEVER committed directly into the repository.
- **Native subagents**: since v3, the 7 roles in `roles/*.md` also exist as ready-to-use
  Claude Code subagent definitions in `.claude/agents/*.md` (YAML frontmatter: `tools`,
  `model`). Reviewer roles are scoped to read-only tools per the Fresh-Context Evaluator
  principle (`roles/ROLE_SYSTEM.md`) — this is enforced by the `tools:` field, not just
  prose instruction.
