-- Harness schema — migration 003
-- Adds per-role token/cost tracking so the harness can measure and later
-- optimize which roles are worth activating for which lane.
-- Depends on: 001-init.sql (trace table), 002-story-verify.sql

PRAGMA foreign_keys = ON;

INSERT INTO schema_version (version) VALUES (3);

----------------------------------------------------------------------
-- Per-role cost breakdown for a given trace (task execution)
----------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS trace_role_cost (
    id                INTEGER PRIMARY KEY AUTOINCREMENT,
    trace_id          INTEGER NOT NULL REFERENCES trace(id),
    role              TEXT    NOT NULL
                      CHECK(role IN (
                        'orchestrator','frontend_engineer','backend_engineer',
                        'devops_engineer','security_auditor','qa_engineer',
                        'devil_advocate'
                      )),
    tokens_used       INTEGER NOT NULL DEFAULT 0,
    duration_seconds  INTEGER,
    verdict           TEXT CHECK(verdict IN ('pass','block','warning') OR verdict IS NULL),
    created_at        TEXT NOT NULL DEFAULT (datetime('now'))
);

CREATE INDEX IF NOT EXISTS idx_trace_role_cost_trace ON trace_role_cost(trace_id);
CREATE INDEX IF NOT EXISTS idx_trace_role_cost_role ON trace_role_cost(role);

----------------------------------------------------------------------
-- Convenience view: total tokens and average verdict rate per role
----------------------------------------------------------------------
CREATE VIEW IF NOT EXISTS v_cost_by_role AS
SELECT
    role,
    COUNT(*)                                            AS review_count,
    SUM(tokens_used)                                    AS total_tokens,
    ROUND(AVG(tokens_used), 1)                          AS avg_tokens_per_review,
    SUM(CASE WHEN verdict = 'block' THEN 1 ELSE 0 END)  AS block_count,
    ROUND(
      100.0 * SUM(CASE WHEN verdict = 'block' THEN 1 ELSE 0 END) / COUNT(*), 1
    )                                                    AS block_rate_pct
FROM trace_role_cost
GROUP BY role;
