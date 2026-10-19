---
description: Run the Solutioning Gate check before any story packet is created
---

Use the architect subagent to evaluate the Solutioning Gate for: $ARGUMENTS

Follow `harness-engineering/discovery/templates/SOLUTIONING_GATE_REPORT.md` — Default-FAIL, every PASS row needs an evidence_ref. Then record the verdict:

```bash
python3 harness-engineering/scripts/harness_cli.py gate decide <id> --verdict <pass|concerns|fail> --reviewed-by architect --notes "..."
```

Do NOT create story packets if the verdict is not `pass`.
