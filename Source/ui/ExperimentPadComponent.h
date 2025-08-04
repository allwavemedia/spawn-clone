#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

//==============================================================================
/**
 * SPAWN-style Experiment Pad - XY controller for real-time pitch and effects morphing
 * 
 * The Experiment Pad is SPAWN's signature feature that allows real-time manipulation
 * of generated patterns through an intuitive XY interface:
 * 
 * - Y-axis: Global pitch shifting (-12 to +12 semitones)
 * - X-axis: Effects morphing with configurable parameters
 * 
 * Features:
 * - Smooth gesture control with visual feedback
 * - Modifier key support for single-axis control
 * - Preset save/recall for pad configurations
 * - Real-time parameter updates without audio dropouts
 */
class ExperimentPadComponent : public juce::Component,
                              public juce::Timer
{
public:
    //==============================================================================
    ExperimentPadComponent();
    ~ExperimentPadComponent() override;

    //==============================================================================
    // Component interface
    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;
    void mouseEnter(const juce::MouseEvent& event) override;
    void mouseExit(const juce::MouseEvent& event) override;
    
    // Timer interface for smooth parameter updates
    void timerCallback() override;

    //==============================================================================
    // Parameter control
    struct PadPosition
    {
        float x = 0.5f;  // 0.0 to 1.0 (left to right)
        float y = 0.5f;  // 0.0 to 1.0 (bottom to top)
    };
    
    void setPadPosition(const PadPosition& position);
    PadPosition getPadPosition() const { return currentPosition; }
    
    // Pitch control (Y-axis)
    void setPitchRange(float minSemitones, float maxSemitones);
    float getCurrentPitchShift() const; // Returns semitones shift
    
    // Effects morphing (X-axis)
    struct EffectMapping
    {
        juce::String effectName;
        float minValue = 0.0f;
        float maxValue = 1.0f;
        float currentValue = 0.5f;
    };
    
    void addEffectMapping(const juce::String& effectName, float minValue, float maxValue);
    void clearEffectMappings();
    std::vector<EffectMapping> getCurrentEffectValues() const;
    
    // Preset management
    struct PadPreset
    {
        juce::String name;
        PadPosition position;
        std::vector<EffectMapping> effectMappings;
        float pitchMin = -12.0f;
        float pitchMax = 12.0f;
    };
    
    void savePreset(const juce::String& name);
    void loadPreset(const PadPreset& preset);
    void setPresetCallback(std::function<void(const PadPreset&)> callback);
    
    //==============================================================================
    // Visual configuration
    void setShowGridLines(bool shouldShow) { showGridLines = shouldShow; repaint(); }
    void setShowParameterLabels(bool shouldShow) { showParameterLabels = shouldShow; repaint(); }
    void setPadColors(juce::Colour background, juce::Colour control, juce::Colour accent);
    
    //==============================================================================
    // Parameter update callbacks
    std::function<void(float)> onPitchChanged;
    std::function<void(const std::vector<EffectMapping>&)> onEffectsChanged;
    std::function<void(const PadPosition&)> onPositionChanged;

private:
    //==============================================================================
    // Internal state
    PadPosition currentPosition { 0.5f, 0.5f };
    PadPosition targetPosition { 0.5f, 0.5f };
    
    bool isDragging = false;
    bool isHovering = false;
    bool constrainToXAxis = false;
    bool constrainToYAxis = false;
    
    // Pitch control
    float pitchMinSemitones = -12.0f;
    float pitchMaxSemitones = 12.0f;
    
    // Effects mappings
    std::vector<EffectMapping> effectMappings;
    
    // Visual settings
    bool showGridLines = true;
    bool showParameterLabels = true;
    juce::Colour backgroundColour = juce::Colour(0xff1a1a1a);
    juce::Colour controlColour = juce::Colour(0xff4a90e2);
    juce::Colour accentColour = juce::Colour(0xff00ff88);
    
    // Preset management
    std::function<void(const PadPreset&)> presetCallback;
    
    //==============================================================================
    // Helper methods
    juce::Point<float> positionToPixels(const PadPosition& position) const;
    PadPosition pixelsToPosition(const juce::Point<float>& pixels) const;
    void updateParametersFromPosition();
    void smoothPositionUpdate();
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ExperimentPadComponent)
};
