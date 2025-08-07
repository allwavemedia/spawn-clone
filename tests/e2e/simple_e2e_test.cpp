#include <gtest/gtest.h>
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>

/**
 * @brief Simple E2E test for basic build validation
 */
class SimpleE2ETest : public ::testing::Test {
protected:
    void SetUp() override {
        // Basic setup
    }
    
    void TearDown() override {
        // Basic cleanup
    }
};

/**
 * @brief Test basic framework functionality
 */
TEST_F(SimpleE2ETest, BasicFrameworkTest) {
    // Test that JUCE core is working
    juce::String testString("Hello E2E Testing");
    EXPECT_EQ(testString.length(), 17);
    
    // Test basic assertions
    EXPECT_TRUE(true);
    EXPECT_FALSE(false);
    EXPECT_EQ(2 + 2, 4);
}

/**
 * @brief Test audio buffer creation
 */
TEST_F(SimpleE2ETest, AudioBufferTest) {
    juce::AudioBuffer<float> buffer(2, 1024);
    buffer.clear();
    
    EXPECT_EQ(buffer.getNumChannels(), 2);
    EXPECT_EQ(buffer.getNumSamples(), 1024);
    
    // Test that buffer is cleared
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            EXPECT_EQ(buffer.getSample(ch, sample), 0.0f);
        }
    }
}

/**
 * @brief Test E2E framework readiness
 */
TEST_F(SimpleE2ETest, FrameworkReadiness) {
    // Test that we can create basic JUCE objects
    juce::AudioDeviceManager deviceManager;
    
    // Test string manipulation
    juce::String genre = "hip_hop";
    EXPECT_TRUE(genre.contains("hip"));
    EXPECT_TRUE(genre.endsWith("hop"));
    
    // Test numeric operations
    double latency = 5.5;
    EXPECT_LT(latency, 10.0);
    EXPECT_GT(latency, 0.0);
}
