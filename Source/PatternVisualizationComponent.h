/*
  ==============================================================================

    PatternVisualizationComponent.h
    Created: 29 Jul 2025
    Author:  Epic 8 Implementation

    Modern pattern visualization component with MIDI note display, grid,
    and interactive features for SpawnClone.

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include "MIDIPattern.h"

//==============================================================================
/**
 * A modern MIDI pattern visualization component that displays notes as
 * rectangles on a piano roll-style grid with customizable colors, zoom,
 * and drag-and-drop MIDI file import/export capabilities.
 */
class PatternVisualizationComponent : public juce::Component,
                                    public juce::FileDragAndDropTarget
{
public:
    PatternVisualizationComponent();
    ~PatternVisualizationComponent() override;

    //==============================================================================
    void paint(juce::Graphics& g) override;
    void resized() override;

    //==============================================================================
    /** Set the MIDI pattern to display */
    void setPattern(const MIDIPattern& pattern);
    
    /** Clear the current pattern display */
    void clearPattern();
    
    /** Set the zoom level (1.0 = default, 2.0 = 2x zoom) */
    void setZoomLevel(float newZoom);
    
    /** Set whether to show the grid lines */
    void setShowGrid(bool shouldShowGrid);
    
    /** Set whether to show note velocity as brightness */
    void setShowVelocity(bool shouldShow);
    
    //==============================================================================
    // Drag and drop support (Epic 8 Story 8.3)
    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void fileDragEnter(const juce::StringArray& files, int x, int y) override;
    void fileDragMove(const juce::StringArray& files, int x, int y) override;
    void fileDragExit(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;
    
    //==============================================================================
    // MIDI export functionality
    void exportCurrentPatternToFile();
    bool exportPatternToMIDI(const juce::File& outputFile, const MIDIPattern& pattern);

private:
    //==============================================================================
    void drawBackground(juce::Graphics& g);
    void drawGrid(juce::Graphics& g);
    void drawNotes(juce::Graphics& g);
    void drawTimeAxis(juce::Graphics& g);
    void drawPitchAxis(juce::Graphics& g);
    void drawDragOverlay(juce::Graphics& g);  // For drag-and-drop visual feedback
    
    juce::Rectangle<int> getNoteRectangle(const Note& note);
    juce::Colour getNoteColour(const Note& note);
    
    //==============================================================================
    // MIDI import functionality
    bool loadMIDIFile(const juce::File& file);
    MIDIPattern convertMidiFileToPattern(const juce::MidiFile& midiFile);
    
    //==============================================================================
    MIDIPattern currentPattern;
    float zoomLevel = 1.0f;
    bool showGrid = true;
    bool showVelocity = true;
    bool isDragOver = false;  // For drag-and-drop visual feedback
    
    // Display parameters
    int gridSpacing = 20;
    int noteHeight = 12;
    int leftMargin = 60;   // Space for pitch labels
    int topMargin = 20;    // Space for time labels
    int bottomMargin = 20;
    int rightMargin = 20;
    
    // Colors
    juce::Colour backgroundColour = juce::Colour(0xff2a2a2a);
    juce::Colour gridColour = juce::Colour(0xff404040);
    juce::Colour noteColour = juce::Colour(0xff4a9eff);
    juce::Colour noteSelectedColour = juce::Colour(0xffff6b4a);
    juce::Colour dragOverColour = juce::Colour(0x664a9eff);  // Semi-transparent for drag overlay
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PatternVisualizationComponent)
};
