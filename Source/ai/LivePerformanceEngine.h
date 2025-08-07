//==============================================================================
/*
    SpawnClone Live Performance Engine - Real-Time AI-Driven Live Performance
    
    Integrates optimized ONNX daemon with AdvancedSynthesisEngine for real-time
    AI-driven pattern generation and synthesis parameter modulation during live performance.
    
    Key Features:
    - Real-time pattern generation using fast ONNX daemon (~10-100ms)
    - AI-driven synthesis parameter modulation
    - Pattern evolution and morphing
    - Live performance triggers and automation
    - Thread-safe integration with JUCE audio thread
*/
//==============================================================================

#pragma once

#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "ONNXDaemonClient.h"
#include "../audio/AdvancedSynthesisEngine.h"
#include "../audio/SynthesisParameterMapper.h"
#include "../GenerationParameters.h"
#include "../MIDIPattern.h"
#include <atomic>
#include <queue>
#include <memory>

namespace spawnclone::ai
{

//==============================================================================
/**
 * Real-Time AI-Driven Live Performance Engine
 * 
 * Combines fast ONNX inference with advanced synthesis for live performance:
 * - Real-time pattern generation and variation
 * - AI-driven synthesis parameter modulation
 * - Pattern evolution and morphing systems
 * - Performance automation and triggers
 */
class LivePerformanceEngine : public juce::Timer
{
public:
    //==============================================================================
    LivePerformanceEngine();
    ~LivePerformanceEngine() override;

    //==============================================================================
    // Engine Lifecycle
    
    /** Initialize the live performance engine */
    bool initialize();
    
    /** Set the ONNX daemon client (must be initialized and ready) */
    void setONNXDaemonClient(std::shared_ptr<ONNXDaemonClient> client);
    
    /** Set the advanced synthesis engine for parameter control */
    void setAdvancedSynthesisEngine(spawnclone::audio::AdvancedSynthesisEngine* engine);
    
    /** Prepare for live performance */
    void prepareForPerformance(double sampleRate, int blockSize);
    
    /** Release performance resources */
    void releaseResources();

    //==============================================================================
    // Performance Modes
    
    enum class PerformanceMode
    {
        Disabled,           // No live AI processing
        PatternGeneration,  // Generate new patterns on triggers
        ParameterModulation, // Modulate synthesis parameters
        PatternEvolution,   // Evolve existing patterns
        FullLive           // All features enabled
    };
    
    /** Set the current performance mode */
    void setPerformanceMode(PerformanceMode mode);
    PerformanceMode getPerformanceMode() const { return currentMode.load(); }

    //==============================================================================
    // Real-Time Pattern Generation
    
    /** Trigger immediate pattern generation (async, ~10-100ms) */
    void triggerPatternGeneration(const GenerationParameters& params);
    
    /** Generate pattern variation based on current pattern */
    void triggerPatternVariation(float variationAmount = 0.3f);
    
    /** Start continuous pattern evolution */
    void startPatternEvolution(float evolutionRate = 0.1f); // mutations per second
    
    /** Stop pattern evolution */
    void stopPatternEvolution();

    //==============================================================================
    // AI-Driven Synthesis Control
    
    /** Enable/disable AI-driven synthesis parameter modulation */
    void setAIParameterModulationEnabled(bool enabled);
    bool isAIParameterModulationEnabled() const { return aiParameterModulation.load(); }
    
    /** Set modulation intensity (0.0 = none, 1.0 = full) */
    void setModulationIntensity(float intensity);
    float getModulationIntensity() const { return modulationIntensity.load(); }
    
    /** Set which synthesis parameters to modulate */
    struct ModulationTargets
    {
        bool filterCutoff = true;
        bool filterResonance = true;
        bool wavetablePosition = true;
        bool lfoDepth = true;
        bool lfoRate = false; // Can be disruptive
        bool envelopeParams = false; // Usually too jarring for live use
        bool effectsParams = true;
    };
    
    void setModulationTargets(const ModulationTargets& targets);
    const ModulationTargets& getModulationTargets() const { return modTargets; }

    //==============================================================================
    // Performance Triggers
    
    /** Performance trigger types */
    enum class TriggerType
    {
        Manual,             // User-triggered
        BeatSync,           // Sync to DAW transport
        Probability,        // Random probability-based
        PatternBased,       // Based on current pattern characteristics
        ExternalMIDI        // MIDI CC or note triggers
    };
    
    /** Configure trigger system */
    void configureTriggers(TriggerType type, float probability = 0.1f, int beatDivision = 16);
    
    /** Manual trigger for pattern generation */
    void manualTrigger();
    
    /** Process MIDI for trigger detection */
    void processMIDITriggers(const juce::MidiBuffer& midiBuffer);

    //==============================================================================
    // Pattern Evolution System
    
    struct EvolutionSettings
    {
        float mutationRate = 0.1f;         // Probability of note mutation
        float rhythmVariation = 0.2f;      // Rhythm pattern changes
        float harmonicShift = 0.15f;       // Harmonic content evolution
        float timbreEvolution = 0.25f;     // Synthesis parameter drift
        bool maintainKey = true;           // Keep patterns in key
        bool maintainStructure = true;     // Preserve basic pattern structure
    };
    
    void setEvolutionSettings(const EvolutionSettings& settings);
    const EvolutionSettings& getEvolutionSettings() const { return evolutionSettings; }

    //==============================================================================
    // Live Performance State
    
    struct PerformanceState
    {
        std::shared_ptr<MIDIPattern> currentPattern;
        GenerationParameters lastGenerationParams;
        double performanceTime = 0.0;
        int generatedPatterns = 0;
        int evolutionCycles = 0;
        bool isGenerating = false;
        
        // Real-time metrics
        float avgGenerationTime = 0.0f;    // ms
        float aiCpuUsage = 0.0f;           // %
        int queuedRequests = 0;
    };
    
    /** Get current performance state */
    const PerformanceState& getPerformanceState() const { return performanceState; }
    
    /** Get performance metrics as string */
    juce::String getPerformanceMetrics() const;

    //==============================================================================
    // Callbacks and Listeners
    
    class Listener
    {
    public:
        virtual ~Listener() = default;
        
        /** Called when new pattern is generated */
        virtual void patternGenerated(std::shared_ptr<MIDIPattern> newPattern) {}
        
        /** Called when synthesis parameters are modulated by AI */
        virtual void synthesisParametersModulated(const juce::StringPairArray& changedParams) {}
        
        /** Called on performance state changes */
        virtual void performanceStateChanged(const PerformanceState& state) {}
        
        /** Called on performance errors */
        virtual void performanceError(const juce::String& errorMessage) {}
    };
    
    void addListener(Listener* listener);
    void removeListener(Listener* listener);

private:
    //==============================================================================
    // Core Components
    std::shared_ptr<ONNXDaemonClient> onnxClient;
    spawnclone::audio::AdvancedSynthesisEngine* synthesisEngine = nullptr;
    std::unique_ptr<spawnclone::audio::SynthesisParameterMapper> parameterMapper;
    
    // Performance State
    std::atomic<PerformanceMode> currentMode{PerformanceMode::Disabled};
    std::atomic<bool> aiParameterModulation{false};
    std::atomic<float> modulationIntensity{0.5f};
    std::atomic<bool> isEvolutionActive{false};
    
    PerformanceState performanceState;
    ModulationTargets modTargets;
    EvolutionSettings evolutionSettings;
    
    // Audio thread integration
    double sampleRate = 44100.0;
    int blockSize = 512;
    
    //==============================================================================
    // Async Generation System
    struct GenerationRequest
    {
        GenerationParameters params;
        juce::Time requestTime;
        enum Type { NewPattern, Variation, Evolution } type;
        float variationAmount = 0.0f;
    };
    
    std::queue<GenerationRequest> generationQueue;
    juce::CriticalSection queueLock;
    std::atomic<bool> isProcessingGeneration{false};
    
    void processGenerationQueue();
    void handleGenerationComplete(std::shared_ptr<MIDIPattern> newPattern, 
                                 const GenerationRequest& request);

    //==============================================================================
    // AI Parameter Modulation
    struct ParameterModulation
    {
        juce::String parameterName;
        float baseValue;
        float targetValue;
        float currentValue;
        float rate;           // modulation rate
        float intensity;      // modulation depth
        juce::Time lastUpdate;
    };
    
    std::vector<ParameterModulation> activeModulations;
    juce::CriticalSection modulationLock;
    
    void updateParameterModulation();
    void analyzePatternForModulation(const MIDIPattern& pattern);
    float calculateAIModulationValue(const juce::String& paramName, 
                                   const MIDIPattern& pattern);

    //==============================================================================
    // Pattern Evolution
    std::shared_ptr<MIDIPattern> evolvePattern(const MIDIPattern& sourcePattern,
                                              const EvolutionSettings& settings);
    void mutateNotes(MIDIPattern& pattern, float mutationRate);
    void varyRhythm(MIDIPattern& pattern, float variationAmount);
    void shiftHarmonicContent(MIDIPattern& pattern, float shiftAmount);

    //==============================================================================
    // Trigger System
    TriggerType triggerType = TriggerType::Manual;
    float triggerProbability = 0.1f;
    int beatDivision = 16;
    juce::Time lastTrigger;
    
    bool shouldTrigger(double transportTime, bool isPlaying);
    void processTriggers(double transportTime, bool isPlaying);

    //==============================================================================
    // Timer callback for evolution and modulation
    void timerCallback() override;
    
    // Listener management
    juce::ListenerList<Listener> listeners;
    void notifyPatternGenerated(std::shared_ptr<MIDIPattern> pattern);
    void notifyParametersModulated(const juce::StringPairArray& params);
    void notifyStateChanged();
    void notifyError(const juce::String& error);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LivePerformanceEngine)
};

} // namespace spawnclone::ai
