#include "ExperimentPadComponent.h"

//==============================================================================
ExperimentPadComponent::ExperimentPadComponent()
{
    // Enable mouse events
    setMouseClickGrabsKeyboardFocus(true);
    
    // Start timer for smooth parameter updates
    startTimer(16); // ~60 FPS for smooth visual feedback
    
    // Set default effect mappings (typical SPAWN-style effects)
    addEffectMapping("Filter Cutoff", 200.0f, 8000.0f);
    addEffectMapping("Reverb Mix", 0.0f, 0.8f);
    addEffectMapping("Delay Feedback", 0.0f, 0.6f);
}

ExperimentPadComponent::~ExperimentPadComponent()
{
    stopTimer();
}

//==============================================================================
void ExperimentPadComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Background with subtle gradient
    juce::ColourGradient backgroundGradient(
        backgroundColour.brighter(0.1f), 0, 0,
        backgroundColour.darker(0.2f), 0, bounds.getHeight(), false
    );
    g.setGradientFill(backgroundGradient);
    g.fillRoundedRectangle(bounds, 8.0f);
    
    // Border
    g.setColour(backgroundColour.brighter(0.3f));
    g.drawRoundedRectangle(bounds.reduced(1.0f), 8.0f, 2.0f);
    
    // Grid lines
    if (showGridLines)
    {
        g.setColour(backgroundColour.brighter(0.2f));
        
        // Vertical grid lines
        for (int i = 1; i < 4; ++i)
        {
            float x = bounds.getX() + (bounds.getWidth() * i / 4.0f);
            g.drawVerticalLine(juce::roundToInt(x), bounds.getY() + 10, bounds.getBottom() - 10);
        }
        
        // Horizontal grid lines  
        for (int i = 1; i < 4; ++i)
        {
            float y = bounds.getY() + (bounds.getHeight() * i / 4.0f);
            g.drawHorizontalLine(juce::roundToInt(y), bounds.getX() + 10, bounds.getRight() - 10);
        }
        
        // Center cross
        g.setColour(backgroundColour.brighter(0.4f));
        float centerX = bounds.getCentreX();
        float centerY = bounds.getCentreY();
        g.drawVerticalLine(juce::roundToInt(centerX), bounds.getY() + 5, bounds.getBottom() - 5);
        g.drawHorizontalLine(juce::roundToInt(centerY), bounds.getX() + 5, bounds.getRight() - 5);
    }
    
    // Parameter labels
    if (showParameterLabels)
    {
        g.setColour(juce::Colours::white.withAlpha(0.7f));
        g.setFont(juce::FontOptions("Arial", "Regular", 11.0f));
        
        // Y-axis label (Pitch)
        float currentPitch = getCurrentPitchShift();
        juce::String pitchText = juce::String(currentPitch, 1) + " ST";
        g.drawText(pitchText, bounds.getX() + 5, bounds.getY() + 5, 60, 20, 
                   juce::Justification::topLeft);
        
        // X-axis labels (Effects)
        if (!effectMappings.empty())
        {
            juce::String effectsText = "Effects: ";
            for (size_t i = 0; i < effectMappings.size() && i < 2; ++i)
            {
                if (i > 0) effectsText += ", ";
                effectsText += effectMappings[i].effectName + " (" + 
                              juce::String(effectMappings[i].currentValue, 2) + ")";
            }
            
            g.drawText(effectsText, bounds.getX() + 5, bounds.getBottom() - 25, 
                      bounds.getWidth() - 10, 20, juce::Justification::bottomLeft);
        }
    }
    
    // Control point
    auto controlPosition = positionToPixels(currentPosition);
    float controlRadius = isDragging ? 12.0f : (isHovering ? 10.0f : 8.0f);
    
    // Control point shadow
    g.setColour(juce::Colours::black.withAlpha(0.3f));
    g.fillEllipse(controlPosition.x - controlRadius + 2, controlPosition.y - controlRadius + 2, 
                  controlRadius * 2, controlRadius * 2);
    
    // Control point with gradient
    juce::ColourGradient controlGradient(
        controlColour.brighter(0.3f), controlPosition.x, controlPosition.y - controlRadius,
        controlColour.darker(0.2f), controlPosition.x, controlPosition.y + controlRadius, false
    );
    g.setGradientFill(controlGradient);
    g.fillEllipse(controlPosition.x - controlRadius, controlPosition.y - controlRadius, 
                  controlRadius * 2, controlRadius * 2);
    
    // Control point border
    g.setColour(accentColour);
    g.drawEllipse(controlPosition.x - controlRadius, controlPosition.y - controlRadius, 
                  controlRadius * 2, controlRadius * 2, 2.0f);
    
    // Modifier key indicators
    if (constrainToXAxis)
    {
        g.setColour(accentColour.withAlpha(0.5f));
        g.drawHorizontalLine(juce::roundToInt(controlPosition.y), bounds.getX(), bounds.getRight());
    }
    else if (constrainToYAxis)
    {
        g.setColour(accentColour.withAlpha(0.5f));
        g.drawVerticalLine(juce::roundToInt(controlPosition.x), bounds.getY(), bounds.getBottom());
    }
}

void ExperimentPadComponent::resized()
{
    // Nothing specific needed for resize
}

//==============================================================================
void ExperimentPadComponent::mouseDown(const juce::MouseEvent& event)
{
    if (event.mods.isLeftButtonDown())
    {
        isDragging = true;
        constrainToXAxis = event.mods.isShiftDown();
        constrainToYAxis = event.mods.isCtrlDown() || event.mods.isCommandDown();
        
        // Update position immediately
        auto newPosition = pixelsToPosition(event.position);
        
        if (constrainToXAxis)
            newPosition.y = currentPosition.y;
        else if (constrainToYAxis)
            newPosition.x = currentPosition.x;
            
        setPadPosition(newPosition);
        repaint();
    }
}

void ExperimentPadComponent::mouseDrag(const juce::MouseEvent& event)
{
    if (isDragging)
    {
        constrainToXAxis = event.mods.isShiftDown();
        constrainToYAxis = event.mods.isCtrlDown() || event.mods.isCommandDown();
        
        auto newPosition = pixelsToPosition(event.position);
        
        if (constrainToXAxis)
            newPosition.y = currentPosition.y;
        else if (constrainToYAxis)
            newPosition.x = currentPosition.x;
            
        setPadPosition(newPosition);
        repaint();
    }
}

void ExperimentPadComponent::mouseUp(const juce::MouseEvent& event)
{
    if (isDragging)
    {
        isDragging = false;
        constrainToXAxis = false;
        constrainToYAxis = false;
        repaint();
    }
}

void ExperimentPadComponent::mouseEnter(const juce::MouseEvent& event)
{
    isHovering = true;
    repaint();
}

void ExperimentPadComponent::mouseExit(const juce::MouseEvent& event)
{
    isHovering = false;
    repaint();
}

//==============================================================================
void ExperimentPadComponent::timerCallback()
{
    smoothPositionUpdate();
}

//==============================================================================
void ExperimentPadComponent::setPadPosition(const PadPosition& position)
{
    // Clamp position to valid range
    PadPosition clampedPosition;
    clampedPosition.x = juce::jlimit(0.0f, 1.0f, position.x);
    clampedPosition.y = juce::jlimit(0.0f, 1.0f, position.y);
    
    if (currentPosition.x != clampedPosition.x || currentPosition.y != clampedPosition.y)
    {
        currentPosition = clampedPosition;
        targetPosition = clampedPosition;
        updateParametersFromPosition();
        
        // Notify callbacks
        if (onPositionChanged)
            onPositionChanged(currentPosition);
    }
}

float ExperimentPadComponent::getCurrentPitchShift() const
{
    // Map Y position (0.0 = bottom = min pitch, 1.0 = top = max pitch)
    return pitchMinSemitones + (currentPosition.y * (pitchMaxSemitones - pitchMinSemitones));
}

void ExperimentPadComponent::setPitchRange(float minSemitones, float maxSemitones)
{
    pitchMinSemitones = minSemitones;
    pitchMaxSemitones = maxSemitones;
    updateParametersFromPosition();
}

void ExperimentPadComponent::addEffectMapping(const juce::String& effectName, float minValue, float maxValue)
{
    EffectMapping mapping;
    mapping.effectName = effectName;
    mapping.minValue = minValue;
    mapping.maxValue = maxValue;
    mapping.currentValue = minValue + (currentPosition.x * (maxValue - minValue));
    
    effectMappings.push_back(mapping);
    updateParametersFromPosition();
}

void ExperimentPadComponent::clearEffectMappings()
{
    effectMappings.clear();
}

std::vector<ExperimentPadComponent::EffectMapping> ExperimentPadComponent::getCurrentEffectValues() const
{
    return effectMappings;
}

//==============================================================================
void ExperimentPadComponent::savePreset(const juce::String& name)
{
    PadPreset preset;
    preset.name = name;
    preset.position = currentPosition;
    preset.effectMappings = effectMappings;
    preset.pitchMin = pitchMinSemitones;
    preset.pitchMax = pitchMaxSemitones;
    
    if (presetCallback)
        presetCallback(preset);
}

void ExperimentPadComponent::loadPreset(const PadPreset& preset)
{
    setPitchRange(preset.pitchMin, preset.pitchMax);
    effectMappings = preset.effectMappings;
    setPadPosition(preset.position);
    repaint();
}

void ExperimentPadComponent::setPresetCallback(std::function<void(const PadPreset&)> callback)
{
    presetCallback = callback;
}

//==============================================================================
void ExperimentPadComponent::setPadColors(juce::Colour background, juce::Colour control, juce::Colour accent)
{
    backgroundColour = background;
    controlColour = control;
    accentColour = accent;
    repaint();
}

//==============================================================================
juce::Point<float> ExperimentPadComponent::positionToPixels(const PadPosition& position) const
{
    auto bounds = getLocalBounds().toFloat().reduced(10.0f);
    
    float x = bounds.getX() + (position.x * bounds.getWidth());
    float y = bounds.getBottom() - (position.y * bounds.getHeight()); // Flip Y for intuitive control
    
    return { x, y };
}

ExperimentPadComponent::PadPosition ExperimentPadComponent::pixelsToPosition(const juce::Point<float>& pixels) const
{
    auto bounds = getLocalBounds().toFloat().reduced(10.0f);
    
    PadPosition position;
    position.x = (pixels.x - bounds.getX()) / bounds.getWidth();
    position.y = (bounds.getBottom() - pixels.y) / bounds.getHeight(); // Flip Y for intuitive control
    
    // Clamp to valid range
    position.x = juce::jlimit(0.0f, 1.0f, position.x);
    position.y = juce::jlimit(0.0f, 1.0f, position.y);
    
    return position;
}

void ExperimentPadComponent::updateParametersFromPosition()
{
    // Update pitch
    if (onPitchChanged)
        onPitchChanged(getCurrentPitchShift());
    
    // Update effects based on X position
    for (auto& mapping : effectMappings)
    {
        mapping.currentValue = mapping.minValue + (currentPosition.x * (mapping.maxValue - mapping.minValue));
    }
    
    if (onEffectsChanged)
        onEffectsChanged(effectMappings);
    
    repaint();
}

void ExperimentPadComponent::smoothPositionUpdate()
{
    // Currently no smoothing - could add spring physics here for enhanced feel
    // This timer callback is available for future smooth animation features
}
