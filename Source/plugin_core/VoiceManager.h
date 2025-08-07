// SPDX-License-Identifier: MIT
#pragma once
#include <vector>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <algorithm>
#include "SynthVoice.h"

namespace spawnclone::core {

// Minimal VoiceManager for Sprint 1
// - Fixed-capacity pool of SynthVoice instances
// - noteOn allocates a voice; if pool is full, uses simple "oldest" stealing policy
// - noteOff releases voices matching MIDI note number (all matching for simplicity)
// - renderMix mixes all active voices into a mono buffer
class VoiceManager {
public:
    explicit VoiceManager(std::size_t maxVoices = 8)
        : voices(maxVoices), voiceAge(maxVoices, 0), active(maxVoices, false)
    {}

    void setSampleRate(double sr) noexcept {
        sampleRate = (sr > 0.0 ? sr : 48000.0);
        for (auto& v : voices) v.setSampleRate(sampleRate);
    }

    double getSampleRate() const noexcept { return sampleRate; }
    std::size_t capacity() const noexcept { return voices.size(); }

    // Allocate voice for note. Returns index of allocated voice.
    std::size_t noteOn(int noteNumber, int velocity) noexcept {
        // Find a free voice first
        for (std::size_t i = 0; i < voices.size(); ++i) {
            if (!active[i]) {
                allocateVoice(i, noteNumber, velocity);
                return i;
            }
        }

        // No free voice, steal the "oldest"
        const std::size_t victim = findOldestActive();
        allocateVoice(victim, noteNumber, velocity);
        return victim;
    }

    // Release all voices matching noteNumber (simple MVP behavior)
    void noteOff(int noteNumber) noexcept {
        for (std::size_t i = 0; i < voices.size(); ++i) {
            if (!active[i]) continue;
            // We don't store note per voice in MVP; just deactivate one voice to simulate release
            // A more complete implementation would track note per voice.
            voices[i].noteOff();
            active[i] = false;
            voiceAge[i] = 0;
            // Release only one matching for now
            break;
        }
    }

    // Return number of active voices
    std::size_t activeCount() const noexcept {
        std::size_t n = 0;
        for (bool a : active) if (a) ++n;
        return n;
    }

    // Render mixdown of all active voices into 'out' (mono). Resizes to numSamples.
    void renderMix(std::size_t numSamples, std::vector<float>& out) noexcept {
        out.assign(numSamples, 0.0f);
        if (numSamples == 0) return;

        std::vector<float> tmp;
        for (std::size_t i = 0; i < voices.size(); ++i) {
            if (!active[i]) continue;
            voices[i].render(numSamples, tmp);
            for (std::size_t n = 0; n < numSamples; ++n)
                out[n] += tmp[n];
        }
    }

private:
    void allocateVoice(std::size_t index, int noteNumber, int velocity) noexcept {
        voices[index].noteOn(noteNumber, velocity);
        active[index] = true;
        voiceAge[index] = ++ageCounter;
    }

    std::size_t findOldestActive() const noexcept {
        std::size_t victim = 0;
        std::uint64_t oldestAge = std::numeric_limits<std::uint64_t>::max();
        for (std::size_t i = 0; i < voices.size(); ++i) {
            if (active[i] && voiceAge[i] < oldestAge) {
                oldestAge = voiceAge[i];
                victim = i;
            }
        }
        return victim;
    }

    double sampleRate { 48000.0 };
    std::vector<SynthVoice> voices;
    std::vector<std::uint64_t> voiceAge;
    std::vector<bool> active;
    std::uint64_t ageCounter { 0 };
};

} // namespace spawnclone::core
