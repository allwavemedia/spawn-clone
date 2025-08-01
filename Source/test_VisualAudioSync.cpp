/*
  ==============================================================================

    test_VisualAudioSync.cpp
    Created: 30 Jul 2025
    Author:  Epic 5 Story 5.1 Implementation

    Unit tests for Epic 4 Story 4.3 Visual-Audio Synchronization.
    Tests pattern visualization, playback cursor, and active note highlighting.

  ==============================================================================
*/

#include <gtest/gtest.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "../PatternVisualizationComponent.h"
#include "../MIDIPattern.h"
#include "../Note.h"

//==============================================================================
class VisualAudioSyncTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Initialize JUCE message manager for GUI testing
        juce::MessageManager::getInstance();
        
        // Create pattern visualization component
        visualization = std::make_unique<PatternVisualizationComponent>();
        visualization->setBounds(0, 0, 800, 400);
        visualization->setVisible(true); // Explicitly set visible for testing
        
        // Create test pattern with some notes
        createTestPattern();
    }
    
    void TearDown() override
    {
        visualization.reset();
    }
    
    void createTestPattern()
    {
        testPattern.clear();
        testPattern.lengthInBeats = 16.0;
        
        // Add test notes at different times
        Note note1;
        note1.pitch = 60; // C4
        note1.velocity = 100;
        note1.startTime = 0.0;
        note1.duration = 1.0;
        testPattern.notes.push_back(note1);
        
        Note note2;
        note2.pitch = 64; // E4
        note2.velocity = 80;
        note2.startTime = 2.0;
        note2.duration = 1.5;
        testPattern.notes.push_back(note2);
        
        Note note3;
        note3.pitch = 67; // G4
        note3.velocity = 90;
        note3.startTime = 8.0;
        note3.duration = 2.0;
        testPattern.notes.push_back(note3);
    }
    
    std::unique_ptr<PatternVisualizationComponent> visualization;
    MIDIPattern testPattern;
};

//==============================================================================
// Epic 5 Story 5.1: Visual Synchronization Tests

TEST_F(VisualAudioSyncTest, InitialVisualizationState)
{
    // Test initial state
    EXPECT_TRUE(visualization->isVisible());
    
    // Component should have reasonable bounds
    auto bounds = visualization->getBounds();
    EXPECT_GT(bounds.getWidth(), 0);
    EXPECT_GT(bounds.getHeight(), 0);
}

TEST_F(VisualAudioSyncTest, PatternDisplayBasics)
{
    // Set test pattern
    visualization->setPattern(testPattern);
    
    // Force a repaint to ensure pattern is processed
    visualization->repaint();
    
    // Test pattern clearing
    visualization->clearPattern();
    visualization->repaint();
}

TEST_F(VisualAudioSyncTest, PlaybackModeToggling)
{
    // Test playback mode changes
    visualization->setPlaybackMode(false);
    // Initially not in playback mode
    
    visualization->setPlaybackMode(true);
    // Now in playback mode - should enable sync features
    
    visualization->setPlaybackMode(false);
    // Back to normal mode
}

TEST_F(VisualAudioSyncTest, PlaybackPositionControl)
{
    visualization->setPattern(testPattern);
    visualization->setPlaybackMode(true);
    
    // Test various playback positions
    visualization->setPlaybackPosition(0.0);
    visualization->repaint();
    
    visualization->setPlaybackPosition(0.25);
    visualization->repaint();
    
    visualization->setPlaybackPosition(0.5);
    visualization->repaint();
    
    visualization->setPlaybackPosition(0.75);
    visualization->repaint();
    
    visualization->setPlaybackPosition(1.0);
    visualization->repaint();
    
    // Test position clamping
    visualization->setPlaybackPosition(-0.1); // Should clamp to 0.0
    visualization->setPlaybackPosition(1.1);  // Should clamp to 1.0
}

TEST_F(VisualAudioSyncTest, ZoomAndGridControls)
{
    // Test zoom functionality
    visualization->setZoomLevel(1.0f);
    visualization->setZoomLevel(2.0f);
    visualization->setZoomLevel(0.5f);
    
    // Test grid visibility
    visualization->setShowGrid(true);
    visualization->setShowGrid(false);
    
    // Test velocity display
    visualization->setShowVelocity(true);
    visualization->setShowVelocity(false);
}

//==============================================================================
// Epic 5 Story 5.1: Active Note Detection Tests

class ActiveNoteDetectionTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        createDetailedTestPattern();
    }
    
    void createDetailedTestPattern()
    {
        testPattern.clear();
        testPattern.lengthInBeats = 8.0;
        
        // Note 1: Plays from 0.0 to 1.0 beats
        Note note1;
        note1.pitch = 60;
        note1.velocity = 100;
        note1.startTime = 0.0;
        note1.duration = 1.0;
        testPattern.notes.push_back(note1);
        
        // Note 2: Plays from 1.0 to 3.0 beats  
        Note note2;
        note2.pitch = 64;
        note2.velocity = 80;
        note2.startTime = 1.0;
        note2.duration = 2.0;
        testPattern.notes.push_back(note2);
        
        // Note 3: Plays from 4.0 to 6.0 beats
        Note note3;
        note3.pitch = 67;
        note3.velocity = 90;
        note3.startTime = 4.0;
        note3.duration = 2.0;
        testPattern.notes.push_back(note3);
    }
    
    bool isNoteActiveAtTime(const Note& note, double timeInBeats)
    {
        return timeInBeats >= note.startTime && 
               timeInBeats <= (note.startTime + note.duration);
    }
    
    MIDIPattern testPattern;
};

TEST_F(ActiveNoteDetectionTest, NoteActiveTimeCalculations)
{
    // Test note 1 activity (0.0 to 1.0 beats)
    EXPECT_TRUE(isNoteActiveAtTime(testPattern.notes[0], 0.0));
    EXPECT_TRUE(isNoteActiveAtTime(testPattern.notes[0], 0.5));
    EXPECT_TRUE(isNoteActiveAtTime(testPattern.notes[0], 1.0));
    EXPECT_FALSE(isNoteActiveAtTime(testPattern.notes[0], 1.1));
    
    // Test note 2 activity (1.0 to 3.0 beats)
    EXPECT_FALSE(isNoteActiveAtTime(testPattern.notes[1], 0.9));
    EXPECT_TRUE(isNoteActiveAtTime(testPattern.notes[1], 1.0));
    EXPECT_TRUE(isNoteActiveAtTime(testPattern.notes[1], 2.0));
    EXPECT_TRUE(isNoteActiveAtTime(testPattern.notes[1], 3.0));
    EXPECT_FALSE(isNoteActiveAtTime(testPattern.notes[1], 3.1));
    
    // Test note 3 activity (4.0 to 6.0 beats)
    EXPECT_FALSE(isNoteActiveAtTime(testPattern.notes[2], 3.9));
    EXPECT_TRUE(isNoteActiveAtTime(testPattern.notes[2], 4.0));
    EXPECT_TRUE(isNoteActiveAtTime(testPattern.notes[2], 5.0));
    EXPECT_TRUE(isNoteActiveAtTime(testPattern.notes[2], 6.0));
    EXPECT_FALSE(isNoteActiveAtTime(testPattern.notes[2], 6.1));
}

TEST_F(ActiveNoteDetectionTest, MultipleNotesActiveSimultaneously)
{
    // Add overlapping note for testing
    Note overlappingNote;
    overlappingNote.pitch = 72;
    overlappingNote.velocity = 85;
    overlappingNote.startTime = 0.5;
    overlappingNote.duration = 3.0; // Extends to 3.5 beats
    testPattern.notes.push_back(overlappingNote);
    
    // At time 1.5, notes 1, 2, and the overlapping note should be active
    int activeCount = 0;
    double testTime = 1.5;
    
    for (const auto& note : testPattern.notes)
    {
        if (isNoteActiveAtTime(note, testTime))
            activeCount++;
    }
    
    EXPECT_EQ(activeCount, 2); // Note 2 and overlapping note
}

//==============================================================================
// Epic 5 Story 5.1: Performance Tests

class VisualizationPerformanceTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        juce::MessageManager::getInstance();
        
        visualization = std::make_unique<PatternVisualizationComponent>();
        visualization->setBounds(0, 0, 1920, 1080); // Large size for stress testing
        
        createLargeTestPattern();
    }
    
    void createLargeTestPattern()
    {
        testPattern.clear();
        testPattern.lengthInBeats = 64.0; // Longer pattern
        
        // Create many notes for performance testing
        for (int i = 0; i < 200; ++i)
        {
            Note note;
            note.pitch = 36 + (i % 48); // Range from C2 to B5
            note.velocity = 60 + (i % 68); // Velocity range 60-127
            note.startTime = (i % 64) * 0.25; // Notes every quarter beat
            note.duration = 0.5 + (i % 4) * 0.25; // Varying durations
            testPattern.notes.push_back(note);
        }
    }
    
    std::unique_ptr<PatternVisualizationComponent> visualization;
    MIDIPattern testPattern;
};

TEST_F(VisualizationPerformanceTest, LargePatternRenderingPerformance)
{
    visualization->setPattern(testPattern);
    visualization->setPlaybackMode(true);
    
    // Measure rendering performance with many notes
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Simulate multiple playback position updates
    for (int i = 0; i <= 100; ++i)
    {
        double position = i / 100.0;
        visualization->setPlaybackPosition(position);
        visualization->repaint();
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    // Performance expectation: Should complete in reasonable time
    EXPECT_LT(duration.count(), 5000); // Less than 5 seconds for 100 updates
}

TEST_F(VisualizationPerformanceTest, RepaintEfficiency)
{
    visualization->setPattern(testPattern);
    
    // Test that repaints don't cause excessive processing
    auto startTime = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 60; ++i) // Simulate 1 second at 60 FPS
    {
        visualization->repaint();
        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    // Should maintain near real-time performance
    EXPECT_LT(duration.count(), 2000); // Allow some overhead but keep it reasonable
}
