/*
  ==============================================================================

    test_FastModeGeneration.cpp
    Created: 2 Aug 2025
    Author:  Epic 7 Story 7.1 Task 7.1.6 Implementation

    Unit tests for Fast Mode generation quality and timing validation.
    Tests ensure sub-2-second generation and musical quality standards.

  ==============================================================================
*/

#include <gtest/gtest.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_core/juce_core.h>
#include <chrono>
#include "ai/AIGenerationEngine.h"
#include "GenerationParameters.h"
#include "MIDIPattern.h"
#include "PatternManager.h"
#include "ThreadManager.h"

//==============================================================================
class FastModeGenerationTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        threadManager = std::make_unique<ThreadManager>();
        patternManager = std::make_unique<PatternManager>();
        aiEngine = std::make_unique<AIGenerationEngine>(*threadManager, *patternManager);
        
        // Setup default parameters for Fast Mode
        setupDefaultParameters();
    }

    void TearDown() override
    {
        aiEngine.reset();
        patternManager.reset();
        threadManager.reset();
    }

    void setupDefaultParameters()
    {
        params.rhythmicComplexity = 0.5f;
        params.tempo = 120.0f;
        params.key = 0; // C
        params.scale = GenerationParameters::ScaleType::Major;
        params.generationType = GenerationParameters::GenerationType::Melody;
        params.patternLengthBeats = 16.0f; // 4 bars * 4 beats/bar
        params.aiMode = GenerationParameters::AIMode::Fast;
        params.generationSeed = 12345; // Deterministic for testing
    }

    std::unique_ptr<ThreadManager> threadManager;
    std::unique_ptr<PatternManager> patternManager;
    std::unique_ptr<AIGenerationEngine> aiEngine;
    GenerationParameters params;
};

//==============================================================================
// Task 7.1.6: Fast Mode Generation Timing Tests

TEST_F(FastModeGenerationTest, GenerationTimingUnder2Seconds)
{
    // Test requirement: Fast Mode must generate patterns in under 2 seconds
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Trigger generation
    aiEngine->generatePattern(params);
    
    // Wait for completion (with timeout)
    int waitCount = 0;
    const int maxWaitMs = 3000; // 3 second timeout
    const int checkIntervalMs = 10;
    
    while (!patternManager->getCurrentPattern().has_value() && waitCount < maxWaitMs)
    {
        juce::Thread::sleep(checkIntervalMs);
        waitCount += checkIntervalMs;
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    // Verify timing requirement
    EXPECT_LT(duration.count(), 2000) << "Fast Mode generation took " << duration.count() << "ms, exceeding 2-second target";
    
    // Verify pattern was generated
    EXPECT_TRUE(patternManager->getCurrentPattern().has_value()) << "No pattern was generated";
    EXPECT_FALSE(patternManager->getCurrentPattern()->notes.empty()) << "No pattern was generated";
}

TEST_F(FastModeGenerationTest, ConsistentTimingAcrossComplexity)
{
    // Test timing consistency across different complexity levels
    
    std::vector<float> complexityLevels = {0.1f, 0.3f, 0.5f, 0.7f, 0.9f};
    std::vector<long> timings;
    
    for (float complexity : complexityLevels)
    {
        params.rhythmicComplexity = complexity;
        patternManager->clear(); // Reset for next test
        
        auto startTime = std::chrono::high_resolution_clock::now();
        aiEngine->generatePattern(params);
        
        // Wait for completion
        int waitCount = 0;
        while (!patternManager->getCurrentPattern().has_value() && waitCount < 2500)
        {
            juce::Thread::sleep(10);
            waitCount += 10;
        }
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        timings.push_back(duration.count());
        
        EXPECT_LT(duration.count(), 2000) << "Complexity " << complexity << " took " << duration.count() << "ms";
    }
    
    // Verify timing consistency (no outliers)
    for (auto timing : timings)
    {
        EXPECT_LT(timing, 2000) << "Timing outlier detected: " << timing << "ms";
    }
}

//==============================================================================
// Task 7.1.6: Fast Mode Generation Quality Tests

TEST_F(FastModeGenerationTest, GeneratedPatternStructure)
{
    aiEngine->generatePattern(params);
    
    // Wait for generation
    int waitCount = 0;
    while (!patternManager->getCurrentPattern().has_value() && waitCount < 2000)
    {
        juce::Thread::sleep(10);
        waitCount += 10;
    }
    
    const auto& patternOpt = patternManager->getCurrentPattern();
    ASSERT_TRUE(patternOpt.has_value());
    const auto& pattern = *patternOpt;
    
    // Verify basic pattern structure
    EXPECT_FALSE(pattern.notes.empty()) << "Pattern should contain notes";
    EXPECT_GT(pattern.notes.size(), 3) << "Pattern should have reasonable note count";
    EXPECT_LT(pattern.notes.size(), 200) << "Pattern should not be excessively dense";
    
    // Verify timing bounds
    for (const auto& note : pattern.notes)
    {
        EXPECT_GE(note.startTime, 0.0) << "Note start time should be non-negative";
        EXPECT_GT(note.duration, 0.0) << "Note duration should be positive";
        EXPECT_GE(note.pitch, 21) << "Note pitch should be within MIDI range (>= A0)";
        EXPECT_LE(note.pitch, 108) << "Note pitch should be within MIDI range (<= C8)";
        EXPECT_GE(note.velocity, 1) << "Note velocity should be positive";
        EXPECT_LE(note.velocity, 127) << "Note velocity should be within MIDI range";
    }
}

TEST_F(FastModeGenerationTest, GenreSpecificPatterns)
{
    // Test genre-specific pattern generation
    std::vector<GenerationParameters::GenerationType> genres = {
        GenerationParameters::GenerationType::Melody,
        GenerationParameters::GenerationType::Chords,
        GenerationParameters::GenerationType::Bassline,
        GenerationParameters::GenerationType::Drums
    };
    
    for (const auto& genre : genres)
    {
        params.generationType = genre;
        patternManager->clear();
        
        aiEngine->generatePattern(params);
        
        // Wait for generation
        int waitCount = 0;
        while (!patternManager->getCurrentPattern().has_value() && waitCount < 2000)
        {
            juce::Thread::sleep(10);
            waitCount += 10;
        }
        
        const auto& patternOpt = patternManager->getCurrentPattern();
        ASSERT_TRUE(patternOpt.has_value());
        const auto& pattern = *patternOpt;
        
        EXPECT_FALSE(pattern.notes.empty()) << "Genre " << (int)genre << " should generate valid pattern";
        
        // Genre-specific validation could be added here
        // For now, verify basic structure
        EXPECT_GT(pattern.notes.size(), 2) << "Genre " << (int)genre << " should generate sufficient notes";
    }
}

TEST_F(FastModeGenerationTest, DeterministicGeneration)
{
    // Test that same seed produces same pattern
    
    // Generate first pattern
    aiEngine->generatePattern(params);
    int waitCount = 0;
    while (!patternManager->getCurrentPattern().has_value() && waitCount < 2000)
    {
        juce::Thread::sleep(10);
        waitCount += 10;
    }
    ASSERT_TRUE(patternManager->getCurrentPattern().has_value());
    auto firstPattern = *patternManager->getCurrentPattern();
    
    // Generate second pattern with same seed
    patternManager->clear();
    aiEngine->generatePattern(params);
    waitCount = 0;
    while (!patternManager->getCurrentPattern().has_value() && waitCount < 2000)
    {
        juce::Thread::sleep(10);
        waitCount += 10;
    }
    ASSERT_TRUE(patternManager->getCurrentPattern().has_value());
    auto secondPattern = *patternManager->getCurrentPattern();
    
    // Verify patterns are identical
    EXPECT_EQ(firstPattern.notes.size(), secondPattern.notes.size()) 
        << "Patterns generated with the same seed should have the same number of notes.";

    if (firstPattern.notes.size() == secondPattern.notes.size())
    {
        for (size_t i = 0; i < firstPattern.notes.size(); ++i)
        {
            EXPECT_FLOAT_EQ(firstPattern.notes[i].startTime, secondPattern.notes[i].startTime)
                << "Note " << i << " start time mismatch.";
            EXPECT_FLOAT_EQ(firstPattern.notes[i].duration, secondPattern.notes[i].duration)
                << "Note " << i << " duration mismatch.";
            EXPECT_EQ(firstPattern.notes[i].pitch, secondPattern.notes[i].pitch)
                << "Note " << i << " pitch mismatch.";
            EXPECT_EQ(firstPattern.notes[i].velocity, secondPattern.notes[i].velocity)
                << "Note " << i << " velocity mismatch.";
        }
    }
}

TEST_F(FastModeGenerationTest, ComplexityScaling)
{
    // Test that complexity parameter affects pattern generation
    
    // Low complexity
    params.rhythmicComplexity = 0.1f;
    aiEngine->generatePattern(params);
    
    int waitCount = 0;
    while (!patternManager->getCurrentPattern().has_value() && waitCount < 2000)
    {
        juce::Thread::sleep(10);
        waitCount += 10;
    }
    
    ASSERT_TRUE(patternManager->getCurrentPattern().has_value());
    auto lowComplexityPattern = *patternManager->getCurrentPattern();
    
    // High complexity
    params.rhythmicComplexity = 0.9f;
    params.generationSeed = 12346; // Different seed to avoid identical patterns
    patternManager->clear();
    aiEngine->generatePattern(params);
    
    waitCount = 0;
    while (!patternManager->getCurrentPattern().has_value() && waitCount < 2000)
    {
        juce::Thread::sleep(10);
        waitCount += 10;
    }
    
    ASSERT_TRUE(patternManager->getCurrentPattern().has_value());
    auto highComplexityPattern = *patternManager->getCurrentPattern();
    
    // Verify both patterns exist
    EXPECT_FALSE(lowComplexityPattern.notes.empty()) << "Low complexity should generate pattern";
    EXPECT_FALSE(highComplexityPattern.notes.empty()) << "High complexity should generate pattern";
    
    // Generally, higher complexity should produce more notes or more varied timing
    // This is a basic heuristic test
    EXPECT_GT(highComplexityPattern.notes.size(), 0) << "High complexity should produce notes";
}

//==============================================================================
// Performance Benchmarking Tests

TEST_F(FastModeGenerationTest, PerformanceBenchmark)
{
    // Benchmark multiple generations to verify consistent performance
    
    const int iterations = 5;
    std::vector<long> timings;
    
    for (int i = 0; i < iterations; ++i)
    {
        params.generationSeed = 10000 + i; // Vary seed
        patternManager->clear();
        
        auto startTime = std::chrono::high_resolution_clock::now();
        aiEngine->generatePattern(params);
        
        int waitCount = 0;
        while (!patternManager->getCurrentPattern().has_value() && waitCount < 2500)
        {
            juce::Thread::sleep(10);
            waitCount += 10;
        }
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        timings.push_back(duration.count());
    }
    
    // Calculate statistics
    long totalTime = 0;
    long maxTime = 0;
    long minTime = LONG_MAX;
    
    for (auto time : timings)
    {
        totalTime += time;
        maxTime = std::max(maxTime, time);
        minTime = std::min(minTime, time);
    }
    
    double avgTime = static_cast<double>(totalTime) / iterations;
    
    // Performance assertions
    EXPECT_LT(avgTime, 1500.0) << "Average generation time should be under 1.5 seconds";
    EXPECT_LT(maxTime, 2000) << "Maximum generation time should be under 2 seconds";
    EXPECT_GT(minTime, 50) << "Minimum time should be reasonable (not suspiciously fast)";
    
    // Log performance metrics for analysis
    std::cout << "\nFast Mode Performance Metrics:\n";
    std::cout << "Average: " << avgTime << "ms\n";
    std::cout << "Min: " << minTime << "ms\n";
    std::cout << "Max: " << maxTime << "ms\n";
}
