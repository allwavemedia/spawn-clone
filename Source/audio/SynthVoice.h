//==============================================================================
/*
  SynthVoice.h
  Author:  BMad Dev Agent

  Represents a single voice in the synthesis engine.
  A voice can be triggered by a MIDI note and will generate audio until
  the note is released and the sound fades out. This class will delegate
  to different synthesis engines (Wavetable, Sample-based, etc.) based on
  the current parameters.
*/
//==============================================================================

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "synthesis/SampleEngine.h"
#include "SynthesisTypes.h"

namespace spawnclone::audio
{
    // Forward declaration
    class AdvancedSynthesisEngine;

    class SynthVoice
    {
    public:
        SynthVoice(SampleCache& cache);

        void prepareToPlay(double sampleRate, int samplesPerBlock);
        void releaseResources();

        void startNote(int midiNoteNumber, float velocity, SynthesisType synthType, const SampleParams& sampleParams, const std::vector<juce::AudioBuffer<float>>& samplePool);
        void stopNote(float velocity, bool allowTailOff);
        void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples);

        bool isVoiceActive() const;
        bool isPlayingNote(int midiNoteNumber) const;

    private:
        void clearCurrentNote();

        SampleEngine sampleEngine;
        // std::unique_ptr<WavetableEngine> wavetableEngine;
        // ... other engines

        SynthesisType currentSynthesisType;

        bool active = false;
        int currentMidiNote = -1;
        float noteVelocity = 0.0f;

        juce::ADSR adsr;
        juce::ADSR::Parameters adsrParams;

        double voiceSampleRate = 44100.0;
    };
}
