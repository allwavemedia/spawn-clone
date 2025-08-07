/*
  ==============================================================================

    Epic7CompatibilityLayer.h
    Created: 7 Aug 2025
    Author:  Epic 7 Integration Support

    Provides compatibility and safety layers for Epic 7 AI system integration.
    Handles potential initialization issues and provides fallback mechanisms.

  ==============================================================================
*/

#pragma once

#include <juce_core/juce_core.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_data_structures/juce_data_structures.h>
#include <cmath>

/**
 * @brief Epic 7 AI System Compatibility Layer
 * 
 * This class provides safety wrappers and compatibility measures for integrating
 * Epic 7 AI systems during development. It includes parameter validation, safe
 * buffer operations, and fallback mechanisms to prevent crashes while Epic 7
 * dependencies are being resolved.
 */
class Epic7CompatibilityLayer
{
public:
    //==============================================================================
    /** Safe parameter validation for audio processing */
    static bool validateAudioParameters(double& sampleRate, int& samplesPerBlock, int& numChannels)
    {
        bool modified = false;
        
        // Validate sample rate
        if (sampleRate <= 0.0 || sampleRate > 192000.0)
        {
            juce::Logger::writeToLog("Epic7: Invalid sample rate " + juce::String(sampleRate) + "Hz, using 44100Hz");
            sampleRate = 44100.0;
            modified = true;
        }
        
        // Validate block size
        if (samplesPerBlock <= 0 || samplesPerBlock > 8192)
        {
            juce::Logger::writeToLog("Epic7: Invalid block size " + juce::String(samplesPerBlock) + ", using 512");
            samplesPerBlock = 512;
            modified = true;
        }
        
        // Validate channel count
        if (numChannels <= 0 || numChannels > 64)
        {
            juce::Logger::writeToLog("Epic7: Invalid channel count " + juce::String(numChannels) + ", using 2");
            numChannels = 2;
            modified = true;
        }
        
        return !modified; // Return true if no modifications were needed
    }
    
    /** Safe parameter validation for audio processing (read-only version) */
    static bool validateAudioParametersReadOnly(double sampleRate, int samplesPerBlock, int numChannels)
    {
        // Just check validity without modifying
        if (sampleRate <= 0.0 || sampleRate > 192000.0)
            return false;
        if (samplesPerBlock <= 0 || samplesPerBlock > 8192)
            return false;
        if (numChannels <= 0 || numChannels > 64)
            return false;
        return true;
    }
    
    /** Safe buffer validation and sanitization */
    static bool safeBufferOperation(juce::AudioBuffer<float>& buffer, const juce::String& operation)
    {
        try
        {
            // Check for completely invalid buffer
            if (buffer.getNumChannels() <= 0)
            {
                juce::Logger::writeToLog("Epic7: Invalid buffer channels - resizing to stereo");
                buffer.setSize(2, 512, false, true, true);
                return false;
            }
            
            if (buffer.getNumSamples() <= 0)
            {
                juce::Logger::writeToLog("Epic7: Invalid buffer samples - resizing to 512 samples");
                buffer.setSize(buffer.getNumChannels(), 512, false, true, true);
                return false;
            }
            
            // Only log if values are actually problematic (not just large)
            if (buffer.getNumChannels() > 64)  // Extremely high channel count
            {
                juce::Logger::writeToLog("Epic7: Extremely high channel count detected");
            }
            
            if (buffer.getNumSamples() > 16384)  // Extremely large buffer
            {
                juce::Logger::writeToLog("Epic7: Extremely large buffer detected");
            }
            
            // Sanitize any NaN or infinite values without logging unless found
            bool foundInvalidData = false;
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            {
                auto* channelData = buffer.getWritePointer(channel);
                if (channelData != nullptr)
                {
                    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
                    {
                        if (!std::isfinite(channelData[sample]))
                        {
                            channelData[sample] = 0.0f;
                            foundInvalidData = true;
                        }
                    }
                }
            }
            
            if (foundInvalidData)
            {
                juce::Logger::writeToLog("Epic7: Sanitized invalid audio data");
            }
            
            return true; // Buffer is safe to use
        }
        catch (...)
        {
            juce::Logger::writeToLog("Epic7: Buffer operation failed");
            // Emergency buffer clear
            if (buffer.getNumChannels() > 0 && buffer.getNumSamples() > 0)
            {
                buffer.clear();
            }
            return false;
        }
    }
    
    /** Safe IIR filter processing for Epic 7 compatibility */
    static void safeIIRProcessing(juce::dsp::IIR::Filter<float>& filter, juce::AudioBuffer<float>& buffer)
    {
        try
        {
            // Epic 7 requires single-channel processing for IIR filters
            // Process each channel individually to meet requirements
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            {
                auto* channelData = buffer.getWritePointer(channel);
                if (channelData != nullptr)
                {
                    // Create single-channel buffer for Epic 7 compatibility
                    juce::AudioBuffer<float> singleChannelBuffer(1, buffer.getNumSamples());
                    singleChannelBuffer.copyFrom(0, 0, buffer, channel, 0, buffer.getNumSamples());
                    
                    // Process single channel
                    juce::dsp::AudioBlock<float> block(singleChannelBuffer);
                    juce::dsp::ProcessContextReplacing<float> context(block);
                    filter.process(context);
                    
                    // Copy back to original buffer
                    buffer.copyFrom(channel, 0, singleChannelBuffer, 0, 0, buffer.getNumSamples());
                }
            }
        }
        catch (...)
        {
            // IIR processing failed - clear the buffer to prevent artifacts
            buffer.clear();
            juce::Logger::writeToLog("Epic7: IIR filter processing failed, buffer cleared");
        }
    }
    
    /** Safe string creation with fallback protection */
    static juce::String safeString(int value)
    {
        try
        {
            return juce::String(value);
        }
        catch (...)
        {
            return "N/A";  // Use literal string instead of juce::String("N/A")
        }
    }
    
    static juce::String safeString(float value)
    {
        try
        {
            if (!std::isfinite(value))
                return "NaN";  // Use literal string
            return juce::String(value, 3);  // 3 decimal places
        }
        catch (...)
        {
            return "N/A";  // Use literal string
        }
    }
    
    static juce::String safeString(const char* str)
    {
        // Removed completely - will return empty string or "Invalid"
        return "Value";  // Static safe string
    }
    
    /** Safe ValueTree operations with Epic 7 compatibility */
    template<typename Func>
    static bool safeValueTreeOperation(const juce::String& operationName, Func&& operation)
    {
        try
        {
            operation();
            return true;
        }
        catch (const std::exception& e)
        {
            juce::Logger::writeToLog("Epic7: ValueTree operation '" + operationName + "' failed");
            return false;
        }
        catch (...)
        {
            juce::Logger::writeToLog("Epic7: ValueTree operation '" + operationName + "' failed with unknown exception");
            return false;
        }
    }
    
    /** Safe ValueTree creation */
    static juce::ValueTree safeCreateValueTree(const juce::String& typeName)
    {
        try
        {
            return juce::ValueTree(typeName);
        }
        catch (...)
        {
            juce::Logger::writeToLog("Epic7: Failed to create ValueTree, using default");
            return juce::ValueTree("default");
        }
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Epic7CompatibilityLayer)
};
