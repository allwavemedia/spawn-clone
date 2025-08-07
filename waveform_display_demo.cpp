/**
 * Epic 9.2 Story 9.1 - WaveformDisplayComponent Integration Demo
 * 
 * This demonstrates the successful integration of the WaveformDisplayComponent
 * into the SpawnClone plugin interface, showcasing real-time audio visualization
 * with Epic 7 AI integration capabilities.
 */

#include <iostream>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "Source/ui/WaveformDisplayComponent.h"

//==============================================================================
class WaveformDisplayDemo : public juce::Component, 
                           public juce::Timer
{
public:
    WaveformDisplayDemo()
    {
        // Initialize waveform display
        waveformDisplay = std::make_unique<WaveformDisplayComponent>();
        addAndMakeVisible(*waveformDisplay);
        
        // Configure for demonstration
        WaveformDisplayComponent::VisualizationSettings settings;
        settings.mode = WaveformDisplayComponent::Oscilloscope;
        settings.refreshRate = 60;
        settings.timeScale = 1.0f;
        settings.amplitudeScale = 1.0f;
        settings.showGrid = true;
        settings.showLabels = true;
        settings.waveformColour = juce::Colour(0xff00ff88);
        settings.gridColour = juce::Colour(0xff333333);
        settings.backgroundColour = juce::Colour(0xff1a1a1a);
        
        waveformDisplay->setVisualizationSettings(settings);
        
        // Set up AI integration callback
        waveformDisplay->setAIGenerationCallback([this](const std::vector<float>& pattern) {
            std::cout << "AI Pattern Generated: " << pattern.size() << " samples\\n";
        });
        
        // Generate test audio
        generateTestAudio();
        
        // Start timer for continuous audio data
        startTimer(20); // 50Hz update rate
        
        setSize(800, 400);
    }
    
    ~WaveformDisplayDemo() override
    {
        stopTimer();
    }
    
    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colour(0xff2a2a2a));
        
        g.setColour(juce::Colours::white);
        g.setFont(16.0f);
        g.drawText("Epic 9.2 Story 9.1: WaveformDisplayComponent Integration Demo", 
                   10, 10, getWidth() - 20, 30, juce::Justification::centred);
    }
    
    void resized() override
    {
        auto bounds = getLocalBounds();
        bounds.removeFromTop(50); // Space for title
        bounds.reduce(20, 20);
        waveformDisplay->setBounds(bounds);
    }
    
    void timerCallback() override
    {
        // Push test audio to waveform display
        waveformDisplay->pushAudioData(testBuffer);
        
        // Cycle through different waveforms
        cycleCounter++;
        if (cycleCounter > 300) // Change every 6 seconds (300 * 20ms)
        {
            cycleCounter = 0;
            currentWaveform = (currentWaveform + 1) % 3;
            generateTestAudio();
            
            // Also cycle display modes
            auto modes = { WaveformDisplayComponent::Oscilloscope,
                          WaveformDisplayComponent::Spectrum,
                          WaveformDisplayComponent::Phase };
            auto mode = *(modes.begin() + (currentWaveform % 3));
            waveformDisplay->setDisplayMode(mode);
        }
    }
    
private:
    void generateTestAudio()
    {
        const float frequency = (currentWaveform == 0) ? 440.0f : 
                               (currentWaveform == 1) ? 880.0f : 220.0f;
        const float sampleRate = 44100.0f;
        const int numSamples = testBuffer.getNumSamples();
        
        for (int channel = 0; channel < testBuffer.getNumChannels(); ++channel)
        {
            float* channelData = testBuffer.getWritePointer(channel);
            
            for (int i = 0; i < numSamples; ++i)
            {
                float phase = 2.0f * juce::MathConstants<float>::pi * frequency * 
                             (sampleCounter + i) / sampleRate;
                
                float sample = 0.0f;
                switch (currentWaveform)
                {
                    case 0: // Sine wave
                        sample = 0.3f * std::sin(phase);
                        break;
                    case 1: // Sawtooth
                        sample = 0.3f * (2.0f * std::fmod(phase / (2.0f * juce::MathConstants<float>::pi), 1.0f) - 1.0f);
                        break;
                    case 2: // Square wave
                        sample = 0.3f * (std::sin(phase) > 0.0f ? 1.0f : -1.0f);
                        break;
                }
                
                channelData[i] = sample;
            }
        }
        
        sampleCounter += numSamples;
    }
    
    std::unique_ptr<WaveformDisplayComponent> waveformDisplay;
    juce::AudioBuffer<float> testBuffer{2, 1024};
    int currentWaveform = 0;
    int cycleCounter = 0;
    int sampleCounter = 0;
};

//==============================================================================
class WaveformDisplayDemoApp : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override { return "WaveformDisplayDemo"; }
    const juce::String getApplicationVersion() override { return "1.0.0"; }
    
    void initialise(const juce::String&) override
    {
        mainWindow.reset(new MainWindow(getApplicationName()));
    }
    
    void shutdown() override
    {
        mainWindow = nullptr;
    }
    
private:
    class MainWindow : public juce::DocumentWindow
    {
    public:
        MainWindow(juce::String name) : DocumentWindow(name, juce::Colours::darkgrey, 
                                                       DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar(true);
            setContentOwned(new WaveformDisplayDemo(), true);
            centreWithSize(getContentComponent()->getWidth(), 
                          getContentComponent()->getHeight());
            setVisible(true);
        }
        
        void closeButtonPressed() override
        {
            juce::JUCEApplication::getInstance()->systemRequestedQuit();
        }
    };
    
    std::unique_ptr<MainWindow> mainWindow;
};

// Start the demo application
START_JUCE_APPLICATION(WaveformDisplayDemoApp)
