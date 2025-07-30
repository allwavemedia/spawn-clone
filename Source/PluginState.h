/*
  ==============================================================================

    PluginState.h
    Created: 29 Jul 2025
    Author:  Developer Agent

    Central state management structure for SpawnClone plugin state.
    Extends current functionality with Instrument Mode, Pattern History, 
    and User-Controlled Preview features.

  ==============================================================================
*/

#pragma once

#include <juce_core/juce_core.h>
#include <vector>
#include "MIDIPattern.h"

//==============================================================================
/**
 * Central state structure that holds all persistent plugin state.
 * This includes both user preferences and generated pattern history.
 */
struct PluginState
{
    // === EXISTING FEATURES ===
    std::vector<MIDIPattern> patternHistory;
    int currentPatternIndex = -1;
    
    // === NEW FEATURE: Instrument Mode ===
    // When enabled, routes incoming DAW MIDI through AudioPreviewEngine 
    // without triggering AI generation (prevents double-triggering)
    bool isInstrumentMode = false;
    
    // === NEW FEATURE: User-Controlled Preview ===
    // Controls whether generated patterns auto-play immediately 
    // (prevents disruption to ongoing DAW playback)
    bool autoPlayOnGenerate = true;
    
    // === NEW FEATURE: Extended Pattern History ===
    // Enhanced history management with navigation
    int currentHistoryIndex = -1;
    static constexpr int MAX_HISTORY_SIZE = 50;
    
    // === FUTURE: AI Model Selection ===
    // Optional cloud/local AI model features (future implementation)
    bool useCloudAI = false;
    juce::String selectedLocalModel;
    
    //==============================================================================
    /** Default constructor with sensible defaults */
    PluginState() = default;
    
    /** Add a pattern to history with automatic size management */
    void addPatternToHistory(const MIDIPattern& pattern)
    {
        // Add pattern to history
        patternHistory.push_back(pattern);
        
        // Maintain max history size
        if (patternHistory.size() > MAX_HISTORY_SIZE)
        {
            patternHistory.erase(patternHistory.begin());
        }
        
        // Update current index to point to the newest pattern
        currentHistoryIndex = static_cast<int>(patternHistory.size()) - 1;
        currentPatternIndex = currentHistoryIndex;
    }
    
    /** Get the current pattern (if any) */
    const MIDIPattern* getCurrentPattern() const
    {
        if (currentHistoryIndex >= 0 && 
            currentHistoryIndex < static_cast<int>(patternHistory.size()))
        {
            return &patternHistory[currentHistoryIndex];
        }
        return nullptr;
    }
    
    /** Navigate to previous pattern in history */
    bool navigateToPreviousPattern()
    {
        if (currentHistoryIndex > 0)
        {
            currentHistoryIndex--;
            currentPatternIndex = currentHistoryIndex;
            return true;
        }
        return false;
    }
    
    /** Navigate to next pattern in history */
    bool navigateToNextPattern()
    {
        if (currentHistoryIndex < static_cast<int>(patternHistory.size()) - 1)
        {
            currentHistoryIndex++;
            currentPatternIndex = currentHistoryIndex;
            return true;
        }
        return false;
    }
    
    /** Check if we can navigate backwards */
    bool canNavigateBackward() const
    {
        return currentHistoryIndex > 0;
    }
    
    /** Check if we can navigate forward */
    bool canNavigateForward() const
    {
        return currentHistoryIndex < static_cast<int>(patternHistory.size()) - 1;
    }
    
    /** Get total number of patterns in history */
    int getHistorySize() const
    {
        return static_cast<int>(patternHistory.size());
    }
    
    /** Clear all history */
    void clearHistory()
    {
        patternHistory.clear();
        currentHistoryIndex = -1;
        currentPatternIndex = -1;
    }
    
    //==============================================================================
    /** Serialize state to ValueTree for plugin persistence */
    juce::ValueTree toValueTree() const
    {
        juce::ValueTree tree("PluginState");
        
        // Basic state
        tree.setProperty("isInstrumentMode", isInstrumentMode, nullptr);
        tree.setProperty("autoPlayOnGenerate", autoPlayOnGenerate, nullptr);
        tree.setProperty("currentHistoryIndex", currentHistoryIndex, nullptr);
        tree.setProperty("useCloudAI", useCloudAI, nullptr);
        tree.setProperty("selectedLocalModel", selectedLocalModel, nullptr);
        
        // Pattern history
        juce::ValueTree historyTree("PatternHistory");
        for (const auto& pattern : patternHistory)
        {
            historyTree.appendChild(pattern.toValueTree(), nullptr);
        }
        tree.appendChild(historyTree, nullptr);
        
        return tree;
    }
    
    /** Deserialize state from ValueTree */
    void fromValueTree(const juce::ValueTree& tree)
    {
        if (tree.hasType("PluginState"))
        {
            // Basic state
            isInstrumentMode = tree.getProperty("isInstrumentMode", false);
            autoPlayOnGenerate = tree.getProperty("autoPlayOnGenerate", true);
            currentHistoryIndex = tree.getProperty("currentHistoryIndex", -1);
            useCloudAI = tree.getProperty("useCloudAI", false);
            selectedLocalModel = tree.getProperty("selectedLocalModel", "");
            
            // Pattern history
            patternHistory.clear();
            auto historyTree = tree.getChildWithName("PatternHistory");
            if (historyTree.isValid())
            {
                for (int i = 0; i < historyTree.getNumChildren(); ++i)
                {
                    MIDIPattern pattern;
                    pattern.fromValueTree(historyTree.getChild(i));
                    patternHistory.push_back(pattern);
                }
            }
            
            // Validate current index
            if (currentHistoryIndex >= static_cast<int>(patternHistory.size()))
            {
                currentHistoryIndex = static_cast<int>(patternHistory.size()) - 1;
            }
            
            currentPatternIndex = currentHistoryIndex;
        }
    }
};
