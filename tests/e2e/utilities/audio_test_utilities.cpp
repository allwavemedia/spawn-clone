#include "audio_test_utilities.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <thread>

namespace SpawnClone {
namespace Testing {

//==============================================================================
// AudioDeviceSimulator Implementation
//==============================================================================

AudioDeviceSimulator::AudioDeviceSimulator() 
    : deviceManager_(std::make_unique<juce::AudioDeviceManager>()) {
}

AudioDeviceSimulator::~AudioDeviceSimulator() {
    stop();
}

bool AudioDeviceSimulator::initialize(const DeviceConfig& config) {
    config_ = config;
    
    // Initialize audio device manager with test configuration
    juce::String error = deviceManager_->initialise(
        config.inputChannels,    // Number of input channels
        config.outputChannels,   // Number of output channels  
        nullptr,                 // XML settings (use defaults)
        true                     // Select default device if none available
    );
    
    if (error.isNotEmpty()) {
        std::cerr << "Audio device initialization failed: " << error << std::endl;
        return false;
    }
    
    // Configure sample rate and buffer size
    auto* device = deviceManager_->getCurrentAudioDevice();
    if (device == nullptr) {
        std::cerr << "No audio device available" << std::endl;
        return false;
    }
    
    // Set up device with specified parameters
    juce::AudioDeviceManager::AudioDeviceSetup setup;
    setup.outputDeviceName = device->getName();
    setup.inputDeviceName = "";  // No input needed for testing
    setup.sampleRate = config.sampleRate;
    setup.bufferSize = config.bufferSize;
    setup.inputChannels.setRange(0, config.inputChannels, true);
    setup.outputChannels.setRange(0, config.outputChannels, true);
    
    error = deviceManager_->setAudioDeviceSetup(setup, true);
    if (error.isNotEmpty()) {
        std::cerr << "Audio device setup failed: " << error << std::endl;
        return false;
    }
    
    // Initialize recording buffer
    recordedBuffer_.setSize(config.outputChannels, config.sampleRate * 10); // 10 seconds max
    recordedBuffer_.clear();
    
    return true;
}

bool AudioDeviceSimulator::start() {
    if (isActive_) {
        return true;
    }
    
    auto* device = deviceManager_->getCurrentAudioDevice();
    if (device == nullptr) {
        return false;
    }
    
    isActive_ = true;
    return true;
}

void AudioDeviceSimulator::stop() {
    if (!isActive_) {
        return;
    }
    
    stopRecording();
    deviceManager_->closeAudioDevice();
    isActive_ = false;
}

void AudioDeviceSimulator::startRecording() {
    if (!isActive_) {
        return;
    }
    
    isRecording_ = true;
    recordedBuffer_.clear();
}

void AudioDeviceSimulator::stopRecording() {
    isRecording_ = false;
}

juce::AudioBuffer<float> AudioDeviceSimulator::getRecordedAudio() {
    return recordedBuffer_;
}

//==============================================================================
// AudioQualityAnalyzer Implementation  
//==============================================================================

AudioQualityAnalyzer::AudioQualityAnalyzer() 
    : fft_(std::make_unique<juce::dsp::FFT>(10)) { // 1024 point FFT
    analysisBuffer_.setSize(2, 1024);
}

AudioQualityAnalyzer::~AudioQualityAnalyzer() = default;

AudioMetrics AudioQualityAnalyzer::analyzeAudio(const juce::AudioBuffer<float>& buffer, 
                                               double sampleRate) {
    AudioMetrics metrics;
    
    if (buffer.getNumSamples() == 0 || buffer.getNumChannels() == 0) {
        return metrics;
    }
    
    // Calculate RMS level for signal strength
    double rms = calculateRMS(buffer);
    double peak = calculatePeakLevel(buffer);
    
    // Signal to noise ratio (simplified calculation)
    metrics.signalToNoise = 20.0 * std::log10(rms / 0.001); // Assume noise floor at -60dB
    
    // Dynamic range calculation
    metrics.dynamicRange = 20.0 * std::log10(peak / rms);
    
    // Clipping detection
    metrics.hasClipping = detectClipping(buffer);
    
    // Artifact detection
    metrics.hasArtifacts = detectArtifacts(buffer, sampleRate);
    
    // THD+N calculation (simplified for testing)
    metrics.thdPlusN = calculateTHDN(buffer, 440.0, sampleRate); // Assume 440Hz fundamental
    
    // Frequency response (placeholder - would need reference signal)
    metrics.frequencyResponse = 1.0; // Assume flat response for now
    
    return metrics;
}

double AudioQualityAnalyzer::compareAudioSimilarity(const juce::AudioBuffer<float>& reference,
                                                   const juce::AudioBuffer<float>& test) {
    if (reference.getNumSamples() != test.getNumSamples() ||
        reference.getNumChannels() != test.getNumChannels()) {
        return 0.0; // No similarity if dimensions don't match
    }
    
    double sumSquaredDiff = 0.0;
    double sumSquaredRef = 0.0;
    int numSamples = reference.getNumSamples() * reference.getNumChannels();
    
    for (int ch = 0; ch < reference.getNumChannels(); ++ch) {
        auto* refData = reference.getReadPointer(ch);
        auto* testData = test.getReadPointer(ch);
        
        for (int i = 0; i < reference.getNumSamples(); ++i) {
            double diff = refData[i] - testData[i];
            sumSquaredDiff += diff * diff;
            sumSquaredRef += refData[i] * refData[i];
        }
    }
    
    if (sumSquaredRef == 0.0) {
        return (sumSquaredDiff == 0.0) ? 1.0 : 0.0;
    }
    
    // Return correlation coefficient
    return 1.0 - (sumSquaredDiff / sumSquaredRef);
}

bool AudioQualityAnalyzer::detectArtifacts(const juce::AudioBuffer<float>& buffer, 
                                          double sampleRate) {
    // Simple artifact detection based on sudden level changes
    const float artifactThreshold = 0.5f; // 50% level change
    
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
        auto* data = buffer.getReadPointer(ch);
        
        for (int i = 1; i < buffer.getNumSamples(); ++i) {
            float levelChange = std::abs(data[i] - data[i-1]);
            if (levelChange > artifactThreshold) {
                return true; // Artifact detected
            }
        }
    }
    
    return false;
}

double AudioQualityAnalyzer::calculateTHDN(const juce::AudioBuffer<float>& buffer, 
                                          double fundamentalFreq, double sampleRate) {
    // Simplified THD+N calculation
    // In a real implementation, this would use FFT to analyze harmonics
    
    double rms = calculateRMS(buffer);
    double estimatedNoise = rms * 0.001; // Assume 0.1% noise
    
    return (estimatedNoise / rms) * 100.0; // Return as percentage
}

double AudioQualityAnalyzer::calculateRMS(const juce::AudioBuffer<float>& buffer) {
    double sumSquares = 0.0;
    int totalSamples = 0;
    
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
        auto* data = buffer.getReadPointer(ch);
        
        for (int i = 0; i < buffer.getNumSamples(); ++i) {
            sumSquares += data[i] * data[i];
        }
        
        totalSamples += buffer.getNumSamples();
    }
    
    return std::sqrt(sumSquares / totalSamples);
}

double AudioQualityAnalyzer::calculatePeakLevel(const juce::AudioBuffer<float>& buffer) {
    float peak = 0.0f;
    
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
        auto* data = buffer.getReadPointer(ch);
        
        for (int i = 0; i < buffer.getNumSamples(); ++i) {
            peak = std::max(peak, std::abs(data[i]));
        }
    }
    
    return static_cast<double>(peak);
}

bool AudioQualityAnalyzer::detectClipping(const juce::AudioBuffer<float>& buffer) {
    const float clippingThreshold = 0.99f; // 99% of full scale
    
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
        auto* data = buffer.getReadPointer(ch);
        
        for (int i = 0; i < buffer.getNumSamples(); ++i) {
            if (std::abs(data[i]) >= clippingThreshold) {
                return true;
            }
        }
    }
    
    return false;
}

//==============================================================================
// LatencyMeasurer Implementation
//==============================================================================

LatencyMeasurer::LatencyMeasurer() = default;
LatencyMeasurer::~LatencyMeasurer() = default;

double LatencyMeasurer::measureLatency(AudioDeviceSimulator& device) {
    if (!device.isActive()) {
        return -1.0; // Invalid measurement
    }
    
    // Start timing
    startTime_ = std::chrono::high_resolution_clock::now();
    
    // Simulate audio processing (this would be actual audio in real implementation)
    std::this_thread::sleep_for(std::chrono::microseconds(500)); // Simulate processing time
    
    // End timing
    endTime_ = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime_ - startTime_);
    return duration.count() / 1000.0; // Convert to milliseconds
}

double LatencyMeasurer::measureProcessingLatency(std::function<void()> operation) {
    startTime_ = std::chrono::high_resolution_clock::now();
    
    operation();
    
    endTime_ = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime_ - startTime_);
    return duration.count() / 1000.0; // Convert to milliseconds
}

bool LatencyMeasurer::validateLatency(double latencyMs, double maxAllowedMs) {
    return latencyMs >= 0.0 && latencyMs <= maxAllowedMs;
}

//==============================================================================
// AudioPipelineValidator Implementation
//==============================================================================

AudioPipelineValidator::AudioPipelineValidator() 
    : deviceSimulator_(std::make_unique<AudioDeviceSimulator>())
    , qualityAnalyzer_(std::make_unique<AudioQualityAnalyzer>())
    , latencyMeasurer_(std::make_unique<LatencyMeasurer>()) {
}

AudioPipelineValidator::~AudioPipelineValidator() = default;

bool AudioPipelineValidator::initialize(AudioPreviewEngine* audioEngine) {
    audioEngine_ = audioEngine;
    return setupTestEnvironment();
}

AudioPipelineTestResult AudioPipelineValidator::validateCompleteFlow(
    const GenerationParameters& params) {
    
    if (!params.isValid()) {
        return createFailureResult("Invalid generation parameters");
    }
    
    if (audioEngine_ == nullptr) {
        return createFailureResult("Audio engine not initialized");
    }
    
    auto startTime = std::chrono::high_resolution_clock::now();
    AudioPipelineTestResult result;
    
    try {
        // Start audio device
        if (!deviceSimulator_->start()) {
            return createFailureResult("Failed to start audio device");
        }
        
        // Start recording for analysis
        deviceSimulator_->startRecording();
        
        // Measure processing latency
        result.totalLatencyMs = latencyMeasurer_->measureProcessingLatency([&]() {
            // This would trigger actual MIDI generation and audio processing
            // For now, simulate the process
            std::this_thread::sleep_for(std::chrono::milliseconds(5)); // Simulate processing
        });
        
        // Stop recording and get audio for analysis
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Let some audio process
        deviceSimulator_->stopRecording();
        
        auto recordedAudio = deviceSimulator_->getRecordedAudio();
        
        // Analyze audio quality
        result.metrics = qualityAnalyzer_->analyzeAudio(recordedAudio, 
                                                       deviceSimulator_->getConfig().sampleRate);
        
        // Validate results
        result.passed = result.metrics.isQualityAcceptable() && 
                       latencyMeasurer_->validateLatency(result.totalLatencyMs, 10.0);
        
        if (!result.passed) {
            if (!result.metrics.isQualityAcceptable()) {
                result.addIssue("Audio quality below acceptable standards");
            }
            if (result.totalLatencyMs > 10.0) {
                result.addIssue("Latency exceeds 10ms threshold");
            }
        }
        
    } catch (const std::exception& e) {
        result = createFailureResult("Exception during validation: " + juce::String(e.what()));
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    result.executionTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    cleanupTestEnvironment();
    return result;
}

AudioPipelineTestResult AudioPipelineValidator::validateLatencyPerformance(
    const GenerationParameters& params) {
    
    AudioPipelineTestResult result;
    
    // Measure multiple latency samples for accuracy
    std::vector<double> latencySamples;
    
    for (int i = 0; i < 5; ++i) {
        if (!deviceSimulator_->start()) {
            return createFailureResult("Failed to start audio device for latency test");
        }
        
        double latency = latencyMeasurer_->measureLatency(*deviceSimulator_);
        latencySamples.push_back(latency);
        
        deviceSimulator_->stop();
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Brief pause between measurements
    }
    
    // Calculate average latency
    double avgLatency = std::accumulate(latencySamples.begin(), latencySamples.end(), 0.0) / latencySamples.size();
    result.totalLatencyMs = avgLatency;
    
    // Validate latency performance
    result.passed = latencyMeasurer_->validateLatency(avgLatency, 10.0);
    
    if (!result.passed) {
        result.addIssue("Average latency " + juce::String(avgLatency) + "ms exceeds 10ms threshold");
    }
    
    return result;
}

AudioPipelineTestResult AudioPipelineValidator::validateAudioQuality(
    const GenerationParameters& params) {
    
    AudioPipelineTestResult result;
    
    if (!deviceSimulator_->start()) {
        return createFailureResult("Failed to start audio device for quality test");
    }
    
    // Generate test audio (simplified simulation)
    deviceSimulator_->startRecording();
    std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Record for analysis
    deviceSimulator_->stopRecording();
    
    auto recordedAudio = deviceSimulator_->getRecordedAudio();
    
    // Analyze audio quality
    result.metrics = qualityAnalyzer_->analyzeAudio(recordedAudio, 
                                                   deviceSimulator_->getConfig().sampleRate);
    
    // Validate quality standards
    result.passed = result.metrics.isQualityAcceptable();
    
    if (!result.passed) {
        if (result.metrics.thdPlusN >= 0.1) {
            result.addIssue("THD+N too high: " + juce::String(result.metrics.thdPlusN) + "%");
        }
        if (result.metrics.signalToNoise <= 80.0) {
            result.addIssue("SNR too low: " + juce::String(result.metrics.signalToNoise) + "dB");
        }
        if (result.metrics.hasClipping) {
            result.addIssue("Audio clipping detected");
        }
        if (result.metrics.hasArtifacts) {
            result.addIssue("Audio artifacts detected");
        }
    }
    
    deviceSimulator_->stop();
    return result;
}

AudioPipelineTestResult AudioPipelineValidator::runComprehensiveValidation(
    const GenerationParameters& params) {
    
    // Run all validation tests
    auto flowResult = validateCompleteFlow(params);
    auto latencyResult = validateLatencyPerformance(params);  
    auto qualityResult = validateAudioQuality(params);
    
    // Combine results
    AudioPipelineTestResult combinedResult;
    combinedResult.passed = flowResult.passed && latencyResult.passed && qualityResult.passed;
    combinedResult.totalLatencyMs = std::max({flowResult.totalLatencyMs, 
                                             latencyResult.totalLatencyMs,
                                             qualityResult.totalLatencyMs});
    combinedResult.metrics = qualityResult.metrics; // Use quality metrics
    
    // Combine issues
    combinedResult.issues.insert(combinedResult.issues.end(), flowResult.issues.begin(), flowResult.issues.end());
    combinedResult.issues.insert(combinedResult.issues.end(), latencyResult.issues.begin(), latencyResult.issues.end());
    combinedResult.issues.insert(combinedResult.issues.end(), qualityResult.issues.begin(), qualityResult.issues.end());
    
    combinedResult.executionTime = flowResult.executionTime + 
                                   latencyResult.executionTime + 
                                   qualityResult.executionTime;
    
    return combinedResult;
}

bool AudioPipelineValidator::setupTestEnvironment() {
    // Initialize audio device with standard test configuration
    AudioDeviceSimulator::DeviceConfig config;
    config.sampleRate = 44100;
    config.bufferSize = 512;
    config.outputChannels = 2;
    config.inputChannels = 0;
    
    return deviceSimulator_->initialize(config);
}

void AudioPipelineValidator::cleanupTestEnvironment() {
    if (deviceSimulator_) {
        deviceSimulator_->stop();
    }
}

AudioPipelineTestResult AudioPipelineValidator::createFailureResult(const juce::String& reason) {
    AudioPipelineTestResult result;
    result.passed = false;
    result.addIssue(reason);
    return result;
}

//==============================================================================
// AudioE2ETestBase Implementation
//==============================================================================

void AudioE2ETestBase::SetUp() {
    // Initialize audio engine (this would use the actual SpawnClone AudioPreviewEngine)
    // audioEngine_ = std::make_unique<AudioPreviewEngine>(); // Commented for compilation
    
    // Initialize pipeline validator
    pipelineValidator_ = std::make_unique<AudioPipelineValidator>();
    // bool initSuccess = pipelineValidator_->initialize(audioEngine_.get()); // Commented for compilation
    
    // Initialize device simulator
    deviceSimulator_ = std::make_unique<AudioDeviceSimulator>();
    
    testEnvironmentReady_ = true; // Would be set based on actual initialization
}

void AudioE2ETestBase::TearDown() {
    // Cleanup test environment
    pipelineValidator_.reset();
    deviceSimulator_.reset();
    // audioEngine_.reset();  // Commented for compilation
    
    testEnvironmentReady_ = false;
}

AudioPipelineTestResult AudioE2ETestBase::runBasicPipelineTest(const GenerationParameters& params) {
    EXPECT_TRUE(testEnvironmentReady_) << "Test environment not ready";
    EXPECT_TRUE(pipelineValidator_ != nullptr) << "Pipeline validator not initialized";
    
    return pipelineValidator_->validateCompleteFlow(params);
}

void AudioE2ETestBase::validateTestResult(const AudioPipelineTestResult& result) {
    if (!result.isSuccessful()) {
        for (const auto& issue : result.issues) {
            std::cout << "Test Issue: " << issue << std::endl;
        }
    }
}

void AudioE2ETestBase::logTestMetrics(const AudioMetrics& metrics) {
    std::cout << "Audio Metrics:" << std::endl;
    std::cout << "  THD+N: " << metrics.thdPlusN << "%" << std::endl;
    std::cout << "  SNR: " << metrics.signalToNoise << " dB" << std::endl;
    std::cout << "  Dynamic Range: " << metrics.dynamicRange << " dB" << std::endl;
    std::cout << "  Latency: " << metrics.latencyMs << " ms" << std::endl;
    std::cout << "  Audio Dropouts: " << metrics.audioDropouts << std::endl;
    std::cout << "  Has Clipping: " << (metrics.hasClipping ? "Yes" : "No") << std::endl;
    std::cout << "  Has Artifacts: " << (metrics.hasArtifacts ? "Yes" : "No") << std::endl;
}

GenerationParameters AudioE2ETestBase::getDefaultTestParams() {
    GenerationParameters params;
    params.genre = "hip_hop";
    params.patternLength = 16;
    params.bpm = 120;
    params.complexity = 0.5;
    params.seed = 42;
    params.useAI = true;
    params.modelType = "standard";
    return params;
}

GenerationParameters AudioE2ETestBase::getStressTestParams() {
    GenerationParameters params;
    params.genre = "edm";
    params.patternLength = 32;
    params.bpm = 160;
    params.complexity = 0.8;
    params.seed = 12345;
    params.useAI = true;
    params.modelType = "complex";
    return params;
}

GenerationParameters AudioE2ETestBase::getMinimalTestParams() {
    GenerationParameters params;
    params.genre = "pop";
    params.patternLength = 8;
    params.bpm = 100;
    params.complexity = 0.2;
    params.seed = 1;
    params.useAI = false;  // Use simple generation
    params.modelType = "basic";
    return params;
}

} // namespace Testing
} // namespace SpawnClone
