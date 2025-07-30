/*
  ==============================================================================

    MIDIExporter.h
    Created: 29 Jul 2025
    Author:  Epic 2 Story 2.3 Implementation

    MIDI export and drag-drop functionality for generated patterns.

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_core/juce_core.h>
#include "MIDIPattern.h"

//==============================================================================
/**
 * Epic 2 Story 2.3: MIDI Export and Drag-Drop Functionality
 * 
 * Provides utilities for converting MIDI patterns to standard MIDI files
 * and supporting drag-and-drop operations to DAW timelines.
 */
class MIDIExporter
{
public:
    //==============================================================================
    MIDIExporter();
    ~MIDIExporter();
    
    //==============================================================================
    // Task 2.3.2: MIDI File Conversion
    
    /** Convert MIDIPattern to standard MIDI file format */
    static juce::MidiFile patternToMidiFile(const MIDIPattern& pattern);
    
    /** Convert MIDIPattern to MIDI file data in memory */
    static juce::MemoryBlock patternToMidiData(const MIDIPattern& pattern);
    
    //==============================================================================
    // Task 2.3.4: Export Functions
    
    /** Export pattern to MIDI file on disk */
    static bool exportPatternToFile(const MIDIPattern& pattern, const juce::File& destinationFile);
    
    /** Copy pattern to system clipboard as MIDI data */
    static bool copyPatternToClipboard(const MIDIPattern& pattern);
    
    /** Get pattern as MIDI data for drag-and-drop operations */
    static juce::String getPatternAsDragData(const MIDIPattern& pattern);
    
    //==============================================================================
    // Utility Functions
    
    /** Get suggested filename for pattern export */
    static juce::String getSuggestedFileName(const MIDIPattern& pattern);
    
    /** Validate pattern before export */
    static bool validatePatternForExport(const MIDIPattern& pattern);
    
    /** Get MIDI file format information */
    static juce::String getMidiFileInfo(const MIDIPattern& pattern);

private:
    //==============================================================================
    /** Convert pattern metadata to MIDI track name */
    static juce::String generateTrackName(const MIDIPattern& pattern);
    
    /** Add tempo and time signature events to MIDI track */
    static void addTempoEvents(juce::MidiMessageSequence& track, const MIDIPattern& pattern);
    
    /** Convert pattern notes to MIDI note events */
    static void addNoteEvents(juce::MidiMessageSequence& track, const MIDIPattern& pattern);
    
    /** Calculate MIDI ticks per quarter note */
    static int getTicksPerQuarterNote();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MIDIExporter)
};

//==============================================================================
/**
 * Epic 2 Story 2.3: Drag and Drop Source Component
 * 
 * UI component that provides drag-and-drop functionality for MIDI patterns.
 */
class MIDIDragSource : public juce::Component,
                       public juce::DragAndDropContainer
{
public:
    //==============================================================================
    MIDIDragSource();
    ~MIDIDragSource() override;
    
    //==============================================================================
    // Task 2.3.1: Drag-and-Drop Implementation
    
    /** Set the pattern to be dragged */
    void setPattern(const MIDIPattern& pattern);
    
    /** Check if pattern is available for dragging */
    bool hasPattern() const { return patternLoaded; }
    
    /** Enable or disable drag functionality */
    void setDragEnabled(bool enabled) { dragEnabled = enabled; }
    
    //==============================================================================
    // Component overrides
    
    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    
protected:
    //==============================================================================
    // Task 2.3.3: Drag Operation Implementation
    
    /** Start the drag operation */
    void startDragOperation(const juce::MouseEvent& event);
    
    /** Create drag description for the pattern */
    juce::var createDragDescription() const;

private:
    //==============================================================================
    MIDIPattern currentPattern;
    bool patternLoaded = false;
    bool dragEnabled = true;
    bool isDragging = false;
    
    // Visual feedback
    juce::Colour normalColour = juce::Colours::lightgrey;
    juce::Colour hoverColour = juce::Colours::lightblue;
    juce::Colour dragColour = juce::Colours::blue;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MIDIDragSource)
};
