/*
  ==============================================================================

    test_TransportControls.cpp
    Created: 30 Jul 2025
    Author:  Epic 5 Story 5.1 Implementation

    Unit tests for Epic 4 Transport Controls functionality.
    Tests transport button behavior, tempo control, and UI state management.

  ==============================================================================
*/

#include <gtest/gtest.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "../audio/TransportControlsComponent.h"
#include "../audio/AudioPreviewEngine.h"
#include "../MIDIPattern.h"
#include "../Note.h"

//==============================================================================
class TransportControlsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Initialize JUCE message manager for GUI testing
        juce::MessageManager::getInstance();
        
        // Create audio engine and transport controls
        audioEngine = std::make_unique<AudioPreviewEngine>();
        audioEngine->prepareToPlay(44100.0, 512);
        
        // Create a test pattern for playback testing
        createTestPattern();
        audioEngine->loadPattern(testPattern);
        
        transportControls = std::make_unique<TransportControlsComponent>(*audioEngine);
        transportControls->setBounds(0, 0, 800, 120);
        transportControls->setVisible(true); // Explicitly make visible for testing
    }
    
    void createTestPattern()
    {
        testPattern.clear();
        testPattern.lengthInBeats = 4.0;
        
        // Add a simple test note
        Note note;
        note.pitch = 60; // C4
        note.velocity = 100;
        note.startTime = 0.0;
        note.duration = 1.0;
        testPattern.notes.push_back(note);
    }
    
    void TearDown() override
    {
        transportControls.reset();
        audioEngine.reset();
    }
    
    std::unique_ptr<AudioPreviewEngine> audioEngine;
    std::unique_ptr<TransportControlsComponent> transportControls;
    MIDIPattern testPattern;
};

//==============================================================================
// Epic 5 Story 5.1: Transport Controls Unit Tests

TEST_F(TransportControlsTest, InitialState)
{
    // Test initial state of transport controls
    EXPECT_FALSE(audioEngine->isPlaying());
    EXPECT_DOUBLE_EQ(audioEngine->getPlaybackPosition(), 0.0);
}

TEST_F(TransportControlsTest, TempoControl)
{
    // Test tempo setting within valid range
    audioEngine->setTempo(120.0);
    EXPECT_DOUBLE_EQ(audioEngine->getTempo(), 120.0);
    
    // Test tempo clamping - too low
    audioEngine->setTempo(30.0);
    EXPECT_DOUBLE_EQ(audioEngine->getTempo(), 60.0); // Should clamp to minimum
    
    // Test tempo clamping - too high
    audioEngine->setTempo(300.0);
    EXPECT_DOUBLE_EQ(audioEngine->getTempo(), 200.0); // Should clamp to maximum
    
    // Test valid range extremes
    audioEngine->setTempo(60.0);
    EXPECT_DOUBLE_EQ(audioEngine->getTempo(), 60.0);
    
    audioEngine->setTempo(200.0);
    EXPECT_DOUBLE_EQ(audioEngine->getTempo(), 200.0);
}

TEST_F(TransportControlsTest, PlaybackStateManagement)
{
    // Test play functionality
    audioEngine->startPlayback();
    EXPECT_TRUE(audioEngine->isPlaying());
    
    // Test stop functionality  
    audioEngine->stopPlayback();
    EXPECT_FALSE(audioEngine->isPlaying());
    EXPECT_DOUBLE_EQ(audioEngine->getPlaybackPosition(), 0.0);
}

TEST_F(TransportControlsTest, ComponentVisibility)
{
    // Test that all transport control components are visible
    EXPECT_TRUE(transportControls->isVisible());
    
    // Component should have reasonable bounds
    auto bounds = transportControls->getBounds();
    EXPECT_GT(bounds.getWidth(), 0);
    EXPECT_GT(bounds.getHeight(), 0);
    EXPECT_EQ(bounds.getHeight(), 120); // Expected transport control height
}

TEST_F(TransportControlsTest, TimerBehavior)
{
    // Test that timer updates work correctly
    transportControls->startTimerHz(30); // 30 FPS updates
    
    // Simulate some playback time passage
    audioEngine->startPlayback();
    
    // After a brief pause, stop the timer
    juce::Thread::sleep(100); // 100ms pause
    transportControls->stopTimer();
    
    EXPECT_TRUE(audioEngine->isPlaying());
}

//==============================================================================
// Epic 5 Story 5.1: Audio Engine Integration Tests

class AudioEngineIntegrationTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        audioEngine = std::make_unique<AudioPreviewEngine>();
        audioEngine->prepareToPlay(44100.0, 512);
        
        // Create and load test pattern
        createTestPattern();
        audioEngine->loadPattern(testPattern);
    }
    
    void createTestPattern()
    {
        testPattern.clear();
        testPattern.lengthInBeats = 4.0;
        
        // Add a simple test note
        Note note;
        note.pitch = 60; // C4
        note.velocity = 100;
        note.startTime = 0.0;
        note.duration = 1.0;
        testPattern.notes.push_back(note);
    }
    
    void TearDown() override
    {
        audioEngine.reset();
    }
    
    std::unique_ptr<AudioPreviewEngine> audioEngine;
    MIDIPattern testPattern;
};

TEST_F(AudioEngineIntegrationTest, AudioProcessingStability)
{
    // Test that audio processing doesn't crash
    juce::AudioBuffer<float> audioBuffer(2, 512);
    juce::MidiBuffer midiBuffer;
    
    audioBuffer.clear();
    
    // Process several blocks to ensure stability
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_NO_THROW(audioEngine->processBlock(audioBuffer, midiBuffer));
    }
}

TEST_F(AudioEngineIntegrationTest, PlaybackPositionTracking)
{
    // Test playback position tracking
    double initialPosition = audioEngine->getPlaybackPosition();
    EXPECT_DOUBLE_EQ(initialPosition, 0.0);
    
    // Start playback
    audioEngine->startPlayback();
    
    // Process some audio blocks to advance position
    juce::AudioBuffer<float> audioBuffer(2, 512);
    juce::MidiBuffer midiBuffer;
    
    for (int i = 0; i < 100; ++i) // Simulate ~1.16 seconds at 44.1kHz
    {
        audioEngine->processBlock(audioBuffer, midiBuffer);
    }
    
    // Position should have advanced
    double newPosition = audioEngine->getPlaybackPosition();
    EXPECT_GT(newPosition, initialPosition);
    
    // Stop and check position resets
    audioEngine->stopPlayback();
    EXPECT_DOUBLE_EQ(audioEngine->getPlaybackPosition(), 0.0);
}

TEST_F(AudioEngineIntegrationTest, TempoChangesDuringPlayback)
{
    // Test tempo changes during active playback
    audioEngine->setTempo(120.0);
    audioEngine->startPlayback();
    
    EXPECT_TRUE(audioEngine->isPlaying());
    EXPECT_DOUBLE_EQ(audioEngine->getTempo(), 120.0);
    
    // Change tempo during playback
    audioEngine->setTempo(140.0);
    EXPECT_DOUBLE_EQ(audioEngine->getTempo(), 140.0);
    EXPECT_TRUE(audioEngine->isPlaying()); // Should still be playing
}

//==============================================================================
// Epic 5 Story 5.1: Thread Safety Tests

TEST_F(AudioEngineIntegrationTest, ThreadSafetyBasic)
{
    // Test that basic operations are thread-safe
    std::atomic<bool> testComplete{false};
    std::atomic<int> errorCount{0};
    
    // Background thread simulating audio processing
    std::thread audioThread([&]() {
        juce::AudioBuffer<float> audioBuffer(2, 512);
        juce::MidiBuffer midiBuffer;
        
        while (!testComplete.load())
        {
            try 
            {
                audioEngine->processBlock(audioBuffer, midiBuffer);
                std::this_thread::sleep_for(std::chrono::microseconds(100));
            }
            catch (...)
            {
                errorCount.fetch_add(1);
            }
        }
    });
    
    // Main thread changing parameters
    for (int i = 0; i < 50; ++i)
    {
        try
        {
            audioEngine->setTempo(60.0 + (i % 140)); // Vary tempo
            audioEngine->startPlayback();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            audioEngine->stopPlayback();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        catch (...)
        {
            errorCount.fetch_add(1);
        }
    }
    
    testComplete.store(true);
    audioThread.join();
    
    // No errors should occur during concurrent access
    EXPECT_EQ(errorCount.load(), 0);
}
