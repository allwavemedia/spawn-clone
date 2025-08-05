#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

// Forward declaration to avoid circular dependency
class SpawnCloneAudioProcessor;
class GenerationParameters;

//==============================================================================
/**
    A professional UI component for controlling real-time synthesis parameters.
*/
class SynthesisControlPanel  : public juce::Component,
                               public juce::Slider::Listener,
                               public juce::ComboBox::Listener,
                               public juce::Button::Listener,
                               public juce::Timer
{
public:
    //==============================================================================
    SynthesisControlPanel(SpawnCloneAudioProcessor& processor);
    ~SynthesisControlPanel() override = default;

    //==============================================================================
    // Component overrides
    void paint(juce::Graphics& g) override;
    void resized() override;

    //==============================================================================
    // Listeners
    void sliderValueChanged(juce::Slider* slider) override;
    void comboBoxChanged(juce::ComboBox* comboBox) override;
    void buttonClicked(juce::Button* button) override;
    void timerCallback() override;

    //==============================================================================
    // Public interface
    void updateFromAIParameters(const GenerationParameters& aiParams);
    void setEnabled(bool shouldBeEnabled);
    void resetToDefaults();

private:
    //==============================================================================
    // UI Setup Methods
    void setupFilterControls();
    void setupEnvelopeControls();
    void setupModulationControls();
    void setupCharacterControls();
    void setupWavetableControls();
    void setupControlButtons();

    //==============================================================================
    // UI Layout Methods
    void layoutFilterControls();
    void layoutEnvelopeControls();
    void layoutModulationControls();
    void layoutCharacterControls();
    void layoutWavetableControls();

    //==============================================================================
    // Helper methods
    void configureSlider(juce::Slider& slider, juce::Label& label, const juce::String& labelText,
                         double rangeStart, double rangeEnd, double interval, double initialValue,
                         const juce::String& parameterId);
    
    void configureComboBox(juce::ComboBox& comboBox, juce::Label& label, const juce::String& labelText,
                           const juce::StringArray& items, int initialId, const juce::String& parameterId);

    void updateParameter(const juce::String& paramId, float value);
    void updateParameter(const juce::String& paramId, int value);
    void applyParameterToEngine(const juce::String& paramName, float value);
    void updateUIFromEngine();
    
    void setupSlider(juce::Slider& slider, float min, float max, float defaultValue, 
                     float interval = 0.01f, const juce::String& suffix = "");
    void setupComboBox(juce::ComboBox& combo, const juce::StringArray& options);

    //==============================================================================
    // Member variables
    SpawnCloneAudioProcessor& audioProcessor;

    //==============================================================================
    // UI Components
    
    // Filter Section
    juce::GroupComponent filterGroup;
    juce::Label filterTypeLabel;
    juce::ComboBox filterTypeComboBox;
    juce::Label cutoffLabel;
    juce::Slider cutoffSlider;
    juce::Label resonanceLabel;
    juce::Slider resonanceSlider;
    juce::Label driveLabel;
    juce::Slider driveSlider;
    
    // Envelope Section
    juce::GroupComponent envelopeGroup;
    juce::Label attackLabel;
    juce::Slider attackSlider;
    juce::Label decayLabel;
    juce::Slider decaySlider;
    juce::Label sustainLabel;
    juce::Slider sustainSlider;
    juce::Label releaseLabel;
    juce::Slider releaseSlider;
    
    // Modulation Section
    juce::GroupComponent modulationGroup;
    juce::Label lfo1WaveformLabel;
    juce::ComboBox lfo1WaveformComboBox;
    juce::Label lfo1RateLabel;
    juce::Slider lfo1RateSlider;
    juce::Label lfo1DepthLabel;
    juce::Slider lfo1DepthSlider;
    
    // Character Section
    juce::GroupComponent characterGroup;
    juce::Label brightnessLabel;
    juce::Slider brightnessSlider;
    juce::Label harmonicsLabel;
    juce::Slider harmonicsSlider;
    juce::Label subOscGainLabel;
    juce::Slider subOscGainSlider;
    juce::Label warmthLabel;
    juce::Slider warmthSlider;
    juce::Label saturationLabel;
    juce::Slider saturationSlider;
    
    // Wavetable Section
    juce::GroupComponent wavetableGroup;
    juce::Label wavetablePositionLabel;
    juce::Slider wavetablePositionSlider;
    juce::Label wavetableSelectorLabel;
    juce::ComboBox wavetableSelectorComboBox;
    
    // Control Buttons
    juce::TextButton randomizeButton;
    juce::TextButton resetButton;
    juce::ToggleButton linkToAIButton;

    //==============================================================================
    // Parameter ID mapping
    std::map<juce::Component*, juce::String> parameterMap;

    //==============================================================================
    // State tracking
    bool aiLinked = true;
    bool isUpdatingFromAI = false;

    //==============================================================================
    // UI Layout Constants
    static constexpr int MARGIN = 8;
    static constexpr int GROUP_HEIGHT = 140;
    static constexpr int SLIDER_HEIGHT = 25;
    static constexpr int LABEL_HEIGHT = 20;
    static constexpr int COMBO_HEIGHT = 25;
    static constexpr int BUTTON_HEIGHT = 30;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SynthesisControlPanel)
};
