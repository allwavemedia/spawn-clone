//==============================================================================
/*
  ModulationEngine.h
  Author: BMad Dev Agent - Epic 6 Story 6.10
  
  Professional modulation effects processor with Chorus, Flanger, Phaser, and Tremolo.
  Follows the same architectural patterns as ReverbEngine and DelayEngine.
*/
//==============================================================================

#pragma once

#include "EffectProcessor.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <array>

namespace spawnclone::audio
{
    /**
     * Advanced modulation effects processor with multiple algorithms
     */
    class ModulationEngine : public EffectProcessor
    {
    public:
        enum class ModulationType
        {
            Chorus,         // Multi-voice chorus with independent tuning
            Flanger,        // Flanger with feedback control
            Phaser,         // 4-stage all-pass phaser with resonance
            Tremolo,        // Amplitude modulation with stereo offset
            Vibrato,        // Pitch modulation effect
            AutoPan,        // Stereo auto-panning effect
            RingMod,        // Ring modulation effect
            Rotary          // Rotary speaker simulation
        };
        
        ModulationEngine();
        ~ModulationEngine() override;

        //==============================================================================
        // EffectProcessor Implementation
        
        void prepareToPlay(double sampleRate, int samplesPerBlock) override;
        void releaseResources() override;
        void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;
        void reset() override;
        
        void setParameter(const juce::String& parameterName, float value, bool useInterpolation = true) override;
        float getParameter(const juce::String& parameterName) const override;
        juce::StringArray getParameterNames() const override;
        
        juce::String getEffectName() const override { return "Professional Modulation"; }
        juce::String getEffectCategory() const override { return "Modulation"; }
        int getLatencySamples() const override { return 0; }

        //==============================================================================
        // Modulation-Specific Interface
        
        /** Set modulation type */
        void setModulationType(ModulationType type);
        ModulationType getModulationType() const { return currentModulationType; }
        
        /** Set modulation rate in Hz */
        void setRate(float rateHz);
        
        /** Set modulation depth (0.0 to 1.0) */
        void setDepth(float depth);
        
        /** Set feedback amount for flanger/phaser (0.0 to 0.95) */
        void setFeedback(float feedback);
        
        /** Set stereo spread/phase offset */
        void setStereoSpread(float spread);
        
        /** Set mix between dry and wet signal */
        void setMix(float mix);
        
        /** Set number of voices for chorus (1-8) */
        void setVoiceCount(int voices);
        
        /** Set center delay for chorus/flanger */
        void setCenterDelay(float delayMs);

    private:
        //==============================================================================
        // Internal Processing Components
        
        /** Modulation LFO with multiple waveforms */
        class ModulationLFO
        {
        public:
            enum class Waveform { Sine, Triangle, Sawtooth, Square, Random };
            
            void prepareToPlay(double sampleRate);
            void reset();
            void setFrequency(float frequency);
            void setWaveform(Waveform waveform);
            float getNextSample();
            
        private:
            double sampleRate = 44100.0;
            float frequency = 1.0f;
            float phase = 0.0f;
            Waveform currentWaveform = Waveform::Sine;
            float randomValue = 0.0f;
            juce::Random random;
        };
        
        /** Variable delay line for modulation effects */
        class ModulationDelayLine
        {
        public:
            void prepareToPlay(double sampleRate, float maxDelayMs);
            void reset();
            
            float processSample(float input, float delayMs, bool interpolate = true);
            void setFeedback(float feedback) { feedbackAmount = feedback; }
            
        private:
            std::vector<float> buffer;
            int writeIndex = 0;
            float feedbackAmount = 0.0f;
            double sampleRate = 44100.0;
            
            float interpolateLinear(float x, float y0, float y1) const;
        };
        
        /** All-pass filter for phaser */
        class AllPassFilter
        {
        public:
            void prepareToPlay(double sampleRate);
            void reset();
            float processSample(float input, float frequency);
            
        private:
            double sampleRate = 44100.0;
            float state = 0.0f;
            float coefficient = 0.0f;
        };
        
        /** Simple one-pole filter for tone shaping */
        class OnePoleFilter
        {
        public:
            void setSampleRate(double sampleRate);
            void setFrequency(float frequency);
            void reset();
            float processLowpass(float input);
            float processHighpass(float input);
            
        private:
            double sampleRate = 44100.0;
            float coefficient = 0.0f;
            float state = 0.0f;
        };

        //==============================================================================
        // Modulation Type Implementations
        
        void processChorus(juce::AudioBuffer<float>& buffer);
        void processFlanger(juce::AudioBuffer<float>& buffer);
        void processPhaser(juce::AudioBuffer<float>& buffer);
        void processTremolo(juce::AudioBuffer<float>& buffer);
        void processVibrato(juce::AudioBuffer<float>& buffer);
        void processAutoPan(juce::AudioBuffer<float>& buffer);
        void processRingMod(juce::AudioBuffer<float>& buffer);
        void processRotary(juce::AudioBuffer<float>& buffer);

        //==============================================================================
        // State Variables
        
        ModulationType currentModulationType = ModulationType::Chorus;
        
        // Parameter smoothers
        ParameterSmoother rateSmoother;
        ParameterSmoother depthSmoother;
        ParameterSmoother feedbackSmoother;
        ParameterSmoother stereoSpreadSmoother;
        ParameterSmoother mixSmoother;
        ParameterSmoother centerDelaySmoother;
        
        // Current parameter values
        float rate = 1.0f;              // Hz
        float depth = 0.5f;             // 0.0 - 1.0
        float feedback = 0.0f;          // 0.0 - 0.95
        float stereoSpread = 0.5f;      // 0.0 - 1.0
        float mix = 0.5f;               // 0.0 - 1.0
        float centerDelay = 5.0f;       // milliseconds
        int voiceCount = 4;             // 1 - 8
        
        // Processing components
        std::array<ModulationLFO, 2> lfos;              // Left and right LFOs
        std::array<ModulationDelayLine, 8> delayLines;  // Up to 8 voices/stages
        std::array<AllPassFilter, 8> allPassFilters;    // For phaser
        std::array<OnePoleFilter, 4> toneFilters;       // Tone shaping
        
        // Chorus voice parameters
        struct ChorusVoice
        {
            float delayOffset = 0.0f;      // Offset from center delay
            float pitchOffset = 0.0f;      // Pitch detuning
            float panPosition = 0.0f;      // Stereo position
            float gain = 1.0f;             // Voice level
        };
        std::array<ChorusVoice, 8> chorusVoices;
        
        // Temporary buffers
        juce::AudioBuffer<float> tempBuffer;
        juce::AudioBuffer<float> wetBuffer;
        
        double currentSampleRate = 44100.0;
        int currentBlockSize = 512;

        //==============================================================================
        // Processing Methods
        
        void applyWetDryMix(juce::AudioBuffer<float>& wetBuffer, const juce::AudioBuffer<float>& dryBuffer);

        //==============================================================================
        // Helper Methods
        
        float millisecondsToSamples(float ms) const;
        float samplesToMilliseconds(float samples) const;
        void initializeChorusVoices();
        void updatePhaserFrequencies(float baseFreq, float lfoValue);

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModulationEngine)
    };

} // namespace spawnclone::audio
