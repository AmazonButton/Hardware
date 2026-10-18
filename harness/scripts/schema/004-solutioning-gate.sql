-- Harness v4 schema — migration 004
-- Solutioning Gate: formal PASS/CONCERNS/FAIL checkpoint between Planning
-- (Project Brief -> PRD -> Architecture -> UX Spec) and Implementation.
-- See discovery/README.md and core/SOLUTIONING_GATE.md.

PRAGMA foreign_keys = ON;

INSERT INTO schema_version (version) VALUES (4);

----------------------------------------------------------------------
-- Solutioning gate: one row per initiative/spec going through Planning
-- before implementation starts. Links back to an intake row.
----------------------------------------------------------------------
CREATE TABLE solutioning_gate (
    id                 INTEGER PRIMARY KEY AUTOINCREMENT,
    created_at         TEXT    NOT NULL DEFAULT (datetime('now')),
    intake_id          INTEGER REFERENCES intake(id),
    initiative_summary TEXT    NOT NULL,
    planning_track     TEXT    NOT NULL
                       CHECK(planning_track IN ('quick_flow','standard','enterprise')),
    brief_doc          TEXT,           -- discovery/briefs/<slug>.md
    prd_doc             TEXT,           -- discovery/prds/<slug>.md
    architecture_doc    TEXT,           -- discovery/architecture/<slug>.md
    ux_spec_doc         TEXT,           -- discovery/ux-specs/<slug>.md (nullable — not all initiatives touch UI)
    verdict             TEXT    NOT NULL DEFAULT 'pending'
                       CHECK(verdict IN ('pending','pass','concerns','fail')),
    verdict_notes        TEXT,
    reviewed_by          TEXT,           -- role(s) that signed off, comma-separated
    story_ids            TEXT            -- JSON array of story.id created after gate passes
);

CREATE INDEX idx_solutioning_gate_intake ON solutioning_gate(intake_id);

----------------------------------------------------------------------
-- View: open gates (not yet pass/fail) — quick "what's stuck in planning" report
----------------------------------------------------------------------
CREATE VIEW v_open_solutioning_gates AS
SELECT id, created_at, initiative_summary, planning_track, verdict
FROM solutioning_gate
WHERE verdict IN ('pending', 'concerns')
ORDER BY created_at ASC;
