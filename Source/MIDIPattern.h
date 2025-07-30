/*
  ==============================================================================

    MIDIPattern.h
    Created: 28 Jul 2025
    Author:  Developer Agent

    Core data structure for representing a generated MIDI pattern.

  ==============================================================================
*/

#pragma once

#include <juce_core/juce_core.h>
#include <juce_data_structures/juce_data_structures.h>
#include <vector>
#include "Note.h"
#include "GenerationParameters.h"

//==============================================================================
/**
 * Represents a single, self-contained musical idea as a collection of MIDI notes
 * with associated metadata. This is the primary output of the AI Generation Engine.
 */
struct MIDIPattern
{
    // A collection of all notes in the pattern
    std::vector<Note> notes;
    
    // The total length of the pattern in musical beats
    double lengthInBeats = 16.0;
    
    // Contextual information about how the pattern was generated
    GenerationParameters metadata;
    
    // A unique identifier for this pattern
    juce::Uuid id;

    //==============================================================================
    MIDIPattern() : id(juce::Uuid::null()) {}

    /** Clears all notes and resets the pattern */
    void clear()
    {
        notes.clear();
        lengthInBeats = 16.0;
        id = juce::Uuid::null();
    }
    
    /** Returns true if the pattern contains no notes */
    bool isEmpty() const
    {
        return notes.empty();
    }
    
    //==============================================================================
    /** Serialize this pattern to a ValueTree for persistence */
    juce::ValueTree toValueTree() const;
    
    /** Deserialize this pattern from a ValueTree */
    void fromValueTree(const juce::ValueTree& tree);
};
