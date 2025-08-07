#include <gtest/gtest.h>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include "plugin_core/SampleEngine.h"

using spawnclone::core::SampleEngine;

static double rms(const std::vector<float>& x)
{
    if (x.empty()) return 0.0;
    long double s = 0.0;
    for (float v : x) s += static_cast<long double>(v) * v;
    return std::sqrt(static_cast<double>(s / static_cast<long double>(x.size())));
}

TEST(SampleEngineFilePlaybackTest, LoadFromTextFileAndRender)
{
    // Create a small mono fixture file in the working directory
    const std::string path = "sample_fixture_test.txt";
    {
        std::ofstream out(path);
        ASSERT_TRUE(out.good());
        // Simple repeating pattern
        out << "0.0 0.5 -0.5 1.0 -1.0 0.25 -0.25 0.75 -0.75";
    }

    SampleEngine eng;
    eng.setSampleRate(48000.0);
    ASSERT_TRUE(eng.loadFromTextFile(path));

    // Render more samples than the file contains to test cyclic behavior
    std::vector<float> out;
    eng.renderFromFile(100, out, 0.8f);

    ASSERT_EQ(out.size(), 100u);
    // Basic energy check
    EXPECT_GT(rms(out), 0.01);

    // Verify first few samples follow the scaled pattern
    const std::vector<float> pattern = {0.0f, 0.5f, -0.5f, 1.0f, -1.0f, 0.25f, -0.25f, 0.75f, -0.75f};
    for (size_t i = 0; i < pattern.size(); ++i)
        EXPECT_NEAR(out[i], 0.8f * pattern[i], 1e-5f);
}
