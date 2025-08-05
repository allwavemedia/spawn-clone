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
#include "AdvancedSynthesisEngine.h"  // Need full definition for ModulationParams

namespace spawnclone::audio
{
    // Forward declarations for types defined in AdvancedSynthesisEngine.h
    struct ModulationParams;

    class SynthVoice
    {
    public:
        SynthVoice(SampleCache& cache);
        ~SynthVoice(); // Need explicit destructor for unique_ptr with forward declaration

        void prepareToPlay(double sampleRate, int samplesPerBlock);
        void releaseResources();

        void startNote(int midiNoteNumber, float velocity, SynthesisType synthType, const SampleParams& sampleParams, const std::vector<juce::AudioBuffer<float>>& samplePool);
        void stopNote(float velocity, bool allowTailOff);
        void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples);
        
        // Wavetable support
        void setWavetable(const juce::AudioBuffer<float>* wavetable);
        
        // Modulation support - using forward declaration
        void setModulationParameters(const AdvancedSynthesisEngine::ModulationParams& modParams);

        bool isVoiceActive() const;
        bool isPlayingNote(int midiNoteNumber) const;

    private:
        void clearCurrentNote();
        void generateBasicOscillator(juce::AudioBuffer<float>& buffer, int numSamples);
        void generateWavetableAudio(juce::AudioBuffer<float>& buffer, int numSamples);

        SampleEngine sampleEngine;
        // Forward declaration - wavetable oscillator will be implemented
        class WavetableOscillator;
        std::unique_ptr<WavetableOscillator> wavetableOscillator;
        
        // Forward declaration - LFO oscillator for modulation
        class LFOscillator;
        std::unique_ptr<LFOscillator> lfo1;
        std::unique_ptr<LFOscillator> lfo2;

        SynthesisType currentSynthesisType;
        AdvancedSynthesisEngine::ModulationParams currentModulationParams;

        bool active = false;
        int currentMidiNote = -1;
        float noteVelocity = 0.0f;
        double oscillatorPhase = 0.0;

        juce::ADSR adsr;
        juce::ADSR::Parameters adsrParams;

        double voiceSampleRate = 44100.0;
    };
}
