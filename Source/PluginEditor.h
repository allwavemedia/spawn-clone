/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "PatternVisualizationComponent.h"
#include "PatternHistoryListBox.h"
#include "audio/TransportControlsComponent.h"
#include "ui/ExperimentPadComponent.h"
#include "ui/PresetBrowserComponent.h"
#include "ui/SynthesisControlPanel.h"
#include "ui/ReverbXYController.h"  // Epic 6 Phase 2: Professional Reverb Control

//==============================================================================
/**
*/
class SpawnCloneAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                       private juce::Slider::Listener,
                                       private juce::ComboBox::Listener,
                                       private juce::Button::Listener,
                                       private juce::ChangeListener,
                                       private juce::Timer
{
public:
    SpawnCloneAudioProcessorEditor (SpawnCloneAudioProcessor&);
    ~SpawnCloneAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void sliderValueChanged (juce::Slider* slider) override;
    void comboBoxChanged (juce::ComboBox* comboBox) override;
    void buttonClicked (juce::Button* button) override;
    void changeListenerCallback (juce::ChangeBroadcaster* source) override;
    void timerCallback() override; // Epic 4 Story 4.3: Visual-Audio synchronization

    void updatePatternList();
    void setupParameterControls();
    void setupPatternDisplay();
    void setupGenerateButton();
    void setupPreviewControls(); // Epic 2 Story 2.2
    void setupMasterVolumeControl(); // Epic 3 Story 3.1
    void setupPatternHistoryPanel(); // Epic 3 Story 3.2
    void setupTransportControls(); // Epic 4 Story 4.2
    void setupAIModeStatusIndicators(); // Epic 7 Story 7.6
    void setupExperimentPad(); // Epic 8 Story 8.1: SPAWN-style XY controller
    void setupProfessionalReverbControl(); // Epic 6 Phase 2: Professional Reverb XY integration
    void setupPresetBrowser(); // Epic 9 Story 9.1: Preset browser component
    void setupSynthesisControls(); // Phase 1B & 1C: Advanced synthesis controls
    void updateAIModeStatus(); // Epic 7 Story 7.6
    void updatePerformanceMetrics(double generationTimeMs, int noteCount); // Epic 7 Story 7.6.7
    void updatePatternDisplay();

    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    SpawnCloneAudioProcessor& audioProcessor;

    ParameterManager& parameterManager;
    PatternManager& patternManager;

    juce::Label titleLabel;

    juce::Label keyLabel;
    juce::ComboBox keySignatureComboBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> keySignatureAttachment;

    juce::Label scaleLabel;
    juce::ComboBox scaleTypeComboBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> scaleTypeAttachment;

    juce::Label tempoLabel;
    juce::Slider tempoSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> tempoAttachment;

    juce::Label complexityLabel;
    juce::Slider complexitySlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> complexityAttachment;

    juce::Label generationTypeLabel;
    juce::ComboBox generationTypeComboBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> generationTypeAttachment;

    juce::Label patternLengthLabel;
    juce::Slider patternLengthSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> patternLengthAttachment;

    // Epic 7: AI Mode Selection Controls
    juce::Label aiModeLabel;
    juce::ComboBox aiModeComboBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> aiModeAttachment;
    
    // Epic 7 Story 7.6: AI Mode Status and Subscription Indicators
    juce::Label aiStatusLabel;
    juce::Label subscriptionStatusLabel;
    juce::TextButton upgradeButton;
    double generationProgress = 0.0;
    juce::ProgressBar generationProgressBar;
    
    // Epic 7 Story 7.6 Task 7.6.7: Performance Metrics Display
    juce::Label performanceMetricsLabel;
    juce::int64 generationStartTime = 0;
    
    juce::Label generationSeedLabel;
    juce::Slider generationSeedSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> generationSeedAttachment;

    juce::TextButton generateButton;
    
    // Epic 2 Story 2.2: Audio Preview Controls
    juce::TextButton previewButton;
    juce::TextButton stopPreviewButton;
    juce::ToggleButton loopToggleButton;
    juce::ToggleButton dawSyncToggleButton;
    
    // Epic 4 Story 4.2: Transport Controls
    std::unique_ptr<TransportControlsComponent> transportControls;
    
    // Epic 3 Story 3.1: Master Volume Control
    juce::Label masterVolumeLabel;
    juce::Slider masterVolumeSlider;
    
    // Epic 8: Pattern Visualization Component
    PatternVisualizationComponent patternVisualization;
    
    // Epic 8 Story 8.1: SPAWN-style Experiment Pad XY Controller
    ExperimentPadComponent experimentPad;
    
    // Epic 6 Phase 2: Professional Reverb XY Controller Integration
    std::unique_ptr<spawnclone::ui::ReverbXYController> reverbXYController;
    
    // Epic 9 Story 9.1: Preset Browser Component
    PresetBrowserComponent presetBrowser;
    
    // Epic 3 Story 3.2: Pattern History Panel
    PatternHistoryListBox patternHistoryListBox;
    
    // Epic 8: Visualization Controls
    juce::ToggleButton showGridButton;
    juce::ToggleButton showVelocityButton;
    juce::Slider zoomSlider;
    juce::Label zoomLabel;
    juce::TextButton exportButton;  // Epic 8 Story 8.3: Export functionality
    
        // Phase 1B/1C: AI Synthesis Parameter Control
    std::unique_ptr<SynthesisControlPanel> synthesisControlPanel;
    std::unique_ptr<juce::TextButton> synthesisExpandButton;
    bool synthesisControlsExpanded = false;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpawnCloneAudioProcessorEditor)
};
