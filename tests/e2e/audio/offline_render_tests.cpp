#include <gtest/gtest.h>
#include <vector>
#include <numeric>
#include <cmath>

// Offline render smoke test placeholder
// Purpose: Provide a deterministic headless render check that can be enabled
// under BUILD_PLUGIN=ON in CI (PR 6). This test is intentionally dependency-light
// and does not require the full plugin target to link. It validates numeric
// properties on a synthetic buffer to stand in for an offline render until
// real wiring is completed.
//
// Once the plugin render path is available, replace the synthetic generator
// with actual offline render code and keep the structure/assertions.

namespace {
static std::vector<float> synthOneBarSine(float frequencyHz,
                                          double sampleRate,
                                          int samples) {
    std::vector<float> buf(samples, 0.0f);
    const double twoPi = 6.283185307179586476925286766559;
    double phase = 0.0;
    const double inc = twoPi * static_cast<double>(frequencyHz) / sampleRate;
    for (int i = 0; i < samples; ++i) {
        buf[i] = static_cast<float>(std::sin(phase));
        phase += inc;
        if (phase > twoPi) phase -= twoPi;
    }
    return buf;
}

static double rms(const std::vector<float>& v) {
    if (v.empty()) return 0.0;
    long double s = 0.0;
    for (float x : v) s += static_cast<long double>(x) * x;
    return std::sqrt(static_cast<double>(s / static_cast<long double>(v.size())));
}
} // namespace

TEST(offline_render, BasicRMSWithinBounds)
{
    constexpr double sampleRate = 44100.0;
    constexpr int    barLengthSamples = 44100; // ~1 second placeholder
    constexpr float  testFreq = 440.0f;

    auto buf = synthOneBarSine(testFreq, sampleRate, barLengthSamples);

    // Sanity checks
    ASSERT_EQ(static_cast<int>(buf.size()), barLengthSamples);

    // RMS of a full-scale sine is ~0.707; our amplitude is 1.0
    const double r = rms(buf);
    EXPECT_GT(r, 0.60);
    EXPECT_LT(r, 0.80);

    // Determinism: repeated call yields identical results for same params
    auto buf2 = synthOneBarSine(testFreq, sampleRate, barLengthSamples);
    ASSERT_EQ(buf2.size(), buf.size());
    for (size_t i = 0; i < buf.size(); ++i) {
        EXPECT_FLOAT_EQ(buf[i], buf2[i]);
    }
}
