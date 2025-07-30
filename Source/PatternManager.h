/*
  ==============================================================================

    PatternManager.h
    Created: 28 Jul 2025
    Author:  Developer Agent

    Manages a collection of generated MIDIPatterns.

  ==============================================================================
*/

#pragma once

#include <juce_core/juce_core.h>
#include <juce_data_structures/juce_data_structures.h>
#include "MIDIPattern.h"
#include "PatternSerializer.h"
#include <optional>

//==============================================================================
/**
    Holds a list of generated MIDIPatterns, manages their state,
    and broadcasts changes to any listeners.
*/
class PatternManager : public juce::ChangeBroadcaster
{
public:
    PatternManager();

    /** Adds a new pattern to the collection and notifies listeners. */
    void addPattern(const MIDIPattern& pattern);

    /** Returns a pointer to the pattern at the given index, or nullptr if invalid. */
    const MIDIPattern* getPattern(int index) const;

    /** Returns the total number of patterns stored. */
    int getNumPatterns() const;
    
    /** Removes all patterns from the manager. */
    void clear();
    
    //==============================================================================
    // NEW FEATURE: Pattern History Management
    
    /** Add pattern to history with automatic size management */
    void addPatternToHistory(const MIDIPattern& pattern);
    
    /** Get recent patterns for UI display */
    std::vector<MIDIPattern> getRecentPatterns(int maxCount = 10) const;
    
    /** Get current active pattern */
    std::optional<MIDIPattern> getCurrentPattern() const;
    
    /** Navigate pattern history */
    bool undoLastGeneration();
    const MIDIPattern* getCurrentHistoryPattern() const;
    int getCurrentHistoryIndex() const { return currentHistoryIndex; }
    
    /** Check navigation availability */
    bool canUndoGeneration() const;

    /** Serializes the entire collection of patterns into a ValueTree. */
    juce::ValueTree toValueTree() const;

    /** Deserializes and repopulates the pattern collection from a ValueTree. */
    void fromValueTree(const juce::ValueTree& tree);

private:
    std::vector<MIDIPattern> patterns;
    
    // NEW: Pattern history management
    std::vector<MIDIPattern> patternHistory;
    int currentHistoryIndex = -1;
    static constexpr int MAX_HISTORY_SIZE = 50;
    
    // Identifiers for serialization
    static const juce::Identifier PATTERN_MANAGER_ID;
    static const juce::Identifier PATTERN_LIST_ID;
};
