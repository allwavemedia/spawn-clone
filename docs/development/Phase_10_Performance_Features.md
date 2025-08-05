# Phase 10: Performance Features and Live Tools

## Overview

Phase 10 transforms SpawnClone into a complete performance instrument by adding advanced performance features, live tools, and real-time creative capabilities. This phase focuses on making SpawnClone stage-ready and perfect for live electronic music performance.

## Performance Architecture

### Core Performance Engine

```cpp
class PerformanceEngine
{
public:
    PerformanceEngine(AdvancedSynthesisEngine& synthesisEngine);
    
    // Performance components
    std::unique_ptr<ArpeggiatorEngine> arpeggiator;
    std::unique_ptr<SequencerEngine> sequencer;
    std::unique_ptr<LiveLooper> looper;
    std::unique_ptr<MacroController> macroController;
    std::unique_ptr<PerformanceRecorder> recorder;
    
    // Performance state
    struct PerformanceState
    {
        bool isLiveMode = false;
        float masterTempo = 120.0f;
        int currentScene = 0;
        std::vector<bool> partMutes;
        std::vector<float> partLevels;
        
        // Real-time controls
        std::map<std::string, float> liveParameters;
        std::vector<MacroAssignment> macroAssignments;
    };
    
    void prepareForPerformance(double sampleRate, int bufferSize);
    void processPerformanceBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiBuffer);
    
private:
    AdvancedSynthesisEngine& synthesisEngine;
    PerformanceState currentState;
};
```

## Advanced Performance Features

### 1. Professional Arpeggiator

```cpp
class ArpeggiatorEngine
{
public:
    enum ArpMode
    {
        Up, Down, UpDown, DownUp, Random, AsPlayed, Chord
    };
    
    enum TimeSignature
    {
        Common_4_4 = 0,
        Waltz_3_4,
        Complex_7_8,
        Custom
    };
    
    struct ArpeggiatorParams
    {
        bool enabled = false;
        ArpMode mode = Up;
        float rate = 1.0f;              // Note divisions (1/4, 1/8, 1/16, etc.)
        int octaveRange = 1;            // 1-4 octaves
        int gateLength = 80;            // Percentage of note length
        float swing = 0.0f;             // Swing timing (-50% to +50%)
        
        // Advanced features
        bool latchMode = false;
        int stepLimit = 0;              // 0 = unlimited
        float velocity = 1.0f;          // Velocity multiplier
        bool retrigger = true;          // Retrigger on new note
        
        // Pattern customization
        std::vector<bool> stepMask;     // Per-step enable/disable
        std::vector<float> stepVelocities; // Per-step velocity
        std::vector<int> stepTranspose; // Per-step transposition
    };
    
    void processArpeggiator(juce::MidiBuffer& outputMidi, double sampleRate);
    void handleNoteOn(int noteNumber, float velocity);
    void handleNoteOff(int noteNumber);
    
private:
    ArpeggiatorParams params;
    std::vector<int> heldNotes;
    std::vector<int> arpPattern;
    
    int currentStep = 0;
    double stepPosition = 0.0;
    double samplesPerStep = 0.0;
    
    void generateArpPattern();
    void updateTiming(double sampleRate);
    float calculateSwingTiming(int step) const;
};
```

### 2. Advanced Step Sequencer

```cpp
class SequencerEngine
{
public:
    static constexpr int MAX_TRACKS = 16;
    static constexpr int MAX_STEPS = 64;
    
    struct SequencerStep
    {
        bool active = false;
        int noteNumber = 60;
        float velocity = 1.0f;
        float gateLength = 0.8f;        // 0.1 to 1.0
        int microtiming = 0;            // -50 to +50 ticks
        
        // Advanced features
        bool slide = false;
        bool accent = false;
        int probability = 100;          // 0-100%
        std::vector<int> chordNotes;    // Additional notes for chords
    };
    
    struct SequencerTrack
    {
        std::array<SequencerStep, MAX_STEPS> steps;
        int length = 16;                // Track length in steps
        int channel = 1;                // MIDI channel
        bool muted = false;
        bool soloed = false;
        
        // Track parameters
        float trackLevel = 1.0f;
        int trackTranspose = 0;
        float trackSwing = 0.0f;
        
        // Pattern variations
        std::vector<std::array<SequencerStep, MAX_STEPS>> variations;
        int currentVariation = 0;
    };
    
    struct SequencerParams
    {
        bool enabled = false;
        float tempo = 120.0f;
        TimeSignature timeSignature = Common_4_4;
        int currentStep = 0;
        bool isPlaying = false;
        bool isRecording = false;
        
        // Global settings
        float globalSwing = 0.0f;
        int quantization = 16;          // Steps per beat
        bool chainMode = false;         // Chain patterns together
    };
    
    void processSequencer(juce::MidiBuffer& outputMidi, double sampleRate);
    void startSequencer();
    void stopSequencer();
    void recordStep(int track, int step, const SequencerStep& stepData);
    
private:
    SequencerParams params;
    std::array<SequencerTrack, MAX_TRACKS> tracks;
    
    double stepPosition = 0.0;
    double samplesPerStep = 0.0;
    juce::Random probabilityRandom;
    
    void processTrack(int trackIndex, juce::MidiBuffer& outputMidi);
    bool shouldTriggerStep(const SequencerStep& step) const;
    void calculateTiming(double sampleRate);
};
```

### 3. Live Looper System

```cpp
class LiveLooper
{
public:
    enum LooperState
    {
        Stopped,
        Recording,
        Playing,
        Overdubbing,
        Muted
    };
    
    struct LoopSlot
    {
        juce::AudioBuffer<float> audioBuffer;
        double length = 0.0;            // Length in seconds
        double position = 0.0;          // Current playback position
        LooperState state = Stopped;
        float level = 1.0f;
        float feedback = 1.0f;          // For overdubbing
        
        // Loop timing
        bool quantizedRecord = true;
        double quantizeLength = 4.0;    // Bars
        bool autoQuantize = true;
        
        // Effects per loop
        std::unique_ptr<DelayProcessor> delay;
        std::unique_ptr<ReverbProcessor> reverb;
        float pitchShift = 0.0f;        // Semitones
    };
    
    static constexpr int MAX_LOOPS = 8;
    std::array<LoopSlot, MAX_LOOPS> loopSlots;
    
    struct LooperParams
    {
        bool enabled = false;
        float masterTempo = 120.0f;
        int masterLoop = 0;             // Master timing reference
        bool syncToTempo = true;
        
        // Global controls
        float crossfade = 0.0f;         // Between loops
        bool fadeInOut = true;
        float fadeTime = 0.1f;          // Seconds
    };
    
    void processLooper(juce::AudioBuffer<float>& buffer, double sampleRate);
    void startRecording(int loopIndex);
    void stopRecording(int loopIndex);
    void startPlayback(int loopIndex);
    void stopPlayback(int loopIndex);
    void clearLoop(int loopIndex);
    
private:
    LooperParams params;
    double sampleRate = 44100.0;
    
    void recordToLoop(int loopIndex, const juce::AudioBuffer<float>& inputBuffer);
    void playFromLoop(int loopIndex, juce::AudioBuffer<float>& outputBuffer);
    void quantizeLoopLength(LoopSlot& loop);
};
```

### 4. Macro Control System

```cpp
class MacroController
{
public:
    struct MacroAssignment
    {
        std::string parameterID;        // Target parameter
        float minValue = 0.0f;
        float maxValue = 1.0f;
        float curve = 0.0f;             // -1 to 1 (exponential curve)
        bool bipolar = false;
        bool enabled = true;
        
        // Advanced mapping
        std::function<float(float)> customMapping;
        std::string expression;         // Mathematical expression
    };
    
    struct MacroControl
    {
        std::string name;
        float value = 0.0f;
        std::vector<MacroAssignment> assignments;
        
        // MIDI/Hardware mapping
        int midiCC = -1;                // -1 = not mapped
        int midiChannel = 1;
        std::string hardwareControl;    // Hardware controller name
        
        // Visual feedback
        juce::Colour controlColour = juce::Colours::blue;
        std::string displayUnit = "";
    };
    
    static constexpr int MAX_MACROS = 16;
    std::array<MacroControl, MAX_MACROS> macroControls;
    
    void setMacroValue(int macroIndex, float value);
    void addAssignment(int macroIndex, const MacroAssignment& assignment);
    void removeAssignment(int macroIndex, const std::string& parameterID);
    
    // MIDI learning
    void startMIDILearn(int macroIndex);
    void stopMIDILearn();
    void handleMIDICC(int channel, int cc, float value);
    
private:
    int currentlyLearning = -1;
    std::map<std::string, float*> parameterMap;
    
    void updateAssignedParameters(int macroIndex);
    float applyCurve(float input, float curve) const;
    float evaluateExpression(const std::string& expression, float input) const;
};
```

### 5. Performance Scene Management

```cpp
class SceneManager
{
public:
    struct Scene
    {
        std::string name;
        std::string description;
        
        // Complete synth state
        SynthesisParameters synthParams;
        ArpeggiatorEngine::ArpeggiatorParams arpParams;
        SequencerEngine::SequencerParams seqParams;
        LiveLooper::LooperParams looperParams;
        
        // Macro control states
        std::array<float, 16> macroValues;
        
        // Performance settings
        float sceneLevel = 1.0f;
        bool muteOnLoad = false;
        float fadeTime = 1.0f;          // Scene transition time
        
        // Visual representation
        juce::Colour sceneColour = juce::Colours::white;
        std::string iconName;
    };
    
    static constexpr int MAX_SCENES = 128;
    std::array<Scene, MAX_SCENES> scenes;
    
    void loadScene(int sceneIndex, bool fade = true);
    void saveCurrentAsScene(int sceneIndex, const std::string& name);
    void morphBetweenScenes(int sceneA, int sceneB, float morphPosition);
    
    // Scene organization
    void copyScene(int sourceIndex, int destIndex);
    void swapScenes(int indexA, int indexB);
    void clearScene(int sceneIndex);
    
    // Performance features
    void setSceneChain(const std::vector<int>& sceneOrder);
    void nextScene();
    void previousScene();
    void randomScene();
    
private:
    int currentScene = 0;
    int previousScene = -1;
    std::vector<int> sceneChain;
    int chainPosition = 0;
    
    // Morphing state
    bool isMorphing = false;
    float morphStartTime = 0.0f;
    float morphDuration = 1.0f;
    
    void interpolateParameters(const Scene& sceneA, const Scene& sceneB, float position);
    void updatePerformanceEngine(const Scene& scene);
};
```

## Hardware Integration

### 1. MIDI Controller Support

```cpp
class MIDIControllerManager
{
public:
    struct ControllerProfile
    {
        std::string name;
        std::string manufacturer;
        std::vector<std::string> deviceNames;  // MIDI device identifiers
        
        // Control mapping
        std::map<int, std::string> ccMappings;  // CC number -> parameter ID
        std::map<int, std::string> buttonMappings; // Note number -> function
        
        // Special features
        bool hasMotorizedFaders = false;
        bool hasRGBButtons = false;
        bool hasDisplays = false;
        
        // Feedback configuration
        std::map<std::string, int> feedbackMappings; // Parameter ID -> CC for feedback
    };
    
    // Popular controller profiles
    void loadControllerProfile(const std::string& profileName);
    void saveControllerProfile(const ControllerProfile& profile);
    
    // Built-in profiles
    void initializeBuiltInProfiles();  // Ableton Push, Novation Launchpad, etc.
    
    // Auto-detection
    std::vector<std::string> detectConnectedControllers();
    void autoConfigureController(const std::string& deviceName);
    
private:
    std::map<std::string, ControllerProfile> controllerProfiles;
    std::vector<std::string> connectedControllers;
    
    void sendControllerFeedback(const std::string& parameterID, float value);
    void handleControllerInput(const juce::MidiMessage& message);
};
```

### 2. OSC Integration

```cpp
class OSCIntegration
{
public:
    struct OSCMapping
    {
        std::string oscAddress;         // e.g., "/synth/filter/cutoff"
        std::string parameterID;
        float minValue = 0.0f;
        float maxValue = 1.0f;
        bool bidirectional = true;      // Send feedback
    };
    
    OSCIntegration();
    
    void startOSCServer(int port = 8000);
    void stopOSCServer();
    void addOSCMapping(const OSCMapping& mapping);
    
    // TouchOSC / Lemur support
    void loadTouchOSCTemplate(const juce::File& templateFile);
    void sendParameterUpdate(const std::string& parameterID, float value);
    
private:
    std::unique_ptr<juce::OSCReceiver> oscReceiver;
    std::unique_ptr<juce::OSCSender> oscSender;
    std::vector<OSCMapping> oscMappings;
    
    void handleOSCMessage(const juce::OSCMessage& message);
    void sendOSCFeedback(const std::string& address, float value);
};
```

## Live Performance Features

### 1. Performance Recorder

```cpp
class PerformanceRecorder
{
public:
    enum RecordingMode
    {
        Audio,              // Record audio output
        MIDI,               // Record MIDI input/output
        Automation,         // Record parameter automation
        Everything          // Record all of the above
    };
    
    struct RecordingSession
    {
        std::string name;
        juce::Time startTime;
        double duration = 0.0;
        RecordingMode mode = Everything;
        
        // Recorded data
        juce::AudioBuffer<float> audioData;
        juce::MidiBuffer midiData;
        std::vector<AutomationEvent> automationData;
        
        // Session metadata
        float tempo = 120.0f;
        int timeSigNumerator = 4;
        int timeSigDenominator = 4;
        std::string key = "C";
    };
    
    void startRecording(RecordingMode mode, const std::string& sessionName);
    void stopRecording();
    void pauseRecording();
    void resumeRecording();
    
    // Playback
    void playbackSession(const RecordingSession& session);
    void exportSession(const RecordingSession& session, const juce::File& outputFile);
    
private:
    std::unique_ptr<RecordingSession> currentSession;
    bool isRecording = false;
    bool isPaused = false;
    
    void recordAudioData(const juce::AudioBuffer<float>& buffer);
    void recordMIDIData(const juce::MidiBuffer& midiBuffer);
    void recordAutomationEvent(const std::string& parameterID, float value, double timestamp);
};
```

### 2. Live Sampling and Resampling

```cpp
class LiveSampler
{
public:
    struct LiveSample
    {
        juce::AudioBuffer<float> audioData;
        float originalPitch = 60.0f;    // MIDI note
        double originalTempo = 120.0f;
        bool isLooped = false;
        
        // Real-time analysis
        float detectedPitch = 60.0f;
        double detectedTempo = 120.0f;
        std::vector<float> onsetTimes;  // Beat detection
        
        // Processing options
        bool timeStretch = false;
        bool pitchCorrection = false;
        float formantCorrection = 0.0f;
    };
    
    void startSampling(int bufferLengthSeconds = 10);
    void stopSampling();
    void triggerSamplePlayback(int sampleIndex, int midiNote, float velocity);
    
    // Real-time resampling
    void resampleToKey(LiveSample& sample, const std::string& targetKey);
    void resampleToTempo(LiveSample& sample, double targetTempo);
    
    // Auto-slicing
    void autoSliceSample(LiveSample& sample, int numSlices);
    std::vector<LiveSample> sliceSampleByOnsets(const LiveSample& sample);
    
private:
    static constexpr int MAX_SAMPLES = 32;
    std::array<LiveSample, MAX_SAMPLES> samples;
    
    bool isSampling = false;
    int currentSampleIndex = 0;
    
    // Analysis engines
    std::unique_ptr<PitchDetector> pitchDetector;
    std::unique_ptr<TempoDetector> tempoDetector;
    std::unique_ptr<OnsetDetector> onsetDetector;
    
    void analyzeSample(LiveSample& sample);
    void applyTimeStretching(LiveSample& sample, double factor);
};
```

## Implementation Timeline

### Sprint 1: Core Performance Engine (Weeks 1-2)

- Performance engine architecture
- Basic arpeggiator implementation
- Macro controller system
- MIDI controller integration

### Sprint 2: Sequencer and Looper (Weeks 3-4)

- Advanced step sequencer
- Live looper system
- Pattern and loop management
- Synchronization systems

### Sprint 3: Scene Management (Weeks 5-6)

- Scene management system
- Parameter morphing
- Performance transitions
- Hardware controller profiles

### Sprint 4: Advanced Features (Weeks 7-8)

- Performance recorder
- Live sampling system
- OSC integration
- Final optimization and testing

## Quality Assurance

### Performance Testing

```cpp
class PerformanceTest : public ::testing::Test
{
public:
    void TestArpeggiatorTiming();
    void TestSequencerAccuracy();
    void TestLooperSynchronization();
    void TestMacroControllerResponse();
    void TestSceneTransitions();
    void TestMIDIControllerLatency();
    void TestOverallPerformanceLatency();
    
protected:
    std::unique_ptr<PerformanceEngine> performanceEngine;
    MockAudioDevice audioDevice;
    MockMIDIDevice midiDevice;
};
```

### Timing and Latency Requirements

- **Arpeggiator Timing**: ±1ms accuracy at all tempos
- **Sequencer Accuracy**: Sample-accurate step timing
- **MIDI Controller Latency**: < 5ms from input to audio change
- **Scene Transitions**: Smooth morphing without audio dropouts
- **Loop Recording**: Frame-accurate loop points

## Success Criteria

### Must Have

✅ Professional arpeggiator with multiple modes and swing
✅ Advanced step sequencer with per-step editing
✅ Live looper with multiple synchronized loops
✅ Macro control system with MIDI learning

### Should Have

✅ Scene management with smooth transitions
✅ Hardware controller integration
✅ Performance recording and playback
✅ Real-time parameter automation

### Could Have

✅ Live sampling and auto-slicing
✅ OSC integration for tablet control
✅ AI-powered performance suggestions
✅ Advanced timing and groove templates

---

**Estimated Timeline**: 8 weeks  
**Team Size**: 3-4 developers (real-time systems expertise required)  
**Dependencies**: Phase 1-9 completion  
**Risk Level**: High (timing-critical systems, hardware integration complexity)
