//==============================================================================
/*
  EffectProcessor.h
  Author: BMad Dev Agent - Epic 6 Story 6.1
  
  Base class for modular effects processing system with professional features:
  - Real-time parameter interpolation
  - CPU usage monitoring
  - Bypass functionality
  - Wet/dry mixing
*/
//==============================================================================

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

namespace spawnclone::audio
{
    /**
     * Abstract base class for all audio effects processors
     * Provides consistent interface for effects chain management
     */
    class EffectProcessor
    {
    public:
        EffectProcessor() = default;
        virtual ~EffectProcessor() = default;

        //==============================================================================
        // Core Processing Interface
        
        /** Prepare the effect for processing */
        virtual void prepareToPlay(double sampleRate, int samplesPerBlock) = 0;
        
        /** Release any resources */
        virtual void releaseResources() = 0;
        
        /** Process audio block */
        virtual void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) = 0;
        
        /** Reset internal state (for note-off scenarios) */
        virtual void reset() = 0;

        //==============================================================================
        // Effect Control Interface
        
        /** Set parameter value with optional interpolation */
        virtual void setParameter(const juce::String& parameterName, float value, bool useInterpolation = true) = 0;
        
        /** Get current parameter value */
        virtual float getParameter(const juce::String& parameterName) const = 0;
        
        /** Get list of available parameters */
        virtual juce::StringArray getParameterNames() const = 0;

        //==============================================================================
        // Effect Properties
        
        /** Get effect name for UI display */
        virtual juce::String getEffectName() const = 0;
        
        /** Get effect category for organization */
        virtual juce::String getEffectCategory() const = 0;
        
        /** Check if effect introduces latency */
        virtual int getLatencySamples() const { return 0; }

        //==============================================================================
        // Bypass and Mixing Controls
        
        /** Enable/disable effect bypass */
        void setBypass(bool shouldBypass) 
        { 
            bypassed = shouldBypass; 
        }
        
        bool isBypassed() const { return bypassed; }
        
        /** Set wet/dry mix (0.0 = fully dry, 1.0 = fully wet) */
        void setWetDryMix(float mix) 
        { 
            wetDryMix = juce::jlimit(0.0f, 1.0f, mix); 
        }
        
        float getWetDryMix() const { return wetDryMix; }

        //==============================================================================
        // Performance Monitoring
        
        /** Get current CPU usage percentage */
        double getCPUUsage() const { return cpuUsage; }
        
        /** Enable/disable CPU monitoring */
        void setCPUMonitoringEnabled(bool enabled) { monitorCPU = enabled; }

    protected:
        //==============================================================================
        // Helper Functions for Derived Classes
        
        /** Apply wet/dry mixing to processed buffer */
        void applyWetDryMix(juce::AudioBuffer<float>& wetBuffer, 
                           const juce::AudioBuffer<float>& dryBuffer);
        
        /** Smooth parameter interpolation */
        class ParameterSmoother
        {
        public:
            void setTargetValue(float target);
            void setSampleRate(double sampleRate);
            void setRampLength(double rampLengthSeconds);
            float getNextValue();
            void reset(float value = 0.0f);
            
        private:
            float currentValue = 0.0f;
            float targetValue = 0.0f;
            float stepSize = 0.0f;
            int remainingSteps = 0;
            double sampleRate = 44100.0;
            double rampLength = 0.05; // 50ms default
        };
        
        /** Start CPU monitoring for this processing block */
        void startCPUMonitoring();
        
        /** End CPU monitoring and update statistics */
        void endCPUMonitoring();

    private:
        //==============================================================================
        // Internal State
        
        bool bypassed = false;
        float wetDryMix = 1.0f;        // Default to fully wet
        bool monitorCPU = false;
        double cpuUsage = 0.0;
        
        // Performance monitoring
        juce::int64 processingStartTime = 0;
        double averageCPUTime = 0.0;
        int cpuSampleCount = 0;
        
        // Temporary buffer for wet/dry mixing
        juce::AudioBuffer<float> dryBuffer;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EffectProcessor)
    };

    //==============================================================================
    // Inline Implementation of ParameterSmoother
    
    inline void EffectProcessor::ParameterSmoother::setTargetValue(float target)
    {
        if (target != targetValue)
        {
            targetValue = target;
            auto rampLengthSamples = static_cast<int>(rampLength * sampleRate);
            stepSize = (targetValue - currentValue) / static_cast<float>(rampLengthSamples);
            remainingSteps = rampLengthSamples;
        }
    }
    
    inline void EffectProcessor::ParameterSmoother::setSampleRate(double sr)
    {
        sampleRate = sr;
    }
    
    inline void EffectProcessor::ParameterSmoother::setRampLength(double rampLengthSeconds)
    {
        rampLength = rampLengthSeconds;
    }
    
    inline float EffectProcessor::ParameterSmoother::getNextValue()
    {
        if (remainingSteps > 0)
        {
            currentValue += stepSize;
            --remainingSteps;
        }
        else
        {
            currentValue = targetValue;
        }
        return currentValue;
    }
    
    inline void EffectProcessor::ParameterSmoother::reset(float value)
    {
        currentValue = targetValue = value;
        remainingSteps = 0;
    }

} // namespace spawnclone::audio
