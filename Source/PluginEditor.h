/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class SpawnCloneAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                       private juce::Slider::Listener,
                                       private juce::ComboBox::Listener,
                                       private juce::Button::Listener,
                                       private juce::ChangeListener
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

    void updatePatternList();
    void setupParameterControls();
    void setupPatternDisplay();
    void setupGenerateButton();
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

    juce::TextButton generateButton;
    juce::TextEditor patternDisplay;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpawnCloneAudioProcessorEditor)
};
