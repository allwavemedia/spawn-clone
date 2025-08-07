// SPDX-License-Identifier: MIT
#pragma once
#include <vector>
#include <cstddef>
#include <algorithm>
#include <cmath>

namespace spawnclone::core {

// Minimal dependency-light EffectsChain MVP:
// - Gain stage
// - One-pole lowpass filter (biquad-free), applied per-sample
// Processing order: Gain -> Lowpass
class EffectsChain {
public:
    EffectsChain() = default;

    // Configure processing options
    void setSampleRate(double sr) noexcept { sampleRate = (sr > 0.0 ? sr : 48000.0); recomputeCoeffs(); }
    double getSampleRate() const noexcept { return sampleRate; }

    void setGainDb(float db) noexcept {
        gainDb = db;
        // gain linear computed on the fly to keep precise for extreme values
    }

    float getGainDb() const noexcept { return gainDb; }

    void setLowpassCutoff(float hz) noexcept {
        cutoffHz = (hz > 0.0f ? hz : cutoffHz);
        recomputeCoeffs();
    }

    float getLowpassCutoff() const noexcept { return cutoffHz; }

    void setBypass(bool shouldBypass) noexcept { bypass = shouldBypass; }
    bool isBypassed() const noexcept { return bypass; }

    // Reset internal filter state
    void reset() noexcept { z1 = 0.0f; }

    // Process a mono buffer in-place
    void process(std::vector<float>& buffer) noexcept {
        if (buffer.empty()) return;
        if (bypass) return;

        // Apply gain
        const float g = dbToLin(gainDb);
        if (std::abs(g - 1.0f) > 1e-6f) {
            for (auto& s : buffer) s *= g;
        }

        // Apply one-pole lowpass
        // y[n] = a0 * x[n] + b1 * y[n-1]
        if (useFilter) {
            float y = z1;
            for (auto& s : buffer) {
                y = a0 * s + b1 * y;
                s = y;
            }
            z1 = y;
        }
    }

private:
    static float dbToLin(float db) noexcept {
        return std::pow(10.0f, db / 20.0f);
    }

    // Recompute one-pole lowpass coefficients using simple bilinear transform.
    // H(s) = wc / (s + wc), with wc = 2*pi*fc
    // Discrete: y[n] = a0*x[n] + b1*y[n-1]
    void recomputeCoeffs() noexcept {
        if (sampleRate <= 0.0 || cutoffHz <= 0.0f) {
            useFilter = false;
            a0 = 1.0f;
            b1 = 0.0f;
            return;
        }

        // Treat very high cutoffs (near Nyquist) as effectively transparent to avoid
        // initial-step smoothing that would alter constant buffers after gain.
        const float fs = static_cast<float>(sampleRate);
        const float nyquist = 0.5f * fs;
        if (cutoffHz >= 0.8f * nyquist) {
            useFilter = false;
            a0 = 1.0f;
            b1 = 0.0f;
            return;
        }

        const float dt = 1.0f / fs;
        const float RC = 1.0f / (2.0f * static_cast<float>(M_PI) * cutoffHz);
        const float alpha = dt / (RC + dt); // standard one-pole lowpass discretization
        // y = y + alpha*(x - y) => y[n] = alpha*x[n] + (1-alpha)*y[n-1]
        a0 = alpha;
        b1 = (1.0f - alpha);
        useFilter = true;
    }

    double sampleRate { 48000.0 };
    float gainDb { 0.0f };
    float cutoffHz { 20000.0f }; // default effectively transparent
    bool bypass { false };

    // one-pole state and coeffs
    float a0 { 1.0f };
    float b1 { 0.0f };
    float z1 { 0.0f };
    bool useFilter { false };
};

} // namespace spawnclone::core
