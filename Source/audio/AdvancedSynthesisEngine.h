#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_core/juce_core.h>
#include <memory>
#include "SampleCache.h"
#include "AsyncSampleLoader.h"
#include "SynthesisTypes.h"

namespace spawnclone::audio
{
    // Forward declaration
    class SynthVoice;

//==============================================================================
/**
 * Epic 9.2 Story 9.2.1: Advanced Synthesis Engine - Wavetable Synthesis
 * 
 * Professional wavetable synthesis engine with real-time morphing,
 * high-quality interpolation, and anti-aliasing for commercial-grade audio quality.
 */
class AdvancedSynthesisEngine
{
public:
    //==============================================================================
    struct WavetableParams
    {
        float wavetablePosition = 0.5f;    // 0.0 to 1.0 (position in wavetable)
        float morphRate = 0.1f;            // 0.01 to 10.0 Hz (automatic morphing rate)
        int wavetableIndex = 0;            // Wavetable selection index
        float morphAmount = 0.0f;          // 0.0 to 1.0 (morph blend amount)
        bool enableMorphing = false;       // Enable automatic morphing
    };
    
    struct FilterParams
    {
        enum Type { LowPass = 0, HighPass = 1, BandPass = 2, Notch = 3, MoogLadder = 4, StateVariable = 5 };
        enum Routing { Serial = 0, Parallel = 1, Split = 2 };
        
        Type filterType = LowPass;
        Routing routing = Serial;
        float cutoff = 1000.0f;            // 20Hz to 20kHz
        float resonance = 0.1f;            // 0.0 to 1.0
        float keyTracking = 0.5f;          // 0.0 to 1.0 (keyboard follow)
        float velocityTracking = 0.0f;     // 0.0 to 1.0 (velocity sensitivity)
        bool selfOscillation = false;      // Enable filter self-oscillation
        bool enabled = false;              // Filter enabled/disabled (start disabled for safety)
        
        // Dual filter parameters (for parallel/split routing)
        Type secondaryFilterType = HighPass;
        float secondaryCutoff = 5000.0f;   // Secondary filter cutoff
        float secondaryResonance = 0.1f;   // Secondary filter resonance
        float filterBalance = 0.5f;        // 0.0=primary only, 1.0=secondary only
    };
    
    struct ModulationParams
    {
        // LFO 1 Parameters
        float lfoRate = 2.0f;              // 0.01 to 20.0 Hz
        float lfoDepth = 0.0f;             // 0.0 to 1.0
        int lfoWaveform = 0;               // 0=Sine, 1=Triangle, 2=Sawtooth, 3=Square, 4=Random
        int modulationTarget = 0;          // 0=None, 1=Pitch, 2=Filter, 3=Amplitude, 4=Wavetable Position
        bool bipolar = false;              // Bipolar (-1 to +1) vs unipolar (0 to +1) modulation
        
        // LFO 2 Parameters (additional modulation source)
        float lfo2Rate = 0.5f;             // 0.01 to 20.0 Hz
        float lfo2Depth = 0.0f;            // 0.0 to 1.0
        int lfo2Waveform = 1;              // Triangle by default
        int lfo2Target = 0;                // Secondary modulation target
        bool lfo2Bipolar = true;           // LFO2 bipolar by default
        
        // Envelope 2 Parameters (filter envelope)
        float env2Attack = 0.01f;          // 0.001 to 5.0 seconds
        float env2Decay = 0.3f;            // 0.001 to 5.0 seconds
        float env2Sustain = 0.3f;          // 0.0 to 1.0 level
        float env2Release = 0.5f;          // 0.001 to 10.0 seconds
        float env2Amount = 0.0f;           // -1.0 to 1.0 (filter envelope amount)
        
        // Modulation Matrix
        bool enableCrossModulation = false; // LFO1 modulates LFO2 rate
        float crossModAmount = 0.0f;       // 0.0 to 1.0
        bool enableEnvFollowing = false;   // Envelope follows note velocity
        float envFollowAmount = 0.5f;      // 0.0 to 1.0
    };
    
    // Make SynthesisType available to this class scope and set up type aliases
    using SynthesisType = spawnclone::audio::SynthesisType;
    using SampleParams = spawnclone::audio::SampleParams;
    using SampleMapEntry = spawnclone::audio::SampleParams::SampleMapEntry;

    struct EnvelopeParams
    {
        float attack = 0.001f;             // 0.001 to 5.0 seconds (faster default for immediate response)
        float decay = 0.1f;                // 0.001 to 5.0 seconds
        float sustain = 1.0f;              // 0.0 to 1.0 level (full sustain for immediate audio)
        float release = 0.3f;              // 0.001 to 10.0 seconds
        float velocity = 1.0f;             // 0.0 to 2.0 velocity sensitivity
        
        // Advanced envelope shapes
        enum Shape { Linear = 0, Exponential = 1, Logarithmic = 2, SCurve = 3 };
        Shape attackShape = Exponential;   // Attack curve shape
        Shape decayShape = Exponential;    // Decay curve shape  
        Shape releaseShape = Exponential;  // Release curve shape
    };
    
    struct UnisonParams
    {
        bool enabled = false;              // Enable/disable unison
        int voiceCount = 2;                // 2 to 8 unison voices
        float detune = 0.1f;               // 0.0 to 1.0 (detune amount in semitones)
        float stereoSpread = 0.5f;         // 0.0 to 1.0 (stereo width)
        float blend = 1.0f;                // 0.0 to 1.0 (dry/wet mix)
        float phaseOffset = 0.0f;          // 0.0 to 1.0 (phase randomization)
        
        // Advanced unison parameters
        float dynamicDetune = 0.0f;        // 0.0 to 1.0 (velocity-sensitive detune)
        float chorusEffect = 0.0f;         // 0.0 to 1.0 (subtle modulation for thickness)
        bool linkToVelocity = false;       // Link voice count to velocity
    };
    
    
    //==============================================================================
    struct SynthesisParameters
    {
        SynthesisType synthesisType = SynthesisType::Wavetable;
        WavetableParams wavetable;
        FilterParams filter;
        ModulationParams modulation;
        EnvelopeParams envelope;
        UnisonParams unison;
        SampleParams sample;
        
        // Global parameters
        float masterVolume = 0.7f;         // 0.0 to 1.0
        float detuneAmount = 0.0f;         // -12.0 to +12.0 semitones
        float pulseWidth = 0.5f;           // 0.1 to 0.9 (for PWM synthesis)
        
        // Character parameters
        float brightness = 0.5f;           // 0.0 to 1.0 (high frequency emphasis)
        float warmth = 0.5f;               // 0.0 to 1.0 (low frequency emphasis)
        float saturation = 0.0f;           // 0.0 to 1.0 (harmonic saturation)
    };
    
    //==============================================================================
    AdvancedSynthesisEngine();
    ~AdvancedSynthesisEngine();

    //==============================================================================
    // Engine Management
    
    /** Prepare the synthesis engine for playback */
    void prepareToPlay(double sampleRate, int samplesPerBlock, int numChannels);
    
    /** Release audio resources */
    void releaseResources();
    
    /** Reset the synthesis engine */
    void reset();
    
    //==============================================================================
    // Parameter Control
    
    /** Set synthesis parameters */
    void setSynthesisParameters(const SynthesisParameters& params);
    
    /** Get current synthesis parameters */
    const SynthesisParameters& getSynthesisParameters() const { return currentParams; }
    
    /** Update single parameter (thread-safe) */
    void setParameter(const juce::String& paramName, float value);
    
    //==============================================================================
    // Voice Management
    
    /** Start a note with velocity */
    void noteOn(int midiNoteNumber, float velocity);
    
        /** Stop a note */
    void noteOff(int midiNoteNumber);
    
    /** Process audio block */
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiBuffer);
    
    //==============================================================================
    // Wavetable Management
    
    /** Load wavetable from audio buffer */
    bool loadWavetable(int index, const juce::AudioBuffer<float>& wavetableData);
    
    /** Get number of available wavetables */
    int getNumWavetables() const { return wavetables.size(); }
    
    /** Generate built-in wavetables */
    void generateBuiltinWavetables();
    
    //==============================================================================
    // Sample Management
    
    /** Load sample from audio buffer */
    bool loadSample(int index, const juce::AudioBuffer<float>& sampleData);
    
    /** Get number of available samples */
    int getNumSamples() const { return samples.size(); }
    
    /** Generate built-in sample library */
    void generateBuiltinSamples();
    
    /** Set sample loop points */
    bool setSampleLoopPoints(int sampleIndex, int loopStart, int loopEnd);
    
    /** Auto-detect loop points in sample */
    bool autoDetectLoopPoints(int sampleIndex);
    
    //==============================================================================
    // Performance Monitoring
    
    /** Get current CPU usage percentage */
    float getCPUUsage() const { return cpuUsage.load(); }
    
    /** Get current voice count */
    int getCurrentVoiceCount() const { return activeVoices.load(); }
    
    /** Get synthesis engine info string */
    juce::String getEngineInfo() const;

private:
    //==============================================================================
    // Internal Classes
    
    class WavetableOscillator;
    class AdvancedFilter;
    class MoogLadderFilter;
    class StateVariableFilter;
    class MultiFilter;
    class ModulationSource;
    class ModulationMatrix;
    class SampleEngine;
    class PitchShifter;
    class LoopManager;
    
    //==============================================================================
    // Internal Methods
    
    /** Find available voice for new note */
    SynthVoice* findAvailableVoice();
    
    /** Initialize built-in wavetables */
    void initializeWavetables();

    /** Initialize built-in samples */
    void initializeSamples();
    
    /** Create basic wavetable shapes */
    void createBasicWavetable(int index, const juce::String& name);
    
    /** Update CPU usage statistics */
    void updateCPUUsage();

    //==============================================================================
    // Member Variables
    
    SynthesisParameters currentParams;
    std::atomic<float> cpuUsage{0.0f};
    std::atomic<int> activeVoices{0};
    std::atomic<bool> isProcessing { false };

    double sampleRate = 44100.0;
    int samplesPerBlock = 512;
    int numChannels = 2;
    
    int currentVoiceIndex = 0;
    
    static constexpr int MAX_WAVETABLES = 64;
    static constexpr int WAVETABLE_SIZE = 2048;
    static constexpr int MAX_SAMPLES = 128;
    static constexpr int MAX_SAMPLE_SIZE = 44100 * 10; // 10 seconds at 44.1kHz
    
    // Wavetable storage
    std::vector<juce::AudioBuffer<float>> wavetables;
    
    // Sample storage
    std::vector<juce::AudioBuffer<float>> samples;
    std::vector<SampleParams> sampleParameters;
    
    // Voice management
    std::array<std::unique_ptr<SynthVoice>, 16> voices;
    
    // Sample caching
    std::unique_ptr<spawnclone::audio::SampleCache> sampleCache;
    std::unique_ptr<spawnclone::audio::AsyncSampleLoader> asyncSampleLoader;
    
    // Performance monitoring
    juce::Time lastCPUMeasurement;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AdvancedSynthesisEngine)
};

} // namespace spawnclone::audio
