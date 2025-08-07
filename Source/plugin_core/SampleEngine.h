// SPDX-License-Identifier: MIT
#pragma once
#include <vector>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <string>

namespace spawnclone::core {

// Minimal, dependency-light sample engine for Sprint 1 scaffolding.
// Generates simple test tones to validate render loop plumbing without JUCE.
class SampleEngine {
public:
    SampleEngine() = default;

    void setSampleRate(double sr) noexcept { sampleRate = (sr > 0.0 ? sr : 48000.0); }
    double getSampleRate() const noexcept { return sampleRate; }

    // Render a sine tone into 'out' with frequency 'freqHz' and amplitude in [0,1].
    // Resizes 'out' to numSamples.
    void renderSine(float freqHz, std::size_t numSamples, std::vector<float>& out, float amplitude = 0.1f) noexcept
    {
        if (numSamples == 0 || sampleRate <= 0.0) {
            out.assign(numSamples, 0.0f);
            return;
        }

        out.resize(numSamples);
        const double twoPi = 6.283185307179586476925286766559;
        const double phaseInc = twoPi * static_cast<double>(freqHz) / sampleRate;
        double p = phase;

        const float amp = clamp(amplitude, 0.0f, 1.0f);
        for (std::size_t i = 0; i < numSamples; ++i) {
            out[i] = amp * static_cast<float>(std::sin(p));
            p += phaseInc;
            if (p > twoPi) p -= twoPi;
        }

        phase = p;
    }

    void resetPhase() noexcept { phase = 0.0; }

    // Load a simple text file containing whitespace-separated float samples (mono).
    // Returns true if at least one sample was loaded.
    bool loadFromTextFile(const std::string& path) noexcept
    {
        std::ifstream in(path);
        if (!in.good())
        {
            fileBuffer.clear();
            filePos = 0;
            return false;
        }
        std::vector<float> tmp;
        tmp.reserve(4096);
        float v;
        while (in >> v) tmp.push_back(v);
        fileBuffer = std::move(tmp);
        filePos = 0;
        return !fileBuffer.empty();
    }

    // Render samples from previously loaded file buffer (cyclic if shorter than numSamples).
    // Applies a simple gain scalar.
    void renderFromFile(std::size_t numSamples, std::vector<float>& out, float gain = 1.0f) noexcept
    {
        if (numSamples == 0 || fileBuffer.empty())
        {
            out.assign(numSamples, 0.0f);
            return;
        }
        out.resize(numSamples);
        const std::size_t N = fileBuffer.size();
        for (std::size_t i = 0; i < numSamples; ++i)
        {
            out[i] = gain * fileBuffer[filePos];
            filePos = (filePos + 1) % N;
        }
    }

private:
    static float clamp(float v, float lo, float hi) noexcept {
        return (v < lo ? lo : (v > hi ? hi : v));
    }

    double sampleRate { 48000.0 };
    double phase { 0.0 };

    // File-backed sample playback (text float samples)
    std::vector<float> fileBuffer;
    std::size_t filePos { 0 };
};

} // namespace spawnclone::core
