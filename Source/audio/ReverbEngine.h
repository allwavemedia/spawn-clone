//==============================================================================
/*
  ReverbEngine.h
  Author: BMad Dev Agent - Epic 6 Story 6.2
  
  Professional algorithmic reverb with multiple algorithms and convolution support
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
     * Advanced reverb processor with multiple algorithms
     */
    class ReverbEngine : public EffectProcessor
    {
    public:
        enum class Algorithm
        {
            Plate,          // Classic plate reverb simulation
            Hall,           // Large hall acoustics
            Room,           // Small room ambience
            Spring,         // Vintage spring reverb
            Convolution,    // Impulse response convolution
            Shimmer,        // Pitch-shifted feedback reverb
            Reverse,        // Reverse reverb effect
            Gated           // Gated reverb with envelope
        };
        
        ReverbEngine();
        ~ReverbEngine() override;

        //==============================================================================
        // EffectProcessor Implementation
        
        void prepareToPlay(double sampleRate, int samplesPerBlock) override;
        void releaseResources() override;
        void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;
        void reset() override;
        
        void setParameter(const juce::String& parameterName, float value, bool useInterpolation = true) override;
        float getParameter(const juce::String& parameterName) const override;
        juce::StringArray getParameterNames() const override;
        
        juce::String getEffectName() const override { return "Professional Reverb"; }
        juce::String getEffectCategory() const override { return "Spatial"; }
        int getLatencySamples() const override { return 0; } // Algorithm dependent

        //==============================================================================
        // Reverb-Specific Interface
        
        /** Set reverb algorithm */
        void setAlgorithm(Algorithm algorithm);
        Algorithm getAlgorithm() const { return currentAlgorithm; }
        
        /** Load impulse response for convolution reverb */
        bool loadImpulseResponse(const juce::AudioBuffer<float>& impulseResponse);
        
        /** Set pre-delay in milliseconds */
        void setPreDelay(float preDelayMs);
        
        /** Set reverb time (decay time) */
        void setReverbTime(float timeSeconds);
        
        /** Set room size (0.0 to 1.0) */
        void setRoomSize(float size);
        
        /** Set damping/brightness */
        void setDamping(float damping);
        
        /** Set stereo width */
        void setStereoWidth(float width);
        
        /** Set early reflections mix */
        void setEarlyReflectionsMix(float mix);

    private:
        //==============================================================================
        // Internal Processing Components
        
        /** Multi-tap delay line for early reflections */
        class EarlyReflections
        {
        public:
            void prepareToPlay(double sampleRate, int samplesPerBlock);
            void reset();
            void processBlock(juce::AudioBuffer<float>& buffer);
            
            void setRoomSize(float size);
            void setMix(float mix);
            
        private:
            static constexpr int NUM_TAPS = 8;
            
            struct DelayTap
            {
                std::vector<float> delayLine;
                int delayLength = 0;
                int writeIndex = 0;
                float gain = 0.0f;
            };
            
            std::array<DelayTap, NUM_TAPS> taps;
            float mix = 0.3f;
            double sampleRate = 44100.0;
        };
        
        /** All-pass delay for diffusion */
        class AllPassDelay
        {
        public:
            void prepareToPlay(double sampleRate, int maxDelaySamples);
            void reset();
            float processSample(float input, int delaySamples, float feedback, float gain = 1.0f);
            
        private:
            std::vector<float> delayLine;
            int writeIndex = 0;
        };
        
        /** Comb filter for late reflections */
        class CombFilter
        {
        public:
            void prepareToPlay(double sampleRate, int maxDelaySamples);
            void reset();
            float processSample(float input, int delaySamples, float feedback, float damping);
            
        private:
            std::vector<float> delayLine;
            int writeIndex = 0;
            float lastOutput = 0.0f;
        };
        
        /** Convolution processor for impulse responses */
        class ConvolutionProcessor
        {
        public:
            void prepareToPlay(double sampleRate, int samplesPerBlock);
            void reset();
            void processBlock(juce::AudioBuffer<float>& buffer);
            bool loadImpulseResponse(const juce::AudioBuffer<float>& impulseResponse);
            
        private:
            juce::dsp::Convolution convolution;
            bool impulseLoaded = false;
        };

        //==============================================================================
        // Algorithm Implementations
        
        void processPlateReverb(juce::AudioBuffer<float>& buffer);
        void processHallReverb(juce::AudioBuffer<float>& buffer);
        void processRoomReverb(juce::AudioBuffer<float>& buffer);
        void processSpringReverb(juce::AudioBuffer<float>& buffer);
        void processConvolutionReverb(juce::AudioBuffer<float>& buffer);
        void processShimmerReverb(juce::AudioBuffer<float>& buffer);
        void processReverseReverb(juce::AudioBuffer<float>& buffer);
        void processGatedReverb(juce::AudioBuffer<float>& buffer);

        //==============================================================================
        // State Variables
        
        Algorithm currentAlgorithm = Algorithm::Plate;
        
        // Parameter smoothers
        ParameterSmoother preDelaySmoother;
        ParameterSmoother reverbTimeSmoother;
        ParameterSmoother roomSizeSmoother;
        ParameterSmoother dampingSmoother;
        ParameterSmoother stereoWidthSmoother;
        ParameterSmoother earlyMixSmoother;
        
        // Current parameter values
        float preDelayMs = 20.0f;
        float reverbTimeSeconds = 2.0f;
        float roomSize = 0.5f;
        float damping = 0.3f;
        float stereoWidth = 1.0f;
        float earlyReflectionsMix = 0.3f;
        
        // Processing components
        EarlyReflections earlyReflections;
        ConvolutionProcessor convolutionProcessor;
        
        // Plate reverb components (Dattorro algorithm inspired)
        std::array<AllPassDelay, 4> inputAllPasses;
        std::array<CombFilter, 8> combFilters;
        std::array<AllPassDelay, 4> outputAllPasses;
        
        // Pre-delay
        std::vector<float> preDelayLine;
        int preDelayWriteIndex = 0;
        
        // Temporary buffers
        juce::AudioBuffer<float> tempBuffer;
        juce::AudioBuffer<float> earlyBuffer;
        juce::AudioBuffer<float> lateBuffer;
        
        // Shimmer reverb components
        juce::dsp::ProcessorDuplicator<juce::dsp::FIR::Filter<float>, juce::dsp::FIR::Coefficients<float>> pitchShifter;
        
        double currentSampleRate = 44100.0;
        int currentBlockSize = 512;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ReverbEngine)
    };

} // namespace spawnclone::audio
