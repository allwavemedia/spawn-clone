# SpawnClone Complete Development Workflow (Execution-Ready)

Purpose
- Convert the existing multi-epic completion plan into a single, actionable workflow for completing development with clear sequencing, CI gating, runbooks, and acceptance criteria.
- Maintain the green Phase 1 UI-only CI while progressively enabling core, AI integration, plugin build, E2E tests, and packaging.
- Canonicalize documentation and eliminate drift.

Scope
- Epics: Audio Engine completion, ONNX/AI integration, Full plugin integration, E2E testing, Packaging, UI/UX polish, Documentation consolidation.
- CI: Expand matrix to include ai-integration (ON/OFF), plugin, E2E, packaging artifacts.
- Docs: Canonical epics structure in docs/epics with archival of duplicates.

Inputs (Current Status Snapshot)
- Green UI-only build; WaveformDisplayComponent tests pass.
- CMake gates exist: BUILD_UI_ONLY=ON (default), BUILD_TESTS=ON, BUILD_PLUGIN=OFF, BUILD_LEGACY_TESTS=OFF, USE_ONNX_RUNTIME optional, ENABLE_DAEMON_IPC=OFF.
- Core tests path green (39/39).
- Engine MVP complete (SampleEngine, SynthVoice, EffectsChain, InstrumentLibraryManager, VoiceManager).
- AI client, plugin build, E2E, packaging, and docs canonicalization still pending.

Repository Switches (Authoritative)
- BUILD_UI_ONLY: ON (default) — safety lane for CI
- BUILD_TESTS: ON (default)
- BUILD_PLUGIN: OFF (enable only for plugin integration lanes)
- BUILD_LEGACY_TESTS: OFF (keep legacy/demo isolated)
- USE_ONNX_RUNTIME: {ON|OFF} — matrix CI coverage with robust fallback
- ENABLE_DAEMON_IPC: OFF (enable later to test process/IPC management)

Guiding Principles
- Keep UI-only CI job green at all times.
- Introduce new capabilities behind feature flags and CI lanes.
- Small, reviewable PRs; every PR updates tests and docs.
- Deterministic tests; seed-based where AI is involved.
- Canonicalize documents; archive superseded content with banners.

--------------------------------------------------------------------

Milestones and PR Sequence

Legend
- Owners: primarily responsible roles
- Reviewers: second-line reviewers (QA + one alternate)
- DoD: Definition of Done checklist
- Branch naming: scoped and prefixed

PR 1 — Docs Reorg (Canonicalization) [docs only]
- Branch: docs/reorg-epics-canonical
- Owners: PM + Architect
- Reviewers: QA + Dev
- Scope:
  - Create docs/epics/{epic-id}/ with index.md, status.md, plan.md, reports/ and templates/ as needed.
    - Example dirs:
      - docs/epics/epic-07-realtime-generation/
      - docs/epics/epic-08-effects-processing/
      - docs/epics/epic-09-sound-design/
      - docs/epics/epic-10-performance-features/
  - Migrate legacy documents under docs/ into these canonical locations.
    - Add archive banners to superseded files in docs/archive/.
  - Update cross-references in:
    - docs/Project_Status_Summary_*.md
    - docs/SpawnClone_Workflow_Guidance.md (if present)
    - docs/workflows/*
  - Add docs/epics/README.md (single source of truth: epic → status → links)
- DoD:
  - [ ] All Epic7/8/9 materials moved or linked from canonical locations
  - [ ] Banners present on superseded files
  - [ ] Link check (local script or CI link-check action) passes
  - [ ] PR description includes “before → after” link map

PR 2 — CMake Refactor (Non-Breaking) [COMPLETE]
- Branch: build/cmake-core-gating
- Status: Merged; keep monitoring nightly.

PR 3 — Engine MVP [COMPLETE]
- Branch: engine/mvp-core
- Status: Merged; follow-up: sanitizers job.

PR 3.1 — Nightly Sanitizers for Core Targets
- Branch: ci/sanitizers-core
- Owners: Dev; Reviewers: QA
- Scope:
  - Add ASan/UBSan nightly job for core libraries and tests.
- DoD:
  - [ ] ASan/UBSan job present, scheduled
  - [ ] Failures block “nightly-green” label
  - [ ] Documented in docs/Testing_Strategy.md

PR 4 — ONNX/AI Core Integration
- Branch: ai/onnx-daemon-client
- Owners: Dev + QA
- Reviewers: Architect
- Scope:
  - Implement ONNXDaemonClient.{h,cpp} lifecycle:
    - connect/start/stop; loadModel; generatePattern; timeouts/retries; telemetry
    - Clear diagnostics when runtime not available (USE_ONNX_RUNTIME=OFF)
    - Deterministic seeds for tests
  - File references (present in repo):
    - Source/ai/ONNXDaemonClient.cpp (implement/complete)
    - tests/e2e/audio/test_epic7_ai_integration.cpp (expand and stabilize)
  - CI matrix:
    - USE_ONNX_RUNTIME={ON,OFF} lanes for core integration targets
  - Flags:
    - ENABLE_DAEMON_IPC=OFF path tested (mock IPC)
    - Prepare ENABLE_DAEMON_IPC=ON for follow-up PR with real process mgmt
- DoD:
  - [ ] Tests green across both ON and OFF runtime lanes
  - [ ] Deterministic integration tests with seeded scenarios
  - [ ] Fallback mode exercised and asserted
  - [ ] Troubleshooting section added to docs/ONNX_Runtime_Setup.md

PR 4.1 — Daemon Process + IPC Contract
- Branch: ai/daemon-ipc
- Owners: Dev
- Reviewers: QA + Architect
- Scope:
  - When ENABLE_DAEMON_IPC=ON:
    - Spawn/shutdown hooks, health checks (HTTP/socket)
    - Timeout/retry policies; structured telemetry
  - Extend integration tests with failure injection
- DoD:
  - [ ] IPC health probe documented and covered by tests
  - [ ] CI lane ENABLE_DAEMON_IPC=ON runs and passes, quarantined if needed

PR 5 — Plugin Integration
- Branch: plugin/enable-core-ui
- Owners: Dev
- Reviewers: QA + Architect
- Scope:
  - Enable plugin target behind BUILD_PLUGIN (juce_add_plugin)
  - Wire plugin_core into UI layer:
    - WaveformDisplayComponent connected to live buffers
  - Replace deprecated font calls with modern FontOptions
  - Parameter mapping audit (SynthesisParameterMapper, ParameterManager)
- DoD:
  - [ ] BUILD_PLUGIN=ON lane builds without breaking other lanes
  - [ ] Live waveform rendering verified in headless debug (if possible)
  - [ ] Parameter mapping documented; no dead controls

PR 6 — E2E Headless Render Tests
- Branch: tests/e2e-offline-render
- Owners: QA
- Reviewers: Dev
- Scope:
  - Add tests/e2e/audio/offline_render_tests.cpp
  - Tests generate 1 bar; assert RMS/spectral envelope and parameter sweep coverage
  - Integrate with BUILD_UI_ONLY=OFF + BUILD_PLUGIN=ON build
- DoD:
  - [ ] E2E tests pass locally and in CI
  - [ ] Seeded presets produce stable metrics within tolerance
  - [ ] Performance budget documented (min baseline)

PR 7 — Packaging (macOS DMG)
- Branch: ci/packaging
- Owners: DevOps (or Dev)
- Reviewers: QA
- Scope:
  - Add CI job to produce DMG artifacts from plugin build
  - Optional notarization scaffold (manual signing for now)
- DoD:
  - [ ] CI artifacts include signed/unsigned DMG
  - [ ] Document manual installation validation

PR 8 — Performance + Docs Finalization
- Branch: perf+docs/final
- Owners: Architect + PM
- Reviewers: QA + Dev
- Scope:
  - Lightweight performance benchmarks; store per-commit metrics
  - Finalize documentation suite:
    - PRD, Architecture, User Guide, Release Notes
    - Epic status indexes updated and consistent
- DoD:
  - [ ] Metrics artifact uploaded in CI
  - [ ] All epics indexes present and synchronized
  - [ ] Release notes ready for publishing

--------------------------------------------------------------------

CI Matrix (Target State)

Jobs
- ui-only (default safety)
  - BUILD_UI_ONLY=ON, BUILD_TESTS=ON
- core-tests
  - BUILD_UI_ONLY=OFF, BUILD_TESTS=ON, BUILD_PLUGIN=OFF, BUILD_LEGACY_TESTS=OFF
- ai-integration (matrix)
  - BUILD_UI_ONLY=OFF, BUILD_TESTS=ON, USE_ONNX_RUNTIME={ON,OFF}
- plugin (manual gate)
  - BUILD_UI_ONLY=OFF, BUILD_PLUGIN=ON
- e2e (headless, after plugin is functional)
  - BUILD_UI_ONLY=OFF, BUILD_PLUGIN=ON; run offline_render tests
- packaging
  - BUILD_UI_ONLY=OFF, BUILD_PLUGIN=ON; produce DMG

YAML Hints (pseudo)
```yaml
jobs:
  ui_only:
    env: { BUILD_UI_ONLY: ON, BUILD_TESTS: ON }
  core_tests:
    env: { BUILD_UI_ONLY: OFF, BUILD_TESTS: ON, BUILD_LEGACY_TESTS: OFF, BUILD_PLUGIN: OFF }
  ai_integration:
    strategy:
      matrix:
        onnx: [ON, OFF]
    env:
      BUILD_UI_ONLY: OFF
      BUILD_TESTS: ON
      USE_ONNX_RUNTIME: ${{ matrix.onnx }}
  plugin:
    if: github.ref == 'refs/heads/main' || startsWith(github.ref, 'refs/heads/plugin/')
    env: { BUILD_UI_ONLY: OFF, BUILD_PLUGIN: ON }
  e2e:
    needs: plugin
    env: { BUILD_UI_ONLY: OFF, BUILD_PLUGIN: ON }
  packaging:
    needs: plugin
    env: { BUILD_UI_ONLY: OFF, BUILD_PLUGIN: ON }
```

--------------------------------------------------------------------

Developer Runbooks

UI-only tests (safety)
- cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_UI_ONLY=ON -DBUILD_TESTS=ON
- cmake --build build -j 4
- ctest --test-dir build --output-on-failure

Engine/core tests
- cmake -S . -B build-core -DCMAKE_BUILD_TYPE=Debug -DBUILD_UI_ONLY=OFF -DBUILD_TESTS=ON -DBUILD_LEGACY_TESTS=OFF -DBUILD_PLUGIN=OFF
- cmake --build build-core -j 4
- ctest --test-dir build-core -R "(PluginCoreSmokeTest|SampleEngineTest|SynthVoiceTest|EffectsChainTest|InstrumentLibraryManagerTest|VoiceManagerTest|SampleEngineFilePlaybackTest|ONNXDaemonClientTest)" --output-on-failure

AI integration (enable in PR 4)
- cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBUILD_UI_ONLY=OFF -DBUILD_TESTS=ON -DUSE_ONNX_RUNTIME=ON
- cmake --build build -j 8
- ctest --test-dir build -R epic7_ai_integration --output-on-failure
- Repeat with -DUSE_ONNX_RUNTIME=OFF (fallback coverage)

Plugin build (PR 5)
- cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBUILD_UI_ONLY=OFF -DBUILD_PLUGIN=ON
- cmake --build build -j 8

E2E headless render (PR 6)
- ctest --test-dir build -R offline_render --output-on-failure

--------------------------------------------------------------------

Risk Controls
- Keep engine work under if(NOT BUILD_UI_ONLY); never break ui-only tests.
- Use target-specific include/link to avoid accidental coupling.
- Deterministic seeds; numeric tolerance in AI tests.
- Quarantine lanes permitted for new features but must be visible.
- Add sanitizers to catch UB/ASan issues early (nightly lane).

--------------------------------------------------------------------

Documentation Governance (Canonicalization Details)

Target Structure (examples)
- docs/epics/epic-07-realtime-generation/{index.md,status.md,plan.md,reports/,templates/}
- docs/epics/epic-08-effects-processing/{index.md,status.md,plan.md,reports/}
- docs/epics/epic-09-sound-design/{index.md,status.md,plan.md,reports/}
- docs/epics/epic-10-performance-features/{index.md,status.md,plan.md,reports/}
- docs/epics/README.md — contains a table mapping epics → status → key docs

Migration Notes (examples found in repo)
- Migrate Epic 7 documents (e.g., docs/Epic7_* and root-level Epic7_*.md) into epic-07 folder.
- Migrate Epic 8 documents (e.g., docs/Epic8_* and root-level items like epic8_story8_4_realtime_demo.cpp references) into epic-08 folder.
- Migrate Epic 9 documents (e.g., docs/Epic9_* and tests/sources like Source/test_epic9_story9_1.cpp) into epic-09 folder.
- Add archive banners to superseded docs (move originals to docs/archive/).
- Update references in docs/workflows/* and status summaries.

Link Checker
- Add a local script (e.g., scripts/check_links.py) or a GitHub Action to verify internal doc links.
- PR 1 must include link audit results.

--------------------------------------------------------------------

Acceptance Criteria (End State)
- All epics indexed with consistent status; duplicates archived with banners.
- CI green across: ui-only, core tests, AI integration (ON/OFF), plugin build, E2E tests.
- Packaging job produces installable artifact (DMG).
- User documentation and release notes complete and in sync with implementation.

--------------------------------------------------------------------

Change Log Footer (PR Template)
- Summary:
- Affected targets:
- Flags required:
- Tests added/updated:
- Docs updated:
- Backward compatibility:
- Risks & mitigations:

--------------------------------------------------------------------

Branching, Ownership, and Reviews

Branch Naming
- docs/reorg-epics-canonical
- engine/mvp-core
- ai/onnx-daemon-client
- ai/daemon-ipc
- plugin/enable-core-ui
- tests/e2e-offline-render
- ci/packaging
- perf+docs/final
- ci/sanitizers-core

Ownership and Review Rotation
- Primary Owners: Dev (code), QA (tests), PM + Architect (docs/structure), DevOps (packaging/CI)
- Reviewers: Ensure at least one cross-functional reviewer (e.g., QA on Dev PRs, Architect on AI/IPC PRs)

Definition of Done (General)
- [ ] All relevant CI lanes pass
- [ ] New/modified code covered by tests; seeded determinism where applicable
- [ ] Documentation updated (workflows, epics, and runbooks)
- [ ] Flags and build switches behavior documented
- [ ] No regressions in ui-only lane

--------------------------------------------------------------------

Do-Now Checklist (Immediate)
- [ ] PR 1: Create docs/epics/*, migrate Epic7/8/9 docs, add archive banners, add docs/epics/README.md, run link checker.
- [ ] Ensure .github/workflows/core-matrix.yml includes at least ui-only and core-tests, and stub entries for ai-integration/plugin/e2e/packaging lanes.
- [ ] PR 4: Fill out Source/ai/ONNXDaemonClient.cpp lifecycle and expand tests/e2e/audio/test_epic7_ai_integration.cpp with deterministic seeds and fallback assertions.
- [ ] Prepare PR 5 and PR 6 branches (draft PRs) to stage early reviews.

--------------------------------------------------------------------

Appendix: Useful Paths Detected in Repo (for planning)
- Core and tests:
  - Source/plugin_core/, tests/unit/audio_engine_tests/*
- AI integration:
  - Source/ai/ONNXDaemonClient.cpp, docs/ONNX_Runtime_Setup.md
  - tests/e2e/audio/test_epic7_ai_integration.cpp
- CI:
  - .github/workflows/core-matrix.yml
- UI waveform:
  - WaveformDisplayComponent (referenced in existing tests)
- Demos/legacy:
  - epic6_*, epic7_*, epic8_*, epic9_* demos under root and build-core (keep under BUILD_LEGACY_TESTS when applicable)

Notes
- Maintain BUILD_UI_ONLY=ON default for safest developer onboarding and PR verification.
- Re-enable targets incrementally in CI as PRs mature; use quarantined lanes to stage flaky tests before GA.
