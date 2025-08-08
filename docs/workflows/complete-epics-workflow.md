# SpawnClone Multi‑Epic Completion Workflow

Purpose
- Provide an actionable, dependency‑resolved workflow to finish all remaining epics:
  - Audio engine completion
  - ONNX/AI integration
  - Full plugin integration
  - End‑to‑End (E2E) testing
  - Packaging
  - UI/UX polish
  - Documentation consolidation
- Preserve Phase 1 UI‑only green CI while incrementally enabling core and plugin functionality.

Progress summary (as of 2025‑08‑07)
- [x] Phase 1 UI‑only green build; WaveformDisplayComponent tests (10/10) passing
- [x] CMake gating added: BUILD_PLUGIN (OFF), BUILD_LEGACY_TESTS (OFF by default), BUILD_UI_ONLY (ON by default)
- [x] Core library scaffold created: plugin_core (STATIC)
- [x] Initial engine module: SampleEngine (dependency‑light, sine renderer)
- [x] SynthVoice implemented with unit tests
- [x] Unit tests added and passing:
  - PluginCoreSmokeTest (link/init)
  - SampleEngineTest (size/energy/phase/reset)
- [x] Docs consolidation (canonical epics structure)
- [x] Engine MVP complete (SynthVoice, EffectsChain, InstrumentLibraryManager)
- [ ] ONNX/AI integration completion
- [ ] Plugin integration + headless E2E
- [ ] Packaging + performance metrics + final docs

Principles
- [x] Keep BUILD_UI_ONLY=ON as default CI job until plugin is ready.
- [x] Add new targets behind feature flags to avoid breaking green builds.
- [x] Make small, reviewable PRs; each PR includes tests and docs updates.
- [x] Canonicalize documents and archive duplicates to prevent drift.

Status Snapshot (inputs)
- [x] Phase 1 UI‑only green build complete (ui_tests target; WaveformDisplayComponent tests pass).
- [x] CMake refactor started: gates in place; legacy/ONNX‑heavy demos wrapped under BUILD_LEGACY_TESTS where applicable.
- [x] Core‑only job established and green (39/39 tests).
- [x] Docs: canonical epics directories created; legacy files migrated with archive banners; link checker added.
- Epic statuses:
  - [x] Epic 7: largely complete (multiple completion/final docs).
  - [x] Epic 8: complete (8.1–8.4 story completion docs).
  - [ ] Epic 9: partial (9.1 instrument library tasks present; 9.2 advanced synthesis plan/progress present).

Repository Switches (CMake options)
- [x] BUILD_UI_ONLY: ON (default) — build UI tests only (Phase 1 safety)
- [x] BUILD_TESTS: ON (default)
- [x] BUILD_PLUGIN: OFF (enable only when integrating plugin)
- [x] BUILD_LEGACY_TESTS: OFF (wrap legacy/demos to avoid accidental linking)
- [x] USE_ONNX_RUNTIME: optional ON (Homebrew or vendored) with fallback
- [x] ENABLE_DAEMON_IPC: OFF (guard real daemon process/IPC paths; ON enables process hooks)

CI Jobs (matrix)
- [x] ui‑only: BUILD_UI_ONLY=ON (wired in .github/workflows/core-matrix.yml)
- [x] core‑tests: BUILD_UI_ONLY=OFF; build plugin_core + unit tests (validated locally; 39/39 pass)
- [x] ai‑integration: BUILD_UI_ONLY=OFF; USE_ONNX_RUNTIME={ON,OFF} (CI: .github/workflows/core-matrix.yml)
- [ ] plugin: BUILD_UI_ONLY=OFF; BUILD_PLUGIN=ON (stubbed in CI; enable post‑PR 5)
- [ ] e2e: BUILD_UI_ONLY=OFF; headless render tests (stubbed in CI; enable post‑PR 6)

Milestones and PR Sequence

## Sprint 1 (Audio Engine Foundation)
Goal
- Replace Phase 1 stubs with functional MVP for:
  - [x] SampleEngine (initial minimal implementation)
  - [x] SynthVoice
  - [x] EffectsChain
  - [x] InstrumentLibraryManager

Deliverables
- [x] New static library: plugin_core (engine core scaffold)
- [x] Unit tests (initial): plugin core smoke + SampleEngine
- [x] CMake refactor: target‑specific gating (BUILD_PLUGIN / BUILD_LEGACY_TESTS) and non‑UI build path
- [x] Unit tests: voice allocation, file‑backed sample playback

PRs
1) Docs Reorg PR (docs only) — Status: Completed
   Summary:
   - Canonicalize epic documents and purge duplicates to create a single source of truth for planning and status.

   Scope:
   - Create canonical epic directories under docs/epics/: epic‑07‑realtime‑generation, epic‑08‑effects‑processing, epic‑09‑sound‑design, epic‑10‑performance‑features
   - Migrate legacy Epic7/8 docs into epic-specific reports/ and archive/
   - Update cross-references in Project_Status_Summary and Workflow_Guidance
   - Provide an index (docs/epics/README.md) with epic statuses and links

   Tasks:
   - [x] Create directories and move files
   - [x] Add archive banners to superseded files
   - [x] Update links in: docs/Project_Status_Summary_*.md, docs/SpawnClone_Workflow_Guidance.md
   - [x] Add docs/epics/README.md (table of epics, status, links)
   - [ ] PR description includes before/after link map

   Artifacts:
   - docs/epics/* with index.md/status.md/plan.md/templates as needed
   - docs/archive/* for superseded items with banner header

   Meta:
   - Branch: docs/reorg-epics-canonical
   - Owners: PM + Architect
   - Reviewers: QA + Dev
   - Risks: Broken links (mitigation: link checker script), outdated references

2) CMake Refactor PR (non‑breaking) — Status: Completed
   Summary:
   - Introduced plugin_core static library and robust build gating to keep Phase 1/UI-only green while enabling core/unit tests.

   Changes:
   - Added plugin_core (STATIC) under if(NOT BUILD_UI_ONLY)
   - Added build gates: BUILD_PLUGIN (OFF), BUILD_LEGACY_TESTS (OFF), preserved BUILD_UI_ONLY (ON)
   - Enabled CTest discovery for core jobs; registered all new unit tests
   - Wrapped legacy/ONNX demos/tests under BUILD_LEGACY_TESTS to isolate core job

   Outcomes:
   - Core job green with 34/34 tests; UI-only unchanged/green
   - Simplified developer runbooks; reduced accidental linkage to incomplete modules

   Meta:
   - Branch: build/cmake-core-gating
   - Status: Merged
   - Follow-ups: None (monitor nightly matrix when added)

3) Engine MVP PR — Status: Completed
   Summary:
   - Delivered minimal-but-functional engine components with comprehensive unit tests.

   Implementations:
   - SampleEngine: sine generation + file-backed sample playback
   - SynthVoice: MIDI note on/off, velocity->amplitude, render via SampleEngine
   - EffectsChain: gain (dB), one-pole LPF, bypass, reset
   - InstrumentLibraryManager: register/update/remove, tag index (case-insensitive), list sorted
   - VoiceManager: fixed-capacity pool, oldest-voice stealing, mono mixdown

   Tests (all passing):
   - SampleEngineTest.*, SampleEngineFilePlaybackTest.*
   - SynthVoiceTest.*, EffectsChainTest.*, InstrumentLibraryManagerTest.*, VoiceManagerTest.*
   - PluginCoreSmokeTest.*

   Meta:
   - Branch: engine/mvp-core
   - Status: Merged
   - Follow-ups: [x] Add nightly sanitizers job (ASan/UBSan) for core targets

## Sprint 2 (ONNX/AI Integration)
Goal
- [ ] Complete ONNXDaemonClient C++ implementation, robust ONNX Runtime integration, and stabilize AI workflows.

Deliverables
- [ ] Source/ai/ONNXDaemonClient.cpp: connect/start/stop, requests, errors
- [ ] Runtime flagging for “Quality vs Speed”; onnxruntime discovery (brew/env/cmake)
- [ ] Integration tests with deterministic seeds; fallback mode when runtime unavailable

PRs
4) ONNX/AI Core PR — Status: In Progress
   Summary:
   - Implement ONNXDaemonClient with robust lifecycle and add deterministic AI integration tests. Validate both with and without ONNX Runtime.

   Scope:
   - ONNXDaemonClient.{h,cpp}: start/stop daemon, health/status, loadModel, generatePattern, timeouts/retries, telemetry
   - Runtime discovery: Homebrew path default with CMake/ENV overrides; clear diagnostics if not found
   - Deterministic tests: fixed seeds, structural assertions (shapes/metadata), tolerant numeric thresholds
   - CI: matrix USE_ONNX_RUNTIME={ON,OFF} for core integration target(s)

   Tasks:
   - [x] Implement client API + error paths, thorough logging
   - [x] Add integration tests under tests/e2e/audio/test_epic7_ai_integration.cpp
   - [x] Wire matrix jobs in CI (ON/OFF)
   - [x] Update docs: AI integration section with troubleshooting

   Meta:
   - Branch: ai/onnx-daemon-client
   - Owners: Dev + QA
   - Risks: Environment variance; mitigate with clear flags and fallbacks

   Next:
   - [ ] Implement real process management when ENABLE_DAEMON_IPC=ON (spawn, shutdown, health)
   - [ ] Define IPC contract and health probe (HTTP/socket); add timeout/retry policies
   - [ ] Structured telemetry and error surfaces for CI diagnostics
   - [ ] Expand E2E coverage (seeded scenarios, failure injection)
   - [ ] Wire Epic7 E2E into release matrix once stable

## Sprint 3 (Plugin Integration + E2E)
Goal
- [ ] Enable full plugin build with plugin_core + UI; add headless render E2E tests.

Deliverables
- [ ] BUILD_PLUGIN=ON path functional (juce_add_plugin)
- [ ] Parameter mapping audit (SynthesisParameterMapper, ParameterManager)
- [ ] Headless tests: render 1 bar; RMS/spectral envelope assertions; parameter sweep coverage

PRs
5) Plugin Integration PR

   - [ ] Enable plugin target behind BUILD_PLUGIN
   - [ ] Wire engine to UI; WaveformDisplayComponent connected to live buffers
   - [ ] Replace deprecated font calls with modern FontOptions usage

6) E2E Tests PR
   - [ ] Add tests/e2e/audio/offline_render_tests.cpp
   - [ ] Expand tests to cover parameter sweeps and a few presets

## Sprint 4 (Packaging, Performance, Documentation)

Goal

- [ ] Produce installable artifacts, improve performance observability, finalize documentation.

Deliverables

- [ ] Packaging pipeline (DMG on macOS), optional notarization scaffold
- [ ] Lightweight performance benchmarks; CI artifacts store metrics
- [ ] Final docs: PRD, Architecture, User Guide, Release Notes; epic status indexes updated

PRs
7) Packaging PR

   - [ ] Add CI job for packaging; produce DMG artifacts

8) Performance + Docs PR

   - [ ] Add timing/profiling hooks; store per‑commit metrics artifact
   - [ ] Finalize documentation suite and epic indexes

Acceptance Criteria (End State)

- [ ] All epic indexes present with consistent status; duplicates archived
- [ ] Green CI across: ui‑only, engine unit tests, AI integration (ON/OFF), plugin build, E2E tests
- [ ] Installable artifact produced in CI
- [ ] User documentation and release notes complete

Runbook (Developer Commands)

UI‑only tests (default safety)

- [x] `cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_UI_ONLY=ON -DBUILD_TESTS=ON`
- [x] `cmake --build build -j 4`
- [x] `ctest --test-dir build --output-on-failure`

Engine/core tests (core job; plugin_core + unit tests)

- [x] `cmake -S . -B build-core -DCMAKE_BUILD_TYPE=Debug -DBUILD_UI_ONLY=OFF -DBUILD_TESTS=ON -DBUILD_LEGACY_TESTS=OFF -DBUILD_PLUGIN=OFF`
- [x] `cmake --build build-core -j 4`
- [x] `ctest --test-dir build-core -R "(PluginCoreSmokeTest|SampleEngineTest)" --output-on-failure`
  - Result: 39/39 passing (PluginCoreSmokeTest.*, SampleEngineTest.*, SynthVoiceTest.*, EffectsChainTest.*, InstrumentLibraryManagerTest.*, VoiceManagerTest.*, SampleEngineFilePlaybackTest.*, ONNXDaemonClientTest.*)

AI integration tests (matrix) – to be enabled in Sprint 2
- [x] `cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBUILD_UI_ONLY=OFF -DBUILD_TESTS=ON -DUSE_ONNX_RUNTIME=ON`
- [x] `cmake --build build -j 8`
- [x] `ctest --test-dir build -R epic7_ai_integration --output-on-failure`
- [x] Repeat with `-DUSE_ONNX_RUNTIME=OFF` for fallback.

Plugin build (manual gate) – Sprint 3
- [ ] `cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBUILD_UI_ONLY=OFF -DBUILD_PLUGIN=ON`
- [ ] `cmake --build build -j 8`

E2E headless render tests – Sprint 3
- [ ] `ctest --test-dir build -R offline_render --output-on-failure`

Risk Controls
- [x] Keep engine work behind `if(NOT BUILD_UI_ONLY)`; never break `ui_tests`.
- [x] Use target‑specific include directories; avoid accidental link of stubs/demos.
- [x] Deterministic seeds in AI tests; assertions tolerant to minor numeric drift.

Documentation Governance
- [x] Canonicalize: `docs/epics/{epic}/index.md`, `status.md`, `plan.md`, `reports/`
- [ ] Archive banner for superseded docs; move to `docs/archive/`
- [x] `docs/epics/README.md`: single source of truth for epic mapping and status.

Change Log Template (PR footer)
- Summary:
- Affected targets:
- Flags required:
- Tests added/updated:
- Docs updated:
- Backward compatibility:
- Risks & mitigations:

Appendix: Initial Task Checklist (Do Now)
- [x] Create/normalize `docs/epics/` directories and move existing Epic7/8/9 docs under canonical structure.
- [x] Add `docs/epics/README.md` with current status table.
- [x] CMake: add `plugin_core` (STATIC), guarded by `if(NOT BUILD_UI_ONLY)`.
- [x] Add tests under `tests/unit/audio_engine_tests` (core smoke + SampleEngine).
- [x] Establish core job runbook and verify tests green.
- [ ] Prepare PRs 1–3 branches and open draft PRs to stage reviews early.

Completed Work Log (for traceability)
- 2025‑08‑07:
  - CMake gating added: `BUILD_PLUGIN`, `BUILD_LEGACY_TESTS`, `BUILD_UI_ONLY` flow fixed; test discovery enabled in core path.
  - `plugin_core` created with init stub; `SampleEngine` (sine renderer) implemented.
  - Unit tests added and executed: 39/39 passing in core job (added SynthVoice, EffectsChain, InstrumentLibraryManager, VoiceManager, SampleEngineFilePlayback, ONNXDaemonClient tests); UI‑only job remains green with WaveformDisplayComponent tests (10/10).
