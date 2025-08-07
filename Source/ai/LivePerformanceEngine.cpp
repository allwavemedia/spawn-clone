//==============================================================================
/*
    SpawnClone Live Performance Engine Implementation
    
    Real-time AI-driven live performance system integrating ONNX daemon
    with AdvancedSynthesisEngine for live pattern generation and synthesis control.
*/
//==============================================================================

#include "LivePerformanceEngine.h"
#include <random>
#include <algorithm>

namespace spawnclone::ai
{

//==============================================================================
LivePerformanceEngine::LivePerformanceEngine()
{
    // Initialize parameter mapper
    parameterMapper = std::make_unique<spawnclone::audio::SynthesisParameterMapper>();
    
    // Set default modulation targets
    modTargets.filterCutoff = true;
    modTargets.filterResonance = true;
    modTargets.wavetablePosition = true;
    modTargets.lfoDepth = true;
    modTargets.effectsParams = true;
    
    // Start at 30 FPS for smooth modulation
    startTimer(33); // ~30 FPS
}

LivePerformanceEngine::~LivePerformanceEngine()
{
    stopTimer();
    releaseResources();
}

//==============================================================================
// Engine Lifecycle

bool LivePerformanceEngine::initialize()
{
    // Verify required components
    if (!onnxClient)
    {
        notifyError("ONNX Daemon Client not set");
        return false;
    }
    
    if (!synthesisEngine)
    {
        notifyError("Advanced Synthesis Engine not set");
        return false;
    }
    
    // Initialize performance state
    performanceState = {};
    performanceState.performanceTime = 0.0;
    performanceState.avgGenerationTime = 0.0f;
    
    return true;
}

void LivePerformanceEngine::setONNXDaemonClient(std::shared_ptr<ONNXDaemonClient> client)
{
    onnxClient = client;
}

void LivePerformanceEngine::setAdvancedSynthesisEngine(spawnclone::audio::AdvancedSynthesisEngine* engine)
{
    synthesisEngine = engine;
}

void LivePerformanceEngine::prepareForPerformance(double newSampleRate, int newBlockSize)
{
    sampleRate = newSampleRate;
    blockSize = newBlockSize;
    
    // Clear any pending operations
    {
        juce::ScopedLock lock(queueLock);
        while (!generationQueue.empty())
            generationQueue.pop();
    }
    
    // Reset performance metrics
    performanceState.generatedPatterns = 0;
    performanceState.evolutionCycles = 0;
    performanceState.performanceTime = 0.0;
}

void LivePerformanceEngine::releaseResources()
{
    stopPatternEvolution();
    setPerformanceMode(PerformanceMode::Disabled);
    
    // Clear generation queue
    {
        juce::ScopedLock lock(queueLock);
        while (!generationQueue.empty())
            generationQueue.pop();
    }
    
    // Clear modulations
    {
        juce::ScopedLock lock(modulationLock);
        activeModulations.clear();
    }
}

//==============================================================================
// Performance Modes

void LivePerformanceEngine::setPerformanceMode(PerformanceMode mode)
{
    currentMode.store(mode);
    
    if (mode == PerformanceMode::Disabled)
    {
        stopPatternEvolution();
        setAIParameterModulationEnabled(false);
    }
    
    notifyStateChanged();
}

//==============================================================================
// Real-Time Pattern Generation

void LivePerformanceEngine::triggerPatternGeneration(const GenerationParameters& params)
{
    if (!onnxClient || currentMode.load() == PerformanceMode::Disabled)
        return;
    
    GenerationRequest request;
    request.params = params;
    request.requestTime = juce::Time::getCurrentTime();
    request.type = GenerationRequest::NewPattern;
    
    {
        juce::ScopedLock lock(queueLock);
        generationQueue.push(request);
        performanceState.queuedRequests = static_cast<int>(generationQueue.size());
    }
    
    // Process queue asynchronously if not already processing
    if (!isProcessingGeneration.load())
    {
        juce::Thread::launch([this]() { processGenerationQueue(); });
    }
}

void LivePerformanceEngine::triggerPatternVariation(float variationAmount)
{
    if (!onnxClient || !performanceState.currentPattern)
        return;
    
    GenerationRequest request;
    request.params = performanceState.lastGenerationParams;
    request.requestTime = juce::Time::getCurrentTime();
    request.type = GenerationRequest::Variation;
    request.variationAmount = juce::jlimit(0.1f, 1.0f, variationAmount);
    
    {
        juce::ScopedLock lock(queueLock);
        generationQueue.push(request);
        performanceState.queuedRequests = static_cast<int>(generationQueue.size());
    }
    
    if (!isProcessingGeneration.load())
    {
        juce::Thread::launch([this]() { processGenerationQueue(); });
    }
}

void LivePerformanceEngine::startPatternEvolution(float evolutionRate)
{
    evolutionSettings.mutationRate = juce::jlimit(0.01f, 0.5f, evolutionRate);
    isEvolutionActive.store(true);
}

void LivePerformanceEngine::stopPatternEvolution()
{
    isEvolutionActive.store(false);
}

//==============================================================================
// AI-Driven Synthesis Control

void LivePerformanceEngine::setAIParameterModulationEnabled(bool enabled)
{
    aiParameterModulation.store(enabled);
    
    if (!enabled)
    {
        // Clear all active modulations
        juce::ScopedLock lock(modulationLock);
        activeModulations.clear();
    }
}

void LivePerformanceEngine::setModulationIntensity(float intensity)
{
    modulationIntensity.store(juce::jlimit(0.0f, 1.0f, intensity));
}

void LivePerformanceEngine::setModulationTargets(const ModulationTargets& targets)
{
    modTargets = targets;
    
    // Remove any modulations for disabled targets
    {
        juce::ScopedLock lock(modulationLock);
        activeModulations.erase(
            std::remove_if(activeModulations.begin(), activeModulations.end(),
                [this](const ParameterModulation& mod) {
                    return !isTargetEnabled(mod.parameterName);
                }),
            activeModulations.end());
    }
}

//==============================================================================
// Performance Triggers

void LivePerformanceEngine::configureTriggers(TriggerType type, float probability, int beatDiv)
{
    triggerType = type;
    triggerProbability = juce::jlimit(0.01f, 1.0f, probability);
    beatDivision = juce::jmax(1, beatDiv);
}

void LivePerformanceEngine::manualTrigger()
{
    if (performanceState.currentPattern)
    {
        triggerPatternVariation(0.3f);
    }
    else if (performanceState.lastGenerationParams.key != 0)
    {
        triggerPatternGeneration(performanceState.lastGenerationParams);
    }
}

void LivePerformanceEngine::processMIDITriggers(const juce::MidiBuffer& midiBuffer)
{
    if (triggerType != TriggerType::ExternalMIDI)
        return;
    
    for (const auto metadata : midiBuffer)
    {
        const auto message = metadata.getMessage();
        
        // Trigger on MIDI CC 20 with value > 64
        if (message.isController() && message.getControllerNumber() == 20)
        {
            if (message.getControllerValue() > 64)
            {
                manualTrigger();
            }
        }
        
        // Trigger on specific note (C#5 = note 73)
        if (message.isNoteOn() && message.getNoteNumber() == 73)
        {
            manualTrigger();
        }
    }
}

//==============================================================================
// Evolution Settings

void LivePerformanceEngine::setEvolutionSettings(const EvolutionSettings& settings)
{
    evolutionSettings = settings;
}

//==============================================================================
// Performance State

juce::String LivePerformanceEngine::getPerformanceMetrics() const
{
    juce::String metrics;
    
    metrics << "Mode: " << [this]() {
        switch (currentMode.load())
        {
            case PerformanceMode::Disabled: return "Disabled";
            case PerformanceMode::PatternGeneration: return "Pattern Generation";
            case PerformanceMode::ParameterModulation: return "Parameter Modulation";
            case PerformanceMode::PatternEvolution: return "Pattern Evolution";
            case PerformanceMode::FullLive: return "Full Live";
            default: return "Unknown";
        }
    }() << "\n";
    
    metrics << "Patterns Generated: " << performanceState.generatedPatterns << "\n";
    metrics << "Evolution Cycles: " << performanceState.evolutionCycles << "\n";
    metrics << "Avg Generation Time: " << juce::String(performanceState.avgGenerationTime, 1) << "ms\n";
    metrics << "Queued Requests: " << performanceState.queuedRequests << "\n";
    metrics << "AI Modulation: " << (aiParameterModulation.load() ? "ON" : "OFF") << "\n";
    metrics << "Modulation Intensity: " << juce::String(modulationIntensity.load() * 100.0f, 1) << "%\n";
    
    return metrics;
}

//==============================================================================
// Listener Management

void LivePerformanceEngine::addListener(Listener* listener)
{
    listeners.add(listener);
}

void LivePerformanceEngine::removeListener(Listener* listener)
{
    listeners.remove(listener);
}

//==============================================================================
// Private Methods

void LivePerformanceEngine::processGenerationQueue()
{
    isProcessingGeneration.store(true);
    
    while (true)
    {
        GenerationRequest request;
        
        // Get next request
        {
            juce::ScopedLock lock(queueLock);
            if (generationQueue.empty())
            {
                performanceState.queuedRequests = 0;
                break;
            }
            
            request = generationQueue.front();
            generationQueue.pop();
            performanceState.queuedRequests = static_cast<int>(generationQueue.size());
        }
        
        // Time the generation
        auto startTime = juce::Time::getCurrentTime();
        
        try
        {
            std::shared_ptr<MIDIPattern> newPattern;
            
            if (request.type == GenerationRequest::NewPattern)
            {
                // Generate completely new pattern
                juce::var params = juce::var(new juce::DynamicObject());
                params.getDynamicObject()->setProperty("key", request.params.key);
                params.getDynamicObject()->setProperty("scale", static_cast<int>(request.params.scale));
                params.getDynamicObject()->setProperty("tempo", static_cast<int>(request.params.tempo));
                params.getDynamicObject()->setProperty("complexity", request.params.rhythmicComplexity);
                params.getDynamicObject()->setProperty("generationType", 
                    request.params.generationType == GenerationParameters::GenerationType::Melody ? "melody" : "chords");
                
                auto result = onnxClient->generatePattern("default", params);
                
                if (result.success && !result.patternData.isEmpty())
                {
                    // Convert patternData to MIDIPattern
                    newPattern = std::make_shared<MIDIPattern>();
                    // TODO: Implement conversion from result.patternData to MIDIPattern
                    // For now, create a basic pattern as placeholder
                    newPattern->notes.clear();
                }
            }
            else if (request.type == GenerationRequest::Variation && performanceState.currentPattern)
            {
                // Generate variation of current pattern
                // For now, use evolution algorithm - could be enhanced with ONNX variation models
                newPattern = evolvePattern(*performanceState.currentPattern, evolutionSettings);
            }
            
            // Calculate generation time
            auto endTime = juce::Time::getCurrentTime();
            float generationTime = static_cast<float>((endTime - startTime).inMilliseconds());
            
            // Update performance metrics
            if (performanceState.generatedPatterns == 0)
            {
                performanceState.avgGenerationTime = generationTime;
            }
            else
            {
                performanceState.avgGenerationTime = 
                    (performanceState.avgGenerationTime * 0.8f) + (generationTime * 0.2f);
            }
            
            if (newPattern)
            {
                handleGenerationComplete(newPattern, request);
            }
        }
        catch (const std::exception& e)
        {
            notifyError("Pattern generation failed: " + juce::String(e.what()));
        }
    }
    
    isProcessingGeneration.store(false);
}

void LivePerformanceEngine::handleGenerationComplete(std::shared_ptr<MIDIPattern> newPattern,
                                                   const GenerationRequest& request)
{
    // Update performance state
    performanceState.currentPattern = newPattern;
    performanceState.lastGenerationParams = request.params;
    performanceState.generatedPatterns++;
    
    if (request.type == GenerationRequest::Evolution)
        performanceState.evolutionCycles++;
    
    // Analyze pattern for AI parameter modulation
    if (aiParameterModulation.load() && (currentMode.load() == PerformanceMode::ParameterModulation ||
                                        currentMode.load() == PerformanceMode::FullLive))
    {
        analyzePatternForModulation(*newPattern);
    }
    
    // Notify listeners
    notifyPatternGenerated(newPattern);
    notifyStateChanged();
}

void LivePerformanceEngine::analyzePatternForModulation(const MIDIPattern& pattern)
{
    if (!synthesisEngine)
        return;
    
    // Analyze pattern characteristics and map to synthesis parameters
    float complexity = calculatePatternComplexity(pattern);
    float avgPitch = calculateAveragePitch(pattern);
    float rhythmDensity = calculateRhythmDensity(pattern);
    float harmonicContent = calculateHarmonicContent(pattern);
    
    juce::ScopedLock lock(modulationLock);
    
    // Clear old modulations
    activeModulations.clear();
    
    // Create new modulations based on pattern analysis
    float intensity = modulationIntensity.load();
    
    if (modTargets.filterCutoff)
    {
        ParameterModulation cutoffMod;
        cutoffMod.parameterName = "filterCutoff";
        cutoffMod.baseValue = synthesisEngine->getSynthesisParameters().filter.cutoff;
        cutoffMod.targetValue = juce::jmap(complexity, 0.0f, 1.0f, 200.0f, 8000.0f);
        cutoffMod.currentValue = cutoffMod.baseValue;
        cutoffMod.rate = 0.1f;
        cutoffMod.intensity = intensity;
        cutoffMod.lastUpdate = juce::Time::getCurrentTime();
        activeModulations.push_back(cutoffMod);
    }
    
    if (modTargets.wavetablePosition)
    {
        ParameterModulation wtMod;
        wtMod.parameterName = "wavetablePosition";
        wtMod.baseValue = synthesisEngine->getSynthesisParameters().wavetable.wavetablePosition;
        wtMod.targetValue = juce::jmap(harmonicContent, 0.0f, 1.0f, 0.1f, 0.9f);
        wtMod.currentValue = wtMod.baseValue;
        wtMod.rate = 0.05f;
        wtMod.intensity = intensity;
        wtMod.lastUpdate = juce::Time::getCurrentTime();
        activeModulations.push_back(wtMod);
    }
    
    if (modTargets.lfoDepth)
    {
        ParameterModulation lfoMod;
        lfoMod.parameterName = "lfoDepth";
        lfoMod.baseValue = synthesisEngine->getSynthesisParameters().modulation.lfoDepth;
        lfoMod.targetValue = juce::jmap(rhythmDensity, 0.0f, 1.0f, 0.0f, 0.6f);
        lfoMod.currentValue = lfoMod.baseValue;
        lfoMod.rate = 0.2f;
        lfoMod.intensity = intensity;
        lfoMod.lastUpdate = juce::Time::getCurrentTime();
        activeModulations.push_back(lfoMod);
    }
}

void LivePerformanceEngine::updateParameterModulation()
{
    if (!aiParameterModulation.load() || !synthesisEngine)
        return;
    
    juce::ScopedLock lock(modulationLock);
    juce::StringPairArray changedParams;
    
    for (auto& mod : activeModulations)
    {
        auto currentTime = juce::Time::getCurrentTime();
        float deltaTime = static_cast<float>((currentTime - mod.lastUpdate).inMilliseconds()) / 1000.0f;
        
        // Move toward target value
        float diff = mod.targetValue - mod.currentValue;
        float step = diff * mod.rate * deltaTime;
        
        mod.currentValue += step;
        mod.lastUpdate = currentTime;
        
        // Apply to synthesis engine
        synthesisEngine->setParameter(mod.parameterName, mod.currentValue);
        changedParams.set(mod.parameterName, juce::String(mod.currentValue));
    }
    
    if (changedParams.size() > 0)
    {
        notifyParametersModulated(changedParams);
    }
}

std::shared_ptr<MIDIPattern> LivePerformanceEngine::evolvePattern(const MIDIPattern& sourcePattern,
                                                                const EvolutionSettings& settings)
{
    auto evolved = std::make_shared<MIDIPattern>(sourcePattern);
    
    // Apply mutations
    mutateNotes(*evolved, settings.mutationRate);
    varyRhythm(*evolved, settings.rhythmVariation);
    shiftHarmonicContent(*evolved, settings.harmonicShift);
    
    return evolved;
}

void LivePerformanceEngine::mutateNotes(MIDIPattern& pattern, float mutationRate)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    
    for (auto& note : pattern.notes)
    {
        if (dis(gen) < mutationRate)
        {
            // Randomly adjust pitch by ±2 semitones
            std::uniform_int_distribution<> pitchDis(-2, 2);
            note.pitch = juce::jlimit(0, 127, note.pitch + pitchDis(gen));
        }
    }
}

void LivePerformanceEngine::varyRhythm(MIDIPattern& pattern, float variationAmount)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(-variationAmount, variationAmount);
    
    for (auto& note : pattern.notes)
    {
        // Slightly adjust timing
        double adjustment = dis(gen) * 0.1; // ±10% timing variation
        note.startTime = juce::jmax(0.0, note.startTime + adjustment);
        note.duration = juce::jmax(0.1, note.duration + adjustment * 0.5);
    }
}

void LivePerformanceEngine::shiftHarmonicContent(MIDIPattern& pattern, float shiftAmount)
{
    // For now, implement as subtle pitch shifts
    // Could be enhanced with more sophisticated harmonic analysis
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(-1.0, 1.0);
    
    int shift = static_cast<int>(dis(gen) * shiftAmount * 3.0f); // ±3 semitones max
    
    for (auto& note : pattern.notes)
    {
        note.pitch = juce::jlimit(0, 127, note.pitch + shift);
    }
}

void LivePerformanceEngine::timerCallback()
{
    // Update parameter modulation
    updateParameterModulation();
    
    // Handle evolution if active
    if (isEvolutionActive.load() && performanceState.currentPattern)
    {
        // Evolve pattern every few seconds based on evolution rate
        static juce::Time lastEvolution;
        auto currentTime = juce::Time::getCurrentTime();
        
        float evolutionInterval = 1.0f / evolutionSettings.mutationRate; // seconds
        
        if ((currentTime - lastEvolution).inSeconds() >= evolutionInterval)
        {
            GenerationRequest request;
            request.params = performanceState.lastGenerationParams;
            request.requestTime = currentTime;
            request.type = GenerationRequest::Evolution;
            
            {
                juce::ScopedLock lock(queueLock);
                generationQueue.push(request);
                performanceState.queuedRequests = static_cast<int>(generationQueue.size());
            }
            
            if (!isProcessingGeneration.load())
            {
                juce::Thread::launch([this]() { processGenerationQueue(); });
            }
            
            lastEvolution = currentTime;
        }
    }
}

//==============================================================================
// Notification Methods

void LivePerformanceEngine::notifyPatternGenerated(std::shared_ptr<MIDIPattern> pattern)
{
    listeners.call([pattern](Listener& l) { l.patternGenerated(pattern); });
}

void LivePerformanceEngine::notifyParametersModulated(const juce::StringPairArray& params)
{
    listeners.call([&params](Listener& l) { l.synthesisParametersModulated(params); });
}

void LivePerformanceEngine::notifyStateChanged()
{
    listeners.call([this](Listener& l) { l.performanceStateChanged(performanceState); });
}

void LivePerformanceEngine::notifyError(const juce::String& error)
{
    listeners.call([&error](Listener& l) { l.performanceError(error); });
}

//==============================================================================
// Helper Methods

float LivePerformanceEngine::calculatePatternComplexity(const MIDIPattern& pattern)
{
    if (pattern.notes.empty()) return 0.0f;
    
    // Calculate complexity based on number of unique pitches and rhythmic density
    std::set<int> uniquePitches;
    for (const auto& note : pattern.notes)
        uniquePitches.insert(note.pitch);
    
    float pitchComplexity = static_cast<float>(uniquePitches.size()) / 12.0f; // Normalize to octave
    float rhythmComplexity = static_cast<float>(pattern.notes.size()) / 32.0f; // Assume max 32 notes
    
    return juce::jlimit(0.0f, 1.0f, (pitchComplexity + rhythmComplexity) * 0.5f);
}

float LivePerformanceEngine::calculateAveragePitch(const MIDIPattern& pattern)
{
    if (pattern.notes.empty()) return 60.0f; // Middle C
    
    float total = 0.0f;
    for (const auto& note : pattern.notes)
        total += static_cast<float>(note.pitch);
    
    return total / static_cast<float>(pattern.notes.size());
}

float LivePerformanceEngine::calculateRhythmDensity(const MIDIPattern& pattern)
{
    if (pattern.notes.empty()) return 0.0f;
    
    // Calculate based on average note duration and spacing
    double totalDuration = 0.0;
    double totalGaps = 0.0;
    
    for (size_t i = 0; i < pattern.notes.size(); ++i)
    {
        totalDuration += pattern.notes[i].duration;
        
        if (i > 0)
        {
            double gap = pattern.notes[i].startTime - 
                        (pattern.notes[i-1].startTime + pattern.notes[i-1].duration);
            totalGaps += juce::jmax(0.0, gap);
        }
    }
    
    float avgDuration = static_cast<float>(totalDuration / pattern.notes.size());
    float avgGap = pattern.notes.size() > 1 ? 
                   static_cast<float>(totalGaps / (pattern.notes.size() - 1)) : 1.0f;
    
    return juce::jlimit(0.0f, 1.0f, avgDuration / (avgDuration + avgGap));
}

float LivePerformanceEngine::calculateHarmonicContent(const MIDIPattern& pattern)
{
    // Simplified harmonic analysis - could be much more sophisticated
    if (pattern.notes.empty()) return 0.5f;
    
    std::map<int, int> pitchClassCounts;
    for (const auto& note : pattern.notes)
    {
        int pitchClass = note.pitch % 12;
        pitchClassCounts[pitchClass]++;
    }
    
    // Calculate how evenly distributed the pitch classes are
    float entropy = 0.0f;
    float totalNotes = static_cast<float>(pattern.notes.size());
    
    for (const auto& [pitchClass, count] : pitchClassCounts)
    {
        float probability = static_cast<float>(count) / totalNotes;
        entropy -= probability * std::log2f(probability);
    }
    
    // Normalize entropy (max is log2(12) for equal distribution)
    return juce::jlimit(0.0f, 1.0f, entropy / std::log2f(12.0f));
}

bool LivePerformanceEngine::isTargetEnabled(const juce::String& paramName)
{
    if (paramName == "filterCutoff") return modTargets.filterCutoff;
    if (paramName == "filterResonance") return modTargets.filterResonance;
    if (paramName == "wavetablePosition") return modTargets.wavetablePosition;
    if (paramName == "lfoDepth") return modTargets.lfoDepth;
    if (paramName == "lfoRate") return modTargets.lfoRate;
    if (paramName.startsWith("envelope.")) return modTargets.envelopeParams;
    if (paramName.startsWith("effects.")) return modTargets.effectsParams;
    
    return false;
}

} // namespace spawnclone::ai
