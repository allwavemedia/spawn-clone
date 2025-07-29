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

    for (const auto& pattern : patterns)
    {
        patternListTree.addChild(PatternSerializer::toValueTree(pattern), -1, nullptr);
    }

    managerTree.addChild(patternListTree, -1, nullptr);
    return managerTree;
}

void PatternManager::fromValueTree(const juce::ValueTree& tree)
{
    if (!tree.hasType(PATTERN_MANAGER_ID))
        return;

    clear(); // Clear existing patterns before loading new ones

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
    
    sendChangeMessage(); // Notify listeners that the data has been loaded
}
