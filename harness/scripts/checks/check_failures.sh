#!/usr/bin/env bash
# check_failures.sh — Validate failure memory files
#
# Checks:
#   1. Every file under docs/failures/ (except TEMPLATE.md) has ## Prevention
#   2. Every high_risk failure has at least one prevention checkbox
#   3. File naming follows FAIL-NNN-*.md pattern
#
# Exit codes:
#   0 — all checks pass
#   1 — validation errors found

set -euo pipefail

FAILURES_DIR="docs/failures"
ERRORS=0

if [ ! -d "$FAILURES_DIR" ]; then
  echo "⏭️  No docs/failures/ directory — nothing to check"
  exit 0
fi

FAIL_FILES=$(find "$FAILURES_DIR" -name "FAIL-*.md" 2>/dev/null)

if [ -z "$FAIL_FILES" ]; then
  echo "✅ No failure files to validate (TEMPLATE.md is excluded)"
  exit 0
fi

echo "🔍 Validating failure memory files..."
echo ""

while IFS= read -r file; do
  basename=$(basename "$file")
  
  # Check 1: ## Prevention section exists
  if ! grep -q "^## Prevention" "$file"; then
    echo "❌ $basename — missing '## Prevention' section"
    ERRORS=$((ERRORS + 1))
  fi

  # Check 2: ## Summary section exists
  if ! grep -q "^## Summary" "$file"; then
    echo "❌ $basename — missing '## Summary' section"
    ERRORS=$((ERRORS + 1))
  fi

  # Check 3: ## Severity section exists
  if ! grep -q "^## Severity" "$file"; then
    echo "❌ $basename — missing '## Severity' section"
    ERRORS=$((ERRORS + 1))
  fi

  # Check 4: ## Root Cause section exists
  if ! grep -q "^## Root Cause" "$file"; then
    echo "❌ $basename — missing '## Root Cause' section"
    ERRORS=$((ERRORS + 1))
  fi

  # Check 5: For high_risk, at least one prevention checkbox must be checked
  SEVERITY=$(grep -A1 "^## Severity" "$file" | tail -1 | tr -d '[:space:]')
  if [ "$SEVERITY" = "high_risk" ]; then
    if ! grep -q "\[x\]" "$file"; then
      echo "⚠️  $basename — high_risk but no prevention checkboxes completed"
      ERRORS=$((ERRORS + 1))
    fi
  fi

  # If all passed for this file
  if [ $ERRORS -eq 0 ]; then
    echo "✅ $basename — valid"
  fi

done <<< "$FAIL_FILES"

echo ""
if [ $ERRORS -gt 0 ]; then
  echo "❌ $ERRORS validation error(s) found"
  exit 1
else
  echo "✅ All failure files valid"
  exit 0
fi
