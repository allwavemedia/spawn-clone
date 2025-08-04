/*
  ==============================================================================

    test_AdvancedGeneration.cpp
    Created: 2 Aug 2025
    Author:  Epic 6 Story 6.1 Implementation

    Unit tests for advanced pattern generation features.

  ==============================================================================
*/

#include <gtest/gtest.h>
#include "../Source/ai/MusicTheoryEngine.h"
#include "../Source/GenerationParameters.h"

//==============================================================================
// Epic 6 Story 6.1: Melodic Contour Generation Tests

class MelodicContourTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        engine = std::make_unique<MusicTheoryEngine>();
        cMajorScale = engine->getScaleNotes(60, MusicTheoryEngine::ScaleType::Major); // C Major
    }

    std::unique_ptr<MusicTheoryEngine> engine;
    std::vector<int> cMajorScale;
};

TEST_F(MelodicContourTest, RisingContour)
{
    auto melody = engine->generateMelodicContour(cMajorScale, 8, GenerationParameters::MelodicContour::Rising, 0.5f);
    ASSERT_EQ(melody.size(), 8);
    for (size_t i = 1; i < melody.size(); ++i)
    {
        EXPECT_GE(melody[i], melody[i - 1]);
    }
}

TEST_F(MelodicContourTest, FallingContour)
{
    auto melody = engine->generateMelodicContour(cMajorScale, 8, GenerationParameters::MelodicContour::Falling, 0.5f);
    ASSERT_EQ(melody.size(), 8);
    for (size_t i = 1; i < melody.size(); ++i)
    {
        EXPECT_LE(melody[i], melody[i - 1]);
    }
}

TEST_F(MelodicContourTest, WaveContour)
{
    auto melody = engine->generateMelodicContour(cMajorScale, 16, GenerationParameters::MelodicContour::Wave, 0.5f);
    ASSERT_EQ(melody.size(), 16);
    // Check for alternating up/down movement
    bool isRising = melody[1] > melody[0];
    for (size_t i = 2; i < melody.size(); ++i)
    {
        if ((i % 4) == 0)
        {
            isRising = !isRising;
        }
        if (isRising)
        {
            EXPECT_GE(melody[i], melody[i - 1]);
        }
        else
        {
            EXPECT_LE(melody[i], melody[i - 1]);
        }
    }
}

//==============================================================================
// Epic 6 Story 6.1: Rhythmic Pattern Generation Tests

class RhythmicPatternTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        engine = std::make_unique<MusicTheoryEngine>();
    }

    std::unique_ptr<MusicTheoryEngine> engine;
};

TEST_F(RhythmicPatternTest, BasicRhythmGeneration)
{
    auto rhythm = engine->generateRhythmicPattern(8, 4.0, 0.2f, 0.5f, 0.0f);
    ASSERT_EQ(rhythm.size(), 8);
    for (size_t i = 1; i < rhythm.size(); ++i)
    {
        EXPECT_GT(rhythm[i], rhythm[i - 1]);
    }
    // With density 0.5, the pattern should span 4.0/0.5 = 8.0 beats, so last note should be around 7.0
    EXPECT_LT(rhythm.back(), 8.0);
}

TEST_F(RhythmicPatternTest, SwingRhythm)
{
    auto straightRhythm = engine->generateRhythmicPattern(8, 4.0, 0.2f, 0.5f, 0.0f);
    auto swingRhythm = engine->generateRhythmicPattern(8, 4.0, 0.2f, 0.5f, 0.6f);
    
    ASSERT_EQ(straightRhythm.size(), 8);
    ASSERT_EQ(swingRhythm.size(), 8);
    
    // Check that off-beats are delayed in swing rhythm
    for (size_t i = 1; i < swingRhythm.size(); i += 2)
    {
        EXPECT_GT(swingRhythm[i], straightRhythm[i]);
    }
}

TEST_F(RhythmicPatternTest, DensityControl)
{
    auto sparseRhythm = engine->generateRhythmicPattern(8, 4.0, 0.2f, 0.2f, 0.0f);
    auto denseRhythm = engine->generateRhythmicPattern(8, 4.0, 0.2f, 0.8f, 0.0f);
    
    ASSERT_EQ(sparseRhythm.size(), 8);
    ASSERT_EQ(denseRhythm.size(), 8);
    
    EXPECT_GT(sparseRhythm.back(), denseRhythm.back());
}
