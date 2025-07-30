#include "PluginEditor.h"

//==============================================================================
SpawnCloneAudioProcessorEditor::SpawnCloneAudioProcessorEditor (SpawnCloneAudioProcessor& p)
    : AudioProcessorEditor (&p), 
      audioProcessor (p),
      parameterManager(p.getParameterManager()),
      patternManager(p.getPatternManager())
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (600, 480);  // Increased height to accommodate AI mode controls

    // Initialize UI components
    setupParameterControls();
    setupPatternDisplay();
    setupGenerateButton();
    
    // Start listening to pattern changes
    audioProcessor.getPatternManager().addChangeListener(this);
}

SpawnCloneAudioProcessorEditor::~SpawnCloneAudioProcessorEditor()
{
    audioProcessor.getPatternManager().removeChangeListener(this);
}

//==============================================================================
void SpawnCloneAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("SpawnClone AI MIDI Generator", getLocalBounds().removeFromTop(30), juce::Justification::centred, 1);
}

void SpawnCloneAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    bounds.removeFromTop(30); // Space for title
    
    // Simple layout without FlexBox
    auto paramHeight = 25;
    auto margin = 10;
    
    // Key row
    auto keyRow = bounds.removeFromTop(paramHeight);
    keyLabel.setBounds(keyRow.removeFromLeft(60));
    keySignatureComboBox.setBounds(keyRow.removeFromLeft(100).reduced(2));
    
    // Scale row  
    auto scaleRow = bounds.removeFromTop(paramHeight);
    scaleLabel.setBounds(scaleRow.removeFromLeft(60));
    scaleTypeComboBox.setBounds(scaleRow.removeFromLeft(100).reduced(2));
    
    // Generation type row
    auto genRow = bounds.removeFromTop(paramHeight);
    generationTypeLabel.setBounds(genRow.removeFromLeft(60));
    generationTypeComboBox.setBounds(genRow.removeFromLeft(100).reduced(2));
    
    // Tempo row
    auto tempoRow = bounds.removeFromTop(paramHeight);
    tempoLabel.setBounds(tempoRow.removeFromLeft(60));
    tempoSlider.setBounds(tempoRow.removeFromLeft(200).reduced(2));
    
    // Complexity row
    auto complexityRow = bounds.removeFromTop(paramHeight);
    complexityLabel.setBounds(complexityRow.removeFromLeft(60));
    complexitySlider.setBounds(complexityRow.removeFromLeft(200).reduced(2));
    
    // Pattern length row
    auto lengthRow = bounds.removeFromTop(paramHeight);
    patternLengthLabel.setBounds(lengthRow.removeFromLeft(60));
    patternLengthSlider.setBounds(lengthRow.removeFromLeft(200).reduced(2));
    
    // AI Mode row
    auto aiModeRow = bounds.removeFromTop(paramHeight);
    aiModeLabel.setBounds(aiModeRow.removeFromLeft(60));
    aiModeComboBox.setBounds(aiModeRow.removeFromLeft(100).reduced(2));
    
    // Generation seed row
    auto seedRow = bounds.removeFromTop(paramHeight);
    generationSeedLabel.setBounds(seedRow.removeFromLeft(60));
    generationSeedSlider.setBounds(seedRow.removeFromLeft(200).reduced(2));
    
    // Generate button
    bounds.removeFromTop(margin);
    generateButton.setBounds(bounds.removeFromTop(40).reduced(margin));
    
    // Visualization controls
    bounds.removeFromTop(margin);
    auto controlsRow = bounds.removeFromTop(paramHeight);
    showGridButton.setBounds(controlsRow.removeFromLeft(80).reduced(2));
    showVelocityButton.setBounds(controlsRow.removeFromLeft(80).reduced(2));
    zoomLabel.setBounds(controlsRow.removeFromLeft(40));
    zoomSlider.setBounds(controlsRow.removeFromLeft(100).reduced(2));
    exportButton.setBounds(controlsRow.removeFromLeft(100).reduced(2));
    
    // Pattern visualization (remaining space)
    bounds.removeFromTop(margin);
    patternVisualization.setBounds(bounds.reduced(margin));
}

//==============================================================================
void SpawnCloneAudioProcessorEditor::setupParameterControls()
{
    auto& params = audioProcessor.getParameterManager();
    auto& valueTreeState = params.getValueTreeState();
    
    // Setup labels
    keyLabel.setText("Key:", juce::dontSendNotification);
    scaleLabel.setText("Scale:", juce::dontSendNotification);
    generationTypeLabel.setText("Type:", juce::dontSendNotification);
    tempoLabel.setText("Tempo:", juce::dontSendNotification);
    complexityLabel.setText("Complexity:", juce::dontSendNotification);
    patternLengthLabel.setText("Length:", juce::dontSendNotification);
    aiModeLabel.setText("AI Mode:", juce::dontSendNotification);
    generationSeedLabel.setText("Seed:", juce::dontSendNotification);
    
    addAndMakeVisible(keyLabel);
    addAndMakeVisible(scaleLabel);
    addAndMakeVisible(generationTypeLabel);
    addAndMakeVisible(tempoLabel);
    addAndMakeVisible(complexityLabel);
    addAndMakeVisible(patternLengthLabel);
    addAndMakeVisible(aiModeLabel);
    addAndMakeVisible(generationSeedLabel);
    
    // Key selection
    keySignatureComboBox.addItemList(params.getKeySignatureItems(), 1);
    keySignatureComboBox.addListener(this);
    addAndMakeVisible(keySignatureComboBox);
    keySignatureAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        valueTreeState, "key", keySignatureComboBox);
    
    // Scale selection
    scaleTypeComboBox.addItemList(params.getScaleTypeItems(), 1);
    scaleTypeComboBox.addListener(this);
    addAndMakeVisible(scaleTypeComboBox);
    scaleTypeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        valueTreeState, "scale", scaleTypeComboBox);
    
    // Generation type selection
    generationTypeComboBox.addItemList(params.getGenerationTypeItems(), 1);
    generationTypeComboBox.addListener(this);
    addAndMakeVisible(generationTypeComboBox);
    generationTypeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        valueTreeState, "generationType", generationTypeComboBox);
    
    // Tempo slider
    tempoSlider.setSliderStyle(juce::Slider::LinearBar);
    tempoSlider.setRange(60.0, 200.0, 1.0);
    tempoSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    tempoSlider.addListener(this);
    addAndMakeVisible(tempoSlider);
    tempoAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        valueTreeState, "tempo", tempoSlider);
    
    // Complexity slider
    complexitySlider.setSliderStyle(juce::Slider::LinearBar);
    complexitySlider.setRange(0.0, 1.0, 0.01);
    complexitySlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    complexitySlider.addListener(this);
    addAndMakeVisible(complexitySlider);
    complexityAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        valueTreeState, "complexity", complexitySlider);
    
    // Pattern length slider
    patternLengthSlider.setSliderStyle(juce::Slider::LinearBar);
    patternLengthSlider.setRange(4.0, 32.0, 4.0);
    patternLengthSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    patternLengthSlider.addListener(this);
    addAndMakeVisible(patternLengthSlider);
    patternLengthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        valueTreeState, "patternLength", patternLengthSlider);
    
    // Epic 7: AI Mode selection
    aiModeComboBox.addItemList(params.getAIModeItems(), 1);
    aiModeComboBox.addListener(this);
    addAndMakeVisible(aiModeComboBox);
    aiModeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        valueTreeState, "aiMode", aiModeComboBox);
    
    // Generation seed slider
    generationSeedSlider.setSliderStyle(juce::Slider::LinearBar);
    generationSeedSlider.setRange(0.0, 10000.0, 1.0);
    generationSeedSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
    generationSeedSlider.addListener(this);
    addAndMakeVisible(generationSeedSlider);
    generationSeedAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        valueTreeState, "generationSeed", generationSeedSlider);
}

void SpawnCloneAudioProcessorEditor::setupPatternDisplay()
{
    // Setup pattern visualization component
    addAndMakeVisible(patternVisualization);
    
    // Setup visualization controls
    showGridButton.setButtonText("Grid");
    showGridButton.setToggleState(true, juce::dontSendNotification);
    showGridButton.addListener(this);
    addAndMakeVisible(showGridButton);
    
    showVelocityButton.setButtonText("Velocity");
    showVelocityButton.setToggleState(true, juce::dontSendNotification);
    showVelocityButton.addListener(this);
    addAndMakeVisible(showVelocityButton);
    
    zoomLabel.setText("Zoom:", juce::dontSendNotification);
    addAndMakeVisible(zoomLabel);
    
    zoomSlider.setSliderStyle(juce::Slider::LinearBar);
    zoomSlider.setRange(0.5, 3.0, 0.1);
    zoomSlider.setValue(1.0);
    zoomSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
    zoomSlider.addListener(this);
    addAndMakeVisible(zoomSlider);
    
    exportButton.setButtonText("Export MIDI");
    exportButton.addListener(this);
    addAndMakeVisible(exportButton);
}

void SpawnCloneAudioProcessorEditor::setupGenerateButton()
{
    generateButton.setButtonText("Generate Pattern");
    generateButton.addListener(this);
    addAndMakeVisible(generateButton);
}

//==============================================================================
void SpawnCloneAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &zoomSlider)
    {
        patternVisualization.setZoomLevel(static_cast<float>(zoomSlider.getValue()));
    }
    // Parameter changes are handled automatically by the attachments
}

void SpawnCloneAudioProcessorEditor::comboBoxChanged(juce::ComboBox* comboBox)
{
    // Parameter changes are handled automatically by the attachments
}

void SpawnCloneAudioProcessorEditor::buttonClicked(juce::Button* button)
{
    if (button == &generateButton)
    {
        // Trigger AI pattern generation
        auto& paramManager = audioProcessor.getParameterManager();
        auto params = paramManager.getCurrentParameters();
        
        audioProcessor.getAIGenerationEngine().generatePattern(params);
        
        // Update UI to show generation is in progress
        patternVisualization.clearPattern();
    }
    else if (button == &showGridButton)
    {
        patternVisualization.setShowGrid(showGridButton.getToggleState());
    }
    else if (button == &showVelocityButton)
    {
        patternVisualization.setShowVelocity(showVelocityButton.getToggleState());
    }
    else if (button == &exportButton)
    {
        // Epic 8 Story 8.3: Export current pattern to MIDI file
        patternVisualization.exportCurrentPatternToFile();
    }
}

void SpawnCloneAudioProcessorEditor::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &audioProcessor.getPatternManager())
    {
        // Pattern manager has new patterns, update display
        updatePatternDisplay();
    }
}

void SpawnCloneAudioProcessorEditor::updatePatternDisplay()
{
    auto& patternManager = audioProcessor.getPatternManager();
    auto patternCount = patternManager.getNumPatterns();
    
    if (patternCount > 0)
    {
        // Get the most recent pattern
        auto pattern = patternManager.getPattern(patternCount - 1);
        if (pattern != nullptr)
        {
            // Update the pattern visualization
            patternVisualization.setPattern(*pattern);
        }
    }
    else
    {
        patternVisualization.clearPattern();
    }
}
