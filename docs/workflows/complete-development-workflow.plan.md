# Workflow Plan — Complete Development Workflow

Purpose
- Activate and track execution of the complete-development-workflow with explicit PR sequencing, CI lanes, and acceptance criteria.
- Provide a living plan with status checkboxes, owners, dependencies, and links.

Source
- Parent: docs/workflows/complete-development-workflow.md (execution-ready spec)
- Derived from: docs/workflows/complete-epics-workflow.md (original)

Governance
- Owners: PM + Architect (docs), Dev (code), QA (tests), DevOps (CI/packaging)
- Reviewers: At least one cross-functional reviewer for each PR
- Change control: Update this plan in each PR to reflect progress; keep Acceptance Criteria in sync.

----------------------------------------------------------------------

Activation Checklist (Do Now)
- [x] Create canonical epics directories and index pages (DONE: README + epic-07/08/09/10 indexes) — Link: docs/epics/README.md
- [x] Ensure CI has lanes: ui-only, core-matrix (ONNX ON/OFF), and stubs for plugin, e2e, packaging — Link: .github/workflows/core-matrix.yml
- [ ] Stage PR 4 (ONNX/AI Core Integration) work-in-progress with deterministic tests
- [ ] Draft PR 5 and PR 6 branches to enable early review of plugin/E2E structure

----------------------------------------------------------------------

Milestones and PRs (Execution)

PR 1 — Docs Reorg (Canonicalization)
- Branch: docs/reorg-epics-canonical
- Scope:
  - Create docs/epics/{epic-id}/; move legacy docs; add archive banners; update cross-references
  - Add docs/epics/README.md
- Owners: PM + Architect
- Reviewers: QA + Dev
- Dependencies: none
- CI: docs-only
- Status:
  - [x] Create canonical epic index pages
  - [ ] Migrate Epic7 documents → epic-07-realtime-generation/
  - [ ] Migrate Epic8 documents → epic-08-effects-processing/
  - [ ] Migrate Epic9 documents → epic-09-sound-design/
  - [ ] Add archive banners and move superseded docs to docs/archive/
  - [ ] Link checker passes
- Acceptance:
  - [ ] All links updated; before→after link map in PR description

PR 2 — CMake Refactor (Non-Breaking)
- Branch: build/cmake-core-gating
- Status: [x] Merged
- Follow-up:
  - [ ] Add nightly sanitizers (ASan/UBSan) job (PR 3.1)

PR 3 — Engine MVP
- Branch: engine/mvp-core
- Status: [x] Merged

PR 3.1 — Nightly Sanitizers (Core)
- Branch: ci/sanitizers-core
- Owners: Dev; Reviewers: QA
- Scope: Add nightly ASan/UBSan for core targets; failures block "nightly-green"
- Status:
  - [x] Add workflow ci/sanitizers-core.yml
  - [ ] Validate locally and on nightly schedule
  - [ ] Document in docs/Testing_Strategy.md

PR 4 — ONNX/AI Core Integration
- Branch: ai/onnx-daemon-client
- Owners: Dev + QA; Reviewers: Architect
- Scope:
  - Implement ONNXDaemonClient lifecycle (connect/start/stop, loadModel, generatePattern)
  - Deterministic tests with seeds; fallback mode (USE_ONNX_RUNTIME=OFF)
  - CI matrix USE_ONNX_RUNTIME={ON,OFF}
- Code/Test Pointers:
  - Source/ai/ONNXDaemonClient.cpp
  - tests/e2e/audio/test_epic7_ai_integration.cpp
  - docs/ONNX_Runtime_Setup.md
- Status:
  - [x] Implement lifecycle + error paths and telemetry
  - [x] Add/expand deterministic tests
  - [ ] Core-matrix CI green ON/OFF
  - [x] Troubleshooting doc updated
- Acceptance:
  - [ ] Green across both runtime lanes; clear diagnostics when runtime missing

PR 4.1 — Daemon Process + IPC
- Branch: ai/daemon-ipc
- Owners: Dev; Reviewers: QA + Architect
- Scope:
  - ENABLE_DAEMON_IPC=ON: spawn/shutdown, health checks (HTTP/socket), timeout policies, telemetry
  - Failure injection tests
- Status:
  - [ ] IPC health probe implemented
  - [ ] CI lane added (quarantinable)
- Acceptance:
  - [ ] IPC tests stable, documented

PR 5 — Plugin Integration
- Branch: plugin/enable-core-ui
- Owners: Dev; Reviewers: QA + Architect
- Scope:
  - BUILD_PLUGIN=ON target (juce_add_plugin)
  - Wire plugin_core to UI (WaveformDisplayComponent live buffers)
  - Replace deprecated font calls with FontOptions
  - Parameter mapping audit (SynthesisParameterMapper, ParameterManager)
- Status:
  - [ ] Plugin target builds under dedicated lane
  - [ ] Live waveform verified (headless if possible)
  - [ ] Mapping documented
- Acceptance:
  - [ ] No regressions in ui-only and core lanes; plugin builds cleanly

PR 6 — E2E Headless Render Tests
- Branch: tests/e2e-offline-render
- Owners: QA; Reviewers: Dev
- Scope:
  - tests/e2e/audio/offline_render_tests.cpp: 1 bar render; RMS/spectral checks; parameter sweeps
  - Integrate with BUILD_UI_ONLY=OFF + BUILD_PLUGIN=ON
- Status:
  - [ ] Tests implemented, stable locally
  - [ ] CI lane green
- Acceptance:
  - [ ] Seeded presets with tolerance thresholds; baseline metrics documented

PR 7 — Packaging (DMG)
- Branch: ci/packaging
- Owners: DevOps/Dev; Reviewers: QA
- Scope:
  - CI job to produce DMG artifacts from plugin build; optional notarization scaffold
- Status:
  - [ ] Packaging lane added (stub → enabled)
  - [ ] DMG artifact visible in PR checks
- Acceptance:
  - [ ] Manual install validation document added

PR 8 — Performance + Final Docs
- Branch: perf+docs/final
- Owners: Architect + PM; Reviewers: QA + Dev
- Scope:
  - Lightweight performance benchmarks; metrics artifact per commit
  - Final docs: PRD, Architecture, User Guide, Release Notes; epics indexes updated
- Status:
  - [ ] Benchmarks implemented and reported in CI
  - [ ] Docs finalized and cross-linked
- Acceptance:
  - [ ] Release notes complete; documentation suite synchronized

----------------------------------------------------------------------

CI Lanes — Target State and Readiness

- ui-only: READY
  - BUILD_UI_ONLY=ON, BUILD_TESTS=ON
  - File: .github/workflows/core-matrix.yml (job: ui-only)
  - [x] Present; [ ] Monitored

- core-matrix: READY
  - BUILD_UI_ONLY=OFF, USE_ONNX_RUNTIME={ON,OFF}
  - File: .github/workflows/core-matrix.yml (job: core-matrix)
  - [x] Present; [ ] ONNX=ON path validated

- plugin: STUB
  - BUILD_PLUGIN=ON
  - [x] Stubbed; [ ] Enable when PR 5 scoped

- e2e: STUB
  - Offline render tests
  - [x] Stubbed; [ ] Implement tests; [ ] Enable

- packaging: STUB
  - DMG artifacts
  - [x] Stubbed; [ ] Enable when PR 7 scoped

----------------------------------------------------------------------

Risks & Mitigations
- ONNX environment variance
  - Mitigate: Clear diagnostics, fallback path, CI matrix ON/OFF
- Flaky E2E DSP assertions
  - Mitigate: Seeded tests, tolerance bands, quarantine lane until stable
- Docs drift
  - Mitigate: Canonical epics directories, archive banners, link checker

----------------------------------------------------------------------

Operating Procedures

Start the Workflow
- Ensure Activation Checklist items are in progress.
- Open draft PRs for PR 4, PR 5, PR 6 to start asynchronous reviews.
- Update this plan per PR with checkboxes and links as items progress.

Update Status
- Append changes under each PR’s Status/Acceptance subsections.
- Keep CI Lanes section updated (READY/STUB/ENABLED).
- Maintain a weekly summary in the Status Log (below).

----------------------------------------------------------------------

Status Log (append newest on top)

- 2025-08-07:
  - Summary: Initiated complete-development workflow; created canonical epics scaffolding and expanded CI lanes (ui-only added, matrix preserved; plugin/e2e/packaging stubbed).
  - Updates:
    - PR 4: Not started (prepare ONNXDaemonClient lifecycle and deterministic tests).
    - PR 5: Draft branch pending.
    - PR 6: Draft branch pending.
    - CI: ui-only job added; core-matrix unchanged; plugin/e2e/packaging stub jobs added; sanitizers workflow added.
    - Docs: Added docs/epics/README.md and epic 07/08/09/10 index pages; added workflow plan.
  - Risks: ONNX environment variance and E2E determinism; mitigations planned in workflow.
  - Next Week: Migrate Epic7/8/9 docs; implement ONNX client and tests; open draft PRs 5/6.

- YYYY-MM-DD:
  - Summary:
  - Updates:
    - PR 4:
    - PR 5:
    - PR 6:
    - CI:
    - Docs:
  - Risks:
  - Next Week:

----------------------------------------------------------------------

Acceptance Criteria (End State)
- [ ] Green CI across: ui-only, core-matrix ON/OFF, plugin, e2e
- [ ] Packaging job produces installable DMG in CI
- [ ] Documentation consolidated and synchronized with canonical epics
- [ ] Change logs complete for all PRs; release notes prepared

Links
- Spec: docs/workflows/complete-development-workflow.md
- Epics index: docs/epics/README.md
- CI workflow: .github/workflows/core-matrix.yml
