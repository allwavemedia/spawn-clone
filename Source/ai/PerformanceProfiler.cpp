//==============================================================================
// PerformanceProfiler.cpp
// Advanced Performance Monitoring and Optimization for ONNX Operations
// Created for Epic 7 Performance Optimization Task
//==============================================================================

#include "PerformanceProfiler.h"
#include <random>

//==============================================================================
// Singleton Implementation

PerformanceProfiler& PerformanceProfiler::getInstance()
{
    static PerformanceProfiler instance;
    return instance;
}

PerformanceProfiler::PerformanceProfiler()
{
    // Initialize with current system state
    currentMemoryUsage = getCurrentMemoryUsage();
    currentCpuUsage = getCurrentCpuUsage();
}

//==============================================================================
// Session Management

juce::String PerformanceProfiler::startInferenceSession(const juce::String& operationType)
{
    std::lock_guard<std::mutex> lock(metricsMutex);
    
    juce::String sessionId = generateSessionId();
    InferenceSession session;
    session.startTime = std::chrono::high_resolution_clock::now();
    session.memoryAtStart = getCurrentMemoryUsage();
    session.operationType = operationType;
    
    activeSessions[sessionId] = session;
    
    if (detailedLoggingEnabled)
    {
        DBG("Started performance session: " + sessionId + " for " + operationType);
    }
    
    return sessionId;
}

void PerformanceProfiler::endInferenceSession(const juce::String& sessionId, bool successful)
{
    std::lock_guard<std::mutex> lock(metricsMutex);
    
    auto it = activeSessions.find(sessionId);
    if (it == activeSessions.end())
    {
        DBG("Warning: Attempted to end non-existent session: " + sessionId);
        return;
    }
    
    InferenceSession& session = it->second;
    session.endTime = std::chrono::high_resolution_clock::now();
    session.memoryAtEnd = getCurrentMemoryUsage();
    session.successful = successful;
    
    // Record metrics
    double durationMs = session.getDuration();
    totalInferences++;
    totalInferenceTime.store(totalInferenceTime.load() + durationMs);
    
    if (successful)
        successfulInferences++;
    else
        failedInferences++;
    
    // Update memory usage
    currentMemoryUsage = session.memoryAtEnd;
    
    // Add to history
    sessionHistory.push_back(session);
    pruneOldHistory();
    
    // Remove from active sessions
    activeSessions.erase(it);
    
    if (detailedLoggingEnabled)
    {
        DBG("Completed session " + sessionId + ": " + 
            juce::String(durationMs, 2) + "ms, " + 
            (successful ? "SUCCESS" : "FAILED"));
    }
    
    // Update running averages
    updateRunningAverages();
}

//==============================================================================
// Direct Recording Methods

void PerformanceProfiler::recordInferenceTime(double timeMs, bool successful)
{
    totalInferences++;
    totalInferenceTime.store(totalInferenceTime.load() + timeMs);
    
    if (successful)
        successfulInferences++;
    else
        failedInferences++;
    
    updateRunningAverages();
}

void PerformanceProfiler::recordMemoryUsage(size_t bytes)
{
    currentMemoryUsage = bytes;
}

void PerformanceProfiler::recordCpuUsage(double percentage)
{
    currentCpuUsage = percentage;
}

//==============================================================================
// Metrics Access

PerformanceProfiler::PerformanceMetrics PerformanceProfiler::getCurrentMetrics() const
{
    std::lock_guard<std::mutex> lock(metricsMutex);
    
    PerformanceMetrics metrics;
    
    // Basic counts
    metrics.totalInferences = totalInferences.load();
    metrics.successfulInferences = successfulInferences.load();
    metrics.failedInferences = failedInferences.load();
    
    // Calculate timing metrics
    if (metrics.totalInferences > 0)
    {
        metrics.averageInferenceTime = totalInferenceTime.load() / metrics.totalInferences;
        
        // Calculate min/max from recent history
        if (!sessionHistory.empty())
        {
            for (const auto& session : sessionHistory)
            {
                double duration = session.getDuration();
                metrics.minInferenceTime = std::min(metrics.minInferenceTime, duration);
                metrics.maxInferenceTime = std::max(metrics.maxInferenceTime, duration);
            }
        }
    }
    
    // Resource metrics
    metrics.currentMemoryUsage = currentMemoryUsage.load();
    metrics.cpuUsagePercent = currentCpuUsage.load();
    
    // Throughput metrics
    metrics.inferencesPerSecond = calculateInferencesPerSecond();
    metrics.successRate = calculateSuccessRate();
    
    // Target compliance
    metrics.meetsLatencyTarget = checkLatencyTarget();
    metrics.meetsMemoryTarget = checkMemoryTarget();
    metrics.meetsCpuTarget = checkCpuTarget();
    
    return metrics;
}

PerformanceProfiler::PerformanceMetrics PerformanceProfiler::getHistoricalMetrics(int periodMinutes) const
{
    std::lock_guard<std::mutex> lock(metricsMutex);
    
    auto cutoffTime = std::chrono::high_resolution_clock::now() - 
                      std::chrono::minutes(periodMinutes);
    
    PerformanceMetrics metrics;
    double totalTime = 0.0;
    int validSessions = 0;
    
    for (const auto& session : sessionHistory)
    {
        if (session.endTime >= cutoffTime)
        {
            double duration = session.getDuration();
            totalTime += duration;
            validSessions++;
            
            if (session.successful)
                metrics.successfulInferences++;
            else
                metrics.failedInferences++;
            
            metrics.minInferenceTime = std::min(metrics.minInferenceTime, duration);
            metrics.maxInferenceTime = std::max(metrics.maxInferenceTime, duration);
            metrics.peakMemoryUsage = std::max(metrics.peakMemoryUsage, session.memoryAtEnd);
        }
    }
    
    metrics.totalInferences = validSessions;
    if (validSessions > 0)
    {
        metrics.averageInferenceTime = totalTime / validSessions;
        metrics.successRate = static_cast<double>(metrics.successfulInferences) / validSessions;
    }
    
    // Current resource state
    metrics.currentMemoryUsage = currentMemoryUsage.load();
    metrics.cpuUsagePercent = currentCpuUsage.load();
    
    return metrics;
}

//==============================================================================
// Optimization Recommendations

std::vector<PerformanceProfiler::OptimizationRecommendation> 
PerformanceProfiler::getOptimizationRecommendations() const
{
    std::vector<OptimizationRecommendation> recommendations;
    auto metrics = getCurrentMetrics();
    
    // Latency issues
    if (!metrics.meetsLatencyTarget)
    {
        OptimizationRecommendation rec;
        rec.category = "Latency";
        rec.issue = "Average inference time (" + juce::String(metrics.averageInferenceTime, 1) + 
                   "ms) exceeds target (" + juce::String(TARGET_INFERENCE_TIME_MS, 0) + "ms)";
        rec.recommendation = "Consider model quantization, batch processing optimization, or GPU acceleration";
        rec.priority = 1; // Critical
        recommendations.push_back(rec);
    }
    
    // Memory issues
    if (!metrics.meetsMemoryTarget)
    {
        OptimizationRecommendation rec;
        rec.category = "Memory";
        rec.issue = "Memory usage (" + juce::String(metrics.currentMemoryUsage / (1024*1024)) + 
                   "MB) exceeds target (" + juce::String(TARGET_MEMORY_MB, 0) + "MB)";
        rec.recommendation = "Implement model caching strategies, reduce batch sizes, or use memory-mapped models";
        rec.priority = 2; // Important
        recommendations.push_back(rec);
    }
    
    // CPU usage issues
    if (!metrics.meetsCpuTarget)
    {
        OptimizationRecommendation rec;
        rec.category = "CPU";
        rec.issue = "CPU usage (" + juce::String(metrics.cpuUsagePercent, 1) + 
                   "%) exceeds target (" + juce::String(TARGET_CPU_PERCENT, 0) + "%)";
        rec.recommendation = "Optimize thread usage, implement async processing, or reduce computational complexity";
        rec.priority = 2; // Important
        recommendations.push_back(rec);
    }
    
    // Success rate issues
    if (metrics.successRate < TARGET_SUCCESS_RATE)
    {
        OptimizationRecommendation rec;
        rec.category = "Reliability";
        rec.issue = "Success rate (" + juce::String(metrics.successRate * 100, 1) + 
                   "%) below target (" + juce::String(TARGET_SUCCESS_RATE * 100, 0) + "%)";
        rec.recommendation = "Implement better error handling, input validation, and fallback mechanisms";
        rec.priority = 1; // Critical
        recommendations.push_back(rec);
    }
    
    // Throughput optimization opportunities
    if (metrics.inferencesPerSecond < 5.0 && metrics.totalInferences > 100)
    {
        OptimizationRecommendation rec;
        rec.category = "Throughput";
        rec.issue = "Low throughput (" + juce::String(metrics.inferencesPerSecond, 2) + " ops/sec)";
        rec.recommendation = "Implement request batching, parallel processing, or model preloading";
        rec.priority = 3; // Minor
        recommendations.push_back(rec);
    }
    
    return recommendations;
}

//==============================================================================
// Configuration

void PerformanceProfiler::setReportingInterval(int intervalSeconds)
{
    reportingIntervalSeconds = intervalSeconds;
}

void PerformanceProfiler::enableDetailedLogging(bool enable)
{
    detailedLoggingEnabled = enable;
}

void PerformanceProfiler::setMaxHistoryEntries(int maxEntries)
{
    maxHistoryEntries = maxEntries;
    pruneOldHistory();
}

//==============================================================================
// Reset and Cleanup

void PerformanceProfiler::reset()
{
    std::lock_guard<std::mutex> lock(metricsMutex);
    
    totalInferences = 0;
    successfulInferences = 0;
    failedInferences = 0;
    totalInferenceTime = 0.0;
    
    clearHistory();
}

void PerformanceProfiler::clearHistory()
{
    std::lock_guard<std::mutex> lock(metricsMutex);
    sessionHistory.clear();
    activeSessions.clear();
}

//==============================================================================
// Private Helper Methods

void PerformanceProfiler::updateRunningAverages()
{
    // Update CPU usage periodically
    currentCpuUsage = getCurrentCpuUsage();
}

size_t PerformanceProfiler::getCurrentMemoryUsage()
{
    // Platform-specific memory usage detection
    // For now, simulate with a reasonable value
#if JUCE_MAC || JUCE_IOS
    // On macOS/iOS, could use task_info or proc_taskallinfo
    return 128 * 1024 * 1024; // 128MB simulation
#elif JUCE_WINDOWS
    // On Windows, could use GetProcessMemoryInfo
    return 128 * 1024 * 1024; // 128MB simulation
#else
    // On Linux, could parse /proc/self/status
    return 128 * 1024 * 1024; // 128MB simulation
#endif
}

double PerformanceProfiler::getCurrentCpuUsage()
{
    // Platform-specific CPU usage detection
    // For now, simulate with reasonable values
    static std::mt19937 gen(std::chrono::steady_clock::now().time_since_epoch().count());
    std::uniform_real_distribution<double> dist(15.0, 45.0);
    return dist(gen); // Simulate 15-45% CPU usage
}

void PerformanceProfiler::pruneOldHistory()
{
    if (static_cast<int>(sessionHistory.size()) > maxHistoryEntries)
    {
        int excess = static_cast<int>(sessionHistory.size()) - maxHistoryEntries;
        sessionHistory.erase(sessionHistory.begin(), sessionHistory.begin() + excess);
    }
}

juce::String PerformanceProfiler::generateSessionId()
{
    static std::atomic<int> counter{0};
    return "session_" + juce::String(counter++) + "_" + 
           juce::String(juce::Time::getCurrentTime().toMilliseconds());
}

double PerformanceProfiler::calculateInferencesPerSecond() const
{
    if (sessionHistory.empty()) return 0.0;
    
    // Calculate over last minute of data
    auto cutoffTime = std::chrono::high_resolution_clock::now() - std::chrono::minutes(1);
    int recentInferences = 0;
    
    for (const auto& session : sessionHistory)
    {
        if (session.endTime >= cutoffTime)
            recentInferences++;
    }
    
    return static_cast<double>(recentInferences) / 60.0; // Per second
}

double PerformanceProfiler::calculateSuccessRate() const
{
    int total = totalInferences.load();
    if (total == 0) return 1.0;
    
    return static_cast<double>(successfulInferences.load()) / total;
}

bool PerformanceProfiler::checkLatencyTarget() const
{
    int total = totalInferences.load();
    if (total == 0) return true;
    
    double avgTime = totalInferenceTime.load() / total;
    return avgTime <= TARGET_INFERENCE_TIME_MS;
}

bool PerformanceProfiler::checkMemoryTarget() const
{
    double memoryMB = static_cast<double>(currentMemoryUsage.load()) / (1024 * 1024);
    return memoryMB <= TARGET_MEMORY_MB;
}

bool PerformanceProfiler::checkCpuTarget() const
{
    return currentCpuUsage.load() <= TARGET_CPU_PERCENT;
}

//==============================================================================
// AutoInferenceProfiler Implementation

AutoInferenceProfiler::AutoInferenceProfiler(const juce::String& operationType)
{
    sessionId = PerformanceProfiler::getInstance().startInferenceSession(operationType);
}

AutoInferenceProfiler::~AutoInferenceProfiler()
{
    PerformanceProfiler::getInstance().endInferenceSession(sessionId, wasSuccessful);
}

void AutoInferenceProfiler::markSuccessful(bool successful)
{
    wasSuccessful = successful;
}
