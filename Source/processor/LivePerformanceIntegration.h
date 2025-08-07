//==============================================================================
/*
    SpawnClone Live Performance Integration
    
    Integrates LivePerformanceEngine with the main plugin processor,
    handling real-time pattern generation and synthesis parameter modulation
    in a thread-safe manner compatible with JUCE audio callbacks.
*/
//==============================================================================

#pragma once

#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "../ai/LivePerformanceEngine.h"
#include "../audio/AudioPreviewEngine.h"
#include "../GenerationParameters.h"

// Forward declarations
class ONNXDaemonClient;

namespace spawnclone::processor
{

//==============================================================================
/**
 * Live Performance Integration Component
 * 
 * Bridges LivePerformanceEngine with the main plugin processor:
 * - Handles real-time pattern generation requests
 * - Manages AI-driven synthesis parameter updates
 * - Provides thread-safe integration with audio processing
 * - Manages performance state and metrics
 */
class LivePerformanceIntegration : public spawnclone::ai::LivePerformanceEngine::Listener
{
public:
    //==============================================================================
    LivePerformanceIntegration();
    ~LivePerformanceIntegration() override;

    //==============================================================================
    // Initialization
    
    /** Initialize with required components */
    void initialize(AudioPreviewEngine* audioEngine,
                   std::shared_ptr<ONNXDaemonClient> onnxClient);
    
    /** Prepare for audio processing */
    void prepareToPlay(double sampleRate, int blockSize);
    
    /** Release audio resources */
    void releaseResources();

    //==============================================================================
    // Live Performance Control
    
    /** Enable/disable live performance mode */
    void setLivePerformanceEnabled(bool enabled);
    bool isLivePerformanceEnabled() const;
    
    /** Set performance mode */
    void setPerformanceMode(spawnclone::ai::LivePerformanceEngine::PerformanceMode mode);
    
    /** Configure AI parameter modulation */
    void configureAIModulation(bool enabled, float intensity = 0.5f);
    
    /** Set which synthesis parameters should be modulated by AI */
    void setModulationTargets(const spawnclone::ai::LivePerformanceEngine::ModulationTargets& targets);

    //==============================================================================
    // Pattern Generation Control
    
    /** Trigger pattern generation with current generation parameters */
    void triggerPatternGeneration();
    
    /** Trigger pattern generation with specific parameters */
    void triggerPatternGeneration(const GenerationParameters& params);
    
    /** Generate variation of current pattern */
    void triggerPatternVariation(float variationAmount = 0.3f);
    
    /** Start/stop continuous pattern evolution */
    void startPatternEvolution(float evolutionRate = 0.1f);
    void stopPatternEvolution();

    //==============================================================================
    // Performance Triggers
    
    /** Configure automatic triggers */
    void configureTriggers(spawnclone::ai::LivePerformanceEngine::TriggerType type,
                          float probability = 0.1f,
                          int beatDivision = 16);
    
    /** Manual trigger for live performance */
    void manualTrigger();

    //==============================================================================
    // Audio Processing Integration
    
    /** Process MIDI buffer for triggers and performance data */
    void processMIDI(const juce::MidiBuffer& midiBuffer, double transportTime, bool isPlaying);
    
    /** Update live performance state (call from audio thread) */
    void updatePerformanceState(double transportTime, bool isPlaying);

    //==============================================================================
    // Pattern Management
    
    /** Set current generation parameters for live mode */
    void setCurrentGenerationParameters(const GenerationParameters& params);
    
    /** Get current generation parameters */
    const GenerationParameters& getCurrentGenerationParameters() const { return currentGenParams; }
    
    /** Check if a pattern is currently being generated */
    bool isGeneratingPattern() const;

    //==============================================================================
    // Performance Metrics
    
    /** Get performance state */
    const spawnclone::ai::LivePerformanceEngine::PerformanceState& getPerformanceState() const;
    
    /** Get performance metrics as formatted string */
    juce::String getPerformanceMetrics() const;
    
    /** Get current CPU usage for AI processing */
    float getAICpuUsage() const;

    //==============================================================================
    // UI Integration
    
    /** Listener interface for UI updates */
    class Listener
    {
    public:
        virtual ~Listener() = default;
        
        /** Called when a new pattern is generated and applied */
        virtual void livePatternGenerated(std::shared_ptr<MIDIPattern> pattern) {}
        
        /** Called when synthesis parameters are modulated */
        virtual void liveSynthesisParametersChanged(const juce::StringPairArray& changes) {}
        
        /** Called when performance state changes */
        virtual void livePerformanceStateChanged() {}
        
        /** Called on live performance errors */
        virtual void livePerformanceError(const juce::String& error) {}
    };
    
    void addListener(Listener* listener);
    void removeListener(Listener* listener);

    //==============================================================================
    // Advanced Configuration
    
    /** Set evolution settings for pattern morphing */
    void setEvolutionSettings(const spawnclone::ai::LivePerformanceEngine::EvolutionSettings& settings);
    
    /** Get current evolution settings */
    const spawnclone::ai::LivePerformanceEngine::EvolutionSettings& getEvolutionSettings() const;

private:
    //==============================================================================
        // Core components
    std::unique_ptr<spawnclone::ai::LivePerformanceEngine> liveEngine;
    AudioPreviewEngine* audioPreviewEngine;
    std::shared_ptr<ONNXDaemonClient> onnxDaemonClient;
    
    // State Management
    GenerationParameters currentGenParams;
    std::atomic<bool> isLiveEnabled{false};
    std::atomic<bool> isInitialized{false};
    
    // Audio thread integration
    double currentSampleRate = 44100.0;
    int currentBlockSize = 512;
    
    //==============================================================================
    // LivePerformanceEngine::Listener Implementation
    void patternGenerated(std::shared_ptr<MIDIPattern> newPattern) override;
    void synthesisParametersModulated(const juce::StringPairArray& changedParams) override;
    void performanceStateChanged(const spawnclone::ai::LivePerformanceEngine::PerformanceState& state) override;
    void performanceError(const juce::String& errorMessage) override;
    
    //==============================================================================
    // Pattern Application
    void applyGeneratedPattern(std::shared_ptr<MIDIPattern> pattern);
    
    //==============================================================================
    // Listener Management
    juce::ListenerList<Listener> listeners;
    
    void notifyPatternGenerated(std::shared_ptr<MIDIPattern> pattern);
    void notifyParametersChanged(const juce::StringPairArray& changes);
    void notifyStateChanged();
    void notifyError(const juce::String& error);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LivePerformanceIntegration)
};

} // namespace spawnclone::processor
