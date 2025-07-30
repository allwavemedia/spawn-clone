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
#include "ONNXModelManager.h"
#include "CloudAPIManager.h"
#include "ModelCacheManager.h"
#include "SecureCredentialManager.h"
#include "MusicTheoryEngine.h"  // Epic 4 Story 4.1: Advanced Music Theory Engine
#include "GenreSpecialist.h"    // Epic 4 Story 4.2: Genre-Specific Intelligence Systems

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
    
    // AI model managers
    std::unique_ptr<ONNXModelManager> onnxModelManager;
    std::unique_ptr<CloudAPIManager> cloudAPIManager;
    std::shared_ptr<ModelCacheManager> modelCacheManager;
    std::shared_ptr<SecureCredentialManager> credentialManager;
    
    // Epic 4 Story 4.1: Advanced Music Theory Engine
    std::unique_ptr<MusicTheoryEngine> musicTheoryEngine;

    // AI Generation methods
    void generateIntelligentPattern(MIDIPattern& pattern, const GenerationParameters& params);
    
    // AI Mode-specific generation
    void generateFastMode(MIDIPattern& pattern, const GenerationParameters& params);
    void generateQualityMode(MIDIPattern& pattern, const GenerationParameters& params);
    void generateCloudMode(MIDIPattern& pattern, const GenerationParameters& params);
    
    // Pattern type generators
    void generateMelody(MIDIPattern& pattern, const GenerationParameters& params);
    void generateChords(MIDIPattern& pattern, const GenerationParameters& params);
    void generateBass(MIDIPattern& pattern, const GenerationParameters& params);
    void generateDrum(MIDIPattern& pattern, const GenerationParameters& params);
    
    // Genre-specific pattern generators (Task 7.1.3)
    void generateHipHopPattern(MIDIPattern& pattern, const GenerationParameters& params);
    void generatePopPattern(MIDIPattern& pattern, const GenerationParameters& params);
    void generateDancePattern(MIDIPattern& pattern, const GenerationParameters& params);
    void generateEDMPattern(MIDIPattern& pattern, const GenerationParameters& params);
    void generateRnBPattern(MIDIPattern& pattern, const GenerationParameters& params);
    
    // Music theory helpers
    std::vector<int> getScaleNotes(int rootNote, GenerationParameters::ScaleType scale);
    std::vector<int> getChordNotes(int rootNote, GenerationParameters::ScaleType scale, int chordDegree);
    int getRandomVelocity(float complexity);
    double getRandomTiming(float complexity, double baseTime);
    
    // Epic 4 Story 4.1: Music Theory Integration
    void applyMusicTheoryConstraints(MIDIPattern& pattern, const GenerationParameters& params);
    std::vector<MusicTheoryEngine::Chord> generateChordProgression(const GenerationParameters& params);
    void enhancePatternWithHarmony(MIDIPattern& pattern, const GenerationParameters& params);
    MusicTheoryEngine::TheoryAnalysis validatePatternTheory(const MIDIPattern& pattern, const GenerationParameters& params);
    
    // Epic 4 Story 4.2: Genre Intelligence Integration
    std::unique_ptr<GenreManager> genreManager;
    void applyGenreIntelligence(MIDIPattern& pattern, const GenerationParameters& params);
    GenreType detectAndSetGenre(const GenerationParameters& params);
    void enhancePatternWithGenreCharacteristics(MIDIPattern& pattern, const GenerationParameters& params, GenreType genre);
    float calculateGenreAuthenticity(const MIDIPattern& pattern, GenreType genre);
    
    // Performance monitoring (Task 7.1.5)
    void profileGenerationTime(const juce::String& mode, double startTime);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AIGenerationEngine)
};
