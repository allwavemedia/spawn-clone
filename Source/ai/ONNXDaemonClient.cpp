// SPDX-License-Identifier: MIT
#include "ONNXDaemonClient.h"
#include <cmath>
#include <cstdint>
#include <cstdlib>

namespace spawnclone::ai {

namespace {
static uint32_t stableSeed(const std::string& mode, std::size_t length) {
    // FNV-1a 32-bit over mode + length bytes for deterministic per-call seed
    uint32_t h = 2166136261u;
    for (unsigned char c : mode) { h ^= static_cast<uint32_t>(c); h *= 16777619u; }
    for (int i = 0; i < 8; ++i) { h ^= static_cast<uint32_t>((length >> (i * 8)) & 0xFFu); h *= 16777619u; }
    return h;
}
} // anonymous namespace

ONNXDaemonClient::ONNXDaemonClient()
{
    // Fixed seed for deterministic tests
    rng.seed(1337u);
}

bool ONNXDaemonClient::startDaemon() noexcept
{
#ifdef ONNX_DAEMON_PROCESS_ENABLED
    if (const char* cmd = std::getenv("SPAWNCLONE_DAEMON_CMD"))
    {
        // Best-effort spawn; ignore exit status to keep CI deterministic.
        std::system(cmd);
    }
#endif
    running.store(true, std::memory_order_release);
    inFlight.store(0, std::memory_order_release);
    avgMs.store(0.0, std::memory_order_release);
    return true;
}

void ONNXDaemonClient::shutdownDaemon() noexcept
{
#ifdef ONNX_DAEMON_PROCESS_ENABLED
    if (const char* cmd = std::getenv("SPAWNCLONE_DAEMON_SHUTDOWN_CMD"))
    {
        // Best-effort shutdown; ignore exit status.
        std::system(cmd);
    }
#endif
    running.store(false, std::memory_order_release);
}

bool ONNXDaemonClient::isDaemonRunning() const noexcept
{
    return running.load(std::memory_order_acquire);
}

ONNXDaemonClient::Status ONNXDaemonClient::getDaemonStatus() const noexcept
{
    return Status{
        running.load(std::memory_order_acquire),
        avgMs.load(std::memory_order_acquire),
        inFlight.load(std::memory_order_acquire)
    };
}

double ONNXDaemonClient::getAverageInferenceTime() const noexcept
{
    return avgMs.load(std::memory_order_acquire);
}

bool ONNXDaemonClient::loadModel(const std::string& /*modelPath*/) noexcept
{
    // Stub: pretend load succeeded as long as daemon is running
    if (!isDaemonRunning())
        return false;

    modelLoaded.store(true, std::memory_order_release);
    return true;
}

std::vector<float> ONNXDaemonClient::generatePattern(const std::string& mode, std::size_t length) noexcept
{
    // Emulate request lifecycle
    inFlight.fetch_add(1, std::memory_order_acq_rel);
    totalCalls.fetch_add(1, std::memory_order_acq_rel);

    // Optional deterministic failure injection for tests (off by default)
    if (config.failureInjection && mode == "fail")
    {
        {
            std::lock_guard<std::mutex> lock(telemetryMutex);
            lastError = "Injected failure: mode=fail";
        }
        totalFailures.fetch_add(1, std::memory_order_acq_rel);
        lastMsValue.store(0.0, std::memory_order_release);
        inFlight.fetch_sub(1, std::memory_order_acq_rel);
        return {};
    }

    // Simulate deterministic "compute" time based on mode + length
    // No sleeping; just compute a value to fold into avgMs
    double simulatedMs = 0.1 * static_cast<double>(length);
    if (mode == "quality") simulatedMs *= 2.0;
    if (mode == "speed") simulatedMs *= 0.5;

    // Generate deterministic data:
    //  - If model is "loaded", combine a sine and PRNG; else pure PRNG
    std::vector<float> out;
    out.resize(length);

    // Deterministic per-call RNG based on inputs (mode, length)
    std::mt19937 gen(stableSeed(mode, length));
    std::uniform_real_distribution<float> uni(-1.0f, 1.0f);

    const bool loaded = modelLoaded.load(std::memory_order_acquire);
    for (std::size_t i = 0; i < length; ++i)
    {
        float r = uni(gen);
        if (loaded)
        {
            // Blend a low-amplitude sine with RNG for deterministic structure
            float s = std::sin(static_cast<float>(i) * 0.05f);
            out[i] = 0.7f * r + 0.3f * s;
        }
        else
        {
            out[i] = r * 0.5f; // lower amplitude when not loaded
        }
    }

#ifdef ONNX_DAEMON_PROCESS_ENABLED
    if (const char* ping = std::getenv("SPAWNCLONE_DAEMON_PING_CMD"))
    {
        // Optional ping to a daemon endpoint/health probe; ignore exit status.
        std::system(ping);
    }
#endif

    // Update telemetry
    lastMsValue.store(simulatedMs, std::memory_order_release);
    {
        std::lock_guard<std::mutex> lock(telemetryMutex);
        lastError.clear();
    }

    updateAverageMs(simulatedMs);

    inFlight.fetch_sub(1, std::memory_order_acq_rel);
    return out;
}

void ONNXDaemonClient::setConfig(const Config& c) noexcept
{
    config = c;
}

ONNXDaemonClient::Config ONNXDaemonClient::getConfig() const noexcept
{
    return config;
}

ONNXDaemonClient::Telemetry ONNXDaemonClient::getTelemetry() const noexcept
{
    Telemetry t;
    t.calls = totalCalls.load(std::memory_order_acquire);
    t.failures = totalFailures.load(std::memory_order_acquire);
    t.lastMs = lastMsValue.load(std::memory_order_acquire);
    {
        std::lock_guard<std::mutex> lock(telemetryMutex);
        t.lastError = lastError;
    }
    return t;
}

bool ONNXDaemonClient::isHealthy() const noexcept
{
    return running.load(std::memory_order_acquire) && inFlight.load(std::memory_order_acquire) == 0;
}

void ONNXDaemonClient::updateAverageMs(double elapsedMs) noexcept
{
    // Simple exponential moving average
    const double alpha = 0.2;
    double prev = avgMs.load(std::memory_order_acquire);
    double next = (1.0 - alpha) * prev + alpha * elapsedMs;
    avgMs.store(next, std::memory_order_release);
}

} // namespace spawnclone::ai
