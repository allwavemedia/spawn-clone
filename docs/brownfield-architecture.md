# SpawnClone Brownfield Architecture Document

## Introduction

This document captures the **current state** of the SpawnClone code-base as of 2025-08-07.  The immediate business goal is to restore a green CI build by compiling and passing the `WaveformDisplayComponent` GTest suite only **while laying the groundwork for completing the partially-implemented audio-engine modules in a follow-up phase**.  The broader code-base contains numerous half-implemented audio-engine modules that must ultimately be finished to achieve a fully-operational plugin and support end-to-end testing.

### Document Scope

The recovery effort will proceed in **two sequential phases**:

**Phase 1 — UI-Only Green Build (current sprint)**
* UI component library under `Source/ui/` – primarily `WaveformDisplayComponent`.
* Minimal helpers required by that component and its unit test (`tests/ui/test_WaveformDisplayComponent.cpp`).
* Build system (CMake) configuration required to isolate a *UI-only* test target.

**Phase 2 — Audio-Engine Completion (next sprint)**
* Finish or stub-in **functional** versions of the audio-engine classes currently incomplete:
  * `SampleEngine`, `SynthVoice`, `EffectsChain`, `InstrumentLibraryManager`, etc.
* Provide real implementations of `GenerationParameters` and `ONNXDaemonClient` sufficient for offline rendering and unit tests.
* Extend test matrix to cover synthesis output, FX chains, and AI pattern generation end-to-end.

### Out-of-scope for Phase 1

* TensorFlow/ONNX inference optimisations beyond minimal stubs.
* Full performance profiling of synthesis path.
* Packaging/installer work.

## Quick Reference – Key Files and Entry Points

| Area | File/Dir | Purpose |
|------|----------|---------|
| UI | `Source/ui/WaveformDisplayComponent.h/.cpp` | Real-time waveform visualisation component under test |
| Tests | `tests/ui/test_WaveformDisplayComponent.cpp` | GTest suite exercising visualisation logic |
| Build | `CMakeLists.txt` (root) | Top-level project definition; currently tries to build **all** demos |

## Current Build Breakage Summary

| Category | Example Error | Root Cause |
|----------|---------------|-----------|
| Missing header | `fatal error: 'JuceHeader.h' file not found` | Demo & test sources include JUCE umbrella header but only JUCE module headers are vendored. Umbrella include path not exported. |
| Phantom include | `#include "GenerationParameters.h"` | File does not exist; leftover from AI pattern generator prototype. |
| Undefined symbols | `ONNXDaemonClient::start()` | Class declared in headers but no implementation compiled or linked. |
| Undefined symbols | `spawnclone::audio::SampleEngine` | Same – implementation in another branch, not merged yet. |

These blockers appear **before** any code in `WaveformDisplayComponent` is compiled.

## Module Inventory (Actual State)

### 1. UI / Plugin Editor

* `Source/ui/` – relatively complete.  Depends only on JUCE core + dsp + gui modules.
* No direct dependency on audio-engine classes if compiled standalone.

### 2. Audio Engine (incomplete)

* `Source/audio/InstrumentLibraryManager.h` includes missing `GenerationParameters.h`.
* Partial classes: `EffectsChain`, `SampleEngine`, `SynthVoice`.
* Several *demo* programs in repo reference those classes.

### 3. AI / ONNX Inference

* `ONNXDaemonManager.h`, `midi_model_daemon.py`, etc.
* No compiled C++ implementation for `ONNXDaemonClient`.
* Heavy external dependencies (onnxruntime arm64 tgz vendored).

## Technical Debt Snapshot

* Mixed include style (`JuceHeader.h` vs module includes).
* CMake builds *everything* by default, causing linkage of half-finished demos.
* No modular CMake targets; large monolithic executable list.
* Unused headers linger in include paths, masking missing implementation early.

## Immediate Recovery Plan (agreed)

_Phase 1 tasks remain unchanged (UI isolation & stubs)._  **Phase 2 will add:**

5. **Audio-Engine Implementation Sprint**
   * Replace Phase 1 header-only stubs with working code or compile-time feature flags.
   * Introduce `audio_engine_tests` covering voice allocation, sample playback and FX chain.
   * Integrate these sources into a new `plugin_core` static library reused by both the plugin and tests.

6. **End-to-End Validation**
   * Build the full plugin target linking UI + audio-engine.
   * Create a headless integration test that renders one bar of audio and asserts RMS / spectral features.

## Risks & Constraints

* Stubs compile but provide zero runtime functionality; unit tests must not exercise audio-engine behaviour.
* Duplicated symbol names may surface if other demos accidentally link against stubs; maintain isolation with target-specific include directories.
* Long-term: divergent build setups if UI target drifts from main plugin.

## Next Steps for Developers

1. Update **CMakeLists.txt** – add `option(BUILD_UI_ONLY "..." ON)` and wrap legacy targets.
2. Create stub headers in `Source/stubs/` and add that directory to include path for UI target only.
3. Confirm all UI tests pass locally; push to CI.
4. Document subsequent reintegration steps in **NEXT_STEPS.md**.

---

_Last updated: 2025-08-07_
