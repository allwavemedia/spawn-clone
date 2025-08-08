# ONNX Runtime Setup and Integration Guide

This document explains how to enable and validate ONNX Runtime-backed workflows for SpawnClone, including local build matrix commands, configuration flags, and troubleshooting. By default, the repository builds in a safe fallback mode without requiring ONNX Runtime.

## Summary

- Default builds work without ONNX Runtime (fallback mode).
- Optional integration is controlled by CMake option `USE_ONNX_RUNTIME` (ON/OFF).
- Real daemon/IPC code paths can be toggled with `ENABLE_DAEMON_IPC` (OFF by default).
- All AI-core tests remain deterministic and pass in both ON/OFF configurations.

---

## Installation (macOS)

The project currently assumes Homebrew paths for ONNX Runtime on Apple Silicon (arm64):

- Include: `/opt/homebrew/Cellar/onnxruntime/1.22.1/include/onnxruntime`
- Library: `/opt/homebrew/lib/libonnxruntime.dylib`

Install or upgrade ONNX Runtime via Homebrew:

```bash
brew update
brew install onnxruntime
# or
brew upgrade onnxruntime
```

If your ONNX Runtime version differs, update the CMake variables in `CMakeLists.txt` accordingly:
```cmake
set(ONNXRUNTIME_INCLUDE_DIR /opt/homebrew/Cellar/onnxruntime/1.22.1/include/onnxruntime)
set(ONNXRUNTIME_LIBRARY /opt/homebrew/lib/libonnxruntime.dylib)
```

Note: If you use a custom installation, you can also set:
- `ONNXRUNTIME_INCLUDE_DIRS` and `ONNXRUNTIME_LIBRARIES` via CMake cache or toolchain files
- Or adjust the `USE_ONNX_RUNTIME` block in `CMakeLists.txt` to auto-discover from env vars:
  - `ONNXRUNTIME_INCLUDE_DIRS`
  - `ONNXRUNTIME_LIBRARIES`

---

## Configuration Flags

- `USE_ONNX_RUNTIME` (default: ON)
  - When ON and ONNX Runtime is found, builds runtime-backed paths and defines `ONNX_RUNTIME_AVAILABLE=1` for relevant targets.
  - When OFF or not found, the build uses fallback deterministic logic (no runtime dependency).
- `ENABLE_DAEMON_IPC` (default: OFF)
  - When ON, compiles daemon/IPC code-path flags (e.g., `ONNX_DAEMON_PROCESS_ENABLED=1` for tests) to support future process management without enabling it by default.
  - When OFF, all ONNX Daemon client code uses safe, deterministic in-process behavior suitable for CI.

Both flags are toggled at configure time:

```bash
# Example (Debug build):
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug \
  -DUSE_ONNX_RUNTIME=ON \
  -DENABLE_DAEMON_IPC=OFF
```

---

## Local Build Matrix (Core + AI)

Run the following configurations locally to validate both paths:

1) Fallback (no runtime), deterministic:
```bash
cmake -S . -B build-core -DCMAKE_BUILD_TYPE=Debug \
  -DBUILD_UI_ONLY=OFF -DBUILD_TESTS=ON -DBUILD_LEGACY_TESTS=OFF -DBUILD_PLUGIN=OFF \
  -DUSE_ONNX_RUNTIME=OFF -DENABLE_DAEMON_IPC=OFF
cmake --build build-core -j 4
ctest --test-dir build-core --output-on-failure
```

2) Runtime-enabled (ONNX runtime present):
```bash
cmake -S . -B build-core -DCMAKE_BUILD_TYPE=Debug \
  -DBUILD_UI_ONLY=OFF -DBUILD_TESTS=ON -DBUILD_LEGACY_TESTS=OFF -DBUILD_PLUGIN=OFF \
  -DUSE_ONNX_RUNTIME=ON -DENABLE_DAEMON_IPC=OFF
cmake --build build-core -j 4
ctest --test-dir build-core --output-on-failure
```

3) Daemon/IPC code-path compilation (no external process required yet):
```bash
cmake -S . -B build-core -DCMAKE_BUILD_TYPE=Debug \
  -DBUILD_UI_ONLY=OFF -DBUILD_TESTS=ON -DBUILD_LEGACY_TESTS=OFF -DBUILD_PLUGIN=OFF \
  -DUSE_ONNX_RUNTIME=OFF -DENABLE_DAEMON_IPC=ON
cmake --build build-core -j 4
ctest --test-dir build-core --output-on-failure
```

Notes:
- The ONNXDaemonClient tests (`ONNXDaemonClientTest.*`) are deterministic and will pass in both ON/OFF modes.
- When `ONNX_RUNTIME_AVAILABLE=1` is defined, E2E tests will acknowledge the runtime-backed path is available for use.

---

## E2E AI Integration Test

An end-to-end AI integration test is provided:

- `tests/e2e/audio/test_epic7_ai_integration.cpp`

What it verifies:
- Deterministic sequences for the same inputs
- Basic signal sanity checks
- Simulated “quality” vs “speed” average-time ordering
- Optional compile-time macro `ONNX_RUNTIME_AVAILABLE` presence (when runtime is found)

Run with:
```bash
# Ensure BUILD_E2E_TESTS=ON (default ON in CMakeLists)
ctest --test-dir build-core -R Epic7_AI_Integration --output-on-failure
```

---

## CI Matrix (Recommended)

Add a matrix with `USE_ONNX_RUNTIME={ON,OFF}`. Pseudocode for GitHub Actions:

```yaml
strategy:
  matrix:
    runtimes: [OFF, ON]
steps:
  - name: Configure
    run: cmake -S . -B build-core -DCMAKE_BUILD_TYPE=RelWithDebInfo \
          -DBUILD_UI_ONLY=OFF -DBUILD_TESTS=ON -DBUILD_LEGACY_TESTS=OFF -DBUILD_PLUGIN=OFF \
          -DUSE_ONNX_RUNTIME=${{ matrix.runtimes }} -DENABLE_DAEMON_IPC=OFF
  - name: Build
    run: cmake --build build-core -j 4
  - name: Test
    run: ctest --test-dir build-core --output-on-failure
```

When `USE_ONNX_RUNTIME=ON`, ensure CI runners have ONNX Runtime installed or cached, or modify `CMakeLists.txt` to discover custom paths via environment variables.

---

## Troubleshooting

- ONNX Runtime not found:
  - The build will proceed in fallback mode with informative status messages and tests will still pass.
  - Verify include/library paths and version in `CMakeLists.txt`.
  - Confirm `onnxruntime_cxx_api.h` exists and the dylib is present.

- Linker errors in CI:
  - Ensure `ONNXRUNTIME_INCLUDE_DIRS` and `ONNXRUNTIME_LIBRARIES` are correctly set or discoverable.
  - Check architecture matches (arm64 vs x86_64).

- Determinism:
  - ONNXDaemonClient uses deterministic per-call seeds for repeatability. If modifying code, keep determinism for test predictability.

---

## Future Work (Daemon/IPC)

- With `ENABLE_DAEMON_IPC=ON`, additional compilation flags are set to support process/IPC paths.
- A future PR will implement actual process management and IPC (e.g., launching a Python-based daemon, socket/HTTP communication, health probes), guarded by this flag.

---

## Contact

Update requests and issues can be noted in the PRs section in `docs/workflows/complete-epics-workflow.md` under “Sprint 2 (ONNX/AI Integration)”.
