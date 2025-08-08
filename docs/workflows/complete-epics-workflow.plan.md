# Workflow Plan — Complete Epics Workflow

Purpose
- Operate the multi-epic completion workflow with concrete owners, status, CI lanes, and acceptance criteria.
- Track progress across Docs, AI integration, Plugin + E2E, Packaging/Performance, and Documentation finalization.

Source
- Spec: docs/workflows/complete-epics-workflow.md (kept updated as the authoritative workflow document)

Governance
- Owners: PM + Architect (docs), Dev (code), QA (tests), DevOps (CI/packaging)
- Reviewers: At least one cross-functional reviewer for each PR
- Change control: Update this plan in each PR; keep Acceptance Criteria synchronized with the spec.

----------------------------------------------------------------------

Activation Checklist (Do Now)
- [x] Canonicalize Epic documents and add archive banners (docs migration complete)
- [x] Epics index present (docs/epics/README.md)
- [x] CI lanes present/stubbed as per spec (core-matrix, ui-only, plugin/e2e/packaging stubs)
- [x] Nightly sanitizers (ASan/UBSan) workflow added (.github/workflows/sanitizers-core.yml)
- [x] Link check utility in place (scripts/check_links.py)
- [ ] PR 4 (ONNX/AI Core) finalize lifecycle/telemetry details and broaden E2E seeds/failure injection
- [ ] Draft PR 5 (plugin enable) + PR 6 (E2E offline render) branches for early review

----------------------------------------------------------------------

Milestones and PRs (Execution)

PR 1 — Docs Reorg (Canonicalization)
- Branch: docs/reorg-epics-canonical
- Status:
  - [x] Migrate Epic7/8/9 docs into docs/epics/{epic}/reports
  - [x] Archive banners for superseded files
  - [x] Add docs/epics/README.md
  - [x] Link checker run (clean)
  - [ ] PR description includes before→after link map (metadata)
- Acceptance:
  - [x] Canonical epics serve as single source of truth without broken links

PR 2 — CMake Refactor (Non-Breaking)
- Branch: build/cmake-core-gating
- Status: [x] Merged

PR 3 — Engine MVP
- Branch: engine/mvp-core
- Status: [x] Merged

PR 3.1 — Nightly Sanitizers (Core)
- Branch: ci/sanitizers-core
- Status:
  - [x] Add workflow (asan/ubsan)
  - [ ] Observe nightly runs; fix regressions if any
  - [x] Document in docs/Testing_Strategy.md

PR 4 — ONNX/AI Core Integration
- Branch: ai/onnx-daemon-client
- Scope:
  - [ ] Lifecycle details: timeouts/retries, telemetry surfaces
  - [ ] Runtime discovery diagnostics (already wired globally in CMake)
  - [ ] E2E tests deterministic; optionally expand failure injection
  - [ ] Docs troubleshooting additions
- Status:
  - [x] Deterministic client present with average time EMA
  - [x] E2E (Epic7) deterministic tests present
  - [x] Matrix CI scaffold present (USE_ONNX_RUNTIME={ON,OFF})
  - [ ] Extend coverage and finalize

PR 5 — Plugin Integration
- Branch: plugin/enable-core-ui
- Scope:
  - [ ] Enable plugin target (BUILD_PLUGIN=ON lane)
  - [ ] Connect engine buffers to UI (e.g., waveform live buffers)
  - [ ] Replace deprecated font calls with FontOptions
  - [ ] Parameter mapping audit
- CI:
  - [ ] Enable “plugin” job (remove stub guard)

PR 6 — E2E Headless Render Tests
- Branch: tests/e2e-offline-render
- Scope:
  - [ ] tests/e2e/audio/offline_render_tests.cpp
  - [ ] 1 bar render; RMS/spectral checks; parameter sweeps
- CI:
  - [ ] Enable “e2e” job (remove stub guard) once tests stable

PR 7 — Packaging (DMG)
- Branch: ci/packaging
- Scope:
  - [ ] Add job to produce macOS DMG
  - [ ] Optional notarization scaffold
- CI:
  - [ ] Enable “packaging” job (remove stub guard)

PR 8 — Performance + Final Docs
- Branch: perf+docs/final
- Scope:
  - [ ] Lightweight performance benchmarks + CI metrics artifact
  - [ ] Final docs pass (PRD, Architecture, User Guide, Release Notes)
  - [ ] Epics indexes updated and synchronized

----------------------------------------------------------------------

CI Lanes — Target State and Readiness

- ui-only: READY
  - BUILD_UI_ONLY=ON, BUILD_TESTS=ON
  - File: .github/workflows/core-matrix.yml (job: ui-only)

- core-matrix: READY
  - BUILD_UI_ONLY=OFF, USE_ONNX_RUNTIME={ON,OFF}
  - File: .github/workflows/core-matrix.yml (job: core-matrix)

- plugin: STUB
  - BUILD_PLUGIN=ON
  - Enable after PR 5 basic build success

- e2e: STUB
  - Offline render tests
  - Enable after PR 6 is stable

- packaging: STUB
  - DMG artifacts
  - Enable after plugin lane is green

- sanitizers: ENABLED
  - Nightly with ASan/UBSan (.github/workflows/sanitizers-core.yml)

----------------------------------------------------------------------

Risks & Mitigations
- ONNX environment variance
  - Mitigate: matrix ON/OFF; clear diagnostics; fallback path
- E2E DSP flakiness
  - Mitigate: seeded determinism, tolerance thresholds, quarantine until stable
- Docs drift
  - Mitigate: canonical epics + archive banners + link checker

----------------------------------------------------------------------

Operating Procedures
- Keep ui-only lane green at all times.
- Land changes behind flags; enable lanes incrementally.
- Update Status Log with every material action (CI lane changes, PR milestones).
- Use the link checker script after large doc changes.

----------------------------------------------------------------------

Status Log (append newest on top)

- 2025-08-07:
  - Summary: Activated complete-epics workflow; canonical epics created; sanitizers added; CI lanes scaffolded; link hygiene enforced.
  - Updates:
    - PR 1: Completed (migration + banners + epics index)
    - PR 3.1: Sanitizers workflow added
    - CI: ui-only, core-matrix active; plugin/e2e/packaging stubbed
    - Docs: ONNX runtime setup doc fixed; link checker integrated; Testing_Strategy.md added
  - Risks: ONNX environment mismatch; E2E determinism; will expand seeded/failure-injection coverage
  - Next: Advance PR 4 lifecycle/telemetry; open PR 5/6 drafts

----------------------------------------------------------------------

Acceptance Criteria (End State)
- [ ] Green CI across: ui-only, core-matrix ON/OFF, plugin, e2e
- [ ] Packaging job produces installable DMG in CI
- [ ] Final documentation synchronized; epics indexes consistent
- [ ] Release notes prepared; change logs complete

Links
- Spec: docs/workflows/complete-epics-workflow.md
- CI: .github/workflows/core-matrix.yml, .github/workflows/sanitizers-core.yml
- Epics index: docs/epics/README.md
