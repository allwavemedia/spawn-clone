#include <gtest/gtest.h>
#include <vector>
#include <cmath>
#include <algorithm>
#include "ai/ONNXDaemonClient.h"

using spawnclone::ai::ONNXDaemonClient;

static double rms(const std::vector<float>& v) {
    if (v.empty()) return 0.0;
    long double s = 0.0;
    for (float x : v) s += static_cast<long double>(x) * x;
    return std::sqrt(static_cast<double>(s / static_cast<long double>(v.size())));
}

static void expectInRange(const std::vector<float>& v, float lo, float hi) {
    for (float x : v) {
        EXPECT_GE(x, lo);
        EXPECT_LE(x, hi);
    }
}

TEST(Epic7_AI_Integration_E2E, DeterministicQualityModeWithModelLoaded)
{
    ONNXDaemonClient client;
    ASSERT_TRUE(client.startDaemon());
    ASSERT_TRUE(client.loadModel("models/midi-model.onnx"));

    const std::size_t N = 256;
    auto a = client.generatePattern("quality", N);
    auto b = client.generatePattern("quality", N);

    ASSERT_EQ(a.size(), N);
    ASSERT_EQ(b.size(), N);

    // Deterministic across repeated calls with same inputs
    for (std::size_t i = 0; i < N; ++i)
        EXPECT_FLOAT_EQ(a[i], b[i]);

    // Basic signal sanity checks
    expectInRange(a, -1.0f, 1.0f);
    EXPECT_GT(rms(a), 0.01);

    // Average time should update after at least one call
    EXPECT_GT(client.getAverageInferenceTime(), 0.0);
}

TEST(Epic7_AI_Integration_E2E, SpeedVsQualityAverageTimeOrdering)
{
    // Different clients to avoid moving averages influencing each other
    ONNXDaemonClient qualityClient;
    qualityClient.startDaemon();
    qualityClient.loadModel("models/midi-model.onnx");
    (void) qualityClient.generatePattern("quality", 256);

    ONNXDaemonClient speedClient;
    speedClient.startDaemon();
    speedClient.loadModel("models/midi-model.onnx");
    (void) speedClient.generatePattern("speed", 256);

    // By design, speed mode simulates lower avg time than quality
    EXPECT_GT(qualityClient.getAverageInferenceTime(), speedClient.getAverageInferenceTime());
}

#ifdef ONNX_RUNTIME_AVAILABLE
TEST(Epic7_AI_Integration_E2E, RuntimeFlagMacroIsVisibleWhenONNXIsAvailable)
{
    // This test only verifies build-time flag visibility under the ON condition.
    SUCCEED() << "ONNX_RUNTIME_AVAILABLE is defined; runtime-backed path can be exercised in matrix job.";
}
#endif
