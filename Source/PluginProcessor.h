/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "ParameterManager.h"
#include "PatternManager.h"
#include "PluginState.h"
#include "ai/ThreadManager.h"
#include "ai/AIGenerationEngine.h"
#include "audio/AudioPreviewEngine.h"
#include "audio/MultiOutputManager.h"
#include "audio/InstrumentLibraryManager.h"
#include "processor/LivePerformanceIntegration.h"
#include "ai/LivePerformanceEngine.h"  // For PerformanceMode enum

//==============================================================================
/**
*/
class SpawnCloneAudioProcessor  : public juce::AudioProcessor,
                                  public juce::ChangeBroadcaster
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
    
    //==============================================================================
    // NEW FEATURE METHODS
    
    /** Get the current plugin state */
    PluginState& getPluginState() { return pluginState; }
    const PluginState& getPluginState() const { return pluginState; }
    
    /** Pattern history navigation */
    bool navigateToPreviousPattern();
    bool navigateToNextPattern();
    
    /** Check if pattern history navigation is possible */
    bool canNavigateBackward() const { return pluginState.canNavigateBackward(); }
    bool canNavigateForward() const { return pluginState.canNavigateForward(); }
    
    /** Trigger pattern generation with current parameters */
    void generateNewPattern();
    
    //==============================================================================
    // Synthesis Parameter Control (Epic 4)
    void updateSynthesisParameter(const juce::String& paramName, float value, bool interpolate);
    void batchUpdateSynthesisParameters(const juce::StringPairArray& newParams);
    void applyAIGenerationParameters(const GenerationParameters& aiParams);
    float getSynthesisParameterValue(const juce::String& paramName) const;

    //==============================================================================
    // Epic 2 Story 2.1: Host DAW Communication (Task 2.1.1-2.1.4)
    
    /** Get current host transport state and tempo */
    struct HostTransportInfo
    {
        double tempo = 120.0;
        int timeSigNumerator = 4;
        int timeSigDenominator = 4;
        bool isPlaying = false;
        bool isRecording = false;
        double ppqPosition = 0.0;
        bool hostTempoAvailable = false;
        bool hostTimeSigAvailable = false;
    };
    
    HostTransportInfo getHostTransportInfo() const;
    bool isHostTempoAvailable() const { return lastHostInfo.hostTempoAvailable; }
    double getHostTempo() const { return lastHostInfo.tempo; }
    
    /** Update UI with host information */
    void updateHostInfo();
    
    //==============================================================================
    //==============================================================================
    // Epic 2 Story 2.2: Audio Preview Engine Methods
    
    /** Preview the current pattern through audio engine */
    void previewCurrentPattern();
    
    /** Stop audio preview playback */
    void stopAudioPreview();
    
    /** Get audio preview engine for UI integration */
    AudioPreviewEngine* getAudioPreviewEngine() { return audioPreviewEngine.get(); }
    
    /** Get instrument library manager for preset management */
    InstrumentLibraryManager* getInstrumentLibraryManager() { return instrumentLibraryManager.get(); }
    
    //==============================================================================
    // Epic 8 Story 8.3: Multi-Output Routing Methods
    
    /** Get multi-output manager for layer routing control */
    MultiOutputManager* getMultiOutputManager() { return multiOutputManager.get(); }
    
    //==============================================================================
    // Live Performance System Integration
    
    /** Enable/disable live performance mode */
    void setLivePerformanceEnabled(bool enabled);
    
    /** Check if live performance mode is enabled */
    bool isLivePerformanceEnabled() const;
    
    /** Set live performance mode (Pattern Generation, Parameter Modulation, Evolution, Full Live) */
    void setLivePerformanceMode(spawnclone::ai::LivePerformanceEngine::PerformanceMode mode);
    
    /** Configure AI-driven parameter modulation */
    void configureLiveAIModulation(bool enabled, float intensity = 0.5f);
    
    /** Trigger manual pattern generation in live performance mode */
    void triggerLivePatternGeneration();
    
    /** Get live performance integration for advanced control */
    spawnclone::processor::LivePerformanceIntegration* getLivePerformanceIntegration() { return livePerformanceIntegration.get(); }


private:
    //==============================================================================
    std::unique_ptr<ParameterManager> parameterManager;
    std::unique_ptr<PatternManager> patternManager;
    
    // NEW: Central plugin state
    PluginState pluginState;
    
    ThreadManager threadManager; // Manages the AI background thread
    std::unique_ptr<AIGenerationEngine> aiEngine;
    
    // Epic 2 Story 2.1: Host communication state
    mutable HostTransportInfo lastHostInfo;
    
    // Epic 2 Story 2.2: Audio preview engine
    std::unique_ptr<AudioPreviewEngine> audioPreviewEngine;
    
    // Epic 8 Story 8.3: Multi-output routing manager
    std::unique_ptr<MultiOutputManager> multiOutputManager;
    
    // Epic 9 Story 9.1: Instrument library for preset management
    std::unique_ptr<InstrumentLibraryManager> instrumentLibraryManager;
    
    // Live Performance System Integration
    std::unique_ptr<spawnclone::processor::LivePerformanceIntegration> livePerformanceIntegration;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpawnCloneAudioProcessor)
};
