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
    class EffectsChain;  // Epic 6: Forward declaration for effects chain

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
        
        // Filter support
        void setFilterParameters(const AdvancedSynthesisEngine::FilterParams& filterParams);
        
        // Unison support
        void setUnisonParameters(const AdvancedSynthesisEngine::UnisonParams& unisonParams);
        
        // Advanced envelope support
        void setEnvelopeParameters(const AdvancedSynthesisEngine::EnvelopeParams& envParams);
        
        // Epic 6: Effects chain control
        EffectsChain* getEffectsChain() const { return effectsChain.get(); }
        void setEffectParameter(int slotIndex, const juce::String& paramName, float value);
        void insertEffect(int slotIndex, const juce::String& effectType);
        void removeEffect(int slotIndex);
        void setEffectsChainEnabled(bool enabled);

        bool isVoiceActive() const;
        bool isPlayingNote(int midiNoteNumber) const;

    private:
        void clearCurrentNote();
        void generateBasicOscillator(juce::AudioBuffer<float>& buffer, int numSamples);
        void generateWavetableAudio(juce::AudioBuffer<float>& buffer, int numSamples);
        
        // Unison audio generation methods
        void generateUnisonAudio(juce::AudioBuffer<float>& buffer, int numSamples, double baseFrequency);
        void generateSingleOscillatorAudio(juce::AudioBuffer<float>& buffer, int numSamples, double baseFrequency, bool hasLFOModulation);

        SampleEngine sampleEngine;
        // Forward declaration - wavetable oscillator will be implemented
        class WavetableOscillator;
        std::unique_ptr<WavetableOscillator> wavetableOscillator;
        
        // Unison oscillators for rich sound
        static constexpr int MAX_UNISON_VOICES = 8;
        std::array<std::unique_ptr<WavetableOscillator>, MAX_UNISON_VOICES> unisonOscillators;
        std::array<float, MAX_UNISON_VOICES> unisonPanning;
        std::array<float, MAX_UNISON_VOICES> unisonDetune;
        
        // Forward declaration - LFO oscillator for modulation
        class LFOscillator;
        std::unique_ptr<LFOscillator> lfo1;
        std::unique_ptr<LFOscillator> lfo2;
        
        // Forward declaration - Filter for audio processing
        class StateVariableFilter;
        std::unique_ptr<StateVariableFilter> filter;
        
        // Forward declaration - Advanced envelope with curve shapes
        class AdvancedEnvelope;
        std::unique_ptr<AdvancedEnvelope> advancedEnvelope;
        
        // Epic 6: Professional effects chain
        std::unique_ptr<EffectsChain> effectsChain;

        SynthesisType currentSynthesisType;
        AdvancedSynthesisEngine::ModulationParams currentModulationParams;
        AdvancedSynthesisEngine::FilterParams currentFilterParams;
        AdvancedSynthesisEngine::UnisonParams currentUnisonParams;
        AdvancedSynthesisEngine::EnvelopeParams currentEnvelopeParams;

        bool active = false;
        int currentMidiNote = -1;
        float noteVelocity = 0.0f;
        double oscillatorPhase = 0.0;

        juce::ADSR adsr;
        juce::ADSR::Parameters adsrParams;

        double voiceSampleRate = 44100.0;
    };
}
