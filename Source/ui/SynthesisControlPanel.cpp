//==============================================================================
/*
  SynthesisControlPanel.cpp
  Created: Aug 5 2025
  Author:  BMad Dev Agent - UI Integration
  
  Implementation of real-time synthesis parameter control panel.
  Integrates with Phase 1B & 1C AI parameter mapp    defaultParams.set("wavetable.wavetablePosition", "0.5");
    
    //audioProcessor.batchUpdateSynthesisParameters(defaultParams);
    
    isUpdatingFromAI = false;stem.
*/
//==============================================================================

#include "SynthesisControlPanel.h"
#include "../PluginProcessor.h"

//==============================================================================
SynthesisControlPanel::SynthesisControlPanel(SpawnCloneAudioProcessor& processor)
    : audioProcessor(processor)
{
    // Setup groups
    addAndMakeVisible(filterGroup);
    filterGroup.setText("Filter");
    filterGroup.setTextLabelPosition(juce::Justification::centredLeft);
    
    addAndMakeVisible(envelopeGroup);
    envelopeGroup.setText("Envelope");
    envelopeGroup.setTextLabelPosition(juce::Justification::centredLeft);
    
    addAndMakeVisible(modulationGroup);
    modulationGroup.setText("Modulation");
    modulationGroup.setTextLabelPosition(juce::Justification::centredLeft);
    
    addAndMakeVisible(characterGroup);
    characterGroup.setText("Character");
    characterGroup.setTextLabelPosition(juce::Justification::centredLeft);
    
    addAndMakeVisible(wavetableGroup);
    wavetableGroup.setText("Wavetable");
    wavetableGroup.setTextLabelPosition(juce::Justification::centredLeft);
    
    // Setup Filter Controls
    setupFilterControls();
    
    // Setup Envelope Controls
    setupEnvelopeControls();
    
    // Setup Modulation Controls
    setupModulationControls();
    
    // Setup Character Controls
    setupCharacterControls();
    
    // Setup Wavetable Controls
    setupWavetableControls();
    
    // Setup Control Buttons
    setupControlButtons();
    
    // Start timer for UI updates
    startTimerHz(30); // 30 FPS for smooth UI updates
}

//==============================================================================
void SynthesisControlPanel::paint(juce::Graphics& g)
{
    // Professional dark background
    g.fillAll(juce::Colour(0xff2a2a2a));
    
    // Subtle border
    g.setColour(juce::Colour(0xff404040));
    g.drawRect(getLocalBounds(), 1);
    
    // Title section
    auto titleArea = getLocalBounds().removeFromTop(30);
    g.setColour(juce::Colour(0xff3a3a3a));
    g.fillRect(titleArea);
    
    g.setColour(juce::Colour(0xffeeeeee));
    g.setFont(juce::FontOptions("Arial", "Bold", 14.0f));
    g.drawFittedText("Advanced Synthesis Controls", titleArea.reduced(MARGIN, 0), 
                     juce::Justification::centredLeft, 1);
    
    // AI Link status indicator
    if (aiLinked)
    {
        g.setColour(juce::Colour(0xff4CAF50)); // Green for linked
        g.fillEllipse(static_cast<float>(getWidth() - 25), 8.0f, 14.0f, 14.0f);
        g.setColour(juce::Colour(0xffeeeeee));
        g.setFont(juce::FontOptions("Arial", "Regular", 10.0f));
        g.drawFittedText("AI", juce::Rectangle<int>(getWidth() - 25, 8, 14, 14), 
                         juce::Justification::centred, 1);
    }
    else
    {
        g.setColour(juce::Colour(0xffFF5722)); // Orange for manual
        g.fillEllipse(static_cast<float>(getWidth() - 25), 8.0f, 14.0f, 14.0f);
        g.setColour(juce::Colour(0xffeeeeee));
        g.setFont(juce::FontOptions("Arial", "Regular", 9.0f));
        g.drawFittedText("M", juce::Rectangle<int>(getWidth() - 25, 8, 14, 14), 
                         juce::Justification::centred, 1);
    }
}

void SynthesisControlPanel::resized()
{
    auto bounds = getLocalBounds();
    bounds.removeFromTop(30); // Title space
    bounds.reduce(MARGIN, MARGIN);
    
    // Calculate layout - 2 columns for optimal space usage
    auto leftColumn = bounds.removeFromLeft(bounds.getWidth() / 2 - MARGIN/2);
    auto rightColumn = bounds.removeFromLeft(bounds.getWidth()).reduced(MARGIN/2, 0);
    
    // Left column: Filter, Envelope, Character
    filterGroup.setBounds(leftColumn.removeFromTop(GROUP_HEIGHT));
    leftColumn.removeFromTop(MARGIN);
    
    envelopeGroup.setBounds(leftColumn.removeFromTop(GROUP_HEIGHT));
    leftColumn.removeFromTop(MARGIN);
    
    characterGroup.setBounds(leftColumn.removeFromTop(GROUP_HEIGHT));
    
    // Right column: Modulation, Wavetable, Controls
    modulationGroup.setBounds(rightColumn.removeFromTop(GROUP_HEIGHT));
    rightColumn.removeFromTop(MARGIN);
    
    wavetableGroup.setBounds(rightColumn.removeFromTop(GROUP_HEIGHT));
    rightColumn.removeFromTop(MARGIN);
    
    // Control buttons at bottom right
    auto buttonRow = rightColumn.removeFromTop(BUTTON_HEIGHT);
    resetButton.setBounds(buttonRow.removeFromLeft(80).reduced(2));
    linkToAIButton.setBounds(buttonRow.removeFromLeft(100).reduced(2));
    
    // Layout individual controls within groups
    layoutFilterControls();
    layoutEnvelopeControls();
    layoutModulationControls();
    layoutCharacterControls();
    layoutWavetableControls();
}

//==============================================================================
void SynthesisControlPanel::sliderValueChanged(juce::Slider* slider)
{
    if (isUpdatingFromAI) return; // Prevent feedback loops
    
    // Map slider to parameter name and apply to engine
    if (slider == &cutoffSlider)
        applyParameterToEngine("filter.cutoff", static_cast<float>(slider->getValue()));
    else if (slider == &resonanceSlider)
        applyParameterToEngine("filter.resonance", static_cast<float>(slider->getValue()));
    else if (slider == &attackSlider)
        applyParameterToEngine("envelope.attack", static_cast<float>(slider->getValue()));
    else if (slider == &decaySlider)
        applyParameterToEngine("envelope.decay", static_cast<float>(slider->getValue()));
    else if (slider == &sustainSlider)
        applyParameterToEngine("envelope.sustain", static_cast<float>(slider->getValue()));
    else if (slider == &releaseSlider)
        applyParameterToEngine("envelope.release", static_cast<float>(slider->getValue()));
    else if (slider == &lfo1RateSlider)
        applyParameterToEngine("modulation.lfoRate", static_cast<float>(slider->getValue()));
    else if (slider == &lfo1DepthSlider)
        applyParameterToEngine("modulation.lfoDepth", static_cast<float>(slider->getValue()));
    else if (slider == &brightnessSlider)
        applyParameterToEngine("brightness", static_cast<float>(slider->getValue()));
    else if (slider == &warmthSlider)
        applyParameterToEngine("warmth", static_cast<float>(slider->getValue()));
    else if (slider == &saturationSlider)
        applyParameterToEngine("saturation", static_cast<float>(slider->getValue()));
    else if (slider == &wavetablePositionSlider)
        applyParameterToEngine("wavetable.wavetablePosition", static_cast<float>(slider->getValue()));
    
    // Mark as manual control
    if (aiLinked)
    {
        aiLinked = false;
        linkToAIButton.setToggleState(false, juce::dontSendNotification);
        repaint();
    }
}

void SynthesisControlPanel::comboBoxChanged(juce::ComboBox* comboBox)
{
    if (isUpdatingFromAI) return;
    
    if (comboBox == &filterTypeComboBox)
    {
        applyParameterToEngine("filter.filterType", static_cast<float>(comboBox->getSelectedId() - 1));
    }
    else if (comboBox == &lfo1WaveformComboBox)
    {
        applyParameterToEngine("modulation.lfoWaveform", static_cast<float>(comboBox->getSelectedId() - 1));
    }
    
    // Mark as manual control
    if (aiLinked)
    {
        aiLinked = false;
        linkToAIButton.setToggleState(false, juce::dontSendNotification);
        repaint();
    }
}

void SynthesisControlPanel::buttonClicked(juce::Button* button)
{
    if (button == &resetButton)
    {
        resetToDefaults();
    }
    else if (button == &linkToAIButton)
    {
        aiLinked = button->getToggleState();
        repaint();
    }
}

void SynthesisControlPanel::timerCallback()
{
    // Update UI from engine if AI-linked (to reflect AI parameter changes)
    if (aiLinked && !isUpdatingFromAI)
    {
        updateUIFromEngine();
    }
}

//==============================================================================
void SynthesisControlPanel::updateFromAIParameters(const GenerationParameters& aiParams)
{
    if (!aiLinked) return;
    
    isUpdatingFromAI = true;
    
    // Apply AI parameters through the parameter mapper
    audioProcessor.applyAIGenerationParameters(aiParams);
    
    // Update UI to reflect the mapped parameters
    updateUIFromEngine();
    
    isUpdatingFromAI = false;
}

void SynthesisControlPanel::setEnabled(bool shouldBeEnabled)
{
    Component::setEnabled(shouldBeEnabled);
    
    // Enable/disable all child components
    for (auto* child : getChildren())
    {
        child->setEnabled(shouldBeEnabled);
    }
}

void SynthesisControlPanel::resetToDefaults()
{
    isUpdatingFromAI = true;
    
    // Reset all sliders to default values
    cutoffSlider.setValue(2000.0, juce::dontSendNotification);
    resonanceSlider.setValue(0.3, juce::dontSendNotification);
    filterTypeComboBox.setSelectedId(1, juce::dontSendNotification); // LowPass
    
    attackSlider.setValue(0.1, juce::dontSendNotification);
    decaySlider.setValue(0.3, juce::dontSendNotification);
    sustainSlider.setValue(0.7, juce::dontSendNotification);
    releaseSlider.setValue(0.5, juce::dontSendNotification);
    
    lfo1RateSlider.setValue(2.0, juce::dontSendNotification);
    lfo1DepthSlider.setValue(0.0, juce::dontSendNotification);
    lfo1WaveformComboBox.setSelectedId(2, juce::dontSendNotification); // Triangle
    
    brightnessSlider.setValue(1.0, juce::dontSendNotification);
    warmthSlider.setValue(1.0, juce::dontSendNotification);
    saturationSlider.setValue(1.0, juce::dontSendNotification);
    
    wavetablePositionSlider.setValue(0.5, juce::dontSendNotification);
    
    // Apply to engine
    juce::StringPairArray defaultParams;
    defaultParams.set("filter.cutoff", "2000.0");
    defaultParams.set("filter.resonance", "0.3");
    defaultParams.set("filter.filterType", "0");
    defaultParams.set("envelope.attack", "0.1");
    defaultParams.set("envelope.decay", "0.3");
    defaultParams.set("envelope.sustain", "0.7");
    defaultParams.set("envelope.release", "0.5");
    defaultParams.set("modulation.lfoRate", "2.0");
    defaultParams.set("modulation.lfoDepth", "0.0");
    defaultParams.set("modulation.lfoWaveform", "1");
    defaultParams.set("brightness", "1.0");
    defaultParams.set("warmth", "1.0");
    defaultParams.set("saturation", "1.0");
    defaultParams.set("wavetable.wavetablePosition", "0.5");
    
    audioProcessor.batchUpdateSynthesisParameters(defaultParams);
    
    isUpdatingFromAI = false;
}

//==============================================================================
// Private helper methods

void SynthesisControlPanel::setupFilterControls()
{
    // Filter Cutoff
    addAndMakeVisible(cutoffLabel);
    cutoffLabel.setText("Cutoff", juce::dontSendNotification);
    cutoffLabel.setFont(juce::FontOptions("Arial", "Regular", 12.0f));
    
    addAndMakeVisible(cutoffSlider);
    setupSlider(cutoffSlider, 20.0f, 20000.0f, 2000.0f, 1.0f, " Hz");
    cutoffSlider.setSkewFactorFromMidPoint(1000.0);
    
    // Filter Resonance
    addAndMakeVisible(resonanceLabel);
    resonanceLabel.setText("Resonance", juce::dontSendNotification);
    resonanceLabel.setFont(juce::FontOptions("Arial", "Regular", 12.0f));
    
    addAndMakeVisible(resonanceSlider);
    setupSlider(resonanceSlider, 0.0f, 1.0f, 0.3f);
    
    // Filter Type
    addAndMakeVisible(filterTypeLabel);
    filterTypeLabel.setText("Type", juce::dontSendNotification);
    filterTypeLabel.setFont(juce::FontOptions("Arial", "Regular", 12.0f));
    
    addAndMakeVisible(filterTypeComboBox);
    juce::StringArray filterTypes = {"Low Pass", "High Pass", "Band Pass", "Notch", "Moog Ladder", "State Variable"};
    setupComboBox(filterTypeComboBox, filterTypes);
    filterTypeComboBox.setSelectedId(1); // Low Pass default
}

void SynthesisControlPanel::setupEnvelopeControls()
{
    // Attack
    addAndMakeVisible(attackLabel);
    attackLabel.setText("Attack", juce::dontSendNotification);
    attackLabel.setFont(juce::FontOptions("Arial", "Regular", 12.0f));
    
    addAndMakeVisible(attackSlider);
    setupSlider(attackSlider, 0.001f, 5.0f, 0.1f, 0.001f, " s");
    
    // Decay
    addAndMakeVisible(decayLabel);
    decayLabel.setText("Decay", juce::dontSendNotification);
    decayLabel.setFont(juce::FontOptions("Arial", "Regular", 12.0f));
    
    addAndMakeVisible(decaySlider);
    setupSlider(decaySlider, 0.001f, 5.0f, 0.3f, 0.001f, " s");
    
    // Sustain
    addAndMakeVisible(sustainLabel);
    sustainLabel.setText("Sustain", juce::dontSendNotification);
    sustainLabel.setFont(juce::FontOptions("Arial", "Regular", 12.0f));
    
    addAndMakeVisible(sustainSlider);
    setupSlider(sustainSlider, 0.0f, 1.0f, 0.7f);
    
    // Release
    addAndMakeVisible(releaseLabel);
    releaseLabel.setText("Release", juce::dontSendNotification);
    releaseLabel.setFont(juce::FontOptions("Arial", "Regular", 12.0f));
    
    addAndMakeVisible(releaseSlider);
    setupSlider(releaseSlider, 0.001f, 5.0f, 0.5f, 0.001f, " s");
}

void SynthesisControlPanel::setupModulationControls()
{
    // LFO Rate
    addAndMakeVisible(lfo1RateLabel);
    lfo1RateLabel.setText("LFO Rate", juce::dontSendNotification);
    lfo1RateLabel.setFont(juce::FontOptions("Arial", "Regular", 12.0f));
    
    addAndMakeVisible(lfo1RateSlider);
    setupSlider(lfo1RateSlider, 0.01f, 20.0f, 2.0f, 0.01f, " Hz");
    
    // LFO Depth
    addAndMakeVisible(lfo1DepthLabel);
    lfo1DepthLabel.setText("LFO Depth", juce::dontSendNotification);
    lfo1DepthLabel.setFont(juce::FontOptions("Arial", "Regular", 12.0f));
    
    addAndMakeVisible(lfo1DepthSlider);
    setupSlider(lfo1DepthSlider, 0.0f, 1.0f, 0.0f);
    
    // LFO Waveform
    addAndMakeVisible(lfo1WaveformLabel);
    lfo1WaveformLabel.setText("LFO Wave", juce::dontSendNotification);
    lfo1WaveformLabel.setFont(juce::FontOptions("Arial", "Regular", 12.0f));
    
    addAndMakeVisible(lfo1WaveformComboBox);
    juce::StringArray waveforms = {"Sine", "Triangle", "Sawtooth", "Square", "Random"};
    setupComboBox(lfo1WaveformComboBox, waveforms);
    lfo1WaveformComboBox.setSelectedId(2); // Triangle default
}

void SynthesisControlPanel::setupCharacterControls()
{
    // Brightness
    addAndMakeVisible(brightnessLabel);
    brightnessLabel.setText("Brightness", juce::dontSendNotification);
    brightnessLabel.setFont(juce::FontOptions("Arial", "Regular", 12.0f));
    
    addAndMakeVisible(brightnessSlider);
    setupSlider(brightnessSlider, 0.5f, 2.0f, 1.0f);
    
    // Warmth
    addAndMakeVisible(warmthLabel);
    warmthLabel.setText("Warmth", juce::dontSendNotification);
    warmthLabel.setFont(juce::FontOptions("Arial", "Regular", 12.0f));
    
    addAndMakeVisible(warmthSlider);
    setupSlider(warmthSlider, 0.5f, 2.0f, 1.0f);
    
    // Saturation
    addAndMakeVisible(saturationLabel);
    saturationLabel.setText("Saturation", juce::dontSendNotification);
    saturationLabel.setFont(juce::FontOptions("Arial", "Regular", 12.0f));
    
    addAndMakeVisible(saturationSlider);
    setupSlider(saturationSlider, 0.5f, 2.0f, 1.0f);
}

void SynthesisControlPanel::setupWavetableControls()
{
    // Wavetable Position
    addAndMakeVisible(wavetablePositionLabel);
    wavetablePositionLabel.setText("Position", juce::dontSendNotification);
    wavetablePositionLabel.setFont(juce::FontOptions("Arial", "Regular", 12.0f));
    
    addAndMakeVisible(wavetablePositionSlider);
    setupSlider(wavetablePositionSlider, 0.0f, 1.0f, 0.5f);
    
}

void SynthesisControlPanel::setupControlButtons()
{
    addAndMakeVisible(resetButton);
    resetButton.setButtonText("Reset");
    resetButton.addListener(this);
    
    addAndMakeVisible(linkToAIButton);
    linkToAIButton.setButtonText("Link to AI");
    linkToAIButton.setToggleState(true, juce::dontSendNotification);
    linkToAIButton.addListener(this);
}

void SynthesisControlPanel::setupSlider(juce::Slider& slider, float min, float max, float defaultValue, 
                                       float interval, const juce::String& suffix)
{
    slider.setRange(min, max, interval);
    slider.setValue(defaultValue, juce::dontSendNotification);
    slider.setSliderStyle(juce::Slider::LinearHorizontal);
    slider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
    slider.setTextValueSuffix(suffix);
    slider.addListener(this);
    
    // Professional styling
    slider.setColour(juce::Slider::trackColourId, juce::Colour(0xff404040));
    slider.setColour(juce::Slider::thumbColourId, juce::Colour(0xff64B5F6));
    slider.setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xffeeeeee));
    slider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff2a2a2a));
}

void SynthesisControlPanel::setupComboBox(juce::ComboBox& combo, const juce::StringArray& options)
{
    combo.clear(juce::dontSendNotification);
    for (int i = 0; i < options.size(); ++i)
    {
        combo.addItem(options[i], i + 1);
    }
    combo.addListener(this);
    
    // Professional styling
    combo.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff3a3a3a));
    combo.setColour(juce::ComboBox::textColourId, juce::Colour(0xffeeeeee));
    combo.setColour(juce::ComboBox::arrowColourId, juce::Colour(0xff64B5F6));
}

void SynthesisControlPanel::applyParameterToEngine(const juce::String& paramName, float value)
{
    audioProcessor.updateSynthesisParameter(paramName, value, true); // With interpolation
}

void SynthesisControlPanel::updateUIFromEngine()
{
    // Update UI controls to match engine state
    // This would typically query the parameter cache, but for now we'll update based on AI mapping
    
    // Note: In a full implementation, we'd query the parameter cache:
    // float cutoff = audioProcessor.getSynthesisParameterValue("filter.cutoff");
    // cutoffSlider.setValue(cutoff, juce::dontSendNotification);
}

// Layout helper methods
void SynthesisControlPanel::layoutFilterControls()
{
    auto bounds = filterGroup.getBounds().reduced(MARGIN);
    bounds.removeFromTop(20); // Group title space
    
    auto row1 = bounds.removeFromTop(SLIDER_HEIGHT);
    cutoffLabel.setBounds(row1.removeFromLeft(70));
    cutoffSlider.setBounds(row1.reduced(2));
    
    bounds.removeFromTop(5);
    auto row2 = bounds.removeFromTop(SLIDER_HEIGHT);
    resonanceLabel.setBounds(row2.removeFromLeft(70));
    resonanceSlider.setBounds(row2.reduced(2));
    
    bounds.removeFromTop(5);
    auto row3 = bounds.removeFromTop(COMBO_HEIGHT);
    filterTypeLabel.setBounds(row3.removeFromLeft(70));
    filterTypeComboBox.setBounds(row3.reduced(2));
}

void SynthesisControlPanel::layoutEnvelopeControls()
{
    auto bounds = envelopeGroup.getBounds().reduced(MARGIN);
    bounds.removeFromTop(20);
    
    auto row1 = bounds.removeFromTop(SLIDER_HEIGHT);
    attackLabel.setBounds(row1.removeFromLeft(70));
    attackSlider.setBounds(row1.reduced(2));
    
    bounds.removeFromTop(5);
    auto row2 = bounds.removeFromTop(SLIDER_HEIGHT);
    decayLabel.setBounds(row2.removeFromLeft(70));
    decaySlider.setBounds(row2.reduced(2));
    
    bounds.removeFromTop(5);
    auto row3 = bounds.removeFromTop(SLIDER_HEIGHT);
    sustainLabel.setBounds(row3.removeFromLeft(70));
    sustainSlider.setBounds(row3.reduced(2));
    
    bounds.removeFromTop(5);
    auto row4 = bounds.removeFromTop(SLIDER_HEIGHT);
    releaseLabel.setBounds(row4.removeFromLeft(70));
    releaseSlider.setBounds(row4.reduced(2));
}

void SynthesisControlPanel::layoutModulationControls()
{
    auto bounds = modulationGroup.getBounds().reduced(MARGIN);
    bounds.removeFromTop(20);
    
    auto row1 = bounds.removeFromTop(SLIDER_HEIGHT);
    lfo1RateLabel.setBounds(row1.removeFromLeft(70));
    lfo1RateSlider.setBounds(row1.reduced(2));
    
    bounds.removeFromTop(5);
    auto row2 = bounds.removeFromTop(SLIDER_HEIGHT);
    lfo1DepthLabel.setBounds(row2.removeFromLeft(70));
    lfo1DepthSlider.setBounds(row2.reduced(2));
    
    bounds.removeFromTop(5);
    auto row3 = bounds.removeFromTop(COMBO_HEIGHT);
    lfo1WaveformLabel.setBounds(row3.removeFromLeft(70));
    lfo1WaveformComboBox.setBounds(row3.reduced(2));
}

void SynthesisControlPanel::layoutCharacterControls()
{
    auto bounds = characterGroup.getBounds().reduced(MARGIN);
    bounds.removeFromTop(20);
    
    auto row1 = bounds.removeFromTop(SLIDER_HEIGHT);
    brightnessLabel.setBounds(row1.removeFromLeft(70));
    brightnessSlider.setBounds(row1.reduced(2));
    
    bounds.removeFromTop(5);
    auto row2 = bounds.removeFromTop(SLIDER_HEIGHT);
    warmthLabel.setBounds(row2.removeFromLeft(70));
    warmthSlider.setBounds(row2.reduced(2));
    
    bounds.removeFromTop(5);
    auto row3 = bounds.removeFromTop(SLIDER_HEIGHT);
    saturationLabel.setBounds(row3.removeFromLeft(70));
    saturationSlider.setBounds(row3.reduced(2));
}

void SynthesisControlPanel::layoutWavetableControls()
{
    auto bounds = wavetableGroup.getBounds().reduced(MARGIN);
    bounds.removeFromTop(20);
    
    auto row1 = bounds.removeFromTop(SLIDER_HEIGHT);
    wavetablePositionLabel.setBounds(row1.removeFromLeft(70));
    wavetablePositionSlider.setBounds(row1.reduced(2));
    
}
