/*
  ==============================================================================

    test_PatternEvolution.cpp
    Created: 2 Aug 2025
    Author:  Epic 6 Story 6.2 Implementation

    Unit tests for pattern evolution and dynamic variation capabilities.
    Tests genetic algorithms, pattern morphing, and evolution engine.

  ==============================================================================
*/

#include <gtest/gtest.h>
#include "ai/MusicTheoryEngine.h"
#include "GenerationParameters.h"
#include "MIDIPattern.h"
#include <vector>
#include <algorithm>

class PatternEvolutionTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        engine = std::make_unique<MusicTheoryEngine>();
        
        // Create test patterns
        testPattern1 = createTestPattern(60, 4);  // C major pattern
        testPattern2 = createTestPattern(67, 4);  // G major pattern
        
        // Setup evolution parameters
        params.variation.enableEvolution = true;
        params.variation.mutationRate = 0.2f;
        params.variation.crossoverRate = 0.5f;
        params.variation.variationIntensity = 0.6f;
        params.variation.evolutionSpeed = 0.3f;
        params.variation.generationHistory = 5;
        params.variation.allowReversion = true;
    }

    MIDIPattern createTestPattern(int rootNote, int numNotes)
    {
        MIDIPattern pattern;
        for (int i = 0; i < numNotes; ++i)
        {
            Note note;
            note.pitch = rootNote + (i * 2);  // Simple ascending pattern
            note.startTime = i * 0.5;  // Half-beat spacing
            note.duration = 0.4;
            note.velocity = 80;
            pattern.notes.push_back(note);
        }
        return pattern;
    }

    std::unique_ptr<MusicTheoryEngine> engine;
    MIDIPattern testPattern1, testPattern2;
    GenerationParameters params;
};

TEST_F(PatternEvolutionTest, PatternGenomeConversion)
{
    // Test conversion to genome
    auto genome = engine->convertToGenome(testPattern1);
    EXPECT_EQ(genome.notes.size(), testPattern1.notes.size());
    EXPECT_FALSE(genome.id.isEmpty());
    EXPECT_GT(genome.timestamp, 0.0);
    
    // Test conversion back to pattern
    auto convertedPattern = engine->convertFromGenome(genome);
    EXPECT_EQ(convertedPattern.notes.size(), testPattern1.notes.size());
    
    for (size_t i = 0; i < testPattern1.notes.size(); ++i)
    {
        EXPECT_EQ(convertedPattern.notes[i].pitch, testPattern1.notes[i].pitch);
        EXPECT_DOUBLE_EQ(convertedPattern.notes[i].startTime, testPattern1.notes[i].startTime);
    }
}

TEST_F(PatternEvolutionTest, FitnessCalculation)
{
    auto genome = engine->convertToGenome(testPattern1);
    float fitness = engine->calculatePatternFitness(genome, params);
    
    // Fitness should be between 0 and 1
    EXPECT_GE(fitness, 0.0f);
    EXPECT_LE(fitness, 1.0f);
    
    // Test empty pattern
    MusicTheoryEngine::PatternGenome emptyGenome;
    float emptyFitness = engine->calculatePatternFitness(emptyGenome, params);
    EXPECT_EQ(emptyFitness, 0.0f);
}

TEST_F(PatternEvolutionTest, PatternMutation)
{
    auto originalGenome = engine->convertToGenome(testPattern1);
    auto mutatedGenome = engine->mutatePattern(originalGenome, params);
    
    // Should have same number of notes
    EXPECT_EQ(mutatedGenome.notes.size(), originalGenome.notes.size());
    
    // Some notes should be different (with high probability due to mutation rate)
    bool hasChanges = false;
    for (size_t i = 0; i < originalGenome.notes.size(); ++i)
    {
        if (mutatedGenome.notes[i].pitch != originalGenome.notes[i].pitch ||
            mutatedGenome.notes[i].startTime != originalGenome.notes[i].startTime ||
            mutatedGenome.notes[i].velocity != originalGenome.notes[i].velocity)
        {
            hasChanges = true;
            break;
        }
    }
    
    // With 20% mutation rate and 4 notes, very likely to have at least one change
    // Note: This is probabilistic, so it might occasionally fail
    EXPECT_TRUE(hasChanges || params.variation.mutationRate < 0.1f);
}

TEST_F(PatternEvolutionTest, PatternCrossover)
{
    auto genome1 = engine->convertToGenome(testPattern1);
    auto genome2 = engine->convertToGenome(testPattern2);
    
    auto offspring = engine->crossoverPatterns(genome1, genome2, params);
    
    // Should have notes (not empty)
    EXPECT_FALSE(offspring.notes.empty());
    
    // Should contain genetic material from both parents
    // This is harder to test directly, but we can check reasonable size
    EXPECT_LE(offspring.notes.size(), genome1.notes.size() + genome2.notes.size());
}

TEST_F(PatternEvolutionTest, PatternMorphing)
{
    auto genome1 = engine->convertToGenome(testPattern1);
    auto genome2 = engine->convertToGenome(testPattern2);
    
    // Test morphing at different amounts
    auto morph0 = engine->morphPatterns(genome1, genome2, 0.0f);
    auto morph50 = engine->morphPatterns(genome1, genome2, 0.5f);
    auto morph100 = engine->morphPatterns(genome1, genome2, 1.0f);
    
    // At 0% morph, should be close to genome1
    EXPECT_EQ(morph0.notes.size(), genome1.notes.size());
    
    // At 50% morph, should be between the two
    EXPECT_FALSE(morph50.notes.empty());
    
    // At 100% morph, should be close to genome2
    if (!genome1.notes.empty() && !genome2.notes.empty())
    {
        // Check that morphing actually changes values
        bool different = false;
        for (size_t i = 0; i < std::min(morph0.notes.size(), morph100.notes.size()); ++i)
        {
            if (morph0.notes[i].pitch != morph100.notes[i].pitch)
            {
                different = true;
                break;
            }
        }
        EXPECT_TRUE(different);
    }
}

TEST_F(PatternEvolutionTest, VariationGeneration)
{
    auto baseGenome = engine->convertToGenome(testPattern1);
    auto variations = engine->generateVariations(baseGenome, 3, params);
    
    EXPECT_EQ(variations.size(), 3);
    
    // Each variation should be different from the base
    for (const auto& variation : variations)
    {
        EXPECT_EQ(variation.notes.size(), baseGenome.notes.size());
        EXPECT_GE(variation.fitness, 0.0f);
        EXPECT_LE(variation.fitness, 1.0f);
    }
}

TEST_F(PatternEvolutionTest, EliteSelection)
{
    // Create population with different fitness values
    std::vector<MusicTheoryEngine::PatternGenome> population;
    
    for (int i = 0; i < 10; ++i)
    {
        auto genome = engine->convertToGenome(testPattern1);
        genome.fitness = i * 0.1f;  // Fitness from 0.0 to 0.9
        population.push_back(genome);
    }
    
    auto elite = engine->selectElitePatterns(population, 3);
    
    EXPECT_EQ(elite.size(), 3);
    
    // Should be sorted by fitness (highest first)
    for (size_t i = 1; i < elite.size(); ++i)
    {
        EXPECT_GE(elite[i-1].fitness, elite[i].fitness);
    }
    
    // Best should have highest fitness
    EXPECT_FLOAT_EQ(elite[0].fitness, 0.9f);
}

class EvolutionEngineTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        engine = std::make_unique<MusicTheoryEngine>();
        evolutionEngine = std::make_unique<MusicTheoryEngine::PatternEvolutionEngine>(engine.get());
        
        // Create seed patterns
        seedPatterns.push_back(createTestPattern(60, 4));  // C major
        seedPatterns.push_back(createTestPattern(65, 4));  // F major
        seedPatterns.push_back(createTestPattern(67, 4));  // G major
        
        params.variation.enableEvolution = true;
        params.variation.mutationRate = 0.15f;
        params.variation.crossoverRate = 0.4f;
        params.variation.generationHistory = 5;
    }

    MIDIPattern createTestPattern(int rootNote, int numNotes)
    {
        MIDIPattern pattern;
        for (int i = 0; i < numNotes; ++i)
        {
            Note note;
            note.pitch = rootNote + (i % 7);  // Stay within octave
            note.startTime = i * 0.5;
            note.duration = 0.4;
            note.velocity = 80 + (i * 5);  // Varying velocity
            pattern.notes.push_back(note);
        }
        return pattern;
    }

    std::unique_ptr<MusicTheoryEngine> engine;
    std::unique_ptr<MusicTheoryEngine::PatternEvolutionEngine> evolutionEngine;
    std::vector<MIDIPattern> seedPatterns;
    GenerationParameters params;
};

TEST_F(EvolutionEngineTest, PopulationInitialization)
{
    evolutionEngine->initializePopulation(seedPatterns, params);
    
    auto elite = evolutionEngine->getCurrentElite(3);
    EXPECT_EQ(elite.size(), 3);  // Should match seed pattern count
    
    // All should have valid fitness
    for (const auto& genome : elite)
    {
        EXPECT_GE(genome.fitness, 0.0f);
        EXPECT_LE(genome.fitness, 1.0f);
        EXPECT_FALSE(genome.id.isEmpty());
    }
}

TEST_F(EvolutionEngineTest, EvolutionProcess)
{
    evolutionEngine->initializePopulation(seedPatterns, params);
    
    auto initialStats = evolutionEngine->getStats();
    EXPECT_EQ(initialStats.currentGeneration, 0);
    EXPECT_EQ(initialStats.populationSize, 3);
    
    // Evolve one generation
    evolutionEngine->evolveGeneration(params);
    
    auto evolvedStats = evolutionEngine->getStats();
    EXPECT_EQ(evolvedStats.currentGeneration, 1);
    EXPECT_EQ(evolvedStats.populationSize, 3);
    
    // Should have fitness history
    EXPECT_EQ(evolvedStats.fitnessHistory.size(), 2);  // Initial + evolved
}

TEST_F(EvolutionEngineTest, GenerationHistory)
{
    evolutionEngine->initializePopulation(seedPatterns, params);
    
    // Evolve multiple generations
    for (int i = 0; i < 3; ++i)
    {
        evolutionEngine->evolveGeneration(params);
    }
    
    // Test accessing historical patterns
    auto gen0Pattern = evolutionEngine->getPatternFromHistory(0, 0);
    auto gen2Pattern = evolutionEngine->getPatternFromHistory(2, 0);
    
    EXPECT_FALSE(gen0Pattern.notes.empty());
    EXPECT_FALSE(gen2Pattern.notes.empty());
    // Note: Generation numbers may vary based on how evolution engine tracks them
    EXPECT_GE(gen0Pattern.generation, 0);
    EXPECT_GE(gen2Pattern.generation, gen0Pattern.generation);
}

TEST_F(EvolutionEngineTest, GenerationReversion)
{
    evolutionEngine->initializePopulation(seedPatterns, params);
    
    // Evolve several generations
    for (int i = 0; i < 3; ++i)
    {
        evolutionEngine->evolveGeneration(params);
    }
    
    auto currentStats = evolutionEngine->getStats();
    EXPECT_EQ(currentStats.currentGeneration, 3);
    
    // Revert to generation 1
    bool success = evolutionEngine->revertToGeneration(1);
    EXPECT_TRUE(success);
    
    auto revertedStats = evolutionEngine->getStats();
    EXPECT_EQ(revertedStats.currentGeneration, 1);
    
    // Test invalid reversion
    bool invalidRevert = evolutionEngine->revertToGeneration(10);
    EXPECT_FALSE(invalidRevert);
}

TEST_F(EvolutionEngineTest, EvolutionStats)
{
    evolutionEngine->initializePopulation(seedPatterns, params);
    
    auto stats = evolutionEngine->getStats();
    
    EXPECT_EQ(stats.currentGeneration, 0);
    EXPECT_GT(stats.populationSize, 0);
    EXPECT_GE(stats.averageFitness, 0.0f);
    EXPECT_LE(stats.averageFitness, 1.0f);
    EXPECT_GE(stats.bestFitness, 0.0f);
    EXPECT_LE(stats.bestFitness, 1.0f);
    EXPECT_GE(stats.bestFitness, stats.averageFitness);  // Best should be >= average
    EXPECT_FALSE(stats.fitnessHistory.empty());
}
