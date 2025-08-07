#include <gtest/gtest.h>
#include <algorithm>
#include <numeric>
#include <cmath>
#include "plugin_core/SynthVoice.h"

using spawnclone::core::SynthVoice;

static double computeRMS(const std::vector<float>& v)
{
    if (v.empty()) return 0.0;
    long double sum = 0.0;
    for (float x : v) sum += static_cast<long double>(x) * x;
    return std::sqrt(static_cast<double>(sum / static_cast<long double>(v.size())));
}

TEST(SynthVoiceTest, DefaultsAndSampleRatePropagation)
{
    SynthVoice v;
    EXPECT_GT(v.getSampleRate(), 0.0);
    v.setSampleRate(44100.0);
    EXPECT_DOUBLE_EQ(v.getSampleRate(), 44100.0);
    EXPECT_FALSE(v.isActive());
}

TEST(SynthVoiceTest, NoteOnActivatesAndProducesAudio)
{
    SynthVoice v;
    v.setSampleRate(48000.0);

    std::vector<float> out;
    v.noteOn(69 /*A4*/, 100);
    EXPECT_TRUE(v.isActive());

    v.render(1024, out);
    ASSERT_EQ(out.size(), 1024u);

    // Should not be all zeros
    const bool allZero = std::all_of(out.begin(), out.end(), [](float x){ return x == 0.0f; });
    EXPECT_FALSE(allZero);

    const double rms = computeRMS(out);
    EXPECT_GT(rms, 0.001);
}

TEST(SynthVoiceTest, NoteOffSilencesAudio)
{
    SynthVoice v;
    v.setSampleRate(48000.0);

    std::vector<float> out;
    v.noteOn(60 /*C4*/, 80);
    v.render(256, out);
    EXPECT_FALSE(std::all_of(out.begin(), out.end(), [](float x){ return x == 0.0f; }));

    v.noteOff();
    EXPECT_FALSE(v.isActive());
    v.render(256, out);
    EXPECT_TRUE(std::all_of(out.begin(), out.end(), [](float x){ return x == 0.0f; }));
}

TEST(SynthVoiceTest, FrequencyRoughlyTracksMidiNote)
{
    SynthVoice v;
    v.setSampleRate(48000.0);

    // Use a higher frequency to reduce cycle length and make counting easier
    const int note = 81; // A5 ~ 880 Hz
    v.noteOn(note, 100);

    std::vector<float> out;
    v.render(2048, out);

    // Count zero-crossings to estimate frequency
    int crossings = 0;
    for (size_t i = 1; i < out.size(); ++i)
    {
        if ((out[i - 1] <= 0.0f && out[i] > 0.0f) ||
            (out[i - 1] >= 0.0f && out[i] < 0.0f))
        {
            ++crossings;
        }
    }

    // For a sine wave, two zero-crossings per cycle (rough heuristic).
    const double estimatedHz = (crossings / 2.0) * (v.getSampleRate() / static_cast<double>(out.size()));
    // Expect within a broad tolerance (sine amplitude/phase can affect counting)
    EXPECT_NEAR(estimatedHz, 880.0, 150.0);
}

TEST(SynthVoiceTest, ResetPhaseResetsWaveStart)
{
    SynthVoice v;
    v.setSampleRate(48000.0);

    std::vector<float> a, b;
    v.noteOn(69, 100);
    v.render(32, a);

    v.resetPhase();
    v.render(32, b);

    ASSERT_EQ(a.size(), b.size());
    // After reset, first samples should be close
    EXPECT_NEAR(a[0], b[0], 1e-4f);
    EXPECT_NEAR(a[1], b[1], 1e-3f);
}
