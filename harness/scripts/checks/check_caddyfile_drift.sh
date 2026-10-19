#!/usr/bin/env bash
# check_caddyfile_drift.sh — Compare repo Caddyfile with live server
#
# Usage:
#   bash scripts/checks/check_caddyfile_drift.sh
#
# Prerequisites:
#   - SSH key configured via SSH_KEY or at infrastructure/secrets/deploy-key.pem
#   - SSH access to live server host
#
# Exit codes:
#   0 — no drift (or server unreachable, treated as skip)
#   1 — drift detected

set -euo pipefail

REPO_CADDYFILE="${REPO_CADDYFILE:-infrastructure/config/Caddyfile}"
SSH_KEY="${SSH_KEY:-infrastructure/secrets/deploy-key.pem}"
SERVER_USER="${SERVER_USER:-ubuntu}"
SERVER_IP="${SERVER_IP:-127.0.0.1}"
REMOTE_CADDYFILE="${REMOTE_CADDYFILE:-~/Caddyfile}"

if [ ! -f "$REPO_CADDYFILE" ]; then
  echo "⚠️  Repo Caddyfile not found at $REPO_CADDYFILE"
  exit 1
fi

if [ ! -f "$SSH_KEY" ]; then
  echo "⏭️  SSH key not found at $SSH_KEY — skipping drift check"
  echo "   (This is expected in CI without SSH access)"
  exit 0
fi

echo "🔍 Fetching live Caddyfile from $SERVER_IP..."

LIVE_CADDYFILE=$(mktemp)
trap 'rm -f "$LIVE_CADDYFILE"' EXIT

if ! ssh -o StrictHostKeyChecking=no -o ConnectTimeout=10 \
       -i "$SSH_KEY" "$SERVER_USER@$SERVER_IP" \
       "cat $REMOTE_CADDYFILE" > "$LIVE_CADDYFILE" 2>/dev/null; then
  echo "⏭️  Could not reach server — skipping drift check"
  exit 0
fi

echo "📋 Comparing repo vs live Caddyfile..."
echo ""

if diff -u "$REPO_CADDYFILE" "$LIVE_CADDYFILE" > /dev/null 2>&1; then
  echo "✅ No drift detected — repo Caddyfile matches live server"
  exit 0
else
  echo "❌ DRIFT DETECTED — repo Caddyfile differs from live server:"
  echo ""
  diff -u "$REPO_CADDYFILE" "$LIVE_CADDYFILE" || true
  echo ""
  echo "⚠️  Action required:"
  echo "   1. SSH to server: ssh -i $SSH_KEY $SERVER_USER@$SERVER_IP"
  echo "   2. Compare: diff ~/Caddyfile <repo>/infrastructure/config/Caddyfile"
  echo "   3. Update repo OR server to match"
  echo "   4. See infrastructure/README.md 'Open inconsistency' section"
  exit 1
fi
