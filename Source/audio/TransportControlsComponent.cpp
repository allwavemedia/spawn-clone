/*
  ==============================================================================

    TransportControlsComponent.cpp
    Created: 30 Jul 2025
    Author:  BMad Master

    Epic 4 Story 4.2: Transport Controls UI Implementation.

  ==============================================================================
*/

#include "TransportControlsComponent.h"

//==============================================================================
TransportControlsComponent::TransportControlsComponent(AudioPreviewEngine& engine)
    : audioEngine(engine), progressBar(progressValue)
{
    setupPlayButton();
    setupPauseButton();
    setupStopButton();
    setupTempoSlider();
    setupProgressBar();
    setupTimeLabel();

    // Start timer for progress updates (30 FPS)
    startTimer(33);
}

//==============================================================================
void TransportControlsComponent::paint(juce::Graphics& g)
{
    // Background
    g.fillAll(backgroundColour);
    
    // Border
    g.setColour(borderColour);
    g.drawRect(getLocalBounds(), 1);
    
    // Title
    auto titleArea = getLocalBounds().removeFromTop(20);
    g.setColour(textColour);
    g.setFont(juce::FontOptions(12.0f, juce::Font::bold));
    g.drawText("Transport", titleArea, juce::Justification::centred);
}

void TransportControlsComponent::resized()
{
    auto bounds = getLocalBounds();
    bounds.removeFromTop(20); // Title space
    bounds.reduce(5, 5);
    
    // Transport buttons row (40px height)
    auto buttonRow = bounds.removeFromTop(40);
    auto buttonWidth = 50;
    auto spacing = 5;
    
    playButton.setBounds(buttonRow.removeFromLeft(buttonWidth));
    buttonRow.removeFromLeft(spacing);
    pauseButton.setBounds(buttonRow.removeFromLeft(buttonWidth));
    buttonRow.removeFromLeft(spacing);
    stopButton.setBounds(buttonRow.removeFromLeft(buttonWidth));
    
    bounds.removeFromTop(10); // Spacing
    
    // Tempo control row (30px height)
    auto tempoRow = bounds.removeFromTop(30);
    tempoLabel.setBounds(tempoRow.removeFromLeft(50));
    tempoRow.removeFromLeft(5);
    tempoSlider.setBounds(tempoRow);
    
    bounds.removeFromTop(10); // Spacing
    
    // Progress bar (20px height)
    auto progressRow = bounds.removeFromTop(20);
    progressBar.setBounds(progressRow);
    
    bounds.removeFromTop(5); // Spacing
    
    // Time display (remaining space)
    timeLabel.setBounds(bounds);
}

//==============================================================================
void TransportControlsComponent::buttonClicked(juce::Button* button)
{
    if (button == &playButton)
    {
        audioEngine.startPlayback();
    }
    else if (button == &pauseButton)
    {
        // For now, just stop (can add pause functionality later)
        audioEngine.stopPlayback();
    }
    else if (button == &stopButton)
    {
        audioEngine.stopPlayback();
    }
    
    updateButtonStates();
}

void TransportControlsComponent::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &tempoSlider)
    {
        // Update audio engine tempo
        // Note: Need to add setTempo method to AudioPreviewEngine
        // audioEngine.setTempo(tempoSlider.getValue());
    }
}

//==============================================================================
void TransportControlsComponent::timerCallback()
{
    // Update progress bar (JUCE ProgressBar automatically monitors progressValue)
    progressValue = audioEngine.getPlaybackPosition();
    
    // Update button states
    updateButtonStates();
    
    // Update time display
    updateTimeDisplay();
}

//==============================================================================
void TransportControlsComponent::updateTransportState()
{
    updateButtonStates();
}

void TransportControlsComponent::setEnabled(bool shouldBeEnabled)
{
    playButton.setEnabled(shouldBeEnabled);
    pauseButton.setEnabled(shouldBeEnabled);
    stopButton.setEnabled(shouldBeEnabled);
    tempoSlider.setEnabled(shouldBeEnabled);
}

//==============================================================================
void TransportControlsComponent::setupPlayButton()
{
    playButton.setButtonText("▶");
    playButton.setColour(juce::TextButton::buttonColourId, buttonColour);
    playButton.setColour(juce::TextButton::textColourOffId, textColour);
    playButton.setColour(juce::TextButton::buttonOnColourId, activeColour);
    playButton.addListener(this);
    addAndMakeVisible(playButton);
}

void TransportControlsComponent::setupPauseButton()
{
    pauseButton.setButtonText("⏸");
    pauseButton.setColour(juce::TextButton::buttonColourId, buttonColour);
    pauseButton.setColour(juce::TextButton::textColourOffId, textColour);
    pauseButton.setColour(juce::TextButton::buttonOnColourId, activeColour);
    pauseButton.addListener(this);
    addAndMakeVisible(pauseButton);
}

void TransportControlsComponent::setupStopButton()
{
    stopButton.setButtonText("⏹");
    stopButton.setColour(juce::TextButton::buttonColourId, buttonColour);
    stopButton.setColour(juce::TextButton::textColourOffId, textColour);
    stopButton.setColour(juce::TextButton::buttonOnColourId, activeColour);
    stopButton.addListener(this);
    addAndMakeVisible(stopButton);
}

void TransportControlsComponent::setupTempoSlider()
{
    tempoLabel.setText("BPM:", juce::dontSendNotification);
    tempoLabel.setColour(juce::Label::textColourId, textColour);
    tempoLabel.setFont(juce::FontOptions(12.0f));
    addAndMakeVisible(tempoLabel);
    
    tempoSlider.setRange(60.0, 200.0, 1.0);
    tempoSlider.setValue(120.0);
    tempoSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    tempoSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
    tempoSlider.setColour(juce::Slider::backgroundColourId, buttonColour);
    tempoSlider.setColour(juce::Slider::thumbColourId, activeColour);
    tempoSlider.setColour(juce::Slider::trackColourId, textColour.withAlpha(0.3f));
    tempoSlider.setColour(juce::Slider::textBoxTextColourId, textColour);
    tempoSlider.setColour(juce::Slider::textBoxBackgroundColourId, buttonColour);
    tempoSlider.addListener(this);
    addAndMakeVisible(tempoSlider);
}

void TransportControlsComponent::setupProgressBar()
{
    progressBar.setPercentageDisplay(false);
    progressBar.setColour(juce::ProgressBar::backgroundColourId, buttonColour);
    progressBar.setColour(juce::ProgressBar::foregroundColourId, activeColour);
    addAndMakeVisible(progressBar);
}

void TransportControlsComponent::setupTimeLabel()
{
    timeLabel.setText("00:00 / 00:00", juce::dontSendNotification);
    timeLabel.setColour(juce::Label::textColourId, textColour);
    timeLabel.setFont(juce::FontOptions(11.0f, juce::Font::plain));
    timeLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(timeLabel);
}

//==============================================================================
void TransportControlsComponent::updateButtonStates()
{
    bool isPlaying = audioEngine.isPlaying();
    
    playButton.setToggleState(isPlaying, juce::dontSendNotification);
    pauseButton.setEnabled(isPlaying);
    stopButton.setEnabled(isPlaying);
}

void TransportControlsComponent::updateTimeDisplay()
{
    // Calculate current and total time
    // Note: Need pattern duration info from AudioPreviewEngine
    double currentTime = progressValue * 10.0; // Placeholder: assume 10 second patterns
    double totalTime = 10.0; // Placeholder
    
    juce::String timeText = formatTime(currentTime) + " / " + formatTime(totalTime);
    timeLabel.setText(timeText, juce::dontSendNotification);
}

juce::String TransportControlsComponent::formatTime(double seconds) const
{
    int minutes = static_cast<int>(seconds) / 60;
    int secs = static_cast<int>(seconds) % 60;
    return juce::String::formatted("%02d:%02d", minutes, secs);
}
