/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "ParameterManager.h"
#include "PatternManager.h"
#include "ai/ThreadManager.h"
#include "ai/AIGenerationEngine.h"

//==============================================================================
/**
*/
class SpawnCloneAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    SpawnCloneAudioProcessor();
    ~SpawnCloneAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // Getter for the ParameterManager
    ParameterManager& getParameterManager() { return *parameterManager; }
    PatternManager& getPatternManager() { return *patternManager; }
    AIGenerationEngine& getAIGenerationEngine() { return *aiEngine; }


private:
    //==============================================================================
    std::unique_ptr<ParameterManager> parameterManager;
    std::unique_ptr<PatternManager> patternManager;
    
    ThreadManager threadManager; // Manages the AI background thread
    std::unique_ptr<AIGenerationEngine> aiEngine;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpawnCloneAudioProcessor)
};
