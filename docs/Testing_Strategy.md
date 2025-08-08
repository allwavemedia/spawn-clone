# Testing Strategy

Purpose
- Define the test lanes, expectations, and procedures to maintain green CI and deterministic results across the project.

Goals
- Keep UI-only lane green at all times.
- Validate core targets across ONNX Runtime ON/OFF matrix deterministically.
- Detect undefined behavior and memory issues nightly via sanitizers.
- Prevent documentation drift using link checks.

Test Lanes Overview
1) UI-only (safety lane)
   - Flags: BUILD_UI_ONLY=ON, BUILD_TESTS=ON
   - Scope: WaveformDisplayComponent and related UI tests
   - CI: .github/workflows/core-matrix.yml (job: ui-only)
   - Local:
     - cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_UI_ONLY=ON -DBUILD_TESTS=ON
     - cmake --build build -j 4
     - ctest --test-dir build --output-on-failure

2) Core matrix (engine + AI deterministic client)
   - Flags: BUILD_UI_ONLY=OFF, BUILD_TESTS=ON, BUILD_PLUGIN=OFF, BUILD_LEGACY_TESTS=OFF, USE_ONNX_RUNTIME={ON,OFF}
   - Scope: plugin_core, audio engine unit tests, ONNXDaemonClient unit/E2E tests
   - CI: .github/workflows/core-matrix.yml (job: core-matrix, matrix runtime=ON/OFF)
   - Local (RelWithDebInfo recommended):
     - cmake -S . -B build-core -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBUILD_UI_ONLY=OFF -DBUILD_TESTS=ON -DBUILD_LEGACY_TESTS=OFF -DBUILD_PLUGIN=OFF -DUSE_ONNX_RUNTIME=ON
     - cmake --build build-core -j 4
     - ctest --test-dir build-core --output-on-failure
     - Repeat with -DUSE_ONNX_RUNTIME=OFF for fallback coverage

   - ONNX detection:
     - Global CMake detection resolves Homebrew paths by default; macro ONNX_RUNTIME_AVAILABLE=1 is defined for tests when present.
     - When runtime is absent or disabled, deterministic fallback paths are exercised.

3) Nightly Sanitizers (ASan/UBSan)
   - Flags: BUILD_UI_ONLY=OFF, BUILD_TESTS=ON, sanitizers enabled via C/CXX/Linker flags
   - CI: .github/workflows/sanitizers-core.yml (scheduled daily, workflow_dispatch)
   - Compiler flags:
     - -fsanitize=address,undefined -fno-omit-frame-pointer (for C/CXX/Linker)
   - Runtime options (env):
     - ASAN_OPTIONS=detect_leaks=1,check_initialization_order=1,strict_init_order=1
     - UBSAN_OPTIONS=print_stacktrace=1
   - Local (example):
     - cmake -S . -B build-sanitizers -DCMAKE_BUILD_TYPE=RelWithDebInfo \
       -DBUILD_UI_ONLY=OFF -DBUILD_TESTS=ON -DBUILD_LEGACY_TESTS=OFF -DBUILD_PLUGIN=OFF \
       -DCMAKE_C_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer" \
       -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer" \
       -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address,undefined" \
       -DCMAKE_SHARED_LINKER_FLAGS="-fsanitize=address,undefined"
     - cmake --build build-sanitizers -j 3
     - ctest --test-dir build-sanitizers --output-on-failure -j 1

4) Plugin lane (to be enabled)
   - Flags: BUILD_UI_ONLY=OFF, BUILD_PLUGIN=ON
   - Scope: plugin build with juce_add_plugin (no UI-only safety)
   - CI: job stub present in core-matrix.yml; enable after PR 5
   - Local:
     - cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBUILD_UI_ONLY=OFF -DBUILD_PLUGIN=ON
     - cmake --build build -j 8

5) E2E headless render (to be enabled)
   - Flags: BUILD_UI_ONLY=OFF, BUILD_PLUGIN=ON
   - Scope: headless render tests (1 bar render, RMS/spectral checks, parameter sweep)
   - CI: job stub present; enable after PR 6
   - Local:
     - ctest --test-dir build -R offline_render --output-on-failure

Determinism & Tolerances
- ONNXDaemonClient is deterministic by design (stable per-call seed based on inputs).
- E2E tests should:
  - Use seeded scenarios for repeatability.
  - Employ numeric tolerances robust to minor drift (e.g., RMS and spectral envelopes within tight bounds).

CTest Filters (useful)
- Core smoke and engine:
  - -R "(PluginCoreSmokeTest|SampleEngineTest|SynthVoiceTest|EffectsChainTest|InstrumentLibraryManagerTest|VoiceManagerTest|SampleEngineFilePlaybackTest|ONNXDaemonClientTest)"
- Epic 7 AI Integration E2E:
  - -R Epic7_AI_Integration

Link Hygiene
- Script: scripts/check_links.py (skips code fences/inline code; checks local links by default)
- Usage:
  - python3 scripts/check_links.py
  - Optional: python3 scripts/check_links.py --external
- Recommendation:
  - Run after large doc moves (added to docs PR runbook).

CI Expectations
- UI-only and core-matrix lanes must remain green.
- Sanitizers nightly failures should block “nightly-green” and prompt triage.
- New code must add/adjust tests and update docs accordingly.

Ownership & Reviews
- Dev: code/tests
- QA: test coverage, E2E design
- PM/Architect: docs, workflow integrity
- DevOps: CI maintenance

Change Log Template (for PR footers)
- Summary:
- Affected targets:
- Flags required:
- Tests added/updated:
- Docs updated:
- Backward compatibility:
- Risks & mitigations:
