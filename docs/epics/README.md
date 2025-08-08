# Epics Index (Canonical Source)

This page is the single source of truth for epic mapping, status, and key documents. All epic-related materials should live under `docs/epics/{epic-id}/`. Superseded/legacy files should be moved to `docs/archive/` with an archive banner.

Statuses are derived from the current workflow baseline and repository state.

| Epic ID | Name                    | Status       | Directory                                                 | Key Docs (canonical)                          |
|--------:|-------------------------|--------------|-----------------------------------------------------------|-----------------------------------------------|
| 07      | Realtime Generation     | Complete     | docs/epics/epic-07-realtime-generation/                   | index.md • status.md • plan.md • reports/     |
| 08      | Effects Processing      | Complete     | docs/epics/epic-08-effects-processing/                    | index.md • status.md • plan.md • reports/     |
| 09      | Sound Design            | In Progress  | docs/epics/epic-09-sound-design/                          | index.md • status.md • plan.md • reports/     |
| 10      | Performance Features    | Planned      | docs/epics/epic-10-performance-features/                  | index.md • status.md • plan.md • reports/     |

Notes
- Canonicalization Plan
  - Create the above directories and seed `index.md`, `status.md`, `plan.md`, and `reports/`.
  - Migrate existing legacy documents (e.g., `docs/Epic7_*`, `docs/Epic8_*`, `docs/Epic9_*`, and any root-level Epic docs) into the appropriate epic directory.
  - Add an archive banner header to superseded files and move them into `docs/archive/`.
  - Update cross-references in any workflow/status pages to point to these canonical locations.

Archive Banner (to prepend to superseded files)
```
> [!IMPORTANT] Archived Document
> This file has been superseded by canonical documents in docs/epics/{epic-id}/.
> It is retained for historical reference only and should not be used for planning.
```

Migration Hints
- Epic 07: consolidate materials from `docs/Epic7_*` and root-level `Epic7_*.md` into `epic-07-realtime-generation/`.
- Epic 08: consolidate from `docs/Epic8_*` and any references in demos/tests.
- Epic 09: consolidate from `docs/Epic9_*` and cross-reference tests like `Source/test_epic9_story9_1.cpp`.
- Update `docs/workflows/*` and any status summaries to link to the canonical epic documents.

Acceptance Criteria (for Docs Reorg PR)
- All Epic7/8/9 materials are either migrated or explicitly linked from the epic `index.md`.
- Archive banners are present on superseded files.
- Link checker passes (update CI or run local script).
