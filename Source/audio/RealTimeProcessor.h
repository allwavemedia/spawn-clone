/*
  ==============================================================================

    RealTimeProcessor.h
    Created: 3 Aug 2025
    Author:  Epic 8 Story 8.4 Implementation

    Advanced real-time processing with lock-free parameter updates,
    smooth interpolation, and performance optimization for professional
    audio thread constraints.

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_core/juce_core.h>
#include <atomic>
#include <array>

//==============================================================================
/**
 * Epic 8 Story 8.4: Advanced Real-Time Processing
 * 
 * Provides lock-free parameter updates, smooth interpolation, and 
 * performance monitoring for real-time audio constraints.
 */
class RealTimeProcessor
{
public:
    //==============================================================================
    /** Parameter update types for performance monitoring */
    enum class ParameterType
    {
        PitchShift = 0,
        FilterCutoff = 1,
        ReverbMix = 2,
        DelayFeedback = 3,
        ChorusMix = 4,
        DistortionMix = 5,
        Volume = 6,
        Pan = 7,
        NumParameterTypes = 8
    };
    
    /** Lock-free parameter container */
    struct ParameterValue
    {
        std::atomic<float> target{0.0f};
        std::atomic<float> current{0.0f};
        std::atomic<bool> needsUpdate{false};
        float smoothingTime = 0.05f; // 50ms default smoothing
    };
    
    /** Performance metrics */
    struct PerformanceMetrics
    {
        float cpuUsage{0.0f};
        float peakCpuUsage{0.0f};
        int droppedFrames{0};
        int parameterUpdates{0};
        double averageBlockTime{0.0};
        bool realTimeViolation{false};
    };
    
    //==============================================================================
    RealTimeProcessor();
    ~RealTimeProcessor();
    
    //==============================================================================
    /** Prepare for real-time processing */
    void prepareToPlay(double sampleRate, int samplesPerBlock);
    
    /** Release real-time resources */
    void releaseResources();
    
    /** Process smooth parameter updates in real-time audio thread */
    void processParameterSmoothing(int numSamples);
    
    //==============================================================================
    /** Lock-free parameter updates from UI thread */
    void setParameterTarget(ParameterType type, float value, float smoothingTime = -1.0f);
    
    /** Get current smoothed parameter value (audio thread safe) */
    float getCurrentParameterValue(ParameterType type) const;
    
    /** Check if parameter needs updating (audio thread) */
    bool parameterNeedsUpdate(ParameterType type) const;
    
    /** Mark parameter as updated (audio thread) */
    void markParameterUpdated(ParameterType type);
    
    //==============================================================================
    /** Performance monitoring methods */
    void startBlockTiming();
    void endBlockTiming();
    void reportRealTimeViolation();
    void updateCPUUsage(float usage);
    
    /** Get performance metrics */
    PerformanceMetrics getPerformanceMetrics() const;
    void resetPerformanceMetrics();
    
    //==============================================================================
    /** Real-time safety validation */
    bool isRealTimeSafe() const;
    void validateRealTimeConstraints();
    
    /** Memory allocation tracking */
    void reportMemoryAllocation(size_t bytes);
    size_t getTotalAllocations() const { return totalAllocations.load(); }
    
    //==============================================================================
    /** Interpolation utilities */
    static float smoothStep(float edge0, float edge1, float x);
    static float linearInterpolate(float a, float b, float t);
    static float exponentialSmoothing(float current, float target, float alpha);

private:
    //==============================================================================
    /** Parameter storage */
    std::array<ParameterValue, static_cast<int>(ParameterType::NumParameterTypes)> parameters;
    
    /** Smoothing coefficients */
    std::array<float, static_cast<int>(ParameterType::NumParameterTypes)> smoothingCoeffs;
    
    /** Audio processing state */
    double sampleRate = 44100.0;
    int blockSize = 512;
    bool prepared = false;
    
    /** Performance tracking */
    struct AtomicMetrics
    {
        std::atomic<float> cpuUsage{0.0f};
        std::atomic<float> peakCpuUsage{0.0f};
        std::atomic<int> droppedFrames{0};
        std::atomic<int> parameterUpdates{0};
        std::atomic<double> averageBlockTime{0.0};
        std::atomic<bool> realTimeViolation{false};
    };
    
    AtomicMetrics atomicMetrics;
    juce::Time blockStartTime;
    std::atomic<size_t> totalAllocations{0};
    
    /** Real-time violation detection */
    static constexpr double MAX_BLOCK_TIME_MS = 10.0; // 10ms max block time
    static constexpr int MAX_CONSECUTIVE_VIOLATIONS = 3;
    std::atomic<int> consecutiveViolations{0};
    
    //==============================================================================
    /** Helper methods */
    void calculateSmoothingCoefficients();
    void updateSmoothingCoefficient(ParameterType type, float smoothingTime);
    float getDefaultSmoothingTime(ParameterType type) const;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RealTimeProcessor)
};
