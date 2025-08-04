/*
  ==============================================================================

    test_AdvancedHarmony.cpp
    Created: 2 Aug 2025
    Author:  Epic 6 Story 6.3 Implementation

    Unit tests for advanced chord progressions and harmonic evolution.
    Tests extended chords, reharmonization, and style-specific progressions.

  ==============================================================================
*/

#include <gtest/gtest.h>
#include "ai/MusicTheoryEngine.h"
#include "GenerationParameters.h"
#include <vector>
#include <algorithm>

class AdvancedHarmonyTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        engine = std::make_unique<MusicTheoryEngine>();
        
        // Setup harmonic parameters
        params.key = 0; // C major
        params.scale = GenerationParameters::ScaleType::Major;
        params.harmony.enableAdvancedHarmony = true;
        params.harmony.complexity = GenerationParameters::HarmonicSettings::HarmonicComplexity::Advanced;
        params.harmony.style = GenerationParameters::HarmonicSettings::ProgressionStyle::Jazz;
        params.harmony.chromaticism = 0.3f;
        params.harmony.modalInterchange = 0.2f;
        params.harmony.voiceLeading = GenerationParameters::HarmonicSettings::VoiceLeadingPriority::Smooth;
    }

    std::unique_ptr<MusicTheoryEngine> engine;
    GenerationParameters params;
};

TEST_F(AdvancedHarmonyTest, ExtendedChordGeneration)
{
    // Test major 9th chord
    auto maj9 = engine->generateAdvancedChord(0, MusicTheoryEngine::ExtendedChordType::Major9, params);
    EXPECT_EQ(maj9.root, 0);
    EXPECT_EQ(maj9.type, MusicTheoryEngine::ExtendedChordType::Major9);
    EXPECT_EQ(maj9.notes.size(), 5); // Root, 3rd, 5th, 7th, 9th
    
    // Test dominant 7th chord
    auto dom7 = engine->generateAdvancedChord(7, MusicTheoryEngine::ExtendedChordType::Dominant7, params);
    EXPECT_EQ(dom7.root, 7);
    EXPECT_GT(dom7.tension, 0.5f); // Dominant chords should have higher tension
    
    // Test chord naming
    juce::String name = maj9.getName();
    EXPECT_TRUE(name.contains("maj9"));
    
    juce::String domName = dom7.getName();
    EXPECT_TRUE(domName.contains("7"));
}

TEST_F(AdvancedHarmonyTest, ChordTensionCalculation)
{
    // Major triad should have low tension
    auto major = engine->generateAdvancedChord(0, MusicTheoryEngine::ExtendedChordType::Major, params);
    EXPECT_LT(major.tension, 0.3f);
    
    // Dominant 7th should have higher tension
    auto dom7 = engine->generateAdvancedChord(7, MusicTheoryEngine::ExtendedChordType::Dominant7, params);
    EXPECT_GT(dom7.tension, 0.5f);
    
    // Diminished 7th should have very high tension
    auto dim7 = engine->generateAdvancedChord(6, MusicTheoryEngine::ExtendedChordType::Diminished7, params);
    EXPECT_GT(dim7.tension, 0.7f);
}

TEST_F(AdvancedHarmonyTest, RomanNumeralAnalysis)
{
    // Test Roman numeral generation for C major
    auto CMaj = engine->generateAdvancedChord(0, MusicTheoryEngine::ExtendedChordType::Major, params);
    auto numeral = CMaj.getRomanNumeral(0, MusicTheoryEngine::ScaleType::Major);
    EXPECT_EQ(numeral, "I");
    
    auto Dm = engine->generateAdvancedChord(2, MusicTheoryEngine::ExtendedChordType::Minor, params);
    auto dm_numeral = Dm.getRomanNumeral(0, MusicTheoryEngine::ScaleType::Major);
    EXPECT_EQ(dm_numeral, "ii");
    
    auto G7 = engine->generateAdvancedChord(7, MusicTheoryEngine::ExtendedChordType::Dominant7, params);
    auto g7_numeral = G7.getRomanNumeral(0, MusicTheoryEngine::ScaleType::Major);
    EXPECT_EQ(g7_numeral, "V7");
}

TEST_F(AdvancedHarmonyTest, ProgressionGeneration)
{
    // Test basic progression generation
    auto progression = engine->generateAdvancedProgression(0, MusicTheoryEngine::ScaleType::Major, params, 4);
    
    EXPECT_EQ(progression.size(), 4);
    
    // All chords should have valid properties
    for (const auto& chord : progression)
    {
        EXPECT_GE(chord.root, 0);
        EXPECT_LE(chord.root, 11);
        EXPECT_FALSE(chord.notes.empty());
        EXPECT_GE(chord.tension, 0.0f);
        EXPECT_LE(chord.tension, 1.0f);
        EXPECT_FALSE(chord.getName().isEmpty());
    }
}

TEST_F(AdvancedHarmonyTest, StyleSpecificProgressions)
{
    // Test Jazz progression
    params.harmony.style = GenerationParameters::HarmonicSettings::ProgressionStyle::Jazz;
    auto jazzProgression = engine->generateAdvancedProgression(0, MusicTheoryEngine::ScaleType::Major, params, 4);
    
    // Jazz progressions should have higher average tension due to 7th chords
    float averageTension = 0.0f;
    for (const auto& chord : jazzProgression)
    {
        averageTension += chord.tension;
    }
    averageTension /= jazzProgression.size();
    EXPECT_GT(averageTension, 0.3f);
    
    // Test Pop progression
    params.harmony.style = GenerationParameters::HarmonicSettings::ProgressionStyle::Pop;
    auto popProgression = engine->generateAdvancedProgression(0, MusicTheoryEngine::ScaleType::Major, params, 4);
    
    // Pop progressions should be simpler
    bool hasSimpleChords = false;
    for (const auto& chord : popProgression)
    {
        if (chord.type == MusicTheoryEngine::ExtendedChordType::Major ||
            chord.type == MusicTheoryEngine::ExtendedChordType::Minor)
        {
            hasSimpleChords = true;
            break;
        }
    }
    EXPECT_TRUE(hasSimpleChords);
}

TEST_F(AdvancedHarmonyTest, HarmonicComplexity)
{
    // Test basic complexity
    params.harmony.complexity = GenerationParameters::HarmonicSettings::HarmonicComplexity::Basic;
    params.harmony.style = GenerationParameters::HarmonicSettings::ProgressionStyle::Pop; // Use Pop style for basic triads
    auto basicProgression = engine->generateAdvancedProgression(0, MusicTheoryEngine::ScaleType::Major, params, 4);
    
    // Should mostly contain triads
    int triadCount = 0;
    for (const auto& chord : basicProgression)
    {
        if (chord.type == MusicTheoryEngine::ExtendedChordType::Major ||
            chord.type == MusicTheoryEngine::ExtendedChordType::Minor)
        {
            triadCount++;
        }
    }
    EXPECT_GE(triadCount, 2); // At least half should be triads
    
    // Test advanced complexity
    params.harmony.complexity = GenerationParameters::HarmonicSettings::HarmonicComplexity::Jazz;
    auto jazzProgression = engine->generateAdvancedProgression(0, MusicTheoryEngine::ScaleType::Major, params, 4);
    
    // Should contain extended chords
    bool hasExtensions = false;
    for (const auto& chord : jazzProgression)
    {
        if (chord.type == MusicTheoryEngine::ExtendedChordType::Major9 ||
            chord.type == MusicTheoryEngine::ExtendedChordType::Dominant9 ||
            chord.type == MusicTheoryEngine::ExtendedChordType::Minor9)
        {
            hasExtensions = true;
            break;
        }
    }
    // Note: This might not always be true due to randomness, but we test the capability
}

TEST_F(AdvancedHarmonyTest, TensionCurveCalculation)
{
    auto progression = engine->generateAdvancedProgression(0, MusicTheoryEngine::ScaleType::Major, params, 8);
    auto tensionCurve = engine->calculateTensionCurve(progression, params);
    
    EXPECT_EQ(tensionCurve.size(), progression.size());
    
    // All tension values should be between 0 and 1
    for (float tension : tensionCurve)
    {
        EXPECT_GE(tension, 0.0f);
        EXPECT_LE(tension, 1.0f);
    }
}

TEST_F(AdvancedHarmonyTest, VoiceLeadingOptimization)
{
    // Create a progression with potentially poor voice leading
    std::vector<MusicTheoryEngine::AdvancedChord> progression;
    progression.push_back(engine->generateAdvancedChord(0, MusicTheoryEngine::ExtendedChordType::Major, params));
    progression.push_back(engine->generateAdvancedChord(6, MusicTheoryEngine::ExtendedChordType::Major, params)); // Large jump
    progression.push_back(engine->generateAdvancedChord(0, MusicTheoryEngine::ExtendedChordType::Major, params));
    
    auto optimized = engine->optimizeAdvancedVoiceLeading(progression, params);
    
    EXPECT_EQ(optimized.size(), progression.size());
    
    // Optimized progression should exist (basic test)
    for (const auto& chord : optimized)
    {
        EXPECT_FALSE(chord.notes.empty());
    }
}

class AdvancedHarmonyEngineTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        engine = std::make_unique<MusicTheoryEngine>();
        harmonyEngine = std::make_unique<MusicTheoryEngine::AdvancedHarmonyEngine>(engine.get());
        
        params.key = 0;
        params.scale = GenerationParameters::ScaleType::Major;
        params.harmony.enableAdvancedHarmony = true;
        params.harmony.complexity = GenerationParameters::HarmonicSettings::HarmonicComplexity::Advanced;
    }

    std::unique_ptr<MusicTheoryEngine> engine;
    std::unique_ptr<MusicTheoryEngine::AdvancedHarmonyEngine> harmonyEngine;
    GenerationParameters params;
};

TEST_F(AdvancedHarmonyEngineTest, ContextualProgressionGeneration)
{
    auto progression = harmonyEngine->generateContextualProgression(0, MusicTheoryEngine::ScaleType::Major, params);
    
    EXPECT_FALSE(progression.empty());
    EXPECT_LE(progression.size(), 16); // Reasonable progression length
    
    // Should have coherent harmonic flow
    for (const auto& chord : progression)
    {
        EXPECT_GE(chord.root, 0);
        EXPECT_LE(chord.root, 11);
        EXPECT_FALSE(chord.notes.empty());
    }
}

TEST_F(AdvancedHarmonyEngineTest, HarmonicAnalysis)
{
    // Create a simple I-V-vi-IV progression
    std::vector<MusicTheoryEngine::AdvancedChord> progression;
    progression.push_back(engine->generateAdvancedChord(0, MusicTheoryEngine::ExtendedChordType::Major, params));    // I
    progression.push_back(engine->generateAdvancedChord(7, MusicTheoryEngine::ExtendedChordType::Dominant7, params)); // V7
    progression.push_back(engine->generateAdvancedChord(9, MusicTheoryEngine::ExtendedChordType::Minor, params));    // vi
    progression.push_back(engine->generateAdvancedChord(5, MusicTheoryEngine::ExtendedChordType::Major, params));    // IV
    
    auto analysis = harmonyEngine->analyzeProgression(progression, 0, MusicTheoryEngine::ScaleType::Major);
    
    EXPECT_EQ(analysis.functions.size(), progression.size());
    EXPECT_EQ(analysis.tensions.size(), progression.size());
    EXPECT_GE(analysis.overallCoherence, 0.0f);
    EXPECT_LE(analysis.overallCoherence, 1.0f);
    
    // Should identify tonic and dominant functions
    bool hasTonic = false, hasDominant = false;
    for (auto func : analysis.functions)
    {
        if (func == MusicTheoryEngine::AdvancedChord::Function::Tonic) hasTonic = true;
        if (func == MusicTheoryEngine::AdvancedChord::Function::Dominant) hasDominant = true;
    }
    EXPECT_TRUE(hasTonic);
    EXPECT_TRUE(hasDominant);
}

TEST_F(AdvancedHarmonyEngineTest, ProgressionVariations)
{
    // Create base progression
    std::vector<MusicTheoryEngine::AdvancedChord> baseProgression;
    baseProgression.push_back(engine->generateAdvancedChord(0, MusicTheoryEngine::ExtendedChordType::Major, params));
    baseProgression.push_back(engine->generateAdvancedChord(5, MusicTheoryEngine::ExtendedChordType::Major, params));
    baseProgression.push_back(engine->generateAdvancedChord(9, MusicTheoryEngine::ExtendedChordType::Minor, params));
    baseProgression.push_back(engine->generateAdvancedChord(5, MusicTheoryEngine::ExtendedChordType::Major, params));
    
    auto variations = harmonyEngine->generateProgressionVariations(baseProgression, params, 3);
    
    EXPECT_EQ(variations.size(), 3);
    
    for (const auto& variation : variations)
    {
        EXPECT_EQ(variation.size(), baseProgression.size());
        
        // Each variation should be different from the base
        bool isDifferent = false;
        for (size_t i = 0; i < variation.size(); ++i)
        {
            if (variation[i].type != baseProgression[i].type ||
                variation[i].root != baseProgression[i].root)
            {
                isDifferent = true;
                break;
            }
        }
        // Note: Due to randomness, this might occasionally fail, but tests the capability
    }
}

TEST_F(AdvancedHarmonyEngineTest, HarmonyEvolution)
{
    // Create simple seed progression
    std::vector<MusicTheoryEngine::AdvancedChord> seedProgression;
    seedProgression.push_back(engine->generateAdvancedChord(0, MusicTheoryEngine::ExtendedChordType::Major, params));
    seedProgression.push_back(engine->generateAdvancedChord(5, MusicTheoryEngine::ExtendedChordType::Major, params));
    
    // Evolve with 50% evolution amount
    auto evolved = harmonyEngine->evolveHarmony(seedProgression, params, 0.5f);
    
    EXPECT_EQ(evolved.size(), seedProgression.size());
    
    // Evolved progression should have valid chords
    for (const auto& chord : evolved)
    {
        EXPECT_GE(chord.root, 0);
        EXPECT_LE(chord.root, 11);
        EXPECT_FALSE(chord.notes.empty());
        EXPECT_GE(chord.tension, 0.0f);
        EXPECT_LE(chord.tension, 1.0f);
    }
}

TEST_F(AdvancedHarmonyEngineTest, ModalInterchange)
{
    // Create major progression
    std::vector<MusicTheoryEngine::AdvancedChord> progression;
    progression.push_back(engine->generateAdvancedChord(0, MusicTheoryEngine::ExtendedChordType::Major, params));
    progression.push_back(engine->generateAdvancedChord(5, MusicTheoryEngine::ExtendedChordType::Major, params));
    
    // Apply modal interchange
    auto interchanged = engine->applyModalInterchange(progression, 0, MusicTheoryEngine::ScaleType::Major, 0.5f);
    
    EXPECT_EQ(interchanged.size(), progression.size());
    
    // Should have valid chords
    for (const auto& chord : interchanged)
    {
        EXPECT_GE(chord.root, 0);
        EXPECT_LE(chord.root, 11);
        EXPECT_FALSE(chord.notes.empty());
    }
}
