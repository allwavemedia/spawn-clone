//==============================================================================
/*
  SampleEngine.h
  Author:  BMad Dev Agent

  Core of the sample-based synthesis voice. This class is responsible for:
  - Selecting the correct sample based on velocity.
  - Handling real-time pitch shifting and time stretching.
  - Managing seamless looping with crossfades.
  - Interacting with the SampleCache to retrieve audio data.
*/
//==============================================================================

#pragma once

#include "../SynthesisTypes.h"
#include <juce_audio_basics/juce_audio_basics.h>
#include <vector>
#include <memory>
#include "LoopManager.h"
#include "PitchShifter.h"

namespace spawnclone::audio
{
    class SampleCache;

    class SampleEngine
    {
    public:
        SampleEngine(SampleCache& cache);

        void prepareToPlay(double sampleRate, int samplesPerBlock);
        void releaseResources();

        void startNote(int midiNoteNumber,
                       float velocity,
                       const SampleParams& params,
                       const std::vector<juce::AudioBuffer<float>>& samplePool);

        void stopNote();

        void processBlock(juce::AudioBuffer<float>& buffer, int startSample, int numSamples);

        bool isNoteFinished() const;

    private:
        void selectSample(float velocity,
                          const SampleParams& params,
                          const std::vector<juce::AudioBuffer<float>>& samplePool);

        SampleCache& sampleCache;
        PitchShifter pitchShifter;
        LoopManager loopManager;

        const juce::AudioBuffer<float>* currentSampleBuffer = nullptr;
        double currentPlaybackPosition = 0.0;
        double pitchRatio = 1.0;
        bool isPlaying = false;
        float noteVelocity = 0.0f;

        double engineSampleRate = 44100.0;
    };
}
