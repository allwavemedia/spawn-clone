// SPDX-License-Identifier: MIT
#pragma once
#include <atomic>
#include <chrono>
#include <cstddef>
#include <random>
#include <string>
#include <vector>

namespace spawnclone::ai {

// Minimal, dependency-light ONNX daemon client stub for Sprint 2.
// Purpose: Provide a robust, deterministic interface suitable for unit/integration tests
// without requiring ONNX Runtime or Python daemon at this stage.
class ONNXDaemonClient {
public:
    struct Status {
        bool running { false };
        double averageInferenceMs { 0.0 };
        int requestsInFlight { 0 };
    };

    ONNXDaemonClient();

    // Lifecycle
    bool startDaemon() noexcept;
    void shutdownDaemon() noexcept;
    bool isDaemonRunning() const noexcept;

    // Status/metrics
    Status getDaemonStatus() const noexcept;
    double getAverageInferenceTime() const noexcept;

    // Model operations (no-ops in stub, persistent flags for tests)
    bool loadModel(const std::string& modelPath) noexcept;

    // Inference (deterministic pattern for tests; updates avg inference time)
    std::vector<float> generatePattern(const std::string& mode, std::size_t length = 128) noexcept;

private:
    void updateAverageMs(double elapsedMs) noexcept;

    std::atomic<bool> running { false };
    std::atomic<int> inFlight { 0 };
    std::atomic<double> avgMs { 0.0 };
    std::atomic<bool> modelLoaded { false };

    // Deterministic RNG for repeatable tests
    std::mt19937 rng;
};

} // namespace spawnclone::ai
