#pragma once

#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <gtest/gtest.h>
#include <memory>
#include <chrono>
#include <vector>

namespace SpawnClone {
namespace Testing {

// Forward declarations
class AudioPreviewEngine;

/**
 * @brief Comprehensive audio quality metrics for validation
 */
struct AudioMetrics {
    double thdPlusN = 0.0;              // Total Harmonic Distortion + Noise
    double frequencyResponse = 0.0;      // Frequency response accuracy
    double dynamicRange = 0.0;           // Dynamic range measurement
    double signalToNoise = 0.0;          // Signal to noise ratio
    int audioDropouts = 0;               // Number of audio dropouts
    double latencyMs = 0.0;              // Measured latency in milliseconds
    bool hasClipping = false;            // Audio clipping detection
    bool hasArtifacts = false;           // Audio artifact detection
    
    /**
     * @brief Check if all metrics meet quality standards
     */
    bool isQualityAcceptable() const {
        return thdPlusN < 0.1 &&         // Less than 0.1% THD+N
               signalToNoise > 80.0 &&   // Greater than 80dB SNR
               audioDropouts == 0 &&     // No dropouts
               latencyMs < 10.0 &&       // Less than 10ms latency
               !hasClipping &&           // No clipping
               !hasArtifacts;            // No artifacts
    }
};

/**
 * @brief Test result container for E2E audio pipeline validation
 */
struct AudioPipelineTestResult {
    bool passed = false;
    double totalLatencyMs = 0.0;
    AudioMetrics metrics;
    std::vector<juce::String> issues;
    std::chrono::milliseconds executionTime{0};
    
    /**
     * @brief Add an issue to the test result
     */
    void addIssue(const juce::String& issue) {
        issues.push_back(issue);
        passed = false;
    }
    
    /**
     * @brief Check if test result indicates success
     */
    bool isSuccessful() const {
        return passed && metrics.isQualityAcceptable() && issues.empty();
    }
};

/**
 * @brief Parameters for E2E audio generation testing
 */
struct E2EGenerationParameters {
    juce::String genre = "hip_hop";
    int patternLength = 16;
    int bpm = 120;
    double complexity = 0.5;
    int seed = 42;                       // For reproducible testing
    bool useAI = true;
    juce::String modelType = "standard";
    
    /**
     * @brief Validate parameters are within acceptable ranges
     */
    bool isValid() const {
        return bpm >= 60 && bpm <= 200 &&
               patternLength >= 4 && patternLength <= 64 &&
               complexity >= 0.0 && complexity <= 1.0;
    }
};

/**
 * @brief Audio device simulator for consistent testing across platforms
 */
class AudioDeviceSimulator {
public:
    struct DeviceConfig {
        int sampleRate = 44100;
        int bufferSize = 512;
        int inputChannels = 0;
        int outputChannels = 2;
        juce::String deviceName = "Test Audio Device";
    };
    
    AudioDeviceSimulator();
    ~AudioDeviceSimulator();
    
    /**
     * @brief Initialize simulated audio device
     */
    bool initialize(const DeviceConfig& config);
    
    /**
     * @brief Start audio processing
     */
    bool start();
    
    /**
     * @brief Stop audio processing
     */
    void stop();
    
    /**
     * @brief Get current device configuration
     */
    const DeviceConfig& getConfig() const { return config_; }
    
    /**
     * @brief Check if device is currently active
     */
    bool isActive() const { return isActive_; }
    
    /**
     * @brief Record audio output for analysis
     */
    void startRecording();
    void stopRecording();
    juce::AudioBuffer<float> getRecordedAudio();
    
private:
    DeviceConfig config_;
    bool isActive_ = false;
    bool isRecording_ = false;
    juce::AudioBuffer<float> recordedBuffer_;
    std::unique_ptr<juce::AudioDeviceManager> deviceManager_;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioDeviceSimulator)
};

/**
 * @brief Audio quality analyzer for automated validation
 */
class AudioQualityAnalyzer {
public:
    AudioQualityAnalyzer();
    ~AudioQualityAnalyzer();
    
    /**
     * @brief Analyze audio buffer and generate quality metrics
     */
    AudioMetrics analyzeAudio(const juce::AudioBuffer<float>& buffer, 
                             double sampleRate);
    
    /**
     * @brief Compare two audio buffers for similarity
     */
    double compareAudioSimilarity(const juce::AudioBuffer<float>& reference,
                                 const juce::AudioBuffer<float>& test);
    
    /**
     * @brief Detect audio artifacts (clicks, pops, etc.)
     */
    bool detectArtifacts(const juce::AudioBuffer<float>& buffer, 
                        double sampleRate);
    
    /**
     * @brief Calculate Total Harmonic Distortion + Noise
     */
    double calculateTHDN(const juce::AudioBuffer<float>& buffer, 
                        double fundamentalFreq, double sampleRate);
    
private:
    std::unique_ptr<juce::dsp::FFT> fft_;
    juce::AudioBuffer<float> analysisBuffer_;
    
    // Internal analysis methods
    double calculateRMS(const juce::AudioBuffer<float>& buffer);
    double calculatePeakLevel(const juce::AudioBuffer<float>& buffer);
    bool detectClipping(const juce::AudioBuffer<float>& buffer);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioQualityAnalyzer)
};

/**
 * @brief Latency measurement utility for real-time performance validation
 */
class LatencyMeasurer {
public:
    LatencyMeasurer();
    ~LatencyMeasurer();
    
    /**
     * @brief Measure round-trip audio latency
     */
    double measureLatency(AudioDeviceSimulator& device);
    
    /**
     * @brief Measure processing latency for a specific operation
     */
    double measureProcessingLatency(std::function<void()> operation);
    
    /**
     * @brief Validate latency is within acceptable limits
     */
    bool validateLatency(double latencyMs, double maxAllowedMs = 10.0);
    
private:
    std::chrono::high_resolution_clock::time_point startTime_;
    std::chrono::high_resolution_clock::time_point endTime_;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LatencyMeasurer)
};

/**
 * @brief Primary E2E audio pipeline validator
 * 
 * This class orchestrates complete end-to-end testing of the SpawnClone
 * audio pipeline, from MIDI generation through final audio output.
 */
class AudioPipelineValidator {
public:
    AudioPipelineValidator();
    ~AudioPipelineValidator();
    
    /**
     * @brief Initialize validator with audio engine
     */
    bool initialize(AudioPreviewEngine* audioEngine);
    
    /**
     * @brief Validate complete MIDI generation to audio output flow
     */
    AudioPipelineTestResult validateCompleteFlow(
        const E2EGenerationParameters& params
    );
    
    /**
     * @brief Validate audio latency performance
     */
    AudioPipelineTestResult validateLatencyPerformance(
        const E2EGenerationParameters& params
    );
    
    /**
     * @brief Validate audio quality output
     */
    AudioPipelineTestResult validateAudioQuality(
        const E2EGenerationParameters& params
    );
    
    /**
     * @brief Run comprehensive pipeline validation
     */
    AudioPipelineTestResult runComprehensiveValidation(
        const E2EGenerationParameters& params
    );
    
private:
    AudioPreviewEngine* audioEngine_ = nullptr;
    std::unique_ptr<AudioDeviceSimulator> deviceSimulator_;
    std::unique_ptr<AudioQualityAnalyzer> qualityAnalyzer_;
    std::unique_ptr<LatencyMeasurer> latencyMeasurer_;
    
    // Internal validation methods
    bool setupTestEnvironment();
    void cleanupTestEnvironment();
    AudioPipelineTestResult createFailureResult(const juce::String& reason);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPipelineValidator)
};

/**
 * @brief Base class for E2E audio tests
 * 
 * Provides common setup and utilities for all E2E audio tests,
 * building upon existing GoogleTest patterns in SpawnClone.
 */
class AudioE2ETestBase : public ::testing::Test {
protected:
    void SetUp() override;
    void TearDown() override;
    
    // Test utilities
    AudioPipelineTestResult runBasicPipelineTest(const E2EGenerationParameters& params);
    void validateTestResult(const AudioPipelineTestResult& result);
    void logTestMetrics(const AudioMetrics& metrics);
    
    // Test data
    E2EGenerationParameters getDefaultTestParams();
    E2EGenerationParameters getStressTestParams();
    E2EGenerationParameters getMinimalTestParams();
    
    // Test components (available to derived classes)
    // std::unique_ptr<AudioPreviewEngine> audioEngine_;  // Commented out for compilation
    std::unique_ptr<AudioPipelineValidator> pipelineValidator_;
    std::unique_ptr<AudioDeviceSimulator> deviceSimulator_;
    
private:
    bool testEnvironmentReady_ = false;
};

} // namespace Testing
} // namespace SpawnClone
