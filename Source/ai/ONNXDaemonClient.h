//==============================================================================
// ONNXDaemonClient.h
// Production-ready client for persistent ONNX Python daemon
// Integrated with JUCE framework for SpawnClone
//==============================================================================

#pragma once

#include <juce_core/juce_core.h>
#include <memory>

/**
 * Production-ready client for persistent ONNX Python daemon.
 * Provides thread-safe, high-performance model inference with session caching.
 */
class ONNXDaemonClient
{
public:
    ONNXDaemonClient();
    ~ONNXDaemonClient();
    
    // Lifecycle management
    bool startDaemon();
    void shutdownDaemon();
    bool isDaemonRunning() const;
    
    // Model operations
    bool loadModel(const juce::String& modelName);
    bool isModelLoaded(const juce::String& modelName) const;
    juce::String getModelStatus(const juce::String& modelName) const;
    
    // Pattern generation
    struct GenerationResult
    {
        bool success = false;
        juce::String errorMessage;
        juce::Array<juce::Array<float>> patternData;
        juce::Array<int> outputShape;
        double inferenceTimeMs = 0.0;
        
        GenerationResult() = default;
    };
    
    GenerationResult generatePattern(const juce::String& modelName, 
                                   const juce::var& parameters = juce::var());
    
    // Status and monitoring
    struct DaemonStatus
    {
        bool running = false;
        juce::StringArray loadedModels;
        double uptimeSeconds = 0.0;
        int totalRequests = 0;
        juce::String lastError;
    };
    
    DaemonStatus getDaemonStatus() const;
    
    // Performance monitoring
    double getAverageInferenceTime() const;
    int getTotalRequests() const;
    
private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
    
    // Thread safety
    juce::CriticalSection commandLock;
    
    // Internal methods
    juce::String sendCommand(const juce::String& jsonCommand) const;
    bool parseDaemonResponse(const juce::String& response, juce::var& result) const;
    juce::String createJsonCommand(const juce::String& action, const juce::var& parameters = juce::var()) const;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ONNXDaemonClient)
};
