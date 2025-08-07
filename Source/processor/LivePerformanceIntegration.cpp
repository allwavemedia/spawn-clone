//==============================================================================
/*
    SpawnClone Live Performance Integration Implementation
    
    Bridges LivePerformanceEngine with main plugin processor for real-time
    AI-driven pattern generation and synthesis control.
*/
//==============================================================================

#include "LivePerformanceIntegration.h"

namespace spawnclone::processor
{

//==============================================================================
LivePerformanceIntegration::LivePerformanceIntegration()
{
    liveEngine = std::make_unique<spawnclone::ai::LivePerformanceEngine>();
    liveEngine->addListener(this);
}

LivePerformanceIntegration::~LivePerformanceIntegration()
{
    if (liveEngine)
    {
        liveEngine->removeListener(this);
        liveEngine->setPerformanceMode(spawnclone::ai::LivePerformanceEngine::PerformanceMode::Disabled);
    }
    
    releaseResources();
}

//==============================================================================
// Initialization

bool LivePerformanceIntegration::initialize(AudioPreviewEngine* audioEngine,
                                          std::shared_ptr<spawnclone::ai::ONNXDaemonClient> onnxClient)
{
    audioPreviewEngine = audioEngine;
    onnxDaemonClient = onnxClient;
    
    if (!audioEngine || !onnxClient)
    {
        notifyError("LivePerformanceIntegration: Invalid components provided");
        return false;
    }
    
    // Configure live engine
    liveEngine->setONNXDaemonClient(onnxClient);
    
    // Get advanced synthesis engine from audio preview engine
    auto* synthesisEngine = audioEngine->getAdvancedSynthesisEngine();
    if (!synthesisEngine)
    {
        notifyError("LivePerformanceIntegration: Advanced Synthesis Engine not available");
        return false;
    }
    
    liveEngine->setAdvancedSynthesisEngine(synthesisEngine);
    
    // Initialize the live engine
    if (!liveEngine->initialize())
    {
        notifyError("LivePerformanceIntegration: Failed to initialize live engine");
        return false;
    }
    
    isInitialized.store(true);
    return true;
}

void LivePerformanceIntegration::prepareToPlay(double sampleRate, int blockSize)
{
    currentSampleRate = sampleRate;
    currentBlockSize = blockSize;
    
    if (liveEngine)
    {
        liveEngine->prepareForPerformance(sampleRate, blockSize);
    }
}

void LivePerformanceIntegration::releaseResources()
{
    if (liveEngine)
    {
        liveEngine->releaseResources();
    }
    
    isLiveEnabled.store(false);
}

//==============================================================================
// Live Performance Control

void LivePerformanceIntegration::setLivePerformanceEnabled(bool enabled)
{
    if (!isInitialized.load())
        return;
    
    isLiveEnabled.store(enabled);
    
    if (enabled)
    {
        // Start with pattern generation mode
        liveEngine->setPerformanceMode(spawnclone::ai::LivePerformanceEngine::PerformanceMode::PatternGeneration);
    }
    else
    {
        liveEngine->setPerformanceMode(spawnclone::ai::LivePerformanceEngine::PerformanceMode::Disabled);
    }
    
    notifyStateChanged();
}

bool LivePerformanceIntegration::isLivePerformanceEnabled() const
{
    return isLiveEnabled.load() && isInitialized.load();
}

void LivePerformanceIntegration::setPerformanceMode(spawnclone::ai::LivePerformanceEngine::PerformanceMode mode)
{
    if (!isInitialized.load())
        return;
    
    liveEngine->setPerformanceMode(mode);
    
    // Update enabled state based on mode
    isLiveEnabled.store(mode != spawnclone::ai::LivePerformanceEngine::PerformanceMode::Disabled);
}

void LivePerformanceIntegration::configureAIModulation(bool enabled, float intensity)
{
    if (!liveEngine)
        return;
    
    liveEngine->setAIParameterModulationEnabled(enabled);
    liveEngine->setModulationIntensity(intensity);
}

void LivePerformanceIntegration::setModulationTargets(const spawnclone::ai::LivePerformanceEngine::ModulationTargets& targets)
{
    if (liveEngine)
    {
        liveEngine->setModulationTargets(targets);
    }
}

//==============================================================================
// Pattern Generation Control

void LivePerformanceIntegration::triggerPatternGeneration()
{
    if (!isLivePerformanceEnabled())
        return;
    
    liveEngine->triggerPatternGeneration(currentGenParams);
}

void LivePerformanceIntegration::triggerPatternGeneration(const GenerationParameters& params)
{
    if (!isLivePerformanceEnabled())
        return;
    
    currentGenParams = params;
    liveEngine->triggerPatternGeneration(params);
}

void LivePerformanceIntegration::triggerPatternVariation(float variationAmount)
{
    if (!isLivePerformanceEnabled())
        return;
    
    liveEngine->triggerPatternVariation(variationAmount);
}

void LivePerformanceIntegration::startPatternEvolution(float evolutionRate)
{
    if (!isLivePerformanceEnabled())
        return;
    
    liveEngine->startPatternEvolution(evolutionRate);
}

void LivePerformanceIntegration::stopPatternEvolution()
{
    if (liveEngine)
    {
        liveEngine->stopPatternEvolution();
    }
}

//==============================================================================
// Performance Triggers

void LivePerformanceIntegration::configureTriggers(spawnclone::ai::LivePerformanceEngine::TriggerType type,
                                                  float probability,
                                                  int beatDivision)
{
    if (liveEngine)
    {
        liveEngine->configureTriggers(type, probability, beatDivision);
    }
}

void LivePerformanceIntegration::manualTrigger()
{
    if (!isLivePerformanceEnabled())
        return;
    
    liveEngine->manualTrigger();
}

//==============================================================================
// Audio Processing Integration

void LivePerformanceIntegration::processMIDI(const juce::MidiBuffer& midiBuffer, 
                                            double transportTime, bool isPlaying)
{
    if (!isLivePerformanceEnabled())
        return;
    
    // Process MIDI triggers
    liveEngine->processMIDITriggers(midiBuffer);
    
    // Update performance state
    updatePerformanceState(transportTime, isPlaying);
}

void LivePerformanceIntegration::updatePerformanceState(double transportTime, bool isPlaying)
{
    if (!isLivePerformanceEnabled())
        return;
    
    // This could be expanded to handle tempo changes, beat detection, etc.
    // For now, just ensure the live engine knows about transport state
}

//==============================================================================
// Pattern Management

void LivePerformanceIntegration::setCurrentGenerationParameters(const GenerationParameters& params)
{
    currentGenParams = params;
}

bool LivePerformanceIntegration::isGeneratingPattern() const
{
    if (!liveEngine)
        return false;
    
    return liveEngine->getPerformanceState().isGenerating;
}

//==============================================================================
// Performance Metrics

const spawnclone::ai::LivePerformanceEngine::PerformanceState& 
LivePerformanceIntegration::getPerformanceState() const
{
    static spawnclone::ai::LivePerformanceEngine::PerformanceState defaultState;
    return liveEngine ? liveEngine->getPerformanceState() : defaultState;
}

juce::String LivePerformanceIntegration::getPerformanceMetrics() const
{
    if (!liveEngine)
        return "Live Performance Engine not available";
    
    return liveEngine->getPerformanceMetrics();
}

float LivePerformanceIntegration::getAICpuUsage() const
{
    if (!liveEngine)
        return 0.0f;
    
    return liveEngine->getPerformanceState().aiCpuUsage;
}

//==============================================================================
// UI Integration

void LivePerformanceIntegration::addListener(Listener* listener)
{
    listeners.add(listener);
}

void LivePerformanceIntegration::removeListener(Listener* listener)
{
    listeners.remove(listener);
}

//==============================================================================
// Advanced Configuration

void LivePerformanceIntegration::setEvolutionSettings(const spawnclone::ai::LivePerformanceEngine::EvolutionSettings& settings)
{
    if (liveEngine)
    {
        liveEngine->setEvolutionSettings(settings);
    }
}

const spawnclone::ai::LivePerformanceEngine::EvolutionSettings& 
LivePerformanceIntegration::getEvolutionSettings() const
{
    static spawnclone::ai::LivePerformanceEngine::EvolutionSettings defaultSettings;
    return liveEngine ? liveEngine->getEvolutionSettings() : defaultSettings;
}

//==============================================================================
// LivePerformanceEngine::Listener Implementation

void LivePerformanceIntegration::patternGenerated(std::shared_ptr<MIDIPattern> newPattern)
{
    if (!newPattern || !audioPreviewEngine)
        return;
    
    // Apply the new pattern to the audio preview engine
    applyGeneratedPattern(newPattern);
    
    // Notify UI listeners
    notifyPatternGenerated(newPattern);
}

void LivePerformanceIntegration::synthesisParametersModulated(const juce::StringPairArray& changedParams)
{
    // Parameters are already applied to the synthesis engine by LivePerformanceEngine
    // Just notify UI listeners for display updates
    notifyParametersChanged(changedParams);
}

void LivePerformanceIntegration::performanceStateChanged(const spawnclone::ai::LivePerformanceEngine::PerformanceState& state)
{
    // Notify UI of state changes
    notifyStateChanged();
}

void LivePerformanceIntegration::performanceError(const juce::String& errorMessage)
{
    notifyError("Live Performance: " + errorMessage);
}

//==============================================================================
// Pattern Application

void LivePerformanceIntegration::applyGeneratedPattern(std::shared_ptr<MIDIPattern> pattern)
{
    if (!audioPreviewEngine || !pattern)
        return;
    
    // Set the new pattern in the audio preview engine
    audioPreviewEngine->setCurrentPattern(pattern);
    
    // Auto-start playback if enabled (this could be configurable)
    if (isLivePerformanceEnabled())
    {
        audioPreviewEngine->startPlayback();
    }
}

//==============================================================================
// Notification Methods

void LivePerformanceIntegration::notifyPatternGenerated(std::shared_ptr<MIDIPattern> pattern)
{
    listeners.call([pattern](Listener& l) { l.livePatternGenerated(pattern); });
}

void LivePerformanceIntegration::notifyParametersChanged(const juce::StringPairArray& changes)
{
    listeners.call([&changes](Listener& l) { l.liveSynthesisParametersChanged(changes); });
}

void LivePerformanceIntegration::notifyStateChanged()
{
    listeners.call([](Listener& l) { l.livePerformanceStateChanged(); });
}

void LivePerformanceIntegration::notifyError(const juce::String& error)
{
    listeners.call([&error](Listener& l) { l.livePerformanceError(error); });
}

} // namespace spawnclone::processor
