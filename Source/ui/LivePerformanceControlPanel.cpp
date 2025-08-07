//==============================================================================
/*
    LivePerformanceControlPanel.cpp
    
    Implementation of revolutionary Live Performance Control Interface
*/
//==============================================================================

#include "LivePerformanceControlPanel.h"
#include "../PluginProcessor.h"

namespace spawnclone::ui
{

//==============================================================================
LivePerformanceControlPanel::LivePerformanceControlPanel(SpawnCloneAudioProcessor& processor)
    : audioProcessor(processor)
    , aiActivityIndicator(aiActivityProgress)
    , evolutionProgressBar(evolutionProgress)
{
    setupLivePerformanceControls();
    setupPerformanceModeControls();
    setupAIModulationControls();
    setupTriggerControls();
    setupStatusDisplay();
    setupStyling();
    
    // Start timer for real-time updates
    startTimer(50); // 20fps for smooth visual feedback
}

LivePerformanceControlPanel::~LivePerformanceControlPanel()
{
    stopTimer();
}

//==============================================================================
void LivePerformanceControlPanel::paint(juce::Graphics& g)
{
    paintBackground(g);
    paintStatusIndicators(g);
    paintPerformanceMetrics(g);
}

void LivePerformanceControlPanel::resized()
{
    auto bounds = getLocalBounds();
    
    // Header section with title and main toggle
    auto headerBounds = bounds.removeFromTop(60);
    headerBounds.reduce(styling.margin, styling.margin);
    
    auto titleBounds = headerBounds.removeFromLeft(200);
    livePerformanceLabel.setBounds(titleBounds.removeFromTop(25));
    livePerformanceToggle.setBounds(titleBounds);
    
    // Status LEDs in header
    auto ledBounds = headerBounds.removeFromRight(200);
    auto ledWidth = ledBounds.getWidth() / 4;
    // LED positions will be painted in paintStatusIndicators
    
    bounds.removeFromTop(styling.margin);
    
    // Main content area
    auto contentBounds = bounds;
    contentBounds.reduce(styling.margin, 0);
    
    // Left column - Performance Mode and AI Modulation
    auto leftColumn = contentBounds.removeFromLeft(250);
    
    // Performance Mode Section
    performanceModeLabel.setBounds(leftColumn.removeFromTop(20));
    performanceModeBox.setBounds(leftColumn.removeFromTop(25));
    leftColumn.removeFromTop(styling.margin);
    
    // AI Modulation Group
    auto aiModBounds = leftColumn.removeFromTop(120);
    aiModulationGroup.setBounds(aiModBounds);
    aiModBounds.reduce(styling.padding, 20);
    
    aiModulationToggle.setBounds(aiModBounds.removeFromTop(25));
    aiModBounds.removeFromTop(5);
    aiIntensityLabel.setBounds(aiModBounds.removeFromTop(20));
    aiIntensitySlider.setBounds(aiModBounds.removeFromTop(25));
    
    contentBounds.removeFromLeft(styling.margin);
    
    // Center column - Trigger Controls
    auto centerColumn = contentBounds.removeFromLeft(200);
    
    triggerControlsGroup.setBounds(centerColumn.removeFromTop(150));
    auto triggerBounds = centerColumn.removeFromTop(150);
    triggerBounds.reduce(styling.padding, 20);
    
    manualTriggerButton.setBounds(triggerBounds.removeFromTop(30));
    triggerBounds.removeFromTop(styling.margin);
    
    triggerTypeLabel.setBounds(triggerBounds.removeFromTop(20));
    triggerTypeBox.setBounds(triggerBounds.removeFromTop(25));
    triggerBounds.removeFromTop(5);
    
    triggerProbabilityLabel.setBounds(triggerBounds.removeFromTop(20));
    triggerProbabilitySlider.setBounds(triggerBounds.removeFromTop(25));
    
    contentBounds.removeFromLeft(styling.margin);
    
    // Right column - Status and Metrics
    auto rightColumn = contentBounds;
    
    statusGroup.setBounds(rightColumn.removeFromTop(180));
    auto statusBounds = rightColumn.removeFromTop(180);
    statusBounds.reduce(styling.padding, 20);
    
    statusLabel.setBounds(statusBounds.removeFromTop(20));
    modeStatusLabel.setBounds(statusBounds.removeFromTop(20));
    statusBounds.removeFromTop(styling.margin);
    
    generationTimeLabel.setBounds(statusBounds.removeFromTop(20));
    aiActivityLabel.setBounds(statusBounds.removeFromTop(20));
    aiActivityIndicator.setBounds(statusBounds.removeFromTop(8));
    statusBounds.removeFromTop(styling.margin);
    
    evolutionStatusLabel.setBounds(statusBounds.removeFromTop(20));
    evolutionProgressBar.setBounds(statusBounds.removeFromTop(8));
    
    // Advanced controls (if visible)
    if (advancedControlsVisible)
    {
        auto advancedBounds = rightColumn.removeFromTop(120);
        advancedControlsContainer.setBounds(advancedBounds);
        // Layout advanced controls within container
    }
    
    advancedToggleButton.setBounds(rightColumn.removeFromTop(25));
}

//==============================================================================
void LivePerformanceControlPanel::buttonClicked(juce::Button* button)
{
    if (button == &livePerformanceToggle)
    {
        enableLivePerformance(livePerformanceToggle.getToggleState());
    }
    else if (button == &manualTriggerButton)
    {
        triggerManualGeneration();
    }
    else if (button == &aiModulationToggle)
    {
        configureAIModulation();
    }
    else if (button == &advancedToggleButton)
    {
        advancedControlsVisible = !advancedControlsVisible;
        advancedControlsContainer.setVisible(advancedControlsVisible);
        advancedToggleButton.setButtonText(advancedControlsVisible ? "Hide Advanced" : "Show Advanced");
        repaint();
    }
}

void LivePerformanceControlPanel::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &aiIntensitySlider)
    {
        configureAIModulation();
    }
    else if (slider == &triggerProbabilitySlider)
    {
        // Update trigger probability
        auto* liveIntegration = audioProcessor.getLivePerformanceIntegration();
        if (liveIntegration)
        {
            // Configure probability-based triggers
            // This would be implemented in the live integration
        }
    }
}

void LivePerformanceControlPanel::comboBoxChanged(juce::ComboBox* comboBox)
{
    if (comboBox == &performanceModeBox)
    {
        updatePerformanceMode();
    }
    else if (comboBox == &triggerTypeBox)
    {
        // Update trigger type configuration
        // Implementation would depend on trigger system design
    }
}

void LivePerformanceControlPanel::timerCallback()
{
    updateStatusIndicators();
    updatePerformanceMetrics();
}

//==============================================================================
void LivePerformanceControlPanel::setupLivePerformanceControls()
{
    // Main Live Performance Toggle
    addAndMakeVisible(livePerformanceLabel);
    livePerformanceLabel.setText("🎭 Live Performance", juce::dontSendNotification);
    livePerformanceLabel.setFont(juce::FontOptions(16.0f).withStyle("Bold"));
    livePerformanceLabel.setColour(juce::Label::textColourId, styling.textColour);
    
    addAndMakeVisible(livePerformanceToggle);
    livePerformanceToggle.setButtonText("Enable Live Performance");
    livePerformanceToggle.addListener(this);
    livePerformanceToggle.setColour(juce::ToggleButton::textColourId, styling.textColour);
    
    // Initialize status LEDs
    livePerformanceLED.tooltip = "Live Performance System Status";
    aiGenerationLED.tooltip = "AI Generation Activity";
    parameterModulationLED.tooltip = "Parameter Modulation Active";
    patternEvolutionLED.tooltip = "Pattern Evolution Status";
}

void LivePerformanceControlPanel::setupPerformanceModeControls()
{
    addAndMakeVisible(performanceModeLabel);
    performanceModeLabel.setText("Performance Mode:", juce::dontSendNotification);
    performanceModeLabel.setColour(juce::Label::textColourId, styling.textColour);
    
    addAndMakeVisible(performanceModeBox);
    performanceModeBox.addItem("Pattern Generation", 1);
    performanceModeBox.addItem("Parameter Modulation", 2);
    performanceModeBox.addItem("Pattern Evolution", 3);
    performanceModeBox.addItem("Full Live Performance", 4);
    performanceModeBox.setSelectedId(1);
    performanceModeBox.addListener(this);
}

void LivePerformanceControlPanel::setupAIModulationControls()
{
    addAndMakeVisible(aiModulationGroup);
    aiModulationGroup.setText("AI Modulation");
    aiModulationGroup.setColour(juce::GroupComponent::textColourId, styling.textColour);
    aiModulationGroup.setColour(juce::GroupComponent::outlineColourId, styling.borderColour);
    
    addAndMakeVisible(aiModulationToggle);
    aiModulationToggle.setButtonText("Enable AI Modulation");
    aiModulationToggle.addListener(this);
    aiModulationToggle.setColour(juce::ToggleButton::textColourId, styling.textColour);
    
    addAndMakeVisible(aiIntensityLabel);
    aiIntensityLabel.setText("Intensity:", juce::dontSendNotification);
    aiIntensityLabel.setColour(juce::Label::textColourId, styling.textColour);
    
    addAndMakeVisible(aiIntensitySlider);
    aiIntensitySlider.setRange(0.0, 1.0, 0.01);
    aiIntensitySlider.setValue(0.5);
    aiIntensitySlider.setSliderStyle(juce::Slider::LinearHorizontal);
    aiIntensitySlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
    aiIntensitySlider.addListener(this);
    aiIntensitySlider.setColour(juce::Slider::textBoxTextColourId, styling.textColour);
    aiIntensitySlider.setColour(juce::Slider::thumbColourId, styling.accentColour);
}

void LivePerformanceControlPanel::setupTriggerControls()
{
    addAndMakeVisible(triggerControlsGroup);
    triggerControlsGroup.setText("Trigger Controls");
    triggerControlsGroup.setColour(juce::GroupComponent::textColourId, styling.textColour);
    triggerControlsGroup.setColour(juce::GroupComponent::outlineColourId, styling.borderColour);
    
    addAndMakeVisible(manualTriggerButton);
    manualTriggerButton.setButtonText("🎲 Generate Pattern");
    manualTriggerButton.addListener(this);
    manualTriggerButton.setColour(juce::TextButton::buttonColourId, styling.accentColour);
    manualTriggerButton.setColour(juce::TextButton::textColourOnId, juce::Colours::white);
    
    addAndMakeVisible(triggerTypeLabel);
    triggerTypeLabel.setText("Trigger Type:", juce::dontSendNotification);
    triggerTypeLabel.setColour(juce::Label::textColourId, styling.textColour);
    
    addAndMakeVisible(triggerTypeBox);
    triggerTypeBox.addItem("Manual", 1);
    triggerTypeBox.addItem("Beat-Sync", 2);
    triggerTypeBox.addItem("Probability", 3);
    triggerTypeBox.addItem("MIDI CC", 4);
    triggerTypeBox.setSelectedId(1);
    triggerTypeBox.addListener(this);
    
    addAndMakeVisible(triggerProbabilityLabel);
    triggerProbabilityLabel.setText("Probability:", juce::dontSendNotification);
    triggerProbabilityLabel.setColour(juce::Label::textColourId, styling.textColour);
    
    addAndMakeVisible(triggerProbabilitySlider);
    triggerProbabilitySlider.setRange(0.0, 1.0, 0.01);
    triggerProbabilitySlider.setValue(0.1);
    triggerProbabilitySlider.setSliderStyle(juce::Slider::LinearHorizontal);
    triggerProbabilitySlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
    triggerProbabilitySlider.addListener(this);
    triggerProbabilitySlider.setColour(juce::Slider::textBoxTextColourId, styling.textColour);
    triggerProbabilitySlider.setColour(juce::Slider::thumbColourId, styling.accentColour);
}

void LivePerformanceControlPanel::setupStatusDisplay()
{
    addAndMakeVisible(statusGroup);
    statusGroup.setText("Performance Status");
    statusGroup.setColour(juce::GroupComponent::textColourId, styling.textColour);
    statusGroup.setColour(juce::GroupComponent::outlineColourId, styling.borderColour);
    
    addAndMakeVisible(statusLabel);
    statusLabel.setText("Status: Disabled", juce::dontSendNotification);
    statusLabel.setColour(juce::Label::textColourId, styling.textColour);
    
    addAndMakeVisible(modeStatusLabel);
    modeStatusLabel.setText("Mode: None", juce::dontSendNotification);
    modeStatusLabel.setColour(juce::Label::textColourId, styling.textColour);
    
    addAndMakeVisible(generationTimeLabel);
    generationTimeLabel.setText("Last Generation: --", juce::dontSendNotification);
    generationTimeLabel.setColour(juce::Label::textColourId, styling.textColour);
    
    addAndMakeVisible(aiActivityLabel);
    aiActivityLabel.setText("AI Activity:", juce::dontSendNotification);
    aiActivityLabel.setColour(juce::Label::textColourId, styling.textColour);
    
    addAndMakeVisible(aiActivityIndicator);
    aiActivityIndicator.setColour(juce::ProgressBar::backgroundColourId, styling.panelColour);
    aiActivityIndicator.setColour(juce::ProgressBar::foregroundColourId, styling.activeColour);
    
    addAndMakeVisible(evolutionStatusLabel);
    evolutionStatusLabel.setText("Evolution Progress:", juce::dontSendNotification);
    evolutionStatusLabel.setColour(juce::Label::textColourId, styling.textColour);
    
    addAndMakeVisible(evolutionProgressBar);
    evolutionProgressBar.setColour(juce::ProgressBar::backgroundColourId, styling.panelColour);
    evolutionProgressBar.setColour(juce::ProgressBar::foregroundColourId, styling.accentColour);
    
    addAndMakeVisible(advancedToggleButton);
    advancedToggleButton.setButtonText("Show Advanced");
    advancedToggleButton.addListener(this);
    
    addChildComponent(advancedControlsContainer);
}

void LivePerformanceControlPanel::setupStyling()
{
    // Apply consistent styling to all components
    setColour(juce::ResizableWindow::backgroundColourId, styling.backgroundColour);
    
    // Set default fonts and colors for better consistency
    getLookAndFeel().setDefaultSansSerifTypefaceName("Arial");
}

//==============================================================================
void LivePerformanceControlPanel::paintBackground(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Background gradient
    juce::ColourGradient gradient(
        styling.backgroundColour.brighter(0.1f), 0.0f, 0.0f,
        styling.backgroundColour.darker(0.1f), 0.0f, bounds.getHeight(),
        false);
    g.setGradientFill(gradient);
    g.fillRoundedRectangle(bounds, styling.cornerRadius);
    
    // Border
    g.setColour(styling.borderColour);
    g.drawRoundedRectangle(bounds.reduced(1.0f), styling.cornerRadius, styling.borderWidth);
    
    // Title area highlight
    auto headerBounds = bounds.removeFromTop(60);
    g.setColour(styling.panelColour.withAlpha(0.3f));
    g.fillRoundedRectangle(headerBounds.reduced(2.0f), styling.cornerRadius);
}

void LivePerformanceControlPanel::paintStatusIndicators(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    auto headerBounds = bounds.removeFromTop(60);
    auto ledBounds = headerBounds.removeFromRight(200).reduced(styling.margin);
    
    // Paint status LEDs
    auto ledSize = 12.0f;
    auto ledSpacing = ledBounds.getWidth() / 4.0f;
    
    StatusLED* leds[] = { &livePerformanceLED, &aiGenerationLED, &parameterModulationLED, &patternEvolutionLED };
    juce::String ledLabels[] = { "Live", "AI Gen", "Mod", "Evol" };
    
    for (int i = 0; i < 4; ++i)
    {
        auto ledX = ledBounds.getX() + i * ledSpacing;
        auto ledY = ledBounds.getCentreY() - ledSize * 0.5f;
        
        // LED background
        g.setColour(juce::Colours::black);
        g.fillEllipse(ledX, ledY, ledSize, ledSize);
        
        // LED color and brightness
        auto ledColour = leds[i]->colour.withBrightness(leds[i]->brightness);
        if (leds[i]->isActive)
        {
            ledColour = ledColour.brighter(0.3f);
        }
        
        g.setColour(ledColour);
        g.fillEllipse(ledX + 1, ledY + 1, ledSize - 2, ledSize - 2);
        
        // LED label
        g.setColour(styling.textColour.withAlpha(0.7f));
        g.setFont(juce::FontOptions(9.0f));
        g.drawText(ledLabels[i], 
                   static_cast<int>(ledX - 5), static_cast<int>(ledY + ledSize + 2), 
                   static_cast<int>(ledSize + 10), 12, 
                   juce::Justification::centred);
    }
}

void LivePerformanceControlPanel::paintPerformanceMetrics(juce::Graphics& g)
{
    // Additional visual feedback could be added here
    // such as performance graphs, timing displays, etc.
}

//==============================================================================
void LivePerformanceControlPanel::enableLivePerformance(bool enabled)
{
    audioProcessor.setLivePerformanceEnabled(enabled);
    
    // Update UI state
    performanceModeBox.setEnabled(enabled);
    aiModulationToggle.setEnabled(enabled);
    aiIntensitySlider.setEnabled(enabled && aiModulationToggle.getToggleState());
    manualTriggerButton.setEnabled(enabled);
    triggerTypeBox.setEnabled(enabled);
    triggerProbabilitySlider.setEnabled(enabled);
    
    // Update status
    livePerformanceLED.isActive = enabled;
    livePerformanceLED.colour = enabled ? styling.activeColour : juce::Colours::grey;
    
    metrics.currentMode = enabled ? "Enabled" : "Disabled";
    statusLabel.setText("Status: " + metrics.currentMode, juce::dontSendNotification);
    
    repaint();
}

void LivePerformanceControlPanel::updatePerformanceMode()
{
    if (!audioProcessor.isLivePerformanceEnabled())
        return;
        
    auto selectedId = performanceModeBox.getSelectedId();
    using PerformanceMode = spawnclone::ai::LivePerformanceEngine::PerformanceMode;
    
    PerformanceMode mode;
    juce::String modeName;
    
    switch (selectedId)
    {
        case 1:
            mode = PerformanceMode::PatternGeneration;
            modeName = "Pattern Generation";
            break;
        case 2:
            mode = PerformanceMode::ParameterModulation;
            modeName = "Parameter Modulation";
            break;
        case 3:
            mode = PerformanceMode::PatternEvolution;
            modeName = "Pattern Evolution";
            break;
        case 4:
            mode = PerformanceMode::FullLive;
            modeName = "Full Live Performance";
            break;
        default:
            return;
    }
    
    audioProcessor.setLivePerformanceMode(mode);
    metrics.currentMode = modeName;
    modeStatusLabel.setText("Mode: " + modeName, juce::dontSendNotification);
}

void LivePerformanceControlPanel::configureAIModulation()
{
    auto enabled = aiModulationToggle.getToggleState();
    auto intensity = static_cast<float>(aiIntensitySlider.getValue());
    
    audioProcessor.configureLiveAIModulation(enabled, intensity);
    
    aiIntensitySlider.setEnabled(enabled);
    parameterModulationLED.isActive = enabled;
    parameterModulationLED.colour = enabled ? styling.activeColour : juce::Colours::grey;
    
    repaint();
}

void LivePerformanceControlPanel::triggerManualGeneration()
{
    audioProcessor.triggerLivePatternGeneration();
    
    // Visual feedback
    aiGenerationLED.isActive = true;
    aiGenerationLED.colour = styling.accentColour;
    aiGenerationLED.brightness = 1.0f;
    
    // Track metrics
    metrics.totalGenerations++;
    
    repaint();
}

void LivePerformanceControlPanel::updateStatusIndicators()
{
    // Update LED states based on actual system status
    bool isEnabled = audioProcessor.isLivePerformanceEnabled();
    
    livePerformanceLED.isActive = isEnabled;
    livePerformanceLED.colour = isEnabled ? styling.activeColour : juce::Colours::grey;
    
    // Fade AI generation LED after trigger
    if (aiGenerationLED.brightness > 0.3f)
    {
        aiGenerationLED.brightness *= 0.95f; // Fade effect
        repaint();
    }
}

void LivePerformanceControlPanel::updatePerformanceMetrics()
{
    // Update performance metrics display
    if (metrics.totalGenerations > 0)
    {
        generationTimeLabel.setText(
            "Last Generation: " + juce::String(metrics.lastGenerationTime, 1) + "ms",
            juce::dontSendNotification);
    }
    
    // Update AI activity indicator based on system state
    // This would connect to actual performance metrics from the live system
}

void LivePerformanceControlPanel::updatePerformanceStatus()
{
    // Public method for external updates
    updateStatusIndicators();
    updatePerformanceMetrics();
}

void LivePerformanceControlPanel::setEnabled(bool shouldBeEnabled)
{
    Component::setEnabled(shouldBeEnabled);
    
    if (!shouldBeEnabled)
    {
        // Reset all controls to disabled state
        livePerformanceToggle.setToggleState(false, juce::dontSendNotification);
        enableLivePerformance(false);
    }
}

} // namespace spawnclone::ui
