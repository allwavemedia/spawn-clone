# Epic 7 Story 7.1: Real-Time Pattern Generation with Live DAW Integration

## **Story Overview**
**As a music producer, I want real-time pattern generation that responds instantly to my DAW's tempo, key changes, and transport state, so that I can create dynamic, evolving musical content during live performance and recording sessions.**

## **Acceptance Criteria**

### **AC1: Live Transport Synchronization**
- GIVEN my DAW is playing with tempo automation
- WHEN the tempo changes during playback
- THEN SpawnClone automatically adapts pattern generation to match the new tempo
- AND generated patterns remain musically coherent across tempo changes

### **AC2: Dynamic Key/Scale Adaptation**
- GIVEN I change the key signature in my DAW project
- WHEN SpawnClone detects the key change
- THEN all subsequent pattern generation uses the new key/scale
- AND patterns can optionally modulate smoothly between keys

### **AC3: Pattern Evolution During Playback**
- GIVEN SpawnClone is generating patterns during DAW playback
- WHEN I enable "Evolution Mode"
- THEN patterns gradually evolve and develop over time
- AND evolution respects my user style profile preferences

### **AC4: Beat-Synchronized Generation**
- GIVEN my DAW is playing at any tempo
- WHEN I trigger pattern generation
- THEN new patterns start exactly on the next beat/bar
- AND pattern lengths align perfectly with DAW timing grid

### **AC5: Performance-Optimized Processing**
- GIVEN SpawnClone is running in real-time mode
- WHEN generating patterns during playback
- THEN generation completes within one audio buffer (≤10ms)
- AND CPU usage remains below 5% on modern systems

## **Technical Requirements**

### **TR1: Real-Time Audio Thread Integration**
```cpp
class RealTimePatternGenerator
{
public:
    /** Initialize with audio settings */
    void prepareToPlay(double sampleRate, int blockSize);
    
    /** Process audio block with real-time pattern generation */
    void processBlock(juce::AudioBuffer<float>& buffer, 
                     juce::MidiBuffer& midiBuffer,
                     const juce::AudioPlayHead::CurrentPositionInfo& positionInfo);
    
    /** Queue pattern generation request for next beat */
    void requestPatternGeneration(const GenerationParameters& params);
    
    /** Check if generation is complete */
    bool isGenerationReady() const;
    
    /** Get the latest generated pattern */
    MIDIPattern getLatestPattern();
    
private:
    /** Lock-free pattern queue for audio thread safety */
    juce::AbstractFifo patternQueue;
    
    /** High-priority generation thread */
    std::unique_ptr<juce::Thread> generationThread;
    
    /** Beat-synchronized pattern scheduling */
    struct PatternScheduler
    {
        double nextGenerationTime = 0.0;
        int beatsPerGeneration = 4;
        bool evolutionEnabled = false;
    } scheduler;
};
```

### **TR2: Tempo-Adaptive Pattern Engine**
```cpp
class TempoAdaptiveEngine
{
public:
    /** Update engine with current tempo */
    void setCurrentTempo(double bpm);
    
    /** Adapt existing pattern to new tempo */
    MIDIPattern adaptPatternToTempo(const MIDIPattern& source, 
                                   double oldTempo, 
                                   double newTempo);
    
    /** Generate tempo-appropriate pattern characteristics */
    GenerationParameters adaptParametersToTempo(const GenerationParameters& base,
                                               double targetTempo);
    
    /** Smooth tempo transitions for musical coherence */
    struct TempoTransition
    {
        double startTempo;
        double endTempo;
        double transitionBeats;
        std::function<double(double)> easingCurve;
    };
    
    void beginTempoTransition(const TempoTransition& transition);
    
private:
    double currentTempo = 120.0;
    std::optional<TempoTransition> activeTransition;
    
    /** Tempo-specific pattern adjustments */
    void adjustRhythmicComplexity(MIDIPattern& pattern, double tempo);
    void adjustNoteVelocities(MIDIPattern& pattern, double tempo);
    void adjustArticulation(MIDIPattern& pattern, double tempo);
};
```

### **TR3: Dynamic Key Modulation System**
```cpp
class KeyModulationEngine
{
public:
    /** Set the current musical key */
    void setCurrentKey(const MusicalKey& key);
    
    /** Detect key from incoming MIDI or audio analysis */
    MusicalKey detectKeyFromContext(const juce::MidiBuffer& recentMidi);
    
    /** Generate smooth key transition */
    MIDIPattern createKeyTransition(const MusicalKey& fromKey,
                                   const MusicalKey& toKey,
                                   int transitionBars = 2);
    
    /** Adapt existing pattern to new key */
    MIDIPattern transposePatternToKey(const MIDIPattern& source,
                                     const MusicalKey& targetKey);
    
    /** Get modulation suggestions based on current context */
    std::vector<MusicalKey> suggestModulations(const MusicalKey& currentKey,
                                              const UserStyleProfile& profile);
    
private:
    MusicalKey currentKey{Key::C, Scale::Major};
    std::queue<MusicalKey> keyHistory;
    
    /** Advanced harmonic analysis for smooth transitions */
    struct ModulationPath
    {
        MusicalKey source;
        MusicalKey destination;
        std::vector<ExtendedChordType> pivotChords;
        int recommendedBars;
    };
    
    ModulationPath calculateOptimalModulation(const MusicalKey& from, 
                                            const MusicalKey& to);
};
```

### **TR4: Pattern Evolution System**
```cpp
class PatternEvolutionEngine
{
public:
    /** Initialize evolution with base pattern */
    void setBasePattern(const MIDIPattern& pattern);
    
    /** Generate next evolutionary step */
    MIDIPattern evolvePattern(const UserStyleProfile& profile,
                             double evolutionIntensity = 0.5);
    
    /** Reset evolution to base pattern */
    void resetEvolution();
    
    /** Get evolution history for undo functionality */
    std::vector<MIDIPattern> getEvolutionHistory() const;
    
    /** Evolution parameters */
    struct EvolutionSettings
    {
        bool rhythmicEvolution = true;
        bool harmonicEvolution = true;
        bool melodicEvolution = true;
        bool dynamicEvolution = true;
        
        float evolutionRate = 0.1f;        // How much change per step
        int maxEvolutionSteps = 32;        // Prevent runaway evolution
        bool respectUserPreferences = true;
    };
    
    void setEvolutionSettings(const EvolutionSettings& settings);
    
private:
    MIDIPattern basePattern;
    std::vector<MIDIPattern> evolutionHistory;
    EvolutionSettings settings;
    int currentEvolutionStep = 0;
    
    /** Evolution algorithms */
    void applyRhythmicEvolution(MIDIPattern& pattern, float intensity);
    void applyHarmonicEvolution(MIDIPattern& pattern, float intensity);
    void applyMelodicEvolution(MIDIPattern& pattern, float intensity);
    void applyDynamicEvolution(MIDIPattern& pattern, float intensity);
};
```

### **TR5: Beat-Synchronized Scheduling**
```cpp
class BeatSynchronizedScheduler
{
public:
    /** Initialize with timing information */
    void initialize(double sampleRate, const juce::AudioPlayHead::CurrentPositionInfo& info);
    
    /** Update with current playhead position */
    void updatePosition(const juce::AudioPlayHead::CurrentPositionInfo& info);
    
    /** Schedule action for next beat/bar */
    void scheduleOnNextBeat(std::function<void()> action);
    void scheduleOnNextBar(std::function<void()> action);
    
    /** Process scheduled actions in audio thread */
    void processScheduledActions(int bufferSize);
    
    /** Check if we're approaching a beat boundary */
    bool isApproachingBeat(double lookAheadMs = 100.0) const;
    bool isApproachingBar(double lookAheadMs = 100.0) const;
    
private:
    double sampleRate = 44100.0;
    double currentBpm = 120.0;
    double currentBeat = 0.0;
    double currentBar = 0.0;
    int beatsPerBar = 4;
    
    struct ScheduledAction
    {
        double executionTime;
        std::function<void()> action;
        bool executed = false;
    };
    
    std::vector<ScheduledAction> scheduledActions;
    
    /** Precise timing calculations */
    double beatsToSamples(double beats) const;
    double samplesToBeats(double samples) const;
    double getNextBeatTime() const;
    double getNextBarTime() const;
};
```

## **Implementation Plan**

### **Phase 1: Real-Time Infrastructure (Tasks 7.1.1-7.1.3)**
1. **Task 7.1.1:** Implement `RealTimePatternGenerator` with lock-free audio thread integration
2. **Task 7.1.2:** Create `BeatSynchronizedScheduler` for precise timing control
3. **Task 7.1.3:** Integrate with `PluginProcessor` audio callback for real-time operation

### **Phase 2: Tempo & Key Adaptation (Tasks 7.1.4-7.1.6)**
4. **Task 7.1.4:** Implement `TempoAdaptiveEngine` with smooth tempo transition handling
5. **Task 7.1.5:** Create `KeyModulationEngine` for dynamic key changes and modulation
6. **Task 7.1.6:** Add tempo/key change detection from DAW host information

### **Phase 3: Pattern Evolution (Tasks 7.1.7-7.1.9)**
7. **Task 7.1.7:** Implement `PatternEvolutionEngine` with gradual pattern development
8. **Task 7.1.8:** Integrate evolution system with user style profiles for personalized evolution
9. **Task 7.1.9:** Add evolution controls to UI for real-time parameter adjustment

### **Phase 4: Performance Optimization (Tasks 7.1.10-7.1.12)**
10. **Task 7.1.10:** Optimize generation algorithms for <10ms latency target
11. **Task 7.1.11:** Implement intelligent pattern caching and prediction
12. **Task 7.1.12:** Add performance monitoring and adaptive quality scaling

## **Success Metrics**

- **Latency**: Pattern generation completes within 10ms (audio buffer size)
- **CPU Usage**: Real-time mode uses <5% CPU on modern systems  
- **Timing Accuracy**: Beat synchronization accurate to ±1ms
- **Musical Quality**: Tempo/key adaptations maintain musical coherence
- **Stability**: No audio dropouts or glitches during real-time generation
- **Evolution Quality**: Pattern evolution feels natural and musically progressive

## **Integration Points**

- **PluginProcessor**: Real-time audio thread integration
- **UserStyleProfile**: Personalized evolution and adaptation preferences  
- **MusicTheoryEngine**: Advanced harmonic analysis for key modulation
- **GenreSpecialist**: Style-appropriate tempo and evolution characteristics
- **AdvancedHarmonyEngine**: Sophisticated chord progression evolution

This story transforms SpawnClone from a pattern generator into a dynamic, living musical partner that evolves and adapts in real-time with your creative process.
