#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <vector>
#include <array>
#include <atomic>
#include <functional>

//==============================================================================
/**
 * Epic 9.2 Story 9.1: Real-time Waveform Visualization Component
 * 
 * Advanced waveform display system providing real-time visualization of audio
 * with multiple display modes and interactive editing capabilities.
 * 
 * Features:
 * - Real-time oscilloscope display (60fps)
 * - Multiple visualization modes (Oscilloscope, Spectrum, Phase, Harmonic)
 * - Interactive wavetable editing
 * - GPU-accelerated rendering where possible
 * - Epic 7 integration for AI-generated pattern visualization
 * - Professional-grade visual feedback
 */
class WaveformDisplayComponent : public juce::Component,
                                public juce::Timer,
                                public juce::AudioProcessorValueTreeState::Listener
{
public:
    //==============================================================================
    enum DisplayMode
    {
        Oscilloscope,       // Real-time waveform display
        Spectrum,           // Frequency domain visualization
        Phase,              // Phase relationships
        Harmonic,           // Harmonic content analysis
        Wavetable,          // Interactive wavetable editing
        Lissajous           // XY phase display
    };
    
    enum TriggerMode
    {
        None,               // Free-running display
        Rising,             // Trigger on rising edge
        Falling,            // Trigger on falling edge
        Auto                // Automatic triggering
    };
    
    struct VisualizationSettings
    {
        DisplayMode mode = Oscilloscope;
        TriggerMode triggerMode = Auto;
        float timeScale = 1.0f;              // Time zoom factor
        float amplitudeScale = 1.0f;         // Amplitude zoom factor
        float triggerLevel = 0.0f;           // Trigger threshold
        bool showGrid = true;                // Display grid lines
        bool showLabels = true;              // Show axis labels
        bool enableGlow = true;              // Glow effect for waveforms
        juce::Colour waveformColour = juce::Colours::cyan;
        juce::Colour gridColour = juce::Colour(0x40ffffff);
        juce::Colour backgroundColour = juce::Colour(0xff1a1a1a);
        int refreshRate = 60;                // FPS target
    };

    //==============================================================================
    WaveformDisplayComponent();
    ~WaveformDisplayComponent() override;

    //==============================================================================
    // Component interface
    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override;
    
    // Timer interface for real-time updates
    void timerCallback() override;
    
    // Parameter listener for Epic 7 integration
    void parameterChanged(const juce::String& parameterID, float newValue) override;

    //==============================================================================
    // Audio data interface
    void pushAudioData(const juce::AudioBuffer<float>& buffer);
    void setAudioSource(juce::AudioSource* source) { audioSource = source; }
    
    // Display configuration
    void setDisplayMode(DisplayMode newMode);
    DisplayMode getDisplayMode() const { return settings.mode; }
    
    void setVisualizationSettings(const VisualizationSettings& newSettings);
    VisualizationSettings getVisualizationSettings() const { return settings; }
    
    // Wavetable editing (for Wavetable mode)
    void setWavetableData(const std::vector<float>& wavetable);
    std::vector<float> getWavetableData() const;
    void enableWavetableEditing(bool enable) { isWavetableEditingEnabled = enable; }
    
    // Epic 7 AI integration
    void setAIGenerationCallback(std::function<void(const std::vector<float>&)> callback);
    void visualizeAIPattern(const std::vector<float>& pattern);

    //==============================================================================
    // Performance monitoring
    struct PerformanceStats
    {
        std::atomic<float> actualFPS{0.0f};
        std::atomic<float> renderTime{0.0f};
        std::atomic<int> droppedFrames{0};
        std::atomic<int> bufferUnderruns{0};
        
        // Copy constructor and assignment operator for atomic members
        PerformanceStats() = default;
        PerformanceStats(const PerformanceStats& other) 
            : actualFPS(other.actualFPS.load())
            , renderTime(other.renderTime.load())
            , droppedFrames(other.droppedFrames.load())
            , bufferUnderruns(other.bufferUnderruns.load()) {}
        
        PerformanceStats& operator=(const PerformanceStats& other) {
            if (this != &other) {
                actualFPS.store(other.actualFPS.load());
                renderTime.store(other.renderTime.load());
                droppedFrames.store(other.droppedFrames.load());
                bufferUnderruns.store(other.bufferUnderruns.load());
            }
            return *this;
        }
    };
    
    PerformanceStats getPerformanceStats() const { return performanceStats; }
    void resetPerformanceStats();

private:
    //==============================================================================
    // Display settings
    VisualizationSettings settings;
    
    // Audio data management
    static constexpr int maxSamples = 2048;
    static constexpr int bufferSize = 4096;
    
    std::array<float, maxSamples> waveformBuffer;
    std::array<float, maxSamples> spectrumBuffer;
    std::array<float, maxSamples> phaseBuffer;
    
    juce::AudioBuffer<float> audioRingBuffer;
    std::atomic<int> writePosition{0};
    std::atomic<int> readPosition{0};
    
    // FFT for spectrum analysis
    std::unique_ptr<juce::dsp::FFT> fftProcessor;
    std::array<float, bufferSize> fftBuffer;
    std::array<float, bufferSize> windowBuffer;
    
    // Trigger detection
    float lastSample = 0.0f;
    bool triggerDetected = false;
    int triggerPosition = 0;
    
    // Wavetable editing
    std::vector<float> wavetableData;
    bool isWavetableEditingEnabled = false;
    int selectedWavetablePoint = -1;
    
    // Epic 7 integration
    juce::AudioSource* audioSource = nullptr;
    std::function<void(const std::vector<float>&)> aiGenerationCallback;
    std::vector<float> currentAIPattern;
    
    // Performance tracking
    mutable PerformanceStats performanceStats;
    juce::int64 lastFrameTime = 0;
    int frameCount = 0;
    
    // Rendering optimization
    juce::Image cachedBackground;
    bool backgroundNeedsUpdate = true;
    juce::Rectangle<int> lastBounds;

    //==============================================================================
    // Rendering methods
    void renderOscilloscope(juce::Graphics& g, const juce::Rectangle<float>& bounds);
    void renderSpectrum(juce::Graphics& g, const juce::Rectangle<float>& bounds);
    void renderPhase(juce::Graphics& g, const juce::Rectangle<float>& bounds);
    void renderHarmonic(juce::Graphics& g, const juce::Rectangle<float>& bounds);
    void renderWavetable(juce::Graphics& g, const juce::Rectangle<float>& bounds);
    void renderLissajous(juce::Graphics& g, const juce::Rectangle<float>& bounds);
    
    void drawGrid(juce::Graphics& g, const juce::Rectangle<float>& bounds);
    void drawLabels(juce::Graphics& g, const juce::Rectangle<float>& bounds);
    void drawCursor(juce::Graphics& g, const juce::Rectangle<float>& bounds);
    
    // Data processing
    void updateWaveformData();
    void updateSpectrumData();
    void updatePhaseData();
    void detectTrigger();
    
    // Wavetable editing helpers
    void handleWavetableMouseDown(const juce::MouseEvent& event, const juce::Rectangle<float>& bounds);
    void handleWavetableMouseDrag(const juce::MouseEvent& event, const juce::Rectangle<float>& bounds);
    int findNearestWavetablePoint(const juce::Point<float>& position, const juce::Rectangle<float>& bounds);
    
    // Coordinate conversion
    juce::Point<float> sampleToPixel(int sampleIndex, float value, const juce::Rectangle<float>& bounds) const;
    juce::Point<int> pixelToSample(const juce::Point<float>& pixel, const juce::Rectangle<float>& bounds) const;
    
    // Performance optimization
    void updatePerformanceStats();
    bool shouldSkipFrame() const;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformDisplayComponent)
};
