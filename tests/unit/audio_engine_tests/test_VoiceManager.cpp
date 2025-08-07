#include <gtest/gtest.h>
#include <vector>
#include <algorithm>
#include "plugin_core/VoiceManager.h"

using spawnclone::core::VoiceManager;

static double rms(const std::vector<float>& x)
{
    if (x.empty()) return 0.0;
    long double s = 0.0;
    for (float v : x) s += static_cast<long double>(v) * v;
    return std::sqrt(static_cast<double>(s / static_cast<long double>(x.size())));
}

TEST(VoiceManagerTest, CapacityAndSampleRatePropagation)
{
    VoiceManager mgr(4);
    EXPECT_EQ(mgr.capacity(), 4u);
    EXPECT_GT(mgr.getSampleRate(), 0.0);
    mgr.setSampleRate(44100.0);
    EXPECT_DOUBLE_EQ(mgr.getSampleRate(), 44100.0);
}

TEST(VoiceManagerTest, AllocateUpToCapacity)
{
    VoiceManager mgr(2);
    mgr.setSampleRate(48000.0);

    EXPECT_EQ(mgr.activeCount(), 0u);
    auto v0 = mgr.noteOn(60, 100);
    auto v1 = mgr.noteOn(64, 100);
    EXPECT_EQ(mgr.activeCount(), 2u);
    EXPECT_NE(v0, v1);
}

TEST(VoiceManagerTest, StealsOldestWhenFull)
{
    VoiceManager mgr(2);
    mgr.setSampleRate(48000.0);

    auto v0 = mgr.noteOn(60, 100);
    auto v1 = mgr.noteOn(64, 100);
    // Third note should steal the oldest (v0)
    auto victim = mgr.noteOn(67, 100);
    EXPECT_EQ(victim, v0);
    EXPECT_EQ(mgr.activeCount(), 2u);
}

TEST(VoiceManagerTest, MixProducesSignalWhenActive)
{
    VoiceManager mgr(2);
    mgr.setSampleRate(48000.0);

    mgr.noteOn(69, 100); // A4
    std::vector<float> mix;
    mgr.renderMix(1024, mix);

    ASSERT_EQ(mix.size(), 1024u);
    const double e = rms(mix);
    EXPECT_GT(e, 0.001);
}

TEST(VoiceManagerTest, NoteOffReducesActiveCount)
{
    VoiceManager mgr(2);
    mgr.setSampleRate(48000.0);

    mgr.noteOn(60, 90);
    mgr.noteOn(64, 90);
    EXPECT_EQ(mgr.activeCount(), 2u);

    mgr.noteOff(60);
    EXPECT_EQ(mgr.activeCount(), 1u);
}
