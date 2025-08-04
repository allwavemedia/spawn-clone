/*
  ==============================================================================

    RealTimeProcessor.cpp
    Created: 3 Aug 2025
    Author:  Epic 8 Story 8.4 Implementation

    Implementation of advanced real-time processing with lock-free updates,
    smooth interpolation, and performance optimization.

  ==============================================================================
*/

#include "RealTimeProcessor.h"

//==============================================================================
RealTimeProcessor::RealTimeProcessor()
{
    // Initialize all parameters to safe defaults
    for (int i = 0; i < static_cast<int>(ParameterType::NumParameterTypes); ++i)
    {
        auto& param = parameters[i];
        param.target.store(0.0f);
        param.current.store(0.0f);
        param.needsUpdate.store(false);
        param.smoothingTime = getDefaultSmoothingTime(static_cast<ParameterType>(i));
    }
    
    resetPerformanceMetrics();
}

RealTimeProcessor::~RealTimeProcessor()
{
    // Ensure clean shutdown
    releaseResources();
}

//==============================================================================
void RealTimeProcessor::prepareToPlay(double newSampleRate, int samplesPerBlock)
{
    sampleRate = newSampleRate;
    blockSize = samplesPerBlock;
    
    // Calculate smoothing coefficients for all parameters
    calculateSmoothingCoefficients();
    
    // Reset performance metrics
    resetPerformanceMetrics();
    
    prepared = true;
    
    juce::Logger::writeToLog("RealTimeProcessor: Prepared for " + 
                           juce::String(sampleRate) + "Hz, " + 
                           juce::String(blockSize) + " samples");
}

void RealTimeProcessor::releaseResources()
{
    prepared = false;
    
    // Log final performance metrics
    auto finalMetrics = getPerformanceMetrics();
    juce::Logger::writeToLog("RealTimeProcessor: Final metrics - CPU: " + 
                           juce::String(finalMetrics.peakCpuUsage, 2) + 
                           "%, Violations: " + 
                           juce::String(finalMetrics.droppedFrames));
}

//==============================================================================
void RealTimeProcessor::processParameterSmoothing(int numSamples)
{
    if (!prepared)
        return;
        
    startBlockTiming();
    
    // Process all parameter smoothing in a lock-free manner
    for (int i = 0; i < static_cast<int>(ParameterType::NumParameterTypes); ++i)
    {
        auto& param = parameters[i];
        
        if (param.needsUpdate.load())
        {
            float target = param.target.load();
            float current = param.current.load();
            float coeff = smoothingCoeffs[i];
            
            // Exponential smoothing for smooth parameter transitions
            float newValue = exponentialSmoothing(current, target, coeff);
            param.current.store(newValue);
            
            // Check if we've reached the target (within 0.1% tolerance)
            if (std::abs(newValue - target) < 0.001f)
            {
                param.current.store(target);
                param.needsUpdate.store(false);
            }
            
            atomicMetrics.parameterUpdates.fetch_add(1);
        }
    }
    
    endBlockTiming();
}

//==============================================================================
void RealTimeProcessor::setParameterTarget(ParameterType type, float value, float smoothingTime)
{
    if (type >= ParameterType::NumParameterTypes)
        return;
        
    int index = static_cast<int>(type);
    auto& param = parameters[index];
    
    // Clamp value to safe range
    value = juce::jlimit(0.0f, 1.0f, value);
    
    // Update smoothing time if provided
    if (smoothingTime > 0.0f)
    {
        param.smoothingTime = smoothingTime;
        updateSmoothingCoefficient(type, smoothingTime);
    }
    
    // Lock-free atomic updates
    param.target.store(value);
    param.needsUpdate.store(true);
}

float RealTimeProcessor::getCurrentParameterValue(ParameterType type) const
{
    if (type >= ParameterType::NumParameterTypes)
        return 0.0f;
        
    int index = static_cast<int>(type);
    return parameters[index].current.load();
}

bool RealTimeProcessor::parameterNeedsUpdate(ParameterType type) const
{
    if (type >= ParameterType::NumParameterTypes)
        return false;
        
    int index = static_cast<int>(type);
    return parameters[index].needsUpdate.load();
}

void RealTimeProcessor::markParameterUpdated(ParameterType type)
{
    if (type >= ParameterType::NumParameterTypes)
        return;
        
    int index = static_cast<int>(type);
    parameters[index].needsUpdate.store(false);
}

//==============================================================================
void RealTimeProcessor::startBlockTiming()
{
    blockStartTime = juce::Time::getCurrentTime();
}

void RealTimeProcessor::endBlockTiming()
{
    auto blockEndTime = juce::Time::getCurrentTime();
    double blockTimeMs = (blockEndTime - blockStartTime).inMilliseconds();
    
    // Update average block time (simple moving average)
    double currentAverage = atomicMetrics.averageBlockTime.load();
    double newAverage = (currentAverage * 0.95) + (blockTimeMs * 0.05);
    atomicMetrics.averageBlockTime.store(newAverage);
    
    // Check for real-time violations
    if (blockTimeMs > MAX_BLOCK_TIME_MS)
    {
        reportRealTimeViolation();
        juce::Logger::writeToLog("RealTimeProcessor: Block time violation: " + 
                               juce::String(blockTimeMs, 2) + "ms");
    }
    else
    {
        consecutiveViolations.store(0);
    }
}

void RealTimeProcessor::reportRealTimeViolation()
{
    atomicMetrics.droppedFrames.fetch_add(1);
    int violations = consecutiveViolations.fetch_add(1);
    
    if (violations >= MAX_CONSECUTIVE_VIOLATIONS)
    {
        atomicMetrics.realTimeViolation.store(true);
    }
}

void RealTimeProcessor::updateCPUUsage(float usage)
{
    atomicMetrics.cpuUsage.store(usage);
    
    float currentPeak = atomicMetrics.peakCpuUsage.load();
    if (usage > currentPeak)
    {
        atomicMetrics.peakCpuUsage.store(usage);
    }
}

//==============================================================================
RealTimeProcessor::PerformanceMetrics RealTimeProcessor::getPerformanceMetrics() const
{
    PerformanceMetrics result;
    result.cpuUsage = atomicMetrics.cpuUsage.load();
    result.peakCpuUsage = atomicMetrics.peakCpuUsage.load();
    result.droppedFrames = atomicMetrics.droppedFrames.load();
    result.parameterUpdates = atomicMetrics.parameterUpdates.load();
    result.averageBlockTime = atomicMetrics.averageBlockTime.load();
    result.realTimeViolation = atomicMetrics.realTimeViolation.load();
    return result;
}

void RealTimeProcessor::resetPerformanceMetrics()
{
    atomicMetrics.cpuUsage.store(0.0f);
    atomicMetrics.peakCpuUsage.store(0.0f);
    atomicMetrics.droppedFrames.store(0);
    atomicMetrics.parameterUpdates.store(0);
    atomicMetrics.averageBlockTime.store(0.0);
    atomicMetrics.realTimeViolation.store(false);
    consecutiveViolations.store(0);
    totalAllocations.store(0);
}

//==============================================================================
bool RealTimeProcessor::isRealTimeSafe() const
{
    return !atomicMetrics.realTimeViolation.load() && 
           atomicMetrics.averageBlockTime.load() < MAX_BLOCK_TIME_MS;
}

void RealTimeProcessor::validateRealTimeConstraints()
{
    if (!isRealTimeSafe())
    {
        juce::Logger::writeToLog("RealTimeProcessor: Real-time constraints violated!");
        juce::Logger::writeToLog("Average block time: " + 
                               juce::String(atomicMetrics.averageBlockTime.load(), 2) + "ms");
        juce::Logger::writeToLog("Dropped frames: " + 
                               juce::String(atomicMetrics.droppedFrames.load()));
    }
}

void RealTimeProcessor::reportMemoryAllocation(size_t bytes)
{
    totalAllocations.fetch_add(bytes);
    
    // Log unexpected allocations in audio thread
    if (prepared)
    {
        juce::Logger::writeToLog("RealTimeProcessor: Memory allocation in audio thread: " + 
                               juce::String(bytes) + " bytes");
    }
}

//==============================================================================
float RealTimeProcessor::smoothStep(float edge0, float edge1, float x)
{
    x = juce::jlimit(0.0f, 1.0f, (x - edge0) / (edge1 - edge0));
    return x * x * (3.0f - 2.0f * x);
}

float RealTimeProcessor::linearInterpolate(float a, float b, float t)
{
    return a + t * (b - a);
}

float RealTimeProcessor::exponentialSmoothing(float current, float target, float alpha)
{
    return current + alpha * (target - current);
}

//==============================================================================
void RealTimeProcessor::calculateSmoothingCoefficients()
{
    for (int i = 0; i < static_cast<int>(ParameterType::NumParameterTypes); ++i)
    {
        auto type = static_cast<ParameterType>(i);
        float smoothingTime = parameters[i].smoothingTime;
        updateSmoothingCoefficient(type, smoothingTime);
    }
}

void RealTimeProcessor::updateSmoothingCoefficient(ParameterType type, float smoothingTime)
{
    int index = static_cast<int>(type);
    
    // Calculate exponential smoothing coefficient
    // alpha = 1 - exp(-dt / tau) where tau is the time constant
    double samplesPerSecond = sampleRate / static_cast<double>(blockSize);
    double dt = 1.0 / samplesPerSecond;
    double tau = smoothingTime; // time constant in seconds
    
    float alpha = static_cast<float>(1.0 - std::exp(-dt / tau));
    smoothingCoeffs[index] = juce::jlimit(0.001f, 1.0f, alpha);
}

float RealTimeProcessor::getDefaultSmoothingTime(ParameterType type) const
{
    switch (type)
    {
        case ParameterType::PitchShift:      return 0.1f;  // 100ms for pitch changes
        case ParameterType::FilterCutoff:    return 0.05f; // 50ms for filter sweeps
        case ParameterType::ReverbMix:       return 0.2f;  // 200ms for reverb
        case ParameterType::DelayFeedback:   return 0.1f;  // 100ms for delay
        case ParameterType::ChorusMix:       return 0.15f; // 150ms for chorus
        case ParameterType::DistortionMix:   return 0.05f; // 50ms for distortion
        case ParameterType::Volume:          return 0.02f; // 20ms for volume
        case ParameterType::Pan:             return 0.03f; // 30ms for panning
        default:                             return 0.05f; // 50ms default
    }
}
