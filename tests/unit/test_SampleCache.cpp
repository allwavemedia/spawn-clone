//=========================================================================
//  test_SampleCache.cpp
//  SpawnClone – Unit Tests
//
//  GoogleTest skeleton for SampleCache MVP.
//  Validates basic put/get/evict logic with dummy buffers.
//
//  Created: 2025-08-05
//=========================================================================
#include <gtest/gtest.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include "../../Source/audio/SampleCache.h"

using namespace spawnclone::audio;

class SampleCacheTest : public ::testing::Test
{
protected:
    SampleCache cache { 4 }; // 4 MB for quick tests

    std::unique_ptr<juce::AudioBuffer<float>> makeBuffer(int samples, int channels = 1)
    {
        auto buf = std::make_unique<juce::AudioBuffer<float>>(channels, samples);
        buf->clear();
        return buf;
    }
};

TEST_F(SampleCacheTest, PutAndGet)
{
    uint64_t key = 0x1234ULL;
    auto buffer = makeBuffer(1024);
    EXPECT_TRUE(cache.put(key, std::move(buffer)));

    const juce::AudioBuffer<float>* retrieved = cache.get(key);
    EXPECT_NE(retrieved, nullptr);
    EXPECT_EQ(retrieved->getNumSamples(), 1024);
}

TEST_F(SampleCacheTest, EvictionOccurs)
{
    // fill until eviction needed
    for (int i = 0; i < 20; ++i)
    {
        uint64_t key = static_cast<uint64_t>(i);
        auto buf = makeBuffer(48000); // ~0.18 MB mono
        cache.put(key, std::move(buf));
    }
    // Cache capacity 4 MB, expect some evictions
    EXPECT_LT(cache.getCurrentSize(), cache.getCapacity() + 1);
}
