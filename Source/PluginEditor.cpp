#include "PluginEditor.h"

//==============================================================================
SpawnCloneAudioProcessorEditor::SpawnCloneAudioProcessorEditor (SpawnCloneAudioProcessor& p)
    : AudioProcessorEditor (&p), 
      audioProcessor (p),
      parameterManager(p.getParameterManager()),
      patternManager(p.getPatternManager()),
      patternHistoryListBox(p.getPatternManager()),
      generationProgressBar(generationProgress)
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
    setupAIModeStatusIndicators(); // Epic 7 Story 7.6
    setupExperimentPad(); // Epic 8 Story 8.1: SPAWN-style XY controller
    setupPresetBrowser(); // Epic 9 Story 9.1: Preset browser component
    setupSynthesisControls(); // Phase 1B/1C: AI synthesis parameter control
    setupWaveformDisplay(); // Epic 9.2 Story 9.1: Real-time waveform visualization
    
    // Start listening to pattern changes
    audioProcessor.getPatternManager().addChangeListener(this);
    
    // Epic 4 Story 4.3: Start timer for visual-audio synchronization
    startTimer(30); // Update at ~33 FPS for smooth visual feedback
}

SpawnCloneAudioProcessorEditor::~SpawnCloneAudioProcessorEditor()
{
    stopTimer(); // Epic 4 Story 4.3: Stop visual sync timer
    audioProcessor.getPatternManager().removeChangeListener(this);
    
    // Epic 9.2 Story 9.1: Disconnect waveform display from processor
    audioProcessor.setWaveformDisplay(nullptr);
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
    g.setColour(juce::Colours::white);
    g.setFont(juce::FontOptions("Arial", "Bold", 18.0f));
    g.drawFittedText("SpawnClone", headerBounds.reduced(10, 0), 
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
    
    // Epic 7 Story 7.6: AI Mode Status Indicators
    auto aiStatusRow = bounds.removeFromTop(paramHeight);
    aiStatusLabel.setBounds(aiStatusRow.removeFromLeft(180));
    subscriptionStatusLabel.setBounds(aiStatusRow.removeFromLeft(120));
    
    auto aiControlsRow = bounds.removeFromTop(paramHeight);
    upgradeButton.setBounds(aiControlsRow.removeFromLeft(140).reduced(2));
    generationProgressBar.setBounds(aiControlsRow.removeFromLeft(160).reduced(2));
    
    // Epic 7 Story 7.6 Task 7.6.7: Performance Metrics Display
    auto metricsRow = bounds.removeFromTop(paramHeight);
    performanceMetricsLabel.setBounds(metricsRow.removeFromLeft(200));
    
    auto seedRow = bounds.removeFromTop(paramHeight);
    generationSeedLabel.setBounds(seedRow.removeFromLeft(80));
    generationSeedSlider.setBounds(seedRow.removeFromLeft(220).reduced(3));

    // Phase 1B/1C: Synthesis Controls Section
    bounds.removeFromTop(margin / 2);
    synthesisExpandButton->setBounds(bounds.removeFromTop(25).reduced(margin, 0));
    if (synthesisControlsExpanded)
    {
        synthesisControlPanel->setBounds(bounds.removeFromTop(220).reduced(margin, 0));
    }
    
    // Revolutionary Live Performance Control Panel
    bounds.removeFromTop(margin / 2);
    livePerformancePanel->setBounds(bounds.removeFromTop(180).reduced(margin, 0));
    
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
    
    // Epic 9 Story 9.1: Preset Browser Component (compact placement)
    bounds.removeFromTop(margin);
    auto presetBrowserHeight = 100; // Compact height for preset browser
    presetBrowser.setBounds(bounds.removeFromTop(presetBrowserHeight).reduced(margin/2));
    
    // Epic 8 Story 8.1: SPAWN-style Experiment Pad (prominent placement)
    bounds.removeFromTop(margin);
    auto experimentPadHeight = 140; // Generous height for XY controller
    experimentPad.setBounds(bounds.removeFromTop(experimentPadHeight).reduced(margin/2));
    
    // Epic 9.2 Story 9.1: Real-time Waveform Visualization (prominent placement)
    bounds.removeFromTop(margin);
    auto waveformDisplayHeight = 160; // Height for real-time waveform display
    if (waveformDisplay != nullptr)
    {
        waveformDisplay->setBounds(bounds.removeFromTop(waveformDisplayHeight).reduced(margin/2));
    }
    
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
    // Epic 7 Story 7.6: Update AI mode status when selection changes
    if (comboBox == &aiModeComboBox)
    {
        updateAIModeStatus();
    }
    
    // Parameter changes are handled automatically by the attachments
}

void SpawnCloneAudioProcessorEditor::buttonClicked(juce::Button* button)
{
    if (button == &generateButton)
    {
        // Epic 7 Story 7.6 Task 7.6.7: Start performance timing
        generationStartTime = juce::Time::getHighResolutionTicks();
        
        // Update progress bar
        generationProgress = 0.1;
        generationProgressBar.setTextToDisplay("Generating...");
        
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
    // Epic 7 Story 7.6: Upgrade button handler
    else if (button == &upgradeButton)
    {
        // Open subscription/upgrade URL in default browser
        juce::URL upgradeURL("https://spawn-clone.com/upgrade");
        upgradeURL.launchInDefaultBrowser();
    }
}

void SpawnCloneAudioProcessorEditor::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &audioProcessor.getPatternManager())
    {
        // Epic 7 Story 7.6 Task 7.6.7: Calculate generation performance
        if (generationStartTime > 0)
        {
            auto endTime = juce::Time::getHighResolutionTicks();
            auto elapsedMs = juce::Time::highResolutionTicksToSeconds(endTime - generationStartTime) * 1000.0;
            
            // Get pattern info for metrics
            auto& patternManager = audioProcessor.getPatternManager();
            int noteCount = 0;
            if (patternManager.getNumPatterns() > 0)
            {
                auto currentPattern = patternManager.getCurrentPattern();
                if (currentPattern.has_value())
                {
                    noteCount = static_cast<int>(currentPattern->notes.size());
                }
            }
            
            // Update performance display
            updatePerformanceMetrics(elapsedMs, noteCount);
            
            // Reset progress bar
            generationProgress = 1.0;
            generationProgressBar.setTextToDisplay("Complete");
            
            // Reset timing
            generationStartTime = 0;
        }
        
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
// Epic 7 Story 7.6: AI Mode Status Indicators Setup
void SpawnCloneAudioProcessorEditor::setupAIModeStatusIndicators()
{
    // AI Status Label
    aiStatusLabel.setText("AI Mode: Fast", juce::dontSendNotification);
    aiStatusLabel.setFont(juce::FontOptions(12.0f));
    aiStatusLabel.setJustificationType(juce::Justification::centredLeft);
    aiStatusLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    aiStatusLabel.setHelpText("Current AI generation mode and processing method");
    addAndMakeVisible(aiStatusLabel);
    
    // Subscription Status Label
    subscriptionStatusLabel.setText("Free Plan", juce::dontSendNotification);
    subscriptionStatusLabel.setFont(juce::FontOptions(11.0f));
    subscriptionStatusLabel.setJustificationType(juce::Justification::centredLeft);
    subscriptionStatusLabel.setColour(juce::Label::textColourId, juce::Colours::orange);
    subscriptionStatusLabel.setHelpText("Current subscription tier and available features");
    addAndMakeVisible(subscriptionStatusLabel);
    
    // Upgrade Button
    upgradeButton.setButtonText("Upgrade to Pro");
    upgradeButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkblue);
    upgradeButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    upgradeButton.setHelpText("Click to upgrade your subscription for premium AI features");
    upgradeButton.addListener(this);
    addAndMakeVisible(upgradeButton);
    
    // Generation Progress Bar
    generationProgress = 0.0;
    generationProgressBar.setTextToDisplay("Ready");
    generationProgressBar.setHelpText("Shows generation progress and current status");
    addAndMakeVisible(generationProgressBar);
    
    // Add tooltips to AI Mode ComboBox
    aiModeComboBox.setHelpText("Select AI generation mode:\n"
                              "• Fast: Rule-based generation (<2s)\n"
                              "• Quality: Local ML models (3-5s)\n"
                              "• Cloud: Premium cloud AI (5-8s)");
    
    // Epic 7 Story 7.6 Task 7.6.7: Performance Metrics Display
    performanceMetricsLabel.setText("Generation Time: --", juce::dontSendNotification);
    performanceMetricsLabel.setFont(juce::FontOptions(10.0f));
    performanceMetricsLabel.setJustificationType(juce::Justification::centredLeft);
    performanceMetricsLabel.setColour(juce::Label::textColourId, juce::Colours::grey);
    performanceMetricsLabel.setHelpText("Shows last generation time and performance metrics");
    addAndMakeVisible(performanceMetricsLabel);
    
    // Update initial status
    updateAIModeStatus();
}

//==============================================================================
// Epic 7 Story 7.6: Update AI Mode Status based on current selection
void SpawnCloneAudioProcessorEditor::updateAIModeStatus()
{
    auto currentMode = aiModeComboBox.getSelectedItemIndex();
    
    switch (currentMode)
    {
        case 0: // Fast Mode
            aiStatusLabel.setText("AI Mode: Fast (Rules-based)", juce::dontSendNotification);
            aiStatusLabel.setColour(juce::Label::textColourId, juce::Colours::lightgreen);
            subscriptionStatusLabel.setText("Free Plan - Available", juce::dontSendNotification);
            subscriptionStatusLabel.setColour(juce::Label::textColourId, juce::Colours::lightgreen);
            upgradeButton.setVisible(true);
            break;
            
        case 1: // Quality Mode
            aiStatusLabel.setText("AI Mode: Quality (Local ML)", juce::dontSendNotification);
            aiStatusLabel.setColour(juce::Label::textColourId, juce::Colours::yellow);
            subscriptionStatusLabel.setText("Pro Plan - Local Processing", juce::dontSendNotification);
            subscriptionStatusLabel.setColour(juce::Label::textColourId, juce::Colours::yellow);
            upgradeButton.setVisible(true);
            upgradeButton.setButtonText("Cloud Upgrade");
            break;
            
        case 2: // Cloud Mode
            {
                // Check subscription status from CloudAPIManager
                auto* cloudManager = audioProcessor.getAIGenerationEngine().getCloudAPIManager();
                if (cloudManager && cloudManager->hasValidSubscription())
                {
                    aiStatusLabel.setText("AI Mode: Cloud (Premium)", juce::dontSendNotification);
                    aiStatusLabel.setColour(juce::Label::textColourId, juce::Colours::cyan);
                    subscriptionStatusLabel.setText("Premium Plan - Active", juce::dontSendNotification);
                    subscriptionStatusLabel.setColour(juce::Label::textColourId, juce::Colours::cyan);
                    upgradeButton.setVisible(false);
                }
                else
                {
                    aiStatusLabel.setText("AI Mode: Cloud (No Subscription)", juce::dontSendNotification);
                    aiStatusLabel.setColour(juce::Label::textColourId, juce::Colours::red);
                    subscriptionStatusLabel.setText("Premium Required", juce::dontSendNotification);
                    subscriptionStatusLabel.setColour(juce::Label::textColourId, juce::Colours::red);
                    upgradeButton.setVisible(true);
                    upgradeButton.setButtonText("Subscribe to Premium");
                }
            }
            break;
            
        default:
            aiStatusLabel.setText("AI Mode: Unknown", juce::dontSendNotification);
            aiStatusLabel.setColour(juce::Label::textColourId, juce::Colours::grey);
            break;
    }
}

//==============================================================================
// Epic 7 Story 7.6 Task 7.6.7: Update Performance Metrics Display
void SpawnCloneAudioProcessorEditor::updatePerformanceMetrics(double generationTimeMs, int noteCount)
{
    juce::String metricsText;
    
    if (generationTimeMs > 0)
    {
        metricsText = juce::String::formatted("Generated %d notes in %.1fms", noteCount, generationTimeMs);
        
        // Add quality indicator based on timing
        if (generationTimeMs < 1000.0)
            metricsText += " (Excellent)";
        else if (generationTimeMs < 2000.0)
            metricsText += " (Good)";
        else if (generationTimeMs < 5000.0)
            metricsText += " (Fair)";
        else
            metricsText += " (Slow)";
    }
    else
    {
        metricsText = "Generation Time: --";
    }
    
    performanceMetricsLabel.setText(metricsText, juce::dontSendNotification);
    
    // Update color based on performance
    if (generationTimeMs < 1000.0)
        performanceMetricsLabel.setColour(juce::Label::textColourId, juce::Colours::lightgreen);
    else if (generationTimeMs < 2000.0)
        performanceMetricsLabel.setColour(juce::Label::textColourId, juce::Colours::yellow);
    else if (generationTimeMs < 5000.0)
        performanceMetricsLabel.setColour(juce::Label::textColourId, juce::Colours::orange);
    else
        performanceMetricsLabel.setColour(juce::Label::textColourId, juce::Colours::red);
}

//==============================================================================
// Epic 8 Story 8.1: Setup SPAWN-style Experiment Pad XY Controller
void SpawnCloneAudioProcessorEditor::setupExperimentPad()
{
    // Make the experiment pad visible and configure basic properties
    addAndMakeVisible(experimentPad);
    
    // Set up the pitch range (typical SPAWN range: -12 to +12 semitones)
    experimentPad.setPitchRange(-12.0f, 12.0f);
    
    // Configure default effects mappings (SPAWN-style)
    experimentPad.clearEffectMappings();
    experimentPad.addEffectMapping("Filter Cutoff", 200.0f, 8000.0f);
    experimentPad.addEffectMapping("Reverb Mix", 0.0f, 0.8f);
    experimentPad.addEffectMapping("Delay Feedback", 0.0f, 0.6f);
    
    // Set professional color scheme matching plugin theme
    experimentPad.setPadColors(
        juce::Colour(0xff1a1a1a), // Background
        juce::Colour(0xff4a90e2), // Control point
        juce::Colour(0xff00ff88)  // Accent
    );
    
    // Set up callbacks for real-time parameter updates
    experimentPad.onPitchChanged = [this](float pitchShift)
    {
        // TODO: Implement global pitch shift in AudioPreviewEngine
        // Apply global pitch shift to audio preview engine
        if (auto* audioEngine = audioProcessor.getAudioPreviewEngine())
        {
            // audioEngine->setGlobalPitchShift(pitchShift);
            DBG("Pitch shift requested: " << pitchShift << " semitones");
        }
    };
    
    experimentPad.onEffectsChanged = [this](const std::vector<ExperimentPadComponent::EffectMapping>& effects)
    {
        // TODO: Implement effects processing in AudioPreviewEngine
        // Apply effects parameters to audio preview engine
        if (auto* audioEngine = audioProcessor.getAudioPreviewEngine())
        {
            // Update effects parameters based on X-axis position
            for (const auto& effect : effects)
            {
                if (effect.effectName == "Filter Cutoff")
                {
                    // TODO: Map to filter cutoff frequency
                    // audioEngine->setFilterCutoff(effect.currentValue);
                    DBG("Filter cutoff: " << effect.currentValue);
                }
                else if (effect.effectName == "Reverb Mix")
                {
                    // TODO: Map to reverb wet/dry mix
                    // audioEngine->setReverbMix(effect.currentValue);
                    DBG("Reverb mix: " << effect.currentValue);
                }
                else if (effect.effectName == "Delay Feedback")
                {
                    // TODO: Map to delay feedback amount
                    // audioEngine->setDelayFeedback(effect.currentValue);
                    DBG("Delay feedback: " << effect.currentValue);
                }
            }
        }
    };
    
    experimentPad.onPositionChanged = [this](const ExperimentPadComponent::PadPosition& position)
    {
        // Could be used for additional visual feedback or logging
        DBG("Experiment Pad position: X=" << position.x << ", Y=" << position.y);
    };
    
    // Set up preset callback for future preset management
    experimentPad.setPresetCallback([this](const ExperimentPadComponent::PadPreset& preset)
    {
        // Future implementation: save pad presets to plugin state
        DBG("Experiment Pad preset saved: " << preset.name);
    });
    
    // Epic 6 Phase 2: Initialize Professional Reverb XY Controller
    setupProfessionalReverbControl();
}

//==============================================================================
// Epic 6 Phase 2: Setup Professional Reverb XY Controller Integration
void SpawnCloneAudioProcessorEditor::setupProfessionalReverbControl()
{
    // Create the reverb XY controller
    reverbXYController = std::make_unique<spawnclone::ui::ReverbXYController>();
    
    // Initialize with default studio mapping for professional sound
    reverbXYController->loadStudioMapping();
    
    // Connect to audio processor's effects chain
    // For now, we'll create a demonstration effects chain
    // In production, this would connect to the actual audio processing pipeline
    
    // Enhanced ExperimentPad callbacks for professional reverb control
    experimentPad.onPositionChanged = [this](const ExperimentPadComponent::PadPosition& position)
    {
        if (reverbXYController)
        {
            // Get modifier key states (would need to implement key state tracking)
            bool shiftHeld = false;  // TODO: Implement modifier key detection
            bool ctrlHeld = false;   
            bool altHeld = false;
            
            // Update reverb parameters from XY position
            reverbXYController->updateFromXYPosition(position.x, position.y, shiftHeld, ctrlHeld, altHeld);
        }
        
        DBG("Professional Reverb Control - X=" << position.x << ", Y=" << position.y);
    };
    
    // Set up reverb state change callback for UI feedback
    reverbXYController->onStateChanged = [this](const spawnclone::ui::ReverbXYController::ReverbState& state)
    {
        // Update any reverb parameter displays in the UI
        DBG("Reverb State Updated:");
        DBG("  Room Size: " << state.roomSize);
        DBG("  Reverb Time: " << state.reverbTime << "s");
        DBG("  Damping: " << state.damping);
        DBG("  Stereo Width: " << state.stereoWidth);
        DBG("  Early Reflections: " << state.earlyReflectionsMix);
        DBG("  Pre-delay: " << state.preDelay << "ms");
        
        // Future: Update parameter labels/displays in UI
    };
    
    // Set up algorithm change callback
    reverbXYController->onAlgorithmChanged = [this](spawnclone::audio::ReverbEngine::Algorithm algorithm)
    {
        juce::String algorithmName = "Unknown";
        switch (algorithm)
        {
            case spawnclone::audio::ReverbEngine::Algorithm::Plate: algorithmName = "Plate"; break;
            case spawnclone::audio::ReverbEngine::Algorithm::Hall: algorithmName = "Hall"; break;
            case spawnclone::audio::ReverbEngine::Algorithm::Room: algorithmName = "Room"; break;
            case spawnclone::audio::ReverbEngine::Algorithm::Spring: algorithmName = "Spring"; break;
            case spawnclone::audio::ReverbEngine::Algorithm::Convolution: algorithmName = "Convolution"; break;
            case spawnclone::audio::ReverbEngine::Algorithm::Shimmer: algorithmName = "Shimmer"; break;
            case spawnclone::audio::ReverbEngine::Algorithm::Reverse: algorithmName = "Reverse"; break;
            case spawnclone::audio::ReverbEngine::Algorithm::Gated: algorithmName = "Gated"; break;
        }
        
        DBG("Reverb Algorithm Changed: " << algorithmName);
        // Future: Update algorithm selector in UI
    };
    
    // Enhanced effect mappings to match professional reverb parameters
    experimentPad.clearEffectMappings();
    experimentPad.addEffectMapping("Room Size", 0.0f, 1.0f);
    experimentPad.addEffectMapping("Reverb Time", 0.5f, 6.0f);
    experimentPad.addEffectMapping("Damping", 0.0f, 1.0f);
    experimentPad.addEffectMapping("Stereo Width", 0.5f, 1.5f);
}

//==============================================================================
// Epic 9 Story 9.1: Setup Preset Browser Component
void SpawnCloneAudioProcessorEditor::setupPresetBrowser()
{
    // Make the preset browser visible
    addAndMakeVisible(presetBrowser);
    
    // Connect to the instrument library manager from the audio processor
    if (auto* instrumentLibrary = audioProcessor.getInstrumentLibraryManager())
    {
        presetBrowser.setInstrumentLibraryManager(instrumentLibrary);
    }
    
    // Set up callback for preset selection
    presetBrowser.onPresetSelected = [this](const InstrumentLibraryManager::PresetData& preset)
    {
        DBG("Preset selected: " << preset.name << " (Category: " << preset.category << ")");
        
        // TODO: Apply preset to current generation parameters
        // This will be implemented when we have parameter mapping system
        
        // For now, show confirmation in the UI
        // Could trigger preview or update generation parameters
        if (auto* instrumentLibrary = audioProcessor.getInstrumentLibraryManager())
        {
            // Trigger preview of selected preset if audio engine is available
            if (auto* audioEngine = audioProcessor.getAudioPreviewEngine())
            {
                // audioEngine->previewPreset(preset);
                DBG("Would preview preset: " << preset.name);
            }
        }
    };
    
    // Set up search and filter callbacks for user interaction feedback
    // Note: These will be handled internally by the PresetBrowserComponent
    // through its TextEditor::Listener and ComboBox::Listener interfaces
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

//==============================================================================
// Phase 1B/1C: AI Synthesis Parameter Control
void SpawnCloneAudioProcessorEditor::setupSynthesisControls()
{
    // Initialize synthesis control panel
    synthesisControlPanel = std::make_unique<SynthesisControlPanel>(audioProcessor);
    addAndMakeVisible(*synthesisControlPanel);
    
    // Initialize expand/collapse button for synthesis controls
    synthesisExpandButton = std::make_unique<juce::TextButton>("Synthesis");
    synthesisExpandButton->setButtonText("Synthesis ▼");
    synthesisExpandButton->setColour(juce::TextButton::buttonColourId, juce::Colour(0xff404040));
    synthesisExpandButton->setColour(juce::TextButton::textColourOffId, juce::Colour(0xffcccccc));
    synthesisExpandButton->onClick = [this]()
    {
        synthesisControlsExpanded = !synthesisControlsExpanded;
        synthesisExpandButton->setButtonText(synthesisControlsExpanded ? "Synthesis ▲" : "Synthesis ▼");
        synthesisControlPanel->setVisible(synthesisControlsExpanded);
        resized(); // Update layout
    };
    addAndMakeVisible(*synthesisExpandButton);
    
    // Start collapsed by default to save screen space
    synthesisControlsExpanded = false;
    synthesisControlPanel->setVisible(false);
    
    // Initialize live performance control panel
    livePerformancePanel = std::make_unique<spawnclone::ui::LivePerformanceControlPanel>(audioProcessor);
    addAndMakeVisible(*livePerformancePanel);
}

//==============================================================================
// Epic 9.2 Story 9.1: Setup Real-time Waveform Visualization
void SpawnCloneAudioProcessorEditor::setupWaveformDisplay()
{
    // Initialize the waveform display component
    waveformDisplay = std::make_unique<WaveformDisplayComponent>();
    addAndMakeVisible(*waveformDisplay);
    
    // Configure default visualization settings
    WaveformDisplayComponent::VisualizationSettings settings;
    settings.mode = WaveformDisplayComponent::Oscilloscope;
    settings.refreshRate = 60; // 60fps for smooth real-time visualization
    settings.timeScale = 1.0f;
    settings.amplitudeScale = 1.0f;
    settings.showGrid = true;
    settings.showLabels = true;
    settings.waveformColour = juce::Colour(0xff00ff88); // Match plugin accent color
    settings.gridColour = juce::Colour(0xff333333);
    settings.backgroundColour = juce::Colour(0xff1a1a1a);
    settings.triggerMode = WaveformDisplayComponent::Auto;
    settings.triggerLevel = 0.1f;
    
    waveformDisplay->setVisualizationSettings(settings);
    
    // Set up Epic 7 AI integration callback
    waveformDisplay->setAIGenerationCallback([this](const std::vector<float>& pattern) {
        // Forward AI-generated patterns to the pattern manager
        auto& patternManager = audioProcessor.getPatternManager();
        // TODO: Convert float pattern to MIDI pattern and add to pattern manager
        DBG("AI pattern received from waveform display: " << pattern.size() << " samples");
    });
    
    // Connect to audio processor for real-time audio data
    // This will be called from the audio processor's audio callback
    audioProcessor.setWaveformDisplay(waveformDisplay.get());
    
    DBG("WaveformDisplayComponent initialized with Epic 7 AI integration");
}
