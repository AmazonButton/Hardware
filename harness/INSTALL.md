# Installing Harness Engineering Framework

This guide explains how to install the HEF into a new project.

> ⚠️ **Trước khi làm bất cứ điều gì**: đọc `scripts/bin/SECURITY_NOTE.md`. Bộ này đi
> kèm một binary CLI đã biên dịch sẵn (`harness-cli` / `harness-cli.exe`) mà không có
> mã nguồn kèm theo. Verify checksum bằng `scripts/checks/check_cli_checksum.sh` trước
> khi cấp quyền thực thi cho binary này trong dự án của bạn.

> 📌 **Không dùng `harness.config.yaml` có sẵn nếu nó thuộc về dự án khác.** Luôn bắt
> đầu từ `harness.config.yaml.example` (generic). File đã điền sẵn trong `examples/`
> (vd. `examples/dico-scan.harness.config.yaml`) chỉ là case study tham khảo cấu trúc,
> không phải file để copy trực tiếp — nó chứa tên dự án, stack, và forbidden_terms của
> một dự án cụ thể khác.

## Option A — Copy (Simplest)

```bash
cp -r harness-engineering/ /path/to/new-project/
cd /path/to/new-project/harness-engineering
```

Then edit `harness.config.yaml` to match your project's surfaces, stacks, and compliance requirements. Chọn `compliance.domain_pack` (`generic` | `health-wellness` | `fintech` | `ecommerce`) — xem `compliance/COMPLIANCE_FRAMEWORK.md` để biết nguyên tắc chung, và file tương ứng trong `compliance/domain-packs/` nếu chọn khác `generic`.

## Option B — Git Submodule (Recommended for teams)

```bash
cd /path/to/new-project
git submodule add https://github.com/your-org/harness-engineering harness-engineering
git submodule update --init --recursive
```

Create your project config:

```bash
cp harness-engineering/harness.config.yaml.example harness-engineering/harness.config.yaml
# Edit harness.config.yaml for your project
```

## Option C — PowerShell Install Script (Windows)

```powershell
# From repo root
.\harness-engineering\scripts\install-harness.ps1 -TargetProject C:\path\to\new-project
```

## Option D — Bash Install Script (macOS/Linux)

```bash
bash harness-engineering/scripts/install-harness.sh /path/to/new-project
```

---

## Post-Install Steps

### 1. Initialize the durable database

```bash
# macOS/Linux
./harness-engineering/scripts/bin/harness-cli init

# Windows
.\harness-engineering\scripts\bin\harness-cli.exe init
```

### 2. Copy the CLI binary to your project's scripts/bin/

The harness CLI binary must be available at `scripts/bin/harness-cli` (or `.exe` on Windows).  
The install script does this automatically. Manually:

```bash
mkdir -p scripts/bin
cp harness-engineering/scripts/bin/harness-cli scripts/bin/
```

### 3. Update your AGENTS.md

Add a Harness block pointing agents to `harness-engineering/`:

```markdown
## Harness

This repo uses the Harness Engineering Framework. Before work, read:

- `harness-engineering/README.md`
- `harness-engineering/core/HARNESS.md`
- `harness-engineering/core/FEATURE_INTAKE.md`
- `harness-engineering/roles/ROLE_SYSTEM.md`
- `harness-engineering/review-protocol/SESSION_REVIEW_PROTOCOL.md`

Run: `scripts/bin/harness-cli query matrix`
```

### 4. Add GitHub Actions to your workflow

Reference the reusable workflows:

```yaml
# .github/workflows/ci.yml
jobs:
  backend:
    uses: ./.github/workflows/reusable/backend-ci.yml
    with:
      java-version: '21'
    secrets: inherit

  harness-gate:
    uses: ./.github/workflows/reusable/harness-quality-gate.yml
    secrets: inherit
```

Or copy workflow files from `harness-engineering/workflows/reusable/` to `.github/workflows/`.

### 5. Set up Commit Convention (Optional but Recommended)

```bash
cd /path/to/new-project
npm install --save-dev @commitlint/cli @commitlint/config-conventional
cp harness-engineering/commit-convention/commitlint.config.js .
```

Add to `package.json`:

```json
{
  "scripts": {
    "commitlint": "commitlint --from HEAD~1 --to HEAD --verbose"
  }
}
```

---

## Configuration

After install, customize `harness-engineering/harness.config.yaml`:

| Key | Description |
|---|---|
| `project.name` | Human-readable project name |
| `surfaces.*` | Which surfaces exist (backend, frontend, mobile) |
| `roles.*` | Which agent roles are active |
| `lanes.*` | Role review requirements per risk lane |
| `quality_gates.*` | Thresholds per surface |
| `branching.*` | Strategy, naming convention |
| `compliance.*` | Legal rules, forbidden terms |
| `dora.*` | DORA metric targets |

---

## Minimal Install (Core only)

If you only want the policy docs and CLI (no workflows, no role system):

```bash
mkdir -p new-project/scripts/bin new-project/docs
cp harness-engineering/scripts/bin/harness-cli new-project/scripts/bin/
cp -r harness-engineering/core/ new-project/docs/harness/
cp -r harness-engineering/templates/ new-project/docs/templates/
cp harness-engineering/harness.config.yaml new-project/harness-engineering/
```

---

## Verifying Installation

```bash
# Should print version
scripts/bin/harness-cli --version

# Should print empty matrix (new project)
scripts/bin/harness-cli query matrix

# Should succeed
scripts/bin/harness-cli intake --type "new-spec" --summary "Project setup" --lane tiny
```
