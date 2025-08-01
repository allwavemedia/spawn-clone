#include "PluginEditor.h"

//==============================================================================
SpawnCloneAudioProcessorEditor::SpawnCloneAudioProcessorEditor (SpawnCloneAudioProcessor& p)
    : AudioProcessorEditor (&p), 
      audioProcessor (p),
      parameterManager(p.getParameterManager()),
      patternManager(p.getPatternManager()),
      patternHistoryListBox(p.getPatternManager())
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (1020, 520);  // Increased width for pattern history panel (700 + 320)
    setResizable(true, true); // Epic 3 Story 3.1: Resizable window
    setResizeLimits(920, 450, 1500, 800); // Updated min and max size constraints

    // Initialize UI components
    setupParameterControls();
    setupPatternDisplay();
    setupGenerateButton();
    setupPreviewControls(); // Epic 2 Story 2.2
    setupMasterVolumeControl(); // Epic 3 Story 3.1
    setupPatternHistoryPanel(); // Epic 3 Story 3.2
    setupTransportControls(); // Epic 4 Story 4.2
    
    // Start listening to pattern changes
    audioProcessor.getPatternManager().addChangeListener(this);
    
    // Epic 4 Story 4.3: Start timer for visual-audio synchronization
    startTimer(30); // Update at ~33 FPS for smooth visual feedback
}

SpawnCloneAudioProcessorEditor::~SpawnCloneAudioProcessorEditor()
{
    stopTimer(); // Epic 4 Story 4.3: Stop visual sync timer
    audioProcessor.getPatternManager().removeChangeListener(this);
}

//==============================================================================
void SpawnCloneAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Epic 3 Story 3.1: Professional styling
    
    // Modern gradient background
    auto bounds = getLocalBounds();
    juce::ColourGradient gradient(juce::Colour(0xff2a2a2a), 0, 0, 
                                  juce::Colour(0xff1a1a1a), 0, static_cast<float>(bounds.getHeight()), false);
    g.setGradientFill(gradient);
    g.fillAll();
    
    // Header section with title
    auto headerBounds = bounds.removeFromTop(40);
    g.setColour(juce::Colour(0xff3a3a3a));
    g.fillRect(headerBounds);
    
    // Title text with professional styling
    g.setColour(juce::Colour(0xffeeeeee));
    g.setFont(juce::FontOptions("Arial", "Bold", 18.0f));
    g.drawFittedText("SpawnClone AI MIDI Generator", headerBounds.reduced(10, 0), 
                     juce::Justification::centredLeft, 1);
    
    // Version/status indicator
    g.setColour(juce::Colour(0xff888888));
    g.setFont(juce::FontOptions("Arial", "Regular", 12.0f));
    g.drawFittedText("v1.0", headerBounds.reduced(10, 0), 
                     juce::Justification::centredRight, 1);
    
    // Section separators
    g.setColour(juce::Colour(0xff404040));
    
    // Parameter section separator
    auto paramSectionY = 40 + (8 * 30) + 10; // After parameter controls
    g.drawHorizontalLine(paramSectionY, 10.0f, static_cast<float>(getWidth() - 10));
    
    // Preview section separator  
    auto previewSectionY = paramSectionY + 60; // After generate button
    g.drawHorizontalLine(previewSectionY, 10.0f, static_cast<float>(getWidth() - 10));
}

void SpawnCloneAudioProcessorEditor::resized()
{
    // Epic 3 Story 3.1 & 3.2: Responsive layout with pattern history panel
    auto bounds = getLocalBounds();
    bounds.removeFromTop(40); // Space for professional header
    
    // Epic 3 Story 3.2: Split layout - main controls left, pattern history right
    auto patternHistoryPanel = bounds.removeFromRight(320); // Right panel for pattern history
    bounds.removeFromRight(10); // Spacing between panels
    
    // Pattern history panel layout
    patternHistoryListBox.setBounds(patternHistoryPanel);
    
    // Main controls layout (left side)
    auto paramHeight = 30; // Slightly increased for better appearance
    auto margin = 12; // Increased margin for professional spacing
    
    // Add some top margin for visual breathing room
    bounds.removeFromTop(margin);
    
    // Parameter controls section with organized layout
    // Musical parameters group
    auto keyRow = bounds.removeFromTop(paramHeight);
    keyLabel.setBounds(keyRow.removeFromLeft(80));
    keySignatureComboBox.setBounds(keyRow.removeFromLeft(120).reduced(3));
    
    auto scaleRow = bounds.removeFromTop(paramHeight);
    scaleLabel.setBounds(scaleRow.removeFromLeft(80));
    scaleTypeComboBox.setBounds(scaleRow.removeFromLeft(120).reduced(3));
    
    auto genRow = bounds.removeFromTop(paramHeight);
    generationTypeLabel.setBounds(genRow.removeFromLeft(80));
    generationTypeComboBox.setBounds(genRow.removeFromLeft(120).reduced(3));
    
    // Timing and complexity parameters
    auto tempoRow = bounds.removeFromTop(paramHeight);
    tempoLabel.setBounds(tempoRow.removeFromLeft(80));
    tempoSlider.setBounds(tempoRow.removeFromLeft(220).reduced(3));
    
    auto complexityRow = bounds.removeFromTop(paramHeight);
    complexityLabel.setBounds(complexityRow.removeFromLeft(80));
    complexitySlider.setBounds(complexityRow.removeFromLeft(220).reduced(3));
    
    auto lengthRow = bounds.removeFromTop(paramHeight);
    patternLengthLabel.setBounds(lengthRow.removeFromLeft(80));
    patternLengthSlider.setBounds(lengthRow.removeFromLeft(220).reduced(3));
    
    // AI Mode controls
    auto aiModeRow = bounds.removeFromTop(paramHeight);
    aiModeLabel.setBounds(aiModeRow.removeFromLeft(80));
    aiModeComboBox.setBounds(aiModeRow.removeFromLeft(120).reduced(3));
    
    auto seedRow = bounds.removeFromTop(paramHeight);
    generationSeedLabel.setBounds(seedRow.removeFromLeft(80));
    generationSeedSlider.setBounds(seedRow.removeFromLeft(220).reduced(3));
    
    // Generate button with enhanced styling
    bounds.removeFromTop(margin);
    generateButton.setBounds(bounds.removeFromTop(45).reduced(margin));
    
    // Preview controls section with proper spacing
    bounds.removeFromTop(margin);
    auto previewRow = bounds.removeFromTop(paramHeight);
    previewButton.setBounds(previewRow.removeFromLeft(85).reduced(3));
    stopPreviewButton.setBounds(previewRow.removeFromLeft(65).reduced(3));
    loopToggleButton.setBounds(previewRow.removeFromLeft(65).reduced(3));
    dawSyncToggleButton.setBounds(previewRow.removeFromLeft(85).reduced(3));
    
    // Master Volume Control with proper spacing
    bounds.removeFromTop(margin/2);
    auto volumeRow = bounds.removeFromTop(paramHeight);
    masterVolumeLabel.setBounds(volumeRow.removeFromLeft(80));
    masterVolumeSlider.setBounds(volumeRow.removeFromLeft(220).reduced(3));
    
    // Epic 4 Story 4.2: Transport Controls
    bounds.removeFromTop(margin);
    auto transportHeight = 120; // Height for transport controls
    if (transportControls != nullptr)
    {
        transportControls->setBounds(bounds.removeFromTop(transportHeight).reduced(margin/2));
    }
    
    // Visualization controls with enhanced layout
    bounds.removeFromTop(margin);
    auto controlsRow = bounds.removeFromTop(paramHeight);
    showGridButton.setBounds(controlsRow.removeFromLeft(85).reduced(3));
    showVelocityButton.setBounds(controlsRow.removeFromLeft(85).reduced(3));
    zoomLabel.setBounds(controlsRow.removeFromLeft(45));
    zoomSlider.setBounds(controlsRow.removeFromLeft(110).reduced(3));
    exportButton.setBounds(controlsRow.removeFromLeft(110).reduced(3));
    
    // Pattern visualization (remaining space with proper margins)
    bounds.removeFromTop(margin);
    patternVisualization.setBounds(bounds.reduced(margin));
}

//==============================================================================
void SpawnCloneAudioProcessorEditor::setupParameterControls()
{
    auto& params = audioProcessor.getParameterManager();
    auto& valueTreeState = params.getValueTreeState();
    
    // Epic 3 Story 3.1: Professional label styling
    auto setupLabel = [this](juce::Label& label, const juce::String& text) {
        label.setText(text, juce::dontSendNotification);
        label.setFont(juce::FontOptions("Arial", "Regular", 14.0f));
        label.setColour(juce::Label::textColourId, juce::Colour(0xffdddddd));
        label.setJustificationType(juce::Justification::centredRight);
        addAndMakeVisible(label);
    };
    
    // Setup all labels with consistent styling
    setupLabel(keyLabel, "Key:");
    setupLabel(scaleLabel, "Scale:");
    setupLabel(generationTypeLabel, "Type:");
    setupLabel(tempoLabel, "Tempo:");
    setupLabel(complexityLabel, "Complexity:");
    setupLabel(patternLengthLabel, "Length:");
    setupLabel(aiModeLabel, "AI Mode:");
    setupLabel(generationSeedLabel, "Seed:");
    
    // Professional ComboBox styling helper
    auto setupComboBox = [this](juce::ComboBox& combo) {
        combo.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff3a3a3a));
        combo.setColour(juce::ComboBox::textColourId, juce::Colour(0xffeeeeee));
        combo.setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff555555));
        combo.setColour(juce::ComboBox::arrowColourId, juce::Colour(0xffcccccc));
        addAndMakeVisible(combo);
    };
    
    // Key selection with enhanced styling
    keySignatureComboBox.addItemList(params.getKeySignatureItems(), 1);
    keySignatureComboBox.addListener(this);
    setupComboBox(keySignatureComboBox);
    keySignatureAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        valueTreeState, "key", keySignatureComboBox);
    
    // Scale selection with enhanced styling
    scaleTypeComboBox.addItemList(params.getScaleTypeItems(), 1);
    scaleTypeComboBox.addListener(this);
    setupComboBox(scaleTypeComboBox);
    scaleTypeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        valueTreeState, "scale", scaleTypeComboBox);
    
    // Generation type selection with enhanced styling
    generationTypeComboBox.addItemList(params.getGenerationTypeItems(), 1);
    generationTypeComboBox.addListener(this);
    setupComboBox(generationTypeComboBox);
    generationTypeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        valueTreeState, "generationType", generationTypeComboBox);
    
    // Professional Slider styling helper
    auto setupSlider = [this](juce::Slider& slider) {
        slider.setSliderStyle(juce::Slider::LinearHorizontal);
        slider.setColour(juce::Slider::backgroundColourId, juce::Colour(0xff2a2a2a));
        slider.setColour(juce::Slider::trackColourId, juce::Colour(0xff444444));
        slider.setColour(juce::Slider::thumbColourId, juce::Colour(0xff66aadd));
        slider.setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xffeeeeee));
        slider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff3a3a3a));
        slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0xff555555));
        slider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 65, 22);
        addAndMakeVisible(slider);
    };
    
    // Tempo slider with enhanced styling
    tempoSlider.setRange(60.0, 200.0, 1.0);
    tempoSlider.addListener(this);
    setupSlider(tempoSlider);
    tempoAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        valueTreeState, "tempo", tempoSlider);
    
    // Complexity slider with enhanced styling
    complexitySlider.setRange(0.0, 1.0, 0.01);
    complexitySlider.addListener(this);
    setupSlider(complexitySlider);
    complexityAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        valueTreeState, "complexity", complexitySlider);
    
    // Pattern length slider with enhanced styling
    patternLengthSlider.setRange(4.0, 32.0, 4.0);
    patternLengthSlider.addListener(this);
    setupSlider(patternLengthSlider);
    patternLengthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        valueTreeState, "patternLength", patternLengthSlider);
    
    // Epic 7: AI Mode selection with enhanced styling
    aiModeComboBox.addItemList(params.getAIModeItems(), 1);
    aiModeComboBox.addListener(this);
    setupComboBox(aiModeComboBox);
    aiModeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        valueTreeState, "aiMode", aiModeComboBox);
    
    // Generation seed slider with enhanced styling
    generationSeedSlider.setRange(0.0, 10000.0, 1.0);
    generationSeedSlider.addListener(this);
    setupSlider(generationSeedSlider);
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
    // Epic 3 Story 3.1: Enhanced generate button styling
    generateButton.setButtonText("🎵 Generate Pattern");
    generateButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff4a90e2));
    generateButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff357abd));
    generateButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffffffff));
    generateButton.setColour(juce::TextButton::textColourOnId, juce::Colour(0xffffffff));
    generateButton.addListener(this);
    addAndMakeVisible(generateButton);
}

void SpawnCloneAudioProcessorEditor::setupPreviewControls()
{
    // Epic 2 Story 2.2: Audio Preview Controls with Epic 3 Story 3.1 styling
    
    // Professional button styling helper
    auto setupButton = [this](juce::Button& button, const juce::Colour& baseColor) {
        button.setColour(juce::TextButton::buttonColourId, baseColor);
        button.setColour(juce::TextButton::buttonOnColourId, baseColor.brighter(0.2f));
        button.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffffffff));
        button.setColour(juce::TextButton::textColourOnId, juce::Colour(0xffffffff));
        addAndMakeVisible(button);
    };
    
    // Preview button with professional styling
    previewButton.setButtonText("▶ Preview");
    previewButton.addListener(this);
    setupButton(previewButton, juce::Colour(0xff5cb85c));
    
    // Stop preview button with professional styling
    stopPreviewButton.setButtonText("⏹ Stop");
    stopPreviewButton.addListener(this);
    setupButton(stopPreviewButton, juce::Colour(0xffd9534f));
    
    // Loop toggle with professional styling
    loopToggleButton.setButtonText("Loop");
    loopToggleButton.setToggleState(true, juce::dontSendNotification); // Default to looping
    loopToggleButton.addListener(this);
    setupButton(loopToggleButton, juce::Colour(0xff5bc0de));
    
    // DAW sync toggle with professional styling
    dawSyncToggleButton.setButtonText("DAW Sync");
    dawSyncToggleButton.setToggleState(false, juce::dontSendNotification);
    dawSyncToggleButton.addListener(this);
    setupButton(dawSyncToggleButton, juce::Colour(0xfff0ad4e));
}

void SpawnCloneAudioProcessorEditor::setupMasterVolumeControl()
{
    // Epic 3 Story 3.1: Master Volume Control with professional styling
    
    // Master volume label with consistent styling
    masterVolumeLabel.setText("Volume:", juce::dontSendNotification);
    masterVolumeLabel.setFont(juce::FontOptions("Arial", "Regular", 14.0f));
    masterVolumeLabel.setColour(juce::Label::textColourId, juce::Colour(0xffdddddd));
    masterVolumeLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(masterVolumeLabel);
    
    // Master volume slider with professional styling
    masterVolumeSlider.setRange(0.0, 1.0, 0.01);
    masterVolumeSlider.setValue(0.7, juce::dontSendNotification); // Default 70%
    masterVolumeSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    masterVolumeSlider.setColour(juce::Slider::backgroundColourId, juce::Colour(0xff2a2a2a));
    masterVolumeSlider.setColour(juce::Slider::trackColourId, juce::Colour(0xff444444));
    masterVolumeSlider.setColour(juce::Slider::thumbColourId, juce::Colour(0xff66dd66)); // Green for volume
    masterVolumeSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xffeeeeee));
    masterVolumeSlider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff3a3a3a));
    masterVolumeSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0xff555555));
    masterVolumeSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 65, 22);
    masterVolumeSlider.setNumDecimalPlacesToDisplay(2);
    masterVolumeSlider.addListener(this);
    addAndMakeVisible(masterVolumeSlider);
}

//==============================================================================
// Epic 4 Story 4.2: Transport Controls Setup

void SpawnCloneAudioProcessorEditor::setupTransportControls()
{
    // Create transport controls component with reference to audio engine
    if (auto* audioEngine = audioProcessor.getAudioPreviewEngine())
    {
        transportControls = std::make_unique<TransportControlsComponent>(*audioEngine);
        addAndMakeVisible(transportControls.get());
    }
}

//==============================================================================
void SpawnCloneAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &zoomSlider)
    {
        patternVisualization.setZoomLevel(static_cast<float>(zoomSlider.getValue()));
    }
    else if (slider == &masterVolumeSlider)
    {
        // Epic 3 Story 3.1: Master Volume Control
        audioProcessor.getAudioPreviewEngine()->setMasterVolume(static_cast<float>(masterVolumeSlider.getValue()));
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
    // Epic 2 Story 2.2: Preview control handlers
    else if (button == &previewButton)
    {
        audioProcessor.previewCurrentPattern();
    }
    else if (button == &stopPreviewButton)
    {
        audioProcessor.stopAudioPreview();
    }
    else if (button == &loopToggleButton)
    {
        if (auto* previewEngine = audioProcessor.getAudioPreviewEngine())
        {
            previewEngine->setLooping(loopToggleButton.getToggleState());
        }
    }
    else if (button == &dawSyncToggleButton)
    {
        if (auto* previewEngine = audioProcessor.getAudioPreviewEngine())
        {
            previewEngine->setDAWTransportSync(dawSyncToggleButton.getToggleState());
        }
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

//==============================================================================
// Epic 3 Story 3.2: Pattern History Panel Setup
void SpawnCloneAudioProcessorEditor::setupPatternHistoryPanel()
{
    addAndMakeVisible(patternHistoryListBox);
    
    // Setup callbacks for pattern interactions
    patternHistoryListBox.onPatternPreview = [this](int patternIndex)
    {
        // Get the pattern from the reversed list (newest first in UI)
        auto recentPatterns = patternManager.getRecentPatterns(20);
        std::reverse(recentPatterns.begin(), recentPatterns.end());
        
        if (patternIndex >= 0 && patternIndex < static_cast<int>(recentPatterns.size()))
        {
            auto& selectedPattern = recentPatterns[patternIndex];
            
            // Update the main pattern visualization
            patternVisualization.setPattern(selectedPattern);
            
            // Trigger the preview using existing preview system
            audioProcessor.previewCurrentPattern();
        }
    };
    
    patternHistoryListBox.onPatternSelect = [this](int patternIndex)
    {
        // Get the pattern from the reversed list (newest first in UI)
        auto recentPatterns = patternManager.getRecentPatterns(20);
        std::reverse(recentPatterns.begin(), recentPatterns.end());
        
        if (patternIndex >= 0 && patternIndex < static_cast<int>(recentPatterns.size()))
        {
            auto& selectedPattern = recentPatterns[patternIndex];
            
            // Update the main pattern visualization
            patternVisualization.setPattern(selectedPattern);
            
            // Update the display
            updatePatternDisplay();
        }
    };
    
    patternHistoryListBox.onPatternDelete = [this](int patternIndex)
    {
        // Note: For now, we don't have a delete method in PatternManager
        // This would require adding removePattern(index) method to PatternManager
        // For MVP, we'll just update the display
        updatePatternDisplay();
    };
}

//==============================================================================
// Epic 4 Story 4.3: Visual-Audio Synchronization
void SpawnCloneAudioProcessorEditor::timerCallback()
{
    // Update visual synchronization with audio playback
    if (auto* audioEngine = audioProcessor.getAudioPreviewEngine())
    {
        bool isPlaying = audioEngine->isPlaying();
        double playbackPosition = audioEngine->getPlaybackPosition();
        
        // Update pattern visualization with current playback state
        patternVisualization.setPlaybackMode(isPlaying);
        patternVisualization.setPlaybackPosition(playbackPosition);
    }
}
