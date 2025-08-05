//==============================================================================
/*
  EffectProcessor.cpp
  Author: BMad Dev Agent - Epic 6 Story 6.1
  
  Implementation of base EffectProcessor class functionality
*/
//==============================================================================

#include "EffectProcessor.h"

namespace spawnclone::audio
{
    void EffectProcessor::applyWetDryMix(juce::AudioBuffer<float>& wetBuffer, 
                                        const juce::AudioBuffer<float>& dryBuffer)
    {
        if (wetDryMix >= 0.999f)
        {
            // Fully wet - no mixing needed
            return;
        }
        
        if (wetDryMix <= 0.001f)
        {
            // Fully dry - copy dry signal
            wetBuffer.makeCopyOf(dryBuffer);
            return;
        }
        
        // Mix wet and dry signals
        float wetGain = wetDryMix;
        float dryGain = 1.0f - wetDryMix;
        
        for (int channel = 0; channel < wetBuffer.getNumChannels(); ++channel)
        {
            auto* wetData = wetBuffer.getWritePointer(channel);
            const auto* dryData = dryBuffer.getReadPointer(channel);
            
            for (int sample = 0; sample < wetBuffer.getNumSamples(); ++sample)
            {
                wetData[sample] = wetData[sample] * wetGain + dryData[sample] * dryGain;
            }
        }
    }

    void EffectProcessor::startCPUMonitoring()
    {
        if (monitorCPU)
        {
            processingStartTime = juce::Time::getHighResolutionTicks();
        }
    }

    void EffectProcessor::endCPUMonitoring()
    {
        if (monitorCPU && processingStartTime > 0)
        {
            auto endTime = juce::Time::getHighResolutionTicks();
            auto elapsedTime = juce::Time::highResolutionTicksToSeconds(endTime - processingStartTime);
            
            // Update rolling average
            averageCPUTime = (averageCPUTime * cpuSampleCount + elapsedTime) / (cpuSampleCount + 1);
            cpuSampleCount = juce::jmin(cpuSampleCount + 1, 100); // Keep last 100 samples
            
            // Convert to percentage (approximate)
            // This is a rough estimate - in real scenarios you'd want more sophisticated CPU monitoring
            cpuUsage = (averageCPUTime / 0.001) * 100.0; // Assume 1ms = 100% CPU for this buffer
            
            processingStartTime = 0;
        }
    }

} // namespace spawnclone::audio
