#include <gtest/gtest.h>
#include <juce_core/juce_core.h>

/**
 * @brief Simple E2E test to verify the testing framework is working
 */
TEST(AudioPipelineE2ETest, BasicFrameworkTest) {
    // Simple test to verify everything compiles and runs
    EXPECT_TRUE(true) << "Basic framework test should always pass";
    
    // Test basic JUCE functionality
    juce::String testString("Hello SpawnClone E2E Testing");
    EXPECT_FALSE(testString.isEmpty()) << "JUCE String should not be empty";
    EXPECT_EQ(testString, "Hello SpawnClone E2E Testing") << "String comparison should work";
}

/**
 * @brief Test that JUCE audio components can be instantiated
 */
TEST(AudioPipelineE2ETest, JUCEAudioComponentsTest) {
    // Test basic JUCE audio buffer functionality
    juce::AudioBuffer<float> testBuffer(2, 1024);
    testBuffer.clear();
    
    EXPECT_EQ(testBuffer.getNumChannels(), 2) << "Buffer should have 2 channels";
    EXPECT_EQ(testBuffer.getNumSamples(), 1024) << "Buffer should have 1024 samples";
    
    // Test that buffer is actually cleared
    bool isCleared = true;
    for (int ch = 0; ch < testBuffer.getNumChannels(); ++ch) {
        auto* data = testBuffer.getReadPointer(ch);
        for (int i = 0; i < testBuffer.getNumSamples(); ++i) {
            if (data[i] != 0.0f) {
                isCleared = false;
                break;
            }
        }
        if (!isCleared) break;
    }
    
    EXPECT_TRUE(isCleared) << "Buffer should be cleared (all zeros)";
}

/**
 * @brief Test performance measurement capabilities
 */
TEST(AudioPipelineE2ETest, PerformanceMeasurementTest) {
    auto start = std::chrono::high_resolution_clock::now();
    
    // Simulate some processing
    volatile int sum = 0;
    for (int i = 0; i < 10000; ++i) {
        sum += i;
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    EXPECT_GT(duration.count(), 0) << "Performance measurement should show elapsed time";
    EXPECT_LT(duration.count(), 100000) << "Simple operation should complete quickly";
    
    std::cout << "Performance test completed in " << duration.count() << " microseconds" << std::endl;
}

/**
 * @brief Test that we can measure latency and timing
 */
TEST(AudioPipelineE2ETest, LatencyMeasurementTest) {
    const int numMeasurements = 5;
    std::vector<double> latencies;
    
    for (int i = 0; i < numMeasurements; ++i) {
        auto start = std::chrono::high_resolution_clock::now();
        
        // Simulate audio processing delay
        std::this_thread::sleep_for(std::chrono::microseconds(500));
        
        auto end = std::chrono::high_resolution_clock::now();
        auto latency = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        latencies.push_back(latency.count() / 1000.0); // Convert to milliseconds
    }
    
    // Calculate average latency
    double avgLatency = 0.0;
    for (double latency : latencies) {
        avgLatency += latency;
    }
    avgLatency /= latencies.size();
    
    EXPECT_GT(avgLatency, 0.4) << "Measured latency should be close to simulated delay";
    EXPECT_LT(avgLatency, 1.0) << "Measured latency should not be too high";
    
    std::cout << "Average latency: " << avgLatency << " ms" << std::endl;
}

/**
 * @brief Test basic audio quality metrics calculation
 */
TEST(AudioPipelineE2ETest, AudioQualityMetricsTest) {
    // Create a test signal (sine wave)
    const int sampleRate = 44100;
    const int numSamples = 1024;
    const float frequency = 440.0f; // A4
    
    juce::AudioBuffer<float> testSignal(1, numSamples);
    
    // Generate sine wave
    auto* data = testSignal.getWritePointer(0);
    for (int i = 0; i < numSamples; ++i) {
        float phase = 2.0f * juce::MathConstants<float>::pi * frequency * i / sampleRate;
        data[i] = 0.5f * std::sin(phase);
    }
    
    // Calculate RMS level
    float rms = 0.0f;
    for (int i = 0; i < numSamples; ++i) {
        rms += data[i] * data[i];
    }
    rms = std::sqrt(rms / numSamples);
    
    // Calculate peak level
    float peak = 0.0f;
    for (int i = 0; i < numSamples; ++i) {
        peak = std::max(peak, std::abs(data[i]));
    }
    
    EXPECT_GT(rms, 0.3f) << "RMS level should be reasonable for 0.5 amplitude sine wave";
    EXPECT_LT(rms, 0.4f) << "RMS level should not exceed expected value";
    EXPECT_NEAR(peak, 0.5f, 0.01f) << "Peak level should match sine wave amplitude";
    
    std::cout << "RMS: " << rms << ", Peak: " << peak << std::endl;
}
