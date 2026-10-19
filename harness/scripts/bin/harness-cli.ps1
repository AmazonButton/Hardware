# Compatibility wrapper — keeps `scripts\bin\harness-cli.ps1 <args>` working
# on Windows, forwarding to the auditable scripts/harness_cli.py (v3+).
# See scripts/bin/SECURITY_NOTE.md for why the old .exe binary was removed.
$Here = Split-Path -Parent $MyInvocation.MyCommand.Path
python3 (Join-Path $Here "..\harness_cli.py") @Args
