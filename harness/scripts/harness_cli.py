#!/usr/bin/env python3
"""
harness_cli.py — source-available reference CLI for the Harness Engineering
Framework (v3). Pure Python 3.9+ stdlib (sqlite3, argparse, json) — no
compiled binary, no external dependencies, fully auditable in a code review.

Why this exists: v0.2.0 shipped `scripts/bin/harness-cli(.exe)` as a
precompiled Rust binary with no source in the bundle — an unverifiable
supply-chain risk flagged as the #1 gap in GAP_ANALYSIS_AND_ROADMAP.md.
This script is the default CLI for v3. A faster/richer compiled CLI (e.g. a
real Rust build with published source + CI-built checksums) MAY still be
adopted later, but only once its source ships in this repo per
`scripts/bin/SECURITY_NOTE.md`.

Usage:
    python3 harness_cli.py init
    python3 harness_cli.py intake --type change_request --summary "..." --lane normal
    python3 harness_cli.py story add --id US-042 --title "..." --lane normal
    python3 harness_cli.py story verify US-042 --command "npm test" --result pass
    python3 harness_cli.py decision add --id 0012 --title "..." --doc docs/decisions/0012.md
    python3 harness_cli.py gate add --summary "New checkout flow" --track standard \
        --brief discovery/briefs/checkout.md --prd discovery/prds/checkout.md
    python3 harness_cli.py gate decide 1 --verdict pass --reviewed-by architect,ux_designer
    python3 harness_cli.py query gates
    python3 harness_cli.py trace --summary "..." --outcome completed --agent qa_engineer
    python3 harness_cli.py trace-cost add --trace-id 3 --role qa_engineer --tokens 4200 --verdict pass
    python3 harness_cli.py query matrix
    python3 harness_cli.py query friction
    python3 harness_cli.py query cost
    python3 harness_cli.py packs resolve   # prints which domain-pack + stack-packs to load
"""
from __future__ import annotations
import argparse
import json
import sqlite3
import sys
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent  # harness-engineering/
DB_PATH = ROOT / "harness.db"
SCHEMA_DIR = ROOT / "scripts" / "schema"
CONFIG_PATH = ROOT / "harness.config.yaml"


def connect() -> sqlite3.Connection:
    conn = sqlite3.connect(DB_PATH)
    conn.row_factory = sqlite3.Row
    conn.execute("PRAGMA foreign_keys = ON")
    return conn


def cmd_init(_args) -> None:
    if DB_PATH.exists():
        print(f"[init] {DB_PATH} already exists — applying any new migrations only.")
    conn = connect()
    applied = set()
    try:
        applied = {r[0] for r in conn.execute("SELECT version FROM schema_version")}
    except sqlite3.OperationalError:
        pass  # schema_version table doesn't exist yet — first run
    migrations = sorted(SCHEMA_DIR.glob("*.sql"))
    if not migrations:
        print(f"[init] No migration files found in {SCHEMA_DIR}", file=sys.stderr)
        sys.exit(1)
    for mig in migrations:
        version = int(mig.stem.split("-")[0])
        if version in applied:
            continue
        sql = mig.read_text(encoding="utf-8")
        conn.executescript(sql)
        conn.commit()
        print(f"[init] applied {mig.name}")
    print(f"[init] database ready at {DB_PATH}")


def cmd_intake(args) -> None:
    conn = connect()
    cur = conn.execute(
        "INSERT INTO intake (input_type, summary, risk_lane, notes) VALUES (?, ?, ?, ?)",
        (args.type, args.summary, args.lane, args.notes),
    )
    conn.commit()
    print(f"[intake] recorded id={cur.lastrowid} lane={args.lane}")


def cmd_story_add(args) -> None:
    conn = connect()
    conn.execute(
        "INSERT INTO story (id, title, risk_lane, contract_doc, status, notes) "
        "VALUES (?, ?, ?, ?, 'planned', ?)",
        (args.id, args.title, args.lane, args.contract_doc, args.notes),
    )
    conn.commit()
    print(f"[story] added {args.id} ({args.lane})")


def cmd_story_verify(args) -> None:
    conn = connect()
    now = datetime.now(timezone.utc).isoformat()
    conn.execute(
        "UPDATE story SET verify_command=?, last_verified_at=?, last_verified_result=?, "
        "status=CASE WHEN ?='pass' THEN 'implemented' ELSE status END WHERE id=?",
        (args.command, now, args.result, args.result, args.id),
    )
    if conn.total_changes == 0:
        print(f"[story] WARNING: no story with id={args.id}", file=sys.stderr)
        sys.exit(1)
    conn.commit()
    print(f"[story] {args.id} verify -> {args.result}")


def cmd_decision_add(args) -> None:
    conn = connect()
    conn.execute(
        "INSERT INTO decision (id, title, doc_path, verify_command, notes) "
        "VALUES (?, ?, ?, ?, ?)",
        (args.id, args.title, args.doc, args.verify_command, args.notes),
    )
    conn.commit()
    print(f"[decision] recorded {args.id}")


def cmd_trace(args) -> None:
    conn = connect()
    cur = conn.execute(
        "INSERT INTO trace (task_summary, agent, outcome, duration_seconds, "
        "token_estimate, harness_friction, notes) VALUES (?, ?, ?, ?, ?, ?, ?)",
        (args.summary, args.agent, args.outcome, args.duration, args.tokens,
         args.friction, args.notes),
    )
    conn.commit()
    print(f"[trace] recorded id={cur.lastrowid} outcome={args.outcome}")


def cmd_trace_cost_add(args) -> None:
    conn = connect()
    conn.execute(
        "INSERT INTO trace_role_cost (trace_id, role, tokens_used, duration_seconds, verdict) "
        "VALUES (?, ?, ?, ?, ?)",
        (args.trace_id, args.role, args.tokens, args.duration, args.verdict),
    )
    conn.commit()
    print(f"[trace-cost] recorded role={args.role} tokens={args.tokens}")


def _print_rows(rows, cols) -> None:
    if not rows:
        print("(no rows)")
        return
    widths = [max(len(c), max((len(str(r[c])) for r in rows), default=0)) for c in cols]
    print(" | ".join(c.ljust(w) for c, w in zip(cols, widths)))
    print("-+-".join("-" * w for w in widths))
    for r in rows:
        print(" | ".join(str(r[c]).ljust(w) for c, w in zip(cols, widths)))


def cmd_query_matrix(_args) -> None:
    conn = connect()
    rows = conn.execute(
        "SELECT id, title, risk_lane, status, last_verified_result FROM story ORDER BY id"
    ).fetchall()
    _print_rows(rows, ["id", "title", "risk_lane", "status", "last_verified_result"])


def cmd_query_friction(_args) -> None:
    conn = connect()
    rows = conn.execute(
        "SELECT id, created_at, task_summary, harness_friction FROM trace "
        "WHERE harness_friction IS NOT NULL AND harness_friction != '' "
        "ORDER BY created_at DESC LIMIT 50"
    ).fetchall()
    _print_rows(rows, ["id", "created_at", "task_summary", "harness_friction"])


def cmd_query_cost(_args) -> None:
    conn = connect()
    try:
        rows = conn.execute(
            "SELECT role, review_count, total_tokens, avg_tokens_per_review, "
            "block_count, block_rate_pct FROM v_cost_by_role ORDER BY total_tokens DESC"
        ).fetchall()
    except sqlite3.OperationalError:
        print("v_cost_by_role view not found — run `harness_cli.py init` "
              "to apply migration 003-cost-tracking.sql", file=sys.stderr)
        sys.exit(1)
    _print_rows(rows, ["role", "review_count", "total_tokens", "avg_tokens_per_review",
                        "block_count", "block_rate_pct"])


def cmd_gate_add(args) -> None:
    conn = connect()
    cur = conn.execute(
        "INSERT INTO solutioning_gate (intake_id, initiative_summary, planning_track, "
        "brief_doc, prd_doc, architecture_doc, ux_spec_doc) VALUES (?, ?, ?, ?, ?, ?, ?)",
        (args.intake_id, args.summary, args.track, args.brief, args.prd,
         args.architecture, args.ux_spec),
    )
    conn.commit()
    print(f"[gate] created id={cur.lastrowid} track={args.track} verdict=pending")


def cmd_gate_decide(args) -> None:
    conn = connect()
    conn.execute(
        "UPDATE solutioning_gate SET verdict=?, verdict_notes=?, reviewed_by=?, "
        "story_ids=? WHERE id=?",
        (args.verdict, args.notes, args.reviewed_by, args.story_ids, args.id),
    )
    if conn.total_changes == 0:
        print(f"[gate] WARNING: no gate with id={args.id}", file=sys.stderr)
        sys.exit(1)
    conn.commit()
    print(f"[gate] {args.id} -> {args.verdict}")
    if args.verdict == "fail":
        print("[gate] FAIL — do not create story packets / start implementation "
              "for this initiative until re-planned and re-gated.", file=sys.stderr)


def cmd_query_gates(_args) -> None:
    conn = connect()
    rows = conn.execute(
        "SELECT id, created_at, initiative_summary, planning_track, verdict "
        "FROM v_open_solutioning_gates"
    ).fetchall()
    _print_rows(rows, ["id", "created_at", "initiative_summary", "planning_track", "verdict"])


def _load_yaml_minimal(path: Path) -> dict:
    """Tiny dependency-free YAML reader for the flat keys this command needs.
    Not a general YAML parser — only handles `key: value` and `key:` (nested)
    at up to 2 levels of 2-space indentation, which is all harness.config.yaml
    uses for the fields read here. For anything more complex, install PyYAML
    and swap this out."""
    result: dict = {}
    stack = [(0, result)]
    for raw in path.read_text(encoding="utf-8").splitlines():
        if not raw.strip() or raw.strip().startswith("#"):
            continue
        indent = len(raw) - len(raw.lstrip(" "))
        line = raw.strip()
        if ":" not in line:
            continue
        key, _, val = line.partition(":")
        key = key.strip()
        val = val.strip().strip('"').split("#")[0].strip().strip('"')
        while stack and indent < stack[-1][0]:
            stack.pop()
        target = stack[-1][1]
        if val == "":
            new_dict: dict = {}
            target[key] = new_dict
            stack.append((indent + 2, new_dict))
        else:
            target[key] = val
    return result


def cmd_packs_resolve(_args) -> None:
    if not CONFIG_PATH.exists():
        print(f"No harness.config.yaml found at {CONFIG_PATH}", file=sys.stderr)
        sys.exit(1)
    cfg = _load_yaml_minimal(CONFIG_PATH)
    domain = cfg.get("project", {}).get("domain", "generic")
    print("=== Context packs to load for this project ===")
    if domain and domain != "generic":
        print(f"compliance/domain-packs/{domain}.md")
    else:
        print("(domain=generic — no domain-pack to load, "
              "compliance/COMPLIANCE_FRAMEWORK.md still applies)")
    surfaces = cfg.get("surfaces", {})
    group_map = {
        "frontend_web": "stack-packs/frontend",
        "mobile": "stack-packs/mobile",
        "backend": "stack-packs/backend",
        "infrastructure": "stack-packs/deploy",
        "firmware": "stack-packs/firmware",
        "hardware": "stack-packs/hardware",
        "test_bench": "stack-packs/frontend",
    }
    for surface, group_dir in group_map.items():
        surf_cfg = surfaces.get(surface, {})
        if isinstance(surf_cfg, dict) and str(surf_cfg.get("enabled", "")).lower() == "true":
            stack = surf_cfg.get("stack", "")
            print(f"{group_dir}/  (surface={surface}, declared stack='{stack}' "
                  f"— pick the matching file, see stack-packs/README.md)")


def build_parser() -> argparse.ArgumentParser:
    p = argparse.ArgumentParser(prog="harness_cli.py", description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = p.add_subparsers(dest="cmd", required=True)

    sub.add_parser("init", help="Apply pending schema migrations to harness.db").set_defaults(func=cmd_init)

    s = sub.add_parser("intake", help="Record an intake classification")
    s.add_argument("--type", required=True,
                    choices=["new_spec", "spec_slice", "change_request", "new_initiative",
                             "maintenance", "harness_improvement"])
    s.add_argument("--summary", required=True)
    s.add_argument("--lane", required=True, choices=["tiny", "normal", "high_risk"])
    s.add_argument("--notes", default=None)
    s.set_defaults(func=cmd_intake)

    story = sub.add_parser("story", help="Story packet operations")
    story_sub = story.add_subparsers(dest="story_cmd", required=True)

    sa = story_sub.add_parser("add")
    sa.add_argument("--id", required=True)
    sa.add_argument("--title", required=True)
    sa.add_argument("--lane", required=True, choices=["tiny", "normal", "high_risk"])
    sa.add_argument("--contract-doc", dest="contract_doc", default=None)
    sa.add_argument("--notes", default=None)
    sa.set_defaults(func=cmd_story_add)

    sv = story_sub.add_parser("verify")
    sv.add_argument("id")
    sv.add_argument("--command", required=True)
    sv.add_argument("--result", required=True, choices=["pass", "fail"])
    sv.set_defaults(func=cmd_story_verify)

    d = sub.add_parser("decision", help="Decision record operations")
    d_sub = d.add_subparsers(dest="decision_cmd", required=True)
    da = d_sub.add_parser("add")
    da.add_argument("--id", required=True)
    da.add_argument("--title", required=True)
    da.add_argument("--doc", required=True)
    da.add_argument("--verify-command", dest="verify_command", default=None)
    da.add_argument("--notes", default=None)
    da.set_defaults(func=cmd_decision_add)

    t = sub.add_parser("trace", help="Record a task execution trace")
    t.add_argument("--summary", required=True)
    t.add_argument("--agent", default=None)
    t.add_argument("--outcome", required=True,
                    choices=["completed", "blocked", "partial", "failed"])
    t.add_argument("--duration", type=int, default=None)
    t.add_argument("--tokens", type=int, default=None)
    t.add_argument("--friction", default=None)
    t.add_argument("--notes", default=None)
    t.set_defaults(func=cmd_trace)

    tc = sub.add_parser("trace-cost", help="Per-role cost tracking")
    tc_sub = tc.add_subparsers(dest="trace_cost_cmd", required=True)
    tca = tc_sub.add_parser("add")
    tca.add_argument("--trace-id", dest="trace_id", type=int, required=True)
    tca.add_argument("--role", required=True,
                      choices=["orchestrator", "frontend_engineer", "backend_engineer",
                               "devops_engineer", "security_auditor", "qa_engineer",
                               "devil_advocate"])
    tca.add_argument("--tokens", type=int, required=True)
    tca.add_argument("--duration", type=int, default=None)
    tca.add_argument("--verdict", choices=["pass", "block", "warning"], default=None)
    tca.set_defaults(func=cmd_trace_cost_add)

    q = sub.add_parser("query", help="Read-only reports")
    q_sub = q.add_subparsers(dest="query_cmd", required=True)
    q_sub.add_parser("matrix").set_defaults(func=cmd_query_matrix)
    q_sub.add_parser("friction").set_defaults(func=cmd_query_friction)
    q_sub.add_parser("cost").set_defaults(func=cmd_query_cost)
    q_sub.add_parser("gates", help="Open solutioning gates (pending/concerns)").set_defaults(func=cmd_query_gates)

    gate = sub.add_parser("gate", help="Solutioning gate (Discovery/Planning -> Implementation checkpoint)")
    gate_sub = gate.add_subparsers(dest="gate_cmd", required=True)
    ga = gate_sub.add_parser("add")
    ga.add_argument("--intake-id", dest="intake_id", type=int, default=None)
    ga.add_argument("--summary", required=True)
    ga.add_argument("--track", required=True, choices=["quick_flow", "standard", "enterprise"])
    ga.add_argument("--brief", default=None)
    ga.add_argument("--prd", default=None)
    ga.add_argument("--architecture", default=None)
    ga.add_argument("--ux-spec", dest="ux_spec", default=None)
    ga.set_defaults(func=cmd_gate_add)

    gd = gate_sub.add_parser("decide")
    gd.add_argument("id", type=int)
    gd.add_argument("--verdict", required=True, choices=["pass", "concerns", "fail"])
    gd.add_argument("--notes", default=None)
    gd.add_argument("--reviewed-by", dest="reviewed_by", default=None)
    gd.add_argument("--story-ids", dest="story_ids", default=None,
                     help='JSON array string, e.g. \'["US-101","US-102"]\'')
    gd.set_defaults(func=cmd_gate_decide)

    packs = sub.add_parser("packs", help="Domain-pack / stack-pack resolution")
    packs_sub = packs.add_subparsers(dest="packs_cmd", required=True)
    packs_sub.add_parser("resolve").set_defaults(func=cmd_packs_resolve)

    return p


def main() -> None:
    parser = build_parser()
    args = parser.parse_args()
    if not DB_PATH.exists() and args.cmd not in ("init",):
        print(f"No database at {DB_PATH} — run `python3 harness_cli.py init` first.",
              file=sys.stderr)
        sys.exit(1)
    args.func(args)


if __name__ == "__main__":
    main()
