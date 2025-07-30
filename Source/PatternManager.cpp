/*
  ==============================================================================

    PatternManager.cpp
    Created: 28 Jul 2025
    Author:  Developer Agent

    Implementation for PatternManager.

  ==============================================================================
*/

#include "PatternManager.h"

const juce::Identifier PatternManager::PATTERN_MANAGER_ID("PatternManager");
const juce::Identifier PatternManager::PATTERN_LIST_ID("PatternList");


//==============================================================================
PatternManager::PatternManager()
{
}

void PatternManager::addPattern(const MIDIPattern& pattern)
{
    patterns.push_back(pattern);
    sendChangeMessage(); // Notify listeners that the list has changed
}

const MIDIPattern* PatternManager::getPattern(int index) const
{
    if (juce::isPositiveAndBelow(index, patterns.size()))
        return &patterns[index];
    
    return nullptr;
}

int PatternManager::getNumPatterns() const
{
    return static_cast<int>(patterns.size());
}

void PatternManager::clear()
{
    patterns.clear();
    sendChangeMessage();
}

juce::ValueTree PatternManager::toValueTree() const
{
    juce::ValueTree managerTree(PATTERN_MANAGER_ID);
    juce::ValueTree patternListTree(PATTERN_LIST_ID);

    // Serialize current patterns
    for (const auto& pattern : patterns)
    {
        patternListTree.addChild(PatternSerializer::toValueTree(pattern), -1, nullptr);
    }
    managerTree.addChild(patternListTree, -1, nullptr);
    
    // NEW: Serialize pattern history
    juce::ValueTree historyTree("PatternHistory");
    for (const auto& pattern : patternHistory)
    {
        historyTree.addChild(PatternSerializer::toValueTree(pattern), -1, nullptr);
    }
    managerTree.addChild(historyTree, -1, nullptr);
    
    // Save current history index
    managerTree.setProperty("currentHistoryIndex", currentHistoryIndex, nullptr);
    
    return managerTree;
}

void PatternManager::fromValueTree(const juce::ValueTree& tree)
{
    if (!tree.hasType(PATTERN_MANAGER_ID))
        return;

    clear(); // Clear existing patterns before loading new ones
    patternHistory.clear(); // Clear history as well

    // Load main patterns
    juce::ValueTree patternListTree = tree.getChildWithName(PATTERN_LIST_ID);
    if (patternListTree.isValid())
    {
        for (const auto& patternTree : patternListTree)
        {
            if (patternTree.hasType(PatternSerializer::PATTERN_ID))
            {
                patterns.push_back(PatternSerializer::fromValueTree(patternTree));
            }
        }
    }
    
    // NEW: Load pattern history
    juce::ValueTree historyTree = tree.getChildWithName("PatternHistory");
    if (historyTree.isValid())
    {
        for (const auto& patternTree : historyTree)
        {
            if (patternTree.hasType(PatternSerializer::PATTERN_ID))
            {
                patternHistory.push_back(PatternSerializer::fromValueTree(patternTree));
            }
        }
    }
    
    // Restore current history index
    currentHistoryIndex = tree.getProperty("currentHistoryIndex", -1);
    
    // Validate index
    if (currentHistoryIndex >= static_cast<int>(patternHistory.size()))
    {
        currentHistoryIndex = static_cast<int>(patternHistory.size()) - 1;
    }
    
    sendChangeMessage(); // Notify listeners that the data has been loaded
}

//==============================================================================
// NEW FEATURE: Pattern History Management Implementation

void PatternManager::addPatternToHistory(const MIDIPattern& pattern)
{
    // Add to main patterns collection
    addPattern(pattern);
    
    // Add to history
    patternHistory.push_back(pattern);
    
    // Maintain max history size
    if (patternHistory.size() > MAX_HISTORY_SIZE)
    {
        patternHistory.erase(patternHistory.begin());
    }
    
    // Update current index to point to the newest pattern
    currentHistoryIndex = static_cast<int>(patternHistory.size()) - 1;
    
    sendChangeMessage();
}

std::vector<MIDIPattern> PatternManager::getRecentPatterns(int maxCount) const
{
    std::vector<MIDIPattern> recent;
    
    int startIndex = juce::jmax(0, static_cast<int>(patternHistory.size()) - maxCount);
    
    for (int i = startIndex; i < static_cast<int>(patternHistory.size()); ++i)
    {
        recent.push_back(patternHistory[i]);
    }
    
    return recent;
}

std::optional<MIDIPattern> PatternManager::getCurrentPattern() const
{
    if (!patternHistory.empty() && currentHistoryIndex >= 0 && currentHistoryIndex < static_cast<int>(patternHistory.size()))
    {
        return patternHistory[currentHistoryIndex];
    }
    return std::nullopt;
}

bool PatternManager::undoLastGeneration()
{
    if (currentHistoryIndex > 0)
    {
        currentHistoryIndex--;
        sendChangeMessage();
        return true;
    }
    return false;
}

const MIDIPattern* PatternManager::getCurrentHistoryPattern() const
{
    if (currentHistoryIndex >= 0 && 
        currentHistoryIndex < static_cast<int>(patternHistory.size()))
    {
        return &patternHistory[currentHistoryIndex];
    }
    return nullptr;
}

bool PatternManager::canUndoGeneration() const
{
    return currentHistoryIndex > 0;
}
