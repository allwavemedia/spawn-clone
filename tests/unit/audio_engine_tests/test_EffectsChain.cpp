#include <gtest/gtest.h>
#include <vector>
#include <algorithm>
#include <numeric>
#include <cmath>
#include "plugin_core/EffectsChain.h"

using spawnclone::core::EffectsChain;

static double rms(const std::vector<float>& x)
{
    if (x.empty()) return 0.0;
    long double s = 0.0;
    for (float v : x) s += static_cast<long double>(v) * v;
    return std::sqrt(static_cast<double>(s / static_cast<long double>(x.size())));
}

TEST(EffectsChainTest, DefaultsAndSampleRate)
{
    EffectsChain fx;
    EXPECT_GT(fx.getSampleRate(), 0.0);
    fx.setSampleRate(44100.0);
    EXPECT_DOUBLE_EQ(fx.getSampleRate(), 44100.0);
    EXPECT_FALSE(fx.isBypassed());
    EXPECT_FLOAT_EQ(fx.getGainDb(), 0.0f);
}

TEST(EffectsChainTest, GainOnlyAppliesApproximately)
{
    EffectsChain fx;
    fx.setSampleRate(48000.0);
    fx.setBypass(false);
    fx.setLowpassCutoff(20000.0f); // effectively transparent
    fx.setGainDb(-6.0f);           // ~0.501x

    std::vector<float> buf(1024, 1.0f);
    fx.process(buf);

    // Expect roughly half amplitude after -6 dB
    // Allow a bit of tolerance
    for (auto v : buf)
        EXPECT_NEAR(v, 0.501f, 0.02f);
}

TEST(EffectsChainTest, LowpassAttenuatesHighFrequencies)
{
    EffectsChain fx;
    fx.setSampleRate(48000.0);
    fx.setBypass(false);
    fx.setGainDb(0.0f);
    fx.setLowpassCutoff(1000.0f);

    // Generate a 5 kHz sine (well above cutoff)
    const size_t N = 4096;
    std::vector<float> in(N), out;
    in.resize(N);
    for (size_t n = 0; n < N; ++n)
        in[n] = std::sin(2.0 * M_PI * 5000.0 * (static_cast<double>(n) / 48000.0));

    out = in;
    fx.process(out);

    // Expect RMS significantly reduced after lowpass
    const double rIn = rms(in);
    const double rOut = rms(out);

    EXPECT_GT(rIn, 0.1);
    EXPECT_LT(rOut, rIn * 0.7); // arbitrary but should be clearly lower
}

TEST(EffectsChainTest, BypassSkipsProcessing)
{
    EffectsChain fx;
    fx.setSampleRate(48000.0);
    fx.setBypass(true);
    fx.setGainDb(-12.0f);
    fx.setLowpassCutoff(500.0f);

    std::vector<float> buf(256);
    for (size_t i = 0; i < buf.size(); ++i)
        buf[i] = static_cast<float>(i) / 256.0f;

    std::vector<float> original = buf;
    fx.process(buf);

    // With bypass active, output should be identical
    ASSERT_EQ(buf.size(), original.size());
    for (size_t i = 0; i < buf.size(); ++i)
        EXPECT_FLOAT_EQ(buf[i], original[i]);
}

TEST(EffectsChainTest, ResetClearsFilterState)
{
    EffectsChain fx;
    fx.setSampleRate(48000.0);
    fx.setBypass(false);
    fx.setGainDb(0.0f);
    fx.setLowpassCutoff(800.0f);

    // Feed a step to build up state
    std::vector<float> step(512, 1.0f);
    fx.process(step);

    // Now reset and process zeros; should be near-zero without prior tail
    fx.reset();
    std::vector<float> zeros(128, 0.0f);
    fx.process(zeros);

    for (auto v : zeros)
        EXPECT_NEAR(v, 0.0f, 1e-5f);
}
