/*
  ==============================================================================

    TransportControlsComponent.h
    Created: 30 Jul 2025
    Author:  BMad Master

    Epic 4 Story 4.2: Transport Controls UI Component.
    Provides play/pause/stop controls and tempo adjustment for audio playback.

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_data_structures/juce_data_structures.h>
#include "AudioPreviewEngine.h"

//==============================================================================
/**
    Professional transport controls for audio pattern playback.
    Includes play/pause/stop buttons, tempo control, and progress display.
*/
class TransportControlsComponent : public juce::Component,
                                  public juce::Button::Listener,
                                  public juce::Slider::Listener,
                                  public juce::Timer
{
public:
    //==============================================================================
    TransportControlsComponent(AudioPreviewEngine& engine);
    ~TransportControlsComponent() override = default;

    //==============================================================================
    // Component overrides
    void paint(juce::Graphics& g) override;
    void resized() override;

    //==============================================================================
    // Button::Listener implementation
    void buttonClicked(juce::Button* button) override;

    //==============================================================================
    // Slider::Listener implementation
    void sliderValueChanged(juce::Slider* slider) override;

    //==============================================================================
    // Timer for progress updates
    void timerCallback() override;

    //==============================================================================
    // Public interface
    void updateTransportState();
    void setEnabled(bool shouldBeEnabled);

private:
    //==============================================================================
    AudioPreviewEngine& audioEngine;

    //==============================================================================
    // Transport buttons
    juce::TextButton playButton;
    juce::TextButton pauseButton;
    juce::TextButton stopButton;

    //==============================================================================
    // Tempo control
    juce::Slider tempoSlider;
    juce::Label tempoLabel;

    //==============================================================================
    // Progress display
    juce::ProgressBar progressBar;
    double progressValue = 0.0;

    //==============================================================================
    // Time display
    juce::Label timeLabel;

    //==============================================================================
    // UI Colors (matching Epic 3 theme)
    juce::Colour backgroundColour = juce::Colour(0xff2a2a2a);
    juce::Colour buttonColour = juce::Colour(0xff404040);
    juce::Colour activeColour = juce::Colour(0xff4a9eff);
    juce::Colour textColour = juce::Colour(0xffcccccc);
    juce::Colour borderColour = juce::Colour(0xff555555);

    //==============================================================================
    // Helper methods
    void setupPlayButton();
    void setupPauseButton();
    void setupStopButton();
    void setupTempoSlider();
    void setupProgressBar();
    void setupTimeLabel();
    
    void updateButtonStates();
    void updateTimeDisplay();
    juce::String formatTime(double seconds) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TransportControlsComponent)
};
