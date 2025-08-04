#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_core/juce_core.h>
#include <memory>

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
    enum class SynthesisType
    {
        Wavetable = 0,
        Subtractive = 1,
        Sample = 2,
        Hybrid = 3
    };
    
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
        enum Type { LowPass = 0, HighPass = 1, BandPass = 2, Notch = 3 };
        Type filterType = LowPass;
        float cutoff = 1000.0f;            // 20Hz to 20kHz
        float resonance = 0.1f;            // 0.0 to 1.0
        float keyTracking = 0.5f;          // 0.0 to 1.0
        bool selfOscillation = false;      // Enable filter self-oscillation
        bool enabled = false;              // Filter enabled/disabled (start disabled for safety)
    };
    
    struct ModulationParams
    {
        float lfoRate = 2.0f;              // 0.01 to 20.0 Hz
        float lfoDepth = 0.0f;             // 0.0 to 1.0
        int lfoWaveform = 0;               // 0=Sine, 1=Triangle, 2=Sawtooth, 3=Square, 4=Random
        int modulationTarget = 0;          // 0=None, 1=Pitch, 2=Filter, 3=Amplitude, 4=Wavetable Position
        bool bipolar = false;              // Bipolar (-1 to +1) vs unipolar (0 to +1) modulation
    };
    
    struct EnvelopeParams
    {
        float attack = 0.001f;             // 0.001 to 5.0 seconds (faster default for immediate response)
        float decay = 0.1f;                // 0.001 to 5.0 seconds
        float sustain = 1.0f;              // 0.0 to 1.0 level (full sustain for immediate audio)
        float release = 0.3f;              // 0.001 to 10.0 seconds
        float velocity = 1.0f;             // 0.0 to 2.0 velocity sensitivity
    };
    
    //==============================================================================
    struct SynthesisParameters
    {
        SynthesisType synthesisType = SynthesisType::Wavetable;
        WavetableParams wavetable;
        FilterParams filter;
        ModulationParams modulation;
        EnvelopeParams envelope;
        
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
    class ModulationSource;
    class SynthVoice;
    
    //==============================================================================
    // Internal Methods
    
    /** Find available voice for new note */
    SynthVoice* findAvailableVoice();
    
    /** Initialize built-in wavetables */
    void initializeWavetables();
    
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
    
    // Wavetable storage
    std::vector<juce::AudioBuffer<float>> wavetables;
    
    // Voice management
    std::array<std::unique_ptr<SynthVoice>, 16> voices;
    
    // Performance monitoring
    juce::Time lastCPUMeasurement;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AdvancedSynthesisEngine)
};
