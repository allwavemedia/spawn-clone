//==============================================================================
/*
    LivePerformanceControlPanel.h
    
    Revolutionary Live Performance Control Interface for SpawnClone
    Provides user access to next-generation real-time AI capabilities
    that exceed the original SPAWN application.
*/
//==============================================================================

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "../ai/LivePerformanceEngine.h"

class SpawnCloneAudioProcessor;

namespace spawnclone::ui
{

/**
 * Professional Live Performance Control Panel
 * 
 * This component provides user access to SpawnClone's revolutionary 
 * live performance capabilities, including:
 * - Real-time AI pattern generation during playback
 * - AI-driven synthesis parameter modulation  
 * - Pattern evolution and morphing
 * - Multiple performance modes and trigger systems
 */
class LivePerformanceControlPanel : public juce::Component,
                                  public juce::Button::Listener,
                                  public juce::Slider::Listener,
                                  public juce::ComboBox::Listener,
                                  public juce::Timer
{
public:
    //==============================================================================
    LivePerformanceControlPanel(SpawnCloneAudioProcessor& processor);
    ~LivePerformanceControlPanel() override;

    //==============================================================================
    void paint(juce::Graphics& g) override;
    void resized() override;

    //==============================================================================
    // Component interaction callbacks
    void buttonClicked(juce::Button* button) override;
    void sliderValueChanged(juce::Slider* slider) override;
    void comboBoxChanged(juce::ComboBox* comboBox) override;
    void timerCallback() override;

    //==============================================================================
    // Public interface for external updates
    void updatePerformanceStatus();
    void setEnabled(bool shouldBeEnabled);

private:
    //==============================================================================
    // UI Layout and Styling
    void setupLivePerformanceControls();
    void setupPerformanceModeControls();
    void setupAIModulationControls();
    void setupTriggerControls();
    void setupStatusDisplay();
    void setupStyling();
    
    void paintBackground(juce::Graphics& g);
    void paintStatusIndicators(juce::Graphics& g);
    void paintPerformanceMetrics(juce::Graphics& g);

    //==============================================================================
    // Control Logic
    void enableLivePerformance(bool enabled);
    void updatePerformanceMode();
    void configureAIModulation();
    void triggerManualGeneration();
    void updateStatusIndicators();
    void updatePerformanceMetrics();

    //==============================================================================
    // Reference to processor
    SpawnCloneAudioProcessor& audioProcessor;

    //==============================================================================
    // Main Controls Section
    juce::GroupComponent mainControlsGroup;
    
    // Enable/Disable Control
    juce::ToggleButton livePerformanceToggle;
    juce::Label livePerformanceLabel;
    
    // Performance Mode Selection
    juce::Label performanceModeLabel;
    juce::ComboBox performanceModeBox;
    
    // AI Modulation Control
    juce::GroupComponent aiModulationGroup;
    juce::ToggleButton aiModulationToggle;
    juce::Label aiIntensityLabel;
    juce::Slider aiIntensitySlider;
    
    //==============================================================================
    // Trigger Controls Section
    juce::GroupComponent triggerControlsGroup;
    
    // Manual Generation Trigger
    juce::TextButton manualTriggerButton;
    juce::Label lastTriggerTimeLabel;
    
    // Trigger Configuration
    juce::Label triggerTypeLabel;
    juce::ComboBox triggerTypeBox;
    juce::Label triggerProbabilityLabel;
    juce::Slider triggerProbabilitySlider;
    
    //==============================================================================
    // Status and Metrics Section
    juce::GroupComponent statusGroup;
    
    // Live Performance Status
    juce::Label statusLabel;
    juce::Label modeStatusLabel;
    
    // Real-time Metrics
    juce::Label generationTimeLabel;
    juce::Label aiActivityLabel;
    juce::ProgressBar aiActivityIndicator;
    
    // Pattern Evolution Status
    juce::Label evolutionStatusLabel;
    juce::ProgressBar evolutionProgressBar;
    
    //==============================================================================
    // Advanced Controls (Collapsible)
    juce::TextButton advancedToggleButton;
    juce::Component advancedControlsContainer;
    bool advancedControlsVisible = false;
    
    // Pattern Evolution Controls
    juce::Label evolutionRateLabel;
    juce::Slider evolutionRateSlider;
    juce::ToggleButton evolutionEnabledToggle;
    
    // Modulation Targets
    juce::Label modulationTargetsLabel;
    juce::ToggleButton filterModulationToggle;
    juce::ToggleButton wavetableModulationToggle;
    juce::ToggleButton lfoModulationToggle;
    juce::ToggleButton effectsModulationToggle;
    
    //==============================================================================
    // Visual Feedback and Animation
    struct StatusLED
    {
        juce::Colour colour = juce::Colours::grey;
        float brightness = 0.3f;
        bool isActive = false;
        juce::String tooltip;
    };
    
    StatusLED livePerformanceLED;
    StatusLED aiGenerationLED;
    StatusLED parameterModulationLED;
    StatusLED patternEvolutionLED;
    
    //==============================================================================
    // Performance Metrics Tracking
    struct PerformanceMetrics
    {
        double lastGenerationTime = 0.0;
        int totalGenerations = 0;
        double averageGenerationTime = 0.0;
        bool aiModulationActive = false;
        float currentEvolutionProgress = 0.0f;
        juce::String currentMode = "Disabled";
    };
    
    PerformanceMetrics metrics;
    
    // Progress tracking for UI components
    double aiActivityProgress = 0.0;
    double evolutionProgress = 0.0;
    
    //==============================================================================
    // Styling and Colors
    struct Styling
    {
        juce::Colour backgroundColour = juce::Colour(0xff2a2a2a);
        juce::Colour panelColour = juce::Colour(0xff3a3a3a);
        juce::Colour textColour = juce::Colour(0xffffffff);
        juce::Colour accentColour = juce::Colour(0xff4a9eff);
        juce::Colour activeColour = juce::Colour(0xff00ff88);
        juce::Colour warningColour = juce::Colour(0xffff6600);
        juce::Colour borderColour = juce::Colour(0xff555555);
        
        float cornerRadius = 6.0f;
        float borderWidth = 1.0f;
        int margin = 8;
        int padding = 6;
    } styling;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LivePerformanceControlPanel)
};

} // namespace spawnclone::ui
