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
    
    // Epic 3 Story 3.2: Pattern History Panel
    PatternHistoryListBox patternHistoryListBox;
    
    // Epic 8: Visualization Controls
    juce::ToggleButton showGridButton;
    juce::ToggleButton showVelocityButton;
    juce::Slider zoomSlider;
    juce::Label zoomLabel;
    juce::TextButton exportButton;  // Epic 8 Story 8.3: Export functionality


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpawnCloneAudioProcessorEditor)
};
