/*
  ==============================================================================

    PatternSerializer.h
    Created: 28 Jul 2025
    Author:  Developer Agent

    Handles serialization of MIDIPattern objects to and from ValueTrees.

  ==============================================================================
*/

#pragma once

#include "MIDIPattern.h"
#include <juce_data_structures/juce_data_structures.h>

//==============================================================================
/**
 * A utility class for serializing and deserializing MIDIPattern objects.
 * This allows patterns to be saved as part of the plugin's state.
 */
class PatternSerializer
{
public:
    /** Converts a MIDIPattern object to a juce::ValueTree. */
    static juce::ValueTree toValueTree(const MIDIPattern& pattern);

    /** Converts a juce::ValueTree back into a MIDIPattern object. */
    static MIDIPattern fromValueTree(const juce::ValueTree& tree);

    // Identifiers for ValueTree properties
    static const juce::Identifier PATTERN_ID;
    static const juce::Identifier ID_PROP;
    static const juce::Identifier LENGTH_PROP;
    
    static const juce::Identifier METADATA_ID;
    static const juce::Identifier KEY_PROP;
    static const juce::Identifier SCALE_PROP;
    static const juce::Identifier TEMPO_PROP;
    static const juce::Identifier COMPLEXITY_PROP;
    static const juce::Identifier GEN_TYPE_PROP;
    static const juce::Identifier PATTERN_LENGTH_PROP;

    static const juce::Identifier NOTES_ID;
    static const juce::Identifier NOTE_ID;
    static const juce::Identifier PITCH_PROP;
    static const juce::Identifier VELOCITY_PROP;
    static const juce::Identifier START_TIME_PROP;
    static const juce::Identifier DURATION_PROP;
};
