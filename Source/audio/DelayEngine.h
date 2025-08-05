//==============================================================================
/*
  DelayEngine.h
  Author: BMad Dev Agent - Epic 6 Story 6.3
  
  Professional delay/echo processor with multiple delay types and advanced features
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
     * Advanced delay processor with multiple delay types and modulation
     */
    class DelayEngine : public EffectProcessor
    {
    public:
        enum class DelayType
        {
            Digital,        // Clean digital delay
            Analog,         // Warm analog-style delay with saturation
            Tape,           // Vintage tape echo simulation
            PingPong,       // Stereo ping-pong delay
            MultiTap,       // Multiple delay taps
            Reverse,        // Reverse delay effect
            Granular,       // Granular delay with pitch shifting
            Modulated       // Chorus-style modulated delay
        };
        
        DelayEngine();
        ~DelayEngine() override;

        //==============================================================================
        // EffectProcessor Implementation
        
        void prepareToPlay(double sampleRate, int samplesPerBlock) override;
        void releaseResources() override;
        void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;
        void reset() override;
        
        void setParameter(const juce::String& parameterName, float value, bool useInterpolation = true) override;
        float getParameter(const juce::String& parameterName) const override;
        juce::StringArray getParameterNames() const override;
        
        juce::String getEffectName() const override { return "Professional Delay"; }
        juce::String getEffectCategory() const override { return "Time-Based"; }
        int getLatencySamples() const override { return 0; }

        //==============================================================================
        // Delay-Specific Interface
        
        /** Set delay type */
        void setDelayType(DelayType type);
        DelayType getDelayType() const { return currentDelayType; }
        
        /** Set delay time in milliseconds */
        void setDelayTime(float timeMs);
        
        /** Set feedback amount (0.0 to 0.95) */
        void setFeedback(float feedback);
        
        /** Set low-cut filter frequency for feedback */
        void setLowCut(float frequency);
        
        /** Set high-cut filter frequency for feedback */  
        void setHighCut(float frequency);
        
        /** Set stereo spread for ping-pong delay */
        void setStereoSpread(float spread);
        
        /** Set modulation rate for modulated delays */
        void setModulationRate(float rateHz);
        
        /** Set modulation depth */
        void setModulationDepth(float depth);
        
        /** Sync to host tempo */
        void setTempoSyncEnabled(bool enabled);
        void setNoteDivision(float division); // 1.0 = quarter note, 0.5 = eighth, etc.

    private:
        //==============================================================================
        // Internal Processing Components
        
        /** Multi-tap delay line */
        class DelayLine
        {
        public:
            void prepareToPlay(double sampleRate, float maxDelaySeconds);
            void reset();
            
            float readSample(float delaySamples, bool interpolate = true) const;
            void writeSample(float sample);
            void processSample(float input);
            
            void setDelayTime(float delaySamples);
            float getDelayTime() const { return currentDelay; }
            
        private:
            std::vector<float> buffer;
            int writeIndex = 0;
            float currentDelay = 0.0f;
            double sampleRate = 44100.0;
            
            float interpolateHermite(float x, float y0, float y1, float y2, float y3) const;
        };
        
        /** Simple one-pole filter for tone control */
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
        
        /** LFO for modulation */
        class LFO
        {
        public:
            void setSampleRate(double sampleRate);
            void setFrequency(float frequency);
            void reset();
            float getNextSample();
            
        private:
            double sampleRate = 44100.0;
            float frequency = 1.0f;
            float phase = 0.0f;
        };
        
        /** Analog-style saturation */
        class AnalogSaturation
        {
        public:
            float processSample(float input, float drive = 1.0f);
            
        private:
            float tanhApprox(float x);
        };

        //==============================================================================
        // Delay Type Implementations
        
        void processDigitalDelay(juce::AudioBuffer<float>& buffer);
        void processAnalogDelay(juce::AudioBuffer<float>& buffer);
        void processTapeDelay(juce::AudioBuffer<float>& buffer);
        void processPingPongDelay(juce::AudioBuffer<float>& buffer);
        void processMultiTapDelay(juce::AudioBuffer<float>& buffer);
        void processReverseDelay(juce::AudioBuffer<float>& buffer);
        void processGranularDelay(juce::AudioBuffer<float>& buffer);
        void processModulatedDelay(juce::AudioBuffer<float>& buffer);

        //==============================================================================
        // State Variables
        
        DelayType currentDelayType = DelayType::Digital;
        
        // Parameter smoothers
        ParameterSmoother delayTimeSmoother;
        ParameterSmoother feedbackSmoother;
        ParameterSmoother lowCutSmoother;
        ParameterSmoother highCutSmoother;
        ParameterSmoother stereoSpreadSmoother;
        ParameterSmoother modRateSmoother;
        ParameterSmoother modDepthSmoother;
        
        // Current parameter values
        float delayTimeMs = 250.0f;
        float feedback = 0.3f;
        float lowCutFreq = 100.0f;
        float highCutFreq = 8000.0f;
        float stereoSpread = 0.5f;
        float modulationRate = 0.5f;
        float modulationDepth = 0.1f;
        
        // Tempo sync
        bool tempoSyncEnabled = false;
        float noteDivision = 0.25f; // Quarter note
        float currentTempo = 120.0f;
        
        // Processing components
        std::array<DelayLine, 2> delayLines;     // Left and right channels
        std::array<OnePoleFilter, 4> toneFilters; // Low/high cut for L/R channels
        std::array<LFO, 2> modulationLFOs;       // Modulation for L/R
        AnalogSaturation saturation;
        
        // Multi-tap delay configuration
        struct DelayTap
        {
            float delayTime = 0.0f;
            float gain = 0.0f;
            float pan = 0.0f;
        };
        std::array<DelayTap, 8> delayTaps;
        
        // Temporary buffers
        juce::AudioBuffer<float> feedbackBuffer;
        juce::AudioBuffer<float> tempBuffer;
        
        double currentSampleRate = 44100.0;
        int currentBlockSize = 512;

        //==============================================================================
        // Helper Methods
        
        float millisecondsToSamples(float ms) const;
        float samplesToMilliseconds(float samples) const;
        float noteDivisionToMilliseconds(float division, float tempo) const;
        void updateDelayTimesFromTempo();

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DelayEngine)
    };

} // namespace spawnclone::audio
