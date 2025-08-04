/*
  ==============================================================================

    test_UserStyleProfiles.cpp
    Created: 2 Aug 2025
    Author:  Epic 6 Story 6.4 Implementation

    Unit tests for user style profile learning and adaptive generation.
    Tests preference learning, parameter adaptation, and feedback collection.

  ==============================================================================
*/

#include <gtest/gtest.h>
#include "UserStyleProfile.h"
#include "UserStyleLearningEngine.h"
#include "ai/MusicTheoryEngine.h"
#include "GenerationParameters.h"
#include <vector>
#include <algorithm>

class UserStyleProfileTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        musicEngine = std::make_unique<MusicTheoryEngine>();
        learningEngine = std::make_unique<UserStyleLearningEngine>(musicEngine.get());
        feedbackCollector = std::make_unique<UserFeedbackCollector>();
        
        // Setup test profile
        testProfile.profileId = "test-profile";
        testProfile.profileName = "Test User";
        testProfile.resetToDefaults();
        
        // Setup test parameters
        testParams.key = 0; // C major
        testParams.scale = GenerationParameters::ScaleType::Major;
        testParams.harmony.enableAdvancedHarmony = true;
        testParams.harmony.complexity = GenerationParameters::HarmonicSettings::HarmonicComplexity::Intermediate;
        testParams.harmony.style = GenerationParameters::HarmonicSettings::ProgressionStyle::Pop;
        testParams.rhythmicComplexity = 0.6f;
    }

    std::unique_ptr<MusicTheoryEngine> musicEngine;
    std::unique_ptr<UserStyleLearningEngine> learningEngine;
    std::unique_ptr<UserFeedbackCollector> feedbackCollector;
    UserStyleProfile testProfile;
    GenerationParameters testParams;
};

//==============================================================================
// UserStyleProfile Tests

TEST_F(UserStyleProfileTest, ProfileSerialization)
{
    // Setup profile with some preferences
    testProfile.harmonicComplexityPreference = 0.8f;
    testProfile.rhythmicComplexityPreference = 0.3f;
    testProfile.totalGenerations = 42;
    testProfile.averageRating = 4.2f;
    testProfile.genreAffinities["Jazz"] = 0.9f;
    testProfile.genreAffinities["Pop"] = 0.6f;
    
    // Serialize to ValueTree
    auto tree = testProfile.toValueTree();
    EXPECT_TRUE(tree.hasType("UserStyleProfile"));
    EXPECT_EQ(tree.getProperty("profileId").toString(), testProfile.profileId);
    EXPECT_FLOAT_EQ(static_cast<float>(tree.getProperty("harmonicComplexityPreference")), 0.8f);
    EXPECT_EQ(static_cast<int>(tree.getProperty("totalGenerations")), 42);
    
    // Deserialize and verify
    UserStyleProfile loadedProfile;
    loadedProfile.fromValueTree(tree);
    
    EXPECT_EQ(loadedProfile.profileId, testProfile.profileId);
    EXPECT_FLOAT_EQ(loadedProfile.harmonicComplexityPreference, 0.8f);
    EXPECT_FLOAT_EQ(loadedProfile.rhythmicComplexityPreference, 0.3f);
    EXPECT_EQ(loadedProfile.totalGenerations, 42);
    EXPECT_FLOAT_EQ(loadedProfile.averageRating, 4.2f);
    EXPECT_FLOAT_EQ(loadedProfile.genreAffinities["Jazz"], 0.9f);
    EXPECT_FLOAT_EQ(loadedProfile.genreAffinities["Pop"], 0.6f);
}

TEST_F(UserStyleProfileTest, PreferenceDefaultsAndReset)
{
    // Modify profile
    testProfile.harmonicComplexityPreference = 0.9f;
    testProfile.syncopationPreference = 0.2f;
    testProfile.totalRatings = 25;
    
    // Reset and verify defaults
    testProfile.resetToDefaults();
    
    EXPECT_FLOAT_EQ(testProfile.harmonicComplexityPreference, 0.5f);
    EXPECT_FLOAT_EQ(testProfile.syncopationPreference, 0.5f);
    EXPECT_EQ(testProfile.totalRatings, 0);
    EXPECT_TRUE(testProfile.chordTypePreferences.empty());
    EXPECT_TRUE(testProfile.genreAffinities.empty());
}

TEST_F(UserStyleProfileTest, CategoryConfidence)
{
    // Test confidence with no data
    EXPECT_FLOAT_EQ(testProfile.getCategoryConfidence("harmonic"), 0.0f);
    
    // Add some data
    testProfile.totalGenerations = 15;
    testProfile.chordTypePreferences[MusicTheoryEngine::ExtendedChordType::Major] = 0.8f;
    testProfile.chordTypePreferences[MusicTheoryEngine::ExtendedChordType::Minor] = 0.6f;
    
    // Test confidence scaling
    float harmonicConfidence = testProfile.getCategoryConfidence("harmonic");
    EXPECT_GT(harmonicConfidence, 0.0f);
    EXPECT_LE(harmonicConfidence, 1.0f);
    
    float rhythmicConfidence = testProfile.getCategoryConfidence("rhythmic");
    EXPECT_GT(rhythmicConfidence, 0.0f);
    EXPECT_LE(rhythmicConfidence, 1.0f);
}

TEST_F(UserStyleProfileTest, PreferenceSummary)
{
    // Test empty profile summary
    auto summary = testProfile.getPreferenceSummary();
    EXPECT_FALSE(summary.isEmpty());
    EXPECT_TRUE(summary[0].contains("Still learning"));
    
    // Add strong preferences
    testProfile.harmonicComplexityPreference = 0.9f;
    testProfile.syncopationPreference = 0.8f;
    testProfile.genreAffinities["Jazz"] = 0.9f;
    
    summary = testProfile.getPreferenceSummary();
    bool hasComplexHarmony = false;
    bool hasSyncopation = false;
    bool hasJazz = false;
    
    for (const auto& item : summary)
    {
        if (item.contains("complex harmony")) hasComplexHarmony = true;
        if (item.contains("syncopated")) hasSyncopation = true;
        if (item.contains("Jazz")) hasJazz = true;
    }
    
    EXPECT_TRUE(hasComplexHarmony);
    EXPECT_TRUE(hasSyncopation);
    EXPECT_TRUE(hasJazz);
}

//==============================================================================
// UserFeedbackCollector Tests

TEST_F(UserStyleProfileTest, FeedbackCollection)
{
    // Record some feedback
    feedbackCollector->recordFeedback("pattern1", PatternRating::Love, testParams, "Jazz");
    feedbackCollector->recordFeedback("pattern2", PatternRating::Like, testParams, "Pop");
    feedbackCollector->recordFeedback("pattern3", PatternRating::Discard, testParams, "Jazz");
    
    // Verify collection
    const auto& allFeedback = feedbackCollector->getAllFeedback();
    EXPECT_EQ(allFeedback.size(), 3);
    
    // Test recent feedback
    auto recent = feedbackCollector->getRecentFeedback(2);
    EXPECT_EQ(recent.size(), 2);
    EXPECT_EQ(recent[1].patternId, "pattern3"); // Most recent
    
    // Test genre filtering
    auto jazzFeedback = feedbackCollector->getFeedbackForGenre("Jazz");
    EXPECT_EQ(jazzFeedback.size(), 2);
    
    auto popFeedback = feedbackCollector->getFeedbackForGenre("Pop");
    EXPECT_EQ(popFeedback.size(), 1);
    
    // Test average rating
    float avgRating = feedbackCollector->getRecentAverageRating(3);
    float expectedAvg = (5.0f + 4.0f + 1.0f) / 3.0f; // Love=5, Like=4, Discard=1
    EXPECT_FLOAT_EQ(avgRating, expectedAvg);
}

TEST_F(UserStyleProfileTest, FeedbackSerialization)
{
    // Add feedback
    feedbackCollector->recordFeedback("pattern1", PatternRating::Love, testParams, "Jazz");
    feedbackCollector->recordFeedback("pattern2", PatternRating::Like, testParams, "Pop");
    
    // Export and import
    auto exportedData = feedbackCollector->exportFeedbackData();
    auto newCollector = std::make_unique<UserFeedbackCollector>();
    newCollector->importFeedbackData(exportedData);
    
    // Verify
    const auto& originalFeedback = feedbackCollector->getAllFeedback();
    const auto& importedFeedback = newCollector->getAllFeedback();
    
    EXPECT_EQ(originalFeedback.size(), importedFeedback.size());
    EXPECT_EQ(originalFeedback[0].patternId, importedFeedback[0].patternId);
    EXPECT_EQ(originalFeedback[0].rating, importedFeedback[0].rating);
    EXPECT_EQ(originalFeedback[0].genreContext, importedFeedback[0].genreContext);
}

//==============================================================================
// UserStyleLearningEngine Tests

TEST_F(UserStyleProfileTest, PatternAnalysis)
{
    // Create test pattern
    MIDIPattern testPattern;
    testPattern.notes = {
        {60, 127, 0.0, 0.5},   // C4
        {64, 120, 0.5, 0.5},   // E4
        {67, 115, 1.0, 0.5},   // G4
        {72, 110, 1.5, 1.0}    // C5
    };
    
    // Analyze pattern
    auto analysis = learningEngine->analyzePattern(testPattern, testParams);
    
    // Verify analysis results
    EXPECT_GT(analysis.patternLength, 0);
    EXPECT_GE(analysis.harmonicConfidence, 0.0f);
    EXPECT_LE(analysis.harmonicConfidence, 1.0f);
    EXPECT_GE(analysis.rhythmicConfidence, 0.0f);
    EXPECT_LE(analysis.rhythmicConfidence, 1.0f);
    EXPECT_GE(analysis.melodicConfidence, 0.0f);
    EXPECT_LE(analysis.melodicConfidence, 1.0f);
    EXPECT_GE(analysis.overallComplexity, 0.0f);
    EXPECT_LE(analysis.overallComplexity, 1.0f);
    EXPECT_FALSE(analysis.inferredGenre.isEmpty());
}

TEST_F(UserStyleProfileTest, PreferenceLearning)
{
    // Create positive feedback for complex patterns
    PatternFeedback positiveFeedback;
    positiveFeedback.patternId = "complex-pattern";
    positiveFeedback.rating = PatternRating::Love;
    positiveFeedback.usedParams = testParams;
    positiveFeedback.usedParams.rhythmicComplexity = 0.8f;
    positiveFeedback.genreContext = "Jazz";
    
    // Record initial preferences
    float initialComplexity = testProfile.rhythmicComplexityPreference;
    
    // Update profile with positive feedback
    learningEngine->updateStyleProfile(positiveFeedback, testProfile);
    
    // Verify learning occurred
    EXPECT_GT(testProfile.totalRatings, 0);
    EXPECT_GT(testProfile.averageRating, 3.0f); // Should be above neutral
    EXPECT_GT(testProfile.learningConfidence, 0.0f);
    
    // Test genre preference learning
    EXPECT_GT(testProfile.genreAffinities["Jazz"], 0.0f);
}

TEST_F(UserStyleProfileTest, ParameterAdaptation)
{
    // Setup profile with learned preferences
    testProfile.harmonicComplexityPreference = 0.8f;
    testProfile.rhythmicComplexityPreference = 0.3f;
    testProfile.totalRatings = 25; // Enough for confident adaptation
    testProfile.totalGenerations = 35; // Enough for rhythmic confidence (35/30 > 1.0)
    testProfile.learningConfidence = 0.6f;
    
    // Adapt parameters
    auto adaptedParams = learningEngine->adaptParameters(testParams, testProfile, 1.0f);
    
    // Verify adaptation
    auto influence = learningEngine->getLastAdaptationInfluence();
    EXPECT_GE(influence.overallPersonalizationWeight, 0.0f);
    EXPECT_FALSE(influence.confidenceLevel.isEmpty());
    EXPECT_FALSE(influence.reasoning.isEmpty());
    
    // Parameters should be adapted based on preferences
    // (Exact values depend on implementation details)
    EXPECT_NE(adaptedParams.rhythmicComplexity, testParams.rhythmicComplexity);
}

TEST_F(UserStyleProfileTest, LowConfidenceAdaptation)
{
    // Setup profile with insufficient learning data
    testProfile.totalRatings = 2;
    testProfile.learningConfidence = 0.1f;
    
    // Attempt adaptation
    auto adaptedParams = learningEngine->adaptParameters(testParams, testProfile, 1.0f);
    
    // Should not adapt much due to low confidence
    auto influence = learningEngine->getLastAdaptationInfluence();
    EXPECT_EQ(influence.confidenceLevel, "Low");
    EXPECT_LT(influence.overallPersonalizationWeight, 0.1f);
}

TEST_F(UserStyleProfileTest, BatchLearning)
{
    std::vector<PatternFeedback> feedbackBatch;
    
    // Simplified test: Just one pattern per genre for clarity
    PatternFeedback jazzLike;
    jazzLike.patternId = "jazz1";
    jazzLike.rating = PatternRating::Like;
    jazzLike.usedParams = testParams;
    jazzLike.genreContext = "Jazz";
    feedbackBatch.push_back(jazzLike);
    
    PatternFeedback popDiscard;
    popDiscard.patternId = "pop1";
    popDiscard.rating = PatternRating::Discard;
    popDiscard.usedParams = testParams;
    popDiscard.genreContext = "Pop";
    feedbackBatch.push_back(popDiscard);
    
    // Apply batch learning
    learningEngine->batchUpdateProfile(feedbackBatch, testProfile);
    
    // Debug: Check what we actually have 
    float jazzAffinity = testProfile.genreAffinities["Jazz"];
    float popAffinity = testProfile.genreAffinities["Pop"];
    
    // Verify batch processing
    EXPECT_EQ(testProfile.totalRatings, 2);
    EXPECT_FLOAT_EQ(jazzAffinity, 0.1f); // Jazz: Like (+0.1)
    EXPECT_FLOAT_EQ(popAffinity, 0.0f);  // Pop: Discard (-0.1) clamped to 0.0
    EXPECT_GT(jazzAffinity, popAffinity);
}

TEST_F(UserStyleProfileTest, LearningEngineConfiguration)
{
    // Test learning rate configuration
    learningEngine->setLearningRate(0.2f);
    EXPECT_FLOAT_EQ(learningEngine->getLearningRate(), 0.2f);
    
    // Test learning rate bounds
    learningEngine->setLearningRate(1.5f); // Should be clamped
    EXPECT_FLOAT_EQ(learningEngine->getLearningRate(), 1.0f);
    
    learningEngine->setLearningRate(-0.1f); // Should be clamped
    EXPECT_FLOAT_EQ(learningEngine->getLearningRate(), 0.0f);
    
    // Test category enablement
    learningEngine->setLearningEnabled("harmonic", false);
    learningEngine->setLearningEnabled("rhythmic", true);
    
    auto stats = learningEngine->getLearningStats();
    EXPECT_TRUE(stats.activeCategories.contains("rhythmic"));
    EXPECT_FALSE(stats.activeCategories.contains("harmonic"));
}

TEST_F(UserStyleProfileTest, LearningStats)
{
    // Get initial stats
    auto initialStats = learningEngine->getLearningStats();
    EXPECT_EQ(initialStats.totalPatternsAnalyzed, 0);
    EXPECT_EQ(initialStats.totalFeedbackProcessed, 0);
    
    // Create and analyze a pattern
    MIDIPattern testPattern;
    testPattern.notes = {{60, 127, 0.0, 0.5}};
    learningEngine->analyzePattern(testPattern, testParams);
    
    // Update profile
    PatternFeedback feedback;
    feedback.rating = PatternRating::Like;
    feedback.usedParams = testParams;
    learningEngine->updateStyleProfile(feedback, testProfile);
    
    // Check updated stats
    auto updatedStats = learningEngine->getLearningStats();
    EXPECT_GT(updatedStats.totalPatternsAnalyzed, initialStats.totalPatternsAnalyzed);
    EXPECT_GT(updatedStats.totalFeedbackProcessed, initialStats.totalFeedbackProcessed);
    EXPECT_FALSE(updatedStats.activeCategories.isEmpty());
}
