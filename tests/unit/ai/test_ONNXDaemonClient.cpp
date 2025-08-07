#include <gtest/gtest.h>
#include <numeric>
#include <cmath>
#include "ai/ONNXDaemonClient.h"

using spawnclone::ai::ONNXDaemonClient;

TEST(ONNXDaemonClientTest, LifecycleAndStatus)
{
    ONNXDaemonClient client;
    EXPECT_FALSE(client.isDaemonRunning());
    EXPECT_DOUBLE_EQ(client.getAverageInferenceTime(), 0.0);

    EXPECT_TRUE(client.startDaemon());
    EXPECT_TRUE(client.isDaemonRunning());

    auto st = client.getDaemonStatus();
    EXPECT_TRUE(st.running);
    EXPECT_EQ(st.requestsInFlight, 0);
    EXPECT_GE(st.averageInferenceMs, 0.0);

    client.shutdownDaemon();
    EXPECT_FALSE(client.isDaemonRunning());
}

TEST(ONNXDaemonClientTest, LoadModelRequiresRunning)
{
    ONNXDaemonClient client;
    EXPECT_FALSE(client.loadModel("models/midi-model.onnx")); // not running yet
    EXPECT_TRUE(client.startDaemon());
    EXPECT_TRUE(client.loadModel("models/midi-model.onnx"));  // stub always ok when running
}

TEST(ONNXDaemonClientTest, GeneratePatternDeterministicAndUpdatesAvg)
{
    ONNXDaemonClient client;
    client.startDaemon();
    client.loadModel("models/midi-model.onnx");

    // Generate two patterns with same inputs => deterministic sequence
    auto a = client.generatePattern("quality", 128);
    auto b = client.generatePattern("quality", 128);
    ASSERT_EQ(a.size(), b.size());
    ASSERT_EQ(a.size(), 128u);

    // Deterministic: exact match
    for (size_t i = 0; i < a.size(); ++i)
        EXPECT_FLOAT_EQ(a[i], b[i]);

    // Average inference should be > 0 after calls
    EXPECT_GT(client.getAverageInferenceTime(), 0.0);

    // speed mode should reduce simulated time relative to quality
    ONNXDaemonClient client2;
    client2.startDaemon();
    client2.loadModel("models/midi-model.onnx");
    (void)client2.generatePattern("speed", 128);
    EXPECT_LT(client2.getAverageInferenceTime(), client.getAverageInferenceTime());
}

TEST(ONNXDaemonClientTest, RequestsInFlightChangesDuringCall)
{
    ONNXDaemonClient client;
    client.startDaemon();

    // We can't hook mid-call without threads; ensure counter settles back to 0
    (void)client.generatePattern("speed", 64);
    auto st = client.getDaemonStatus();
    EXPECT_EQ(st.requestsInFlight, 0);
}
