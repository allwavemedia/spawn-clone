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
    setSize (600, 400);

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
    
    // Generate button
    bounds.removeFromTop(margin);
    generateButton.setBounds(bounds.removeFromTop(40).reduced(margin));
    
    // Pattern display (remaining space)
    bounds.removeFromTop(margin);
    patternDisplay.setBounds(bounds.reduced(margin));
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
    
    addAndMakeVisible(keyLabel);
    addAndMakeVisible(scaleLabel);
    addAndMakeVisible(generationTypeLabel);
    addAndMakeVisible(tempoLabel);
    addAndMakeVisible(complexityLabel);
    addAndMakeVisible(patternLengthLabel);
    
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
}

void SpawnCloneAudioProcessorEditor::setupPatternDisplay()
{
    patternDisplay.setFont(juce::Font(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 12.0f, juce::Font::plain)));
    patternDisplay.setMultiLine(true);
    patternDisplay.setReturnKeyStartsNewLine(true);
    patternDisplay.setReadOnly(true);
    patternDisplay.setScrollbarsShown(true);
    patternDisplay.setCaretVisible(false);
    patternDisplay.setPopupMenuEnabled(false);
    patternDisplay.setText("No pattern generated yet...");
    addAndMakeVisible(patternDisplay);
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
        patternDisplay.setText("Generating pattern...");
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
            // Format pattern information for display
            juce::String displayText;
            displayText += "Pattern ID: " + pattern->id.toString() + "\n";
            displayText += "Length: " + juce::String(pattern->lengthInBeats, 1) + " beats\n";
            displayText += "Key: " + juce::String(static_cast<int>(pattern->metadata.key)) + "\n";
            displayText += "Scale: " + juce::String(static_cast<int>(pattern->metadata.scale)) + "\n";
            displayText += "Tempo: " + juce::String(pattern->metadata.tempo, 1) + " BPM\n";
            displayText += "Notes: " + juce::String(pattern->notes.size()) + "\n\n";
            
            // Add note information
            displayText += "MIDI Notes:\n";
            for (const auto& note : pattern->notes)
            {
                displayText += "Note " + juce::String(note.pitch) + 
                              " @ " + juce::String(note.startTime, 2) + 
                              "s (vel " + juce::String(note.velocity) + ")\n";
            }
            
            patternDisplay.setText(displayText);
        }
    }
    else
    {
        patternDisplay.setText("No patterns generated yet...");
    }
}
