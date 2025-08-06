//==============================================================================
// PerformanceProfiler.h
// Advanced Performance Monitoring and Optimization for ONNX Operations
// Created for Epic 7 Performance Optimization Task
//==============================================================================

#pragma once

#include <juce_core/juce_core.h>
#include <memory>
#include <vector>
#include <chrono>
#include <atomic>
#include <mutex>

class PerformanceProfiler
{
public:
    struct PerformanceMetrics
    {
        // Timing metrics
        double averageInferenceTime = 0.0;
        double minInferenceTime = std::numeric_limits<double>::max();
        double maxInferenceTime = 0.0;
        double totalProcessingTime = 0.0;
        
        // Resource metrics
        size_t peakMemoryUsage = 0;
        size_t currentMemoryUsage = 0;
        double cpuUsagePercent = 0.0;
        
        // Throughput metrics
        int totalInferences = 0;
        double inferencesPerSecond = 0.0;
        
        // Quality metrics
        int successfulInferences = 0;
        int failedInferences = 0;
        double successRate = 0.0;
        
        // System health
        bool meetsLatencyTarget = false;
        bool meetsMemoryTarget = false;
        bool meetsCpuTarget = false;
        
        PerformanceMetrics() = default;
    };
    
    struct InferenceSession
    {
        std::chrono::high_resolution_clock::time_point startTime;
        std::chrono::high_resolution_clock::time_point endTime;
        size_t memoryAtStart = 0;
        size_t memoryAtEnd = 0;
        bool successful = false;
        juce::String operationType;
        
        double getDuration() const {
            return std::chrono::duration<double, std::milli>(endTime - startTime).count();
        }
    };
    
    // Singleton access
    static PerformanceProfiler& getInstance();
    
    // Session management
    juce::String startInferenceSession(const juce::String& operationType = "inference");
    void endInferenceSession(const juce::String& sessionId, bool successful = true);
    
    // Direct timing methods
    void recordInferenceTime(double timeMs, bool successful = true);
    void recordMemoryUsage(size_t bytes);
    void recordCpuUsage(double percentage);
    
    // Metrics access
    PerformanceMetrics getCurrentMetrics() const;
    PerformanceMetrics getHistoricalMetrics(int periodMinutes = 60) const;
    
    // Performance targets (Epic 7 requirements)
    static constexpr double TARGET_INFERENCE_TIME_MS = 2000.0; // 2 second max
    static constexpr double TARGET_MEMORY_MB = 512.0; // 512MB max
    static constexpr double TARGET_CPU_PERCENT = 80.0; // 80% max CPU
    static constexpr double TARGET_SUCCESS_RATE = 0.95; // 95% success rate
    
    // Optimization recommendations
    struct OptimizationRecommendation
    {
        juce::String category;
        juce::String issue;
        juce::String recommendation;
        int priority; // 1=critical, 2=important, 3=minor
    };
    
    std::vector<OptimizationRecommendation> getOptimizationRecommendations() const;
    
    // Configuration
    void setReportingInterval(int intervalSeconds);
    void enableDetailedLogging(bool enable);
    void setMaxHistoryEntries(int maxEntries);
    
    // Reset and cleanup
    void reset();
    void clearHistory();
    
private:
    PerformanceProfiler();
    ~PerformanceProfiler() = default;
    PerformanceProfiler(const PerformanceProfiler&) = delete;
    PerformanceProfiler& operator=(const PerformanceProfiler&) = delete;
    
    // Internal data structures
    mutable std::mutex metricsMutex;
    std::vector<InferenceSession> sessionHistory;
    std::map<juce::String, InferenceSession> activeSessions;
    
    // Running metrics
    std::atomic<int> totalInferences{0};
    std::atomic<int> successfulInferences{0};
    std::atomic<int> failedInferences{0};
    std::atomic<double> totalInferenceTime{0.0};
    std::atomic<size_t> currentMemoryUsage{0};
    std::atomic<double> currentCpuUsage{0.0};
    
    // Configuration
    int reportingIntervalSeconds = 60;
    bool detailedLoggingEnabled = false;
    int maxHistoryEntries = 1000;
    
    // Helper methods
    void updateRunningAverages();
    size_t getCurrentMemoryUsage();
    double getCurrentCpuUsage();
    void pruneOldHistory();
    juce::String generateSessionId();
    
    // Metrics calculation helpers
    double calculateInferencesPerSecond() const;
    double calculateSuccessRate() const;
    bool checkLatencyTarget() const;
    bool checkMemoryTarget() const;
    bool checkCpuTarget() const;
};

// RAII helper for automatic session management
class AutoInferenceProfiler
{
public:
    explicit AutoInferenceProfiler(const juce::String& operationType = "inference");
    ~AutoInferenceProfiler();
    
    void markSuccessful(bool successful = true);
    
private:
    juce::String sessionId;
    bool wasSuccessful = false;
};

// Convenience macros for easy profiling
#define PROFILE_INFERENCE() AutoInferenceProfiler _prof("inference")
#define PROFILE_OPERATION(name) AutoInferenceProfiler _prof(name)
#define PROFILE_SUCCESS() _prof.markSuccessful(true)
#define PROFILE_FAILURE() _prof.markSuccessful(false)
