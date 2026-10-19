#!/usr/bin/env bash
# check_cli_checksum.sh — Optional integrity check for scripts/harness_cli.py
# (the source-available CLI, v3+) before allowing it to run.
#
# Since v3, harness-cli is plain-text Python source (auditable in any code
# review), NOT a compiled binary — so this checksum gate is no longer a
# security requirement, only an OPTIONAL extra tripwire for teams that want
# to detect unexpected local modification of the script (e.g. shared/CI
# runners with looser file permissions). Most projects can skip configuring
# expected_sha256 entirely.
#
# Exit codes:
#   0 — checksum matches (or no expected_sha256 configured: info + pass)
#   1 — checksum mismatch — investigate before trusting the script
#   2 — script not found

set -euo pipefail

CONFIG_FILE="harness.config.yaml"
BIN="./scripts/harness_cli.py"

if [ ! -f "$BIN" ]; then
  echo "❌ harness_cli.py not found at $BIN"
  exit 2
fi

if [ ! -f "$CONFIG_FILE" ]; then
  echo "⚠️  $CONFIG_FILE not found — cannot verify expected checksum. Proceeding without verification."
  exit 0
fi

EXPECTED=$(grep -E '^\s*expected_sha256:' "$CONFIG_FILE" | sed -E 's/.*expected_sha256:\s*"?([a-fA-F0-9]*)"?.*/\1/' | tr -d '[:space:]')

if [ -z "$EXPECTED" ] || [ "$EXPECTED" = "<điền" ] || [[ "$EXPECTED" == *"<"* ]]; then
  echo "ℹ️  No expected_sha256 configured in $CONFIG_FILE — skipping (this is fine,"
  echo "    scripts/harness_cli.py is source, not a binary — read it instead of hashing it)."
  exit 0
fi

ACTUAL=$(sha256sum "$BIN" | awk '{print $1}')

if [ "$ACTUAL" != "$EXPECTED" ]; then
  echo "❌ CHECKSUM MISMATCH for $BIN"
  echo "   expected: $EXPECTED"
  echo "   actual:   $ACTUAL"
  echo "   Investigate before trusting this script. See scripts/bin/SECURITY_NOTE.md."
  exit 1
fi

echo "✅ Checksum verified for $BIN"
exit 0
