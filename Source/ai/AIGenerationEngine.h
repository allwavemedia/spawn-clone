/*
  ==============================================================================

    AIGenerationEngine.h
    Created: 28 Jul 2025
    Author:  Developer Agent

    Generates MIDI patterns using an AI model.

  ==============================================================================
*/

#pragma once

#include <juce_core/juce_core.h>
#include "../GenerationParameters.h"
#include "../MIDIPattern.h"

class PatternManager;
class ThreadManager;

//==============================================================================
/**
    The core class for handling AI-based MIDI pattern generation.
*/
class AIGenerationEngine
{
public:
    AIGenerationEngine(ThreadManager& tm, PatternManager& pm);

    /** 
        Triggers the AI to generate a new pattern based on the provided parameters.
        The result will be delivered asynchronously via the PatternManager.
    */
    void generatePattern(const GenerationParameters& params);

private:
    ThreadManager& threadManager;
    PatternManager& patternManager;

    // AI Generation methods
    void generateIntelligentPattern(MIDIPattern& pattern, const GenerationParameters& params);
    void generateMelody(MIDIPattern& pattern, const GenerationParameters& params);
    void generateChords(MIDIPattern& pattern, const GenerationParameters& params);
    void generateBass(MIDIPattern& pattern, const GenerationParameters& params);
    void generateDrum(MIDIPattern& pattern, const GenerationParameters& params);
    
    // Music theory helpers
    std::vector<int> getScaleNotes(int rootNote, GenerationParameters::ScaleType scale);
    std::vector<int> getChordNotes(int rootNote, GenerationParameters::ScaleType scale, int chordDegree);
    int getRandomVelocity(float complexity);
    double getRandomTiming(float complexity, double baseTime);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AIGenerationEngine)
};
