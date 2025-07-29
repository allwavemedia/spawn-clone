/*
  ==============================================================================

    ParameterManager.h
    Created: 28 Jul 2025
    Author:  Developer Agent

    Thread-safe parameter management using JUCE AudioProcessorValueTreeState.

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>
#include "GenerationParameters.h"

//==============================================================================
/**
 * Provides a thread-safe interface to all plugin parameters.
 * Uses atomic values to allow the real-time audio thread to access parameter states without locking.
 */
class ParameterManager : public juce::AudioProcessorValueTreeState::Listener
{
public:
    //==============================================================================
    ParameterManager(juce::AudioProcessor& processor);
    ~ParameterManager() = default;

    //==============================================================================
    /** Returns the current GenerationParameters safely for any thread */
    GenerationParameters getCurrentParameters() const;
    
    /** Gets the AudioProcessorValueTreeState for UI binding */
    juce::AudioProcessorValueTreeState& getValueTreeState() { return valueTreeState; }
    juce::AudioProcessorValueTreeState& getState();
    
    /** Helper methods for UI population */
    const juce::StringArray getKeySignatureItems() const;
    const juce::StringArray getScaleTypeItems() const;
    const juce::StringArray getGenerationTypeItems() const;
    
    /** Parameter IDs - public for UI access */
    static const juce::String KEY_PARAM_ID;
    static const juce::String SCALE_PARAM_ID;
    static const juce::String TEMPO_PARAM_ID;
    static const juce::String COMPLEXITY_PARAM_ID;
    static const juce::String GENERATION_TYPE_PARAM_ID;
    static const juce::String PATTERN_LENGTH_PARAM_ID;

private:
    //==============================================================================
    /** Creates the parameter layout for AudioProcessorValueTreeState */
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    /** Called when any parameter changes - implements AudioProcessorValueTreeState::Listener */
    void parameterChanged(const juce::String& parameterID, float newValue) override;
    
    //==============================================================================
    juce::AudioProcessorValueTreeState valueTreeState;
    
    // Atomic storage for thread-safe parameter access
    std::atomic<int> atomicKey { 0 };
    std::atomic<int> atomicScale { 0 };
    std::atomic<float> atomicTempo { 120.0f };
    std::atomic<float> atomicComplexity { 0.5f };
    std::atomic<int> atomicGenerationType { 0 };
    std::atomic<float> atomicPatternLength { 16.0f };
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ParameterManager)
};
