# Harness CLI Quick Runner for Windows PowerShell (Hardware Surface)
param(
    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]]$CliArgs
)

$HarnessScript = Join-Path (Split-Path $PSScriptRoot -Parent) "harness\scripts\harness_cli.py"

if (-not (Test-Path $HarnessScript)) {
    Write-Error "Cannot find harness_cli.py at $HarnessScript"
    exit 1
}

python $HarnessScript @CliArgs
