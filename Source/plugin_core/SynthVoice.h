// SPDX-License-Identifier: MIT
#pragma once
#include <vector>
#include <cstddef>
#include <cstdint>
#include <cmath>
#include "SampleEngine.h"

namespace spawnclone::core {

// Minimal dependency-light voice that uses SampleEngine to render a tone for a note.
// This is an MVP to validate voice allocation and render plumbing without external deps.
class SynthVoice {
public:
    SynthVoice() = default;

    void setSampleRate(double sr) noexcept {
        sampleRate = (sr > 0.0 ? sr : 48000.0);
        osc.setSampleRate(sampleRate);
    }

    double getSampleRate() const noexcept { return sampleRate; }

    // MIDI note number (0..127); velocity 0..127. amplitude scales with velocity.
    void noteOn(int noteNumber, int velocity) noexcept {
        const float freq = midiNoteToHz(noteNumber);
        const float amp = velocityToAmplitude(velocity);
        targetFreqHz = freq;
        amplitude = amp;
        active = true;
        // do not reset phase to preserve legato/continuity; users can call reset if needed
    }

    void noteOff() noexcept {
        active = false;
        amplitude = 0.0f;
    }

    bool isActive() const noexcept { return active; }

    // Render mono output into 'out'. Resizes out to numSamples.
    // If inactive, writes zeros.
    void render(std::size_t numSamples, std::vector<float>& out) noexcept {
        if (!active || amplitude <= 0.0f) {
            out.assign(numSamples, 0.0f);
            return;
        }
        // Use SampleEngine to render a simple sine at the current target freq
        osc.renderSine(targetFreqHz, numSamples, out, amplitude);
    }

    void resetPhase() noexcept { osc.resetPhase(); }

    // Optional convenience setters
    void setAmplitude(float amp) noexcept { amplitude = clamp(amp, 0.0f, 1.0f); }
    void setFrequency(float hz) noexcept { targetFreqHz = (hz > 0.0f ? hz : targetFreqHz); }

private:
    static float clamp(float v, float lo, float hi) noexcept {
        return (v < lo ? lo : (v > hi ? hi : v));
    }

    static float midiNoteToHz(int noteNumber) noexcept {
        // A4 = 69 -> 440Hz
        return 440.0f * std::pow(2.0f, (static_cast<float>(noteNumber) - 69.0f) / 12.0f);
    }

    static float velocityToAmplitude(int velocity) noexcept {
        // Map 0..127 -> 0..1 with simple curve
        if (velocity <= 0) return 0.0f;
        const float v = static_cast<float>(velocity) / 127.0f;
        return std::sqrt(v); // slight emphasis for mid velocities
    }

    double sampleRate { 48000.0 };
    SampleEngine osc{};
    float targetFreqHz { 440.0f };
    float amplitude { 0.0f };
    bool active { false };
};

} // namespace spawnclone::core
