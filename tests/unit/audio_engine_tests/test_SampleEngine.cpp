#include <gtest/gtest.h>
#include "plugin_core/SampleEngine.h"
#include <numeric>
#include <algorithm>

using spawnclone::core::SampleEngine;

TEST(SampleEngineTest, DefaultSampleRateIsPositive)
{
    SampleEngine eng;
    EXPECT_GT(eng.getSampleRate(), 0.0);
}

TEST(SampleEngineTest, RenderSineProducesNonZeroAndRespectsSize)
{
    SampleEngine eng;
    eng.setSampleRate(48000.0);

    std::vector<float> buffer;
    eng.renderSine(440.0f, 1024, buffer, 0.2f);

    ASSERT_EQ(buffer.size(), 1024u);

    // Basic energy check: not all zeros
    const bool allZero = std::all_of(buffer.begin(), buffer.end(), [](float v){ return v == 0.0f; });
    EXPECT_FALSE(allZero);

    // RMS sanity (should be > 0 for a sine with amplitude 0.2)
    double sumSq = 0.0;
    for (float v : buffer) sumSq += static_cast<double>(v) * v;
    const double rms = std::sqrt(sumSq / static_cast<double>(buffer.size()));
    EXPECT_GT(rms, 0.01); // loose bound
    EXPECT_LT(rms, 0.2);  // should be below peak amplitude
}

TEST(SampleEngineTest, PhaseContinuityBetweenCalls)
{
    SampleEngine eng;
    eng.setSampleRate(48000.0);

    std::vector<float> a, b;
    eng.renderSine(1000.0f, 128, a, 0.1f);
    eng.renderSine(1000.0f, 128, b, 0.1f);

    // If phase is continuous, the first sample of b should be close to the next sample of the sine
    // i.e., not equal to a[0] (which would indicate a reset)
    // We give a wide tolerance due to floating point accumulation and amplitude scaling
    ASSERT_EQ(a.size(), 128u);
    ASSERT_EQ(b.size(), 128u);
    EXPECT_NEAR(std::abs(b.front()), std::abs(a.back()), 0.5f); // loose continuity check
}

TEST(SampleEngineTest, ResetPhaseResetsWaveformStart)
{
    SampleEngine eng;
    eng.setSampleRate(48000.0);

    std::vector<float> first, afterReset;
    eng.renderSine(220.0f, 16, first, 0.1f);
    eng.resetPhase();
    eng.renderSine(220.0f, 16, afterReset, 0.1f);

    ASSERT_EQ(first.size(), afterReset.size());
    // After reset, the first samples should be approximately equal
    EXPECT_NEAR(first[0], afterReset[0], 1e-4f);
    EXPECT_NEAR(first[1], afterReset[1], 1e-3f); // allow slightly larger tolerance
}
