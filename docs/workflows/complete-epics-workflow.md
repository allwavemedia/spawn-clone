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

Principles
- Keep BUILD_UI_ONLY=ON as default CI job until plugin is ready.
- Add new targets behind feature flags to avoid breaking green builds.
- Make small, reviewable PRs; each PR includes tests and docs updates.
- Canonicalize documents and archive duplicates to prevent drift.

Status Snapshot (inputs)
- Phase 1 UI‑only green build complete (ui_tests target; WaveformDisplayComponent tests pass).
- Epic 7 largely complete (multiple completion/final docs).
- Epic 8 complete (8.1–8.4 story completion docs).
- Epic 9 partial: 9.1 instrument library tasks present; 9.2 advanced synthesis plan/progress present.
- Brownfield architecture doc defines Phase 2 scope.

Repository Switches (CMake options)
- BUILD_UI_ONLY: ON (default) — build UI tests only (Phase 1 safety)
- BUILD_TESTS: ON (default)
- BUILD_PLUGIN: OFF (enable only when integrating plugin)
- USE_ONNX_RUNTIME: optional ON (Homebrew or vendored) with fallback

CI Jobs (matrix)
- ui-only: BUILD_UI_ONLY=ON (always green)
- core-tests: BUILD_UI_ONLY=OFF; build plugin_core + unit tests
- ai-integration: BUILD_UI_ONLY=OFF; USE_ONNX_RUNTIME={ON,OFF}
- plugin: BUILD_UI_ONLY=OFF; BUILD_PLUGIN=ON
- e2e: BUILD_UI_ONLY=OFF; headless render tests

Milestones and PR Sequence

Sprint 1 (Audio Engine Foundation)
Goal
- Replace Phase 1 stubs with functional MVP for:
  - SampleEngine
  - SynthVoice
  - EffectsChain
  - InstrumentLibraryManager

Deliverables
- New static library: plugin_core (engine core, MIDI/parameters, synthesis, FX).
- Unit tests: audio_engine_tests for voice allocation, sample playback, FX routing.
- CMake refactor: target-specific include dirs and guards to keep UI-only job green.

PRs
1) Docs Reorg PR (docs only)
   - Create canonical epic directories under docs/epics/:
     - epic-07-realtime-generation, epic-08-effects-processing, epic-09-sound-design, epic-10-performance-features
   - Move legacy reports (Epic7_*.md, Epic8_*.md) into their epic folders under reports/ and archive/ as needed.
   - Update docs/Project_Status_Summary_*.md and docs/SpawnClone_Workflow_Guidance.md to point to canonical indexes.
   - Add docs/epics/README.md with a table of epics, status, and canonical links.
   - Mark superseded documents with an archival banner and move to docs/archive/.

2) CMake Refactor PR (non-breaking)
   - Add plugin_core (STATIC) target under if(NOT BUILD_UI_ONLY).
   - Do NOT link or build plugin by default; keep BUILD_PLUGIN=OFF.
   - Ensure target_include_directories isolate engine headers from UI-only target.

3) Engine MVP PR
   - Implement minimal SampleEngine, SynthVoice, EffectsChain, InstrumentLibraryManager (load minimal sample, basic voice mixdown, linear gain FX).
   - Add tests: tests/unit/audio_engine_tests/*.cpp
   - Add sanitizers job (nightly) for engine target.
   - Keep UI-only CI job green.

Sprint 2 (ONNX/AI Integration)
Goal
- Complete ONNXDaemonClient C++ implementation, robust ONNX Runtime integration, and stabilize AI workflows.

Deliverables
- Source/ai/ONNXDaemonClient.cpp: non-blocking connect/start/stop, request lifecycle, error handling.
- Runtime flagging for “Quality vs Speed” paths; proper discovery of onnxruntime (Homebrew path default, env/cmake override).
- Integration tests with deterministic seeds; fallback mode when ONNX runtime unavailable.

PRs
4) ONNX/AI Core PR
   - Finalize ONNXDaemonClient implementation and tests.
   - Update tests/e2e/audio/test_epic7_ai_integration to be deterministic and CI-stable.
   - Matrix: USE_ONNX_RUNTIME={ON,OFF}.

Sprint 3 (Plugin Integration + E2E)
Goal
- Enable full plugin build with plugin_core + UI; add headless render E2E tests.

Deliverables
- BUILD_PLUGIN=ON path functional (juce_add_plugin target).
- Parameter mapping (SynthesisParameterMapper, ParameterManager) reviewed and aligned with engine.
- Headless tests: render 1 bar, assert RMS/spectral envelopes; parameter sweep coverage.

PRs
5) Plugin Integration PR
   - Enable plugin target behind BUILD_PLUGIN.
   - Wire up engine to UI; WaveformDisplayComponent connects to real buffers.
   - Replace deprecated font calls with modern FontOptions usage.

6) E2E Tests PR
   - Add tests/e2e/audio/offline_render_tests.cpp (RMS/spectral assertions).
   - Expand tests to cover parameter sweeps and a few presets.

Sprint 4 (Packaging, Performance, Documentation)
Goal
- Produce installable artifacts, improve performance observability, finalize documentation.

Deliverables
- Packaging pipeline (DMG on macOS) via scripts/build_dmg.sh; optional notarization scaffold.
- Lightweight performance benchmarks; CI artifacts store metrics.
- Final docs: PRD, Architecture, User Guide, Release Notes; epic status indexes updated.

PRs
7) Packaging PR
   - Add CI job for packaging; produce DMG artifacts.

8) Performance + Docs PR
   - Add timing/profiling hooks; store per-commit metrics artifact.
   - Finalize documentation suite and epic indexes.

Acceptance Criteria (End State)
- All epic indexes present with consistent status; duplicates archived.
- Green CI across: ui-only, engine unit tests, AI integration (with and without ONNX Runtime), plugin build, E2E tests.
- Installable artifact produced in CI.
- User documentation and release notes complete.

Runbook (Developer Commands)

UI‑only tests (default safety)
- cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_UI_ONLY=ON -DBUILD_TESTS=ON
- cmake --build build -j 4
- ctest --test-dir build --output-on-failure

Engine/core tests (when implementing plugin_core)
- cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBUILD_UI_ONLY=OFF -DBUILD_TESTS=ON
- cmake --build build -j 8
- ctest --test-dir build -R audio_engine_tests --output-on-failure

AI integration tests (matrix)
- cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBUILD_UI_ONLY=OFF -DBUILD_TESTS=ON -DUSE_ONNX_RUNTIME=ON
- cmake --build build -j 8
- ctest --test-dir build -R epic7_ai_integration --output-on-failure
- Repeat with -DUSE_ONNX_RUNTIME=OFF for fallback.

Plugin build (manual gate)
- cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBUILD_UI_ONLY=OFF -DBUILD_PLUGIN=ON
- cmake --build build -j 8

E2E headless render tests
- ctest --test-dir build -R offline_render --output-on-failure

Risk Controls
- Keep engine work behind if(NOT BUILD_UI_ONLY); never break ui_tests.
- Use target‑specific include directories; avoid accidental link of stubs into unrelated targets.
- Deterministic seeds in AI tests; make assertions tolerant to minor numeric drift.

Documentation Governance
- Canonicalize: docs/epics/{epic}/index.md, status.md, plan.md, reports/
- Archive banner for superseded docs; move to docs/archive/
- docs/epics/README.md: single source of truth for epic mapping and status.

Change Log Template (PR footer)
- Summary:
- Affected targets:
- Flags required:
- Tests added/updated:
- Docs updated:
- Backward compatibility:
- Risks & mitigations:

Appendix: Initial Task Checklist (Do Now)
- [ ] Create/normalize docs/epics/ directories and move existing Epic7/8/9 docs under canonical structure.
- [ ] Add docs/epics/README.md with current status table.
- [ ] CMake: add plugin_core (STATIC), guarded by if(NOT BUILD_UI_ONLY).
- [ ] Add tests/unit/audio_engine_tests skeleton; keep disabled until first MVP lands (or mark EXPECT_TRUE(true) placeholders gated by macro).
- [ ] Prepare PRs 1–3 branches and open draft PRs to stage reviews early.
