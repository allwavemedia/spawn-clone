//==============================================================================
// ONNXModelManager.h
// ONNX Model Management for Epic 7 Week 2 Local Inference
//==============================================================================

#pragma once

#include <juce_core/juce_core.h>
#include "PerformanceProfiler.h"
#include <memory>
#include <vector>
#include <string>
#include <functional>

#ifdef ONNX_RUNTIME_AVAILABLE
    #include <onnxruntime_cxx_api.h>
#endif

// Forward declarations
class ModelCacheManager;
class ONNXDaemonClient;
struct MIDIPattern;
struct GenerationParameters;

class ONNXModelManager
{
public:
    struct InferenceResult
    {
        std::vector<float> embeddings;
        float inferenceTime;
        bool success;
        juce::String errorMessage;
        
        InferenceResult() : inferenceTime(0.0f), success(false) {}
    };
    
    struct ModelInfo
    {
        juce::String modelPath;
        juce::String modelType;
        bool isLoaded;
        float avgInferenceTime;
        int numInferences;
        
        ModelInfo() : isLoaded(false), avgInferenceTime(0.0f), numInferences(0) {}
    };
    
    ONNXModelManager();
    ~ONNXModelManager();
    
    // Model Loading
    bool loadModel(const juce::String& modelPath);
    bool isModelLoaded() const;
    ModelInfo getModelInfo() const;
    void unloadModel();
    
    // Inference Operations
    InferenceResult runInference(const std::vector<int>& inputIds);
    InferenceResult generateMIDIEmbeddings(const juce::String& genre, 
                                         const juce::String& style,
                                         int patternLength = 128);
    
    // Pattern Generation (Epic 7 Integration)
    std::vector<uint8_t> generateMIDIPattern(const juce::String& genre,
                                           const juce::String& style, 
                                           int lengthInBeats = 32,
                                           int tempo = 120);
    
    // Performance Monitoring
    float getAverageInferenceTime() const;
    int getTotalInferences() const;
    bool meetsPerformanceTarget() const; // <2s target
    
    // Cost Tracking
    float getCostPerInference() const; // Should be ~$0.001
    float getTotalSavings() const; // vs cloud costs
    
    // Week 2 Status
    bool isWeek2Ready() const;
    juce::String getWeek2Status() const;
    
    // Integration methods for AIGenerationEngine
    void setModelCacheManager(std::shared_ptr<ModelCacheManager> manager);
    bool requiresFallback() const;
    bool generatePattern(std::vector<uint8_t>& pattern, const struct GenerationParameters& params);
    juce::String getLastError() const;
    
    // Daemon Status and Performance Monitoring
    juce::String getDaemonStatus() const;
    
    /** Get daemon client for live performance integration */
    std::shared_ptr<ONNXDaemonClient> getDaemonClient() const;
    
    // Performance Optimization Methods (Epic 7 Task)
    void optimizeMemoryUsage();
    void optimizeInferenceSpeed();
    bool enableBatchProcessing(int batchSize = 4);
    void enableModelCaching(bool enable = true);
    void setPerformanceMode(const juce::String& mode); // "fast", "balanced", "quality"
    
    // Advanced monitoring
    struct DetailedPerformanceReport
    {
        // Timing breakdown
        double preprocessTime = 0.0;
        double inferenceTime = 0.0;
        double postprocessTime = 0.0;
        double totalTime = 0.0;
        
        // Resource usage
        size_t memoryBefore = 0;
        size_t memoryAfter = 0;
        size_t memoryPeak = 0;
        double cpuUsage = 0.0;
        
        // Quality metrics
        int outputPatternLength = 0;
        int numGeneratedNotes = 0;
        double patternComplexity = 0.0;
    };
    
    DetailedPerformanceReport getLastInferenceReport() const;
    juce::String getPerformanceRecommendations() const;
    
private:
    // Helper Methods
    bool initializeRuntime();
    bool validateModel(const juce::File& modelFile);
    bool verifyModelIntegrity(const juce::File& modelFile);
    bool checkModelCompatibility(const juce::File& modelFile);
    bool autoLoadBestModel();
    
    // Member variables
    juce::String lastError;
    bool runtimeInitialized = false;
    bool modelLoaded = false;
    bool runtimeAvailable = false;
    juce::File currentModelFile;
    std::shared_ptr<ModelCacheManager> modelCacheManager;
    ModelInfo currentModelInfo;  // Track current model performance metrics
    
    // Enhanced performance tracking
    DetailedPerformanceReport lastInferenceReport;
    juce::String currentPerformanceMode = "balanced";
    bool batchProcessingEnabled = false;
    int currentBatchSize = 1;
    bool modelCachingEnabled = true;
    
    // Python daemon integration (persistent process for optimal performance)
    juce::String currentModelName;
    juce::var pythonModelInputs;
    juce::var pythonModelOutputs;
    std::shared_ptr<ONNXDaemonClient> daemonClient;
    bool initializeDaemon();
    bool loadModelViaDaemon(const juce::String& modelPath);
    bool generatePatternViaDaemon(std::vector<uint8_t>& pattern, const struct GenerationParameters& params);
    void shutdownDaemon();
    
    #ifdef ONNX_RUNTIME_AVAILABLE
        // ONNX Runtime session and environment (currently disabled due to segfaults)
        std::unique_ptr<Ort::Env> ortEnv;
        std::unique_ptr<Ort::Session> ortSession;
        std::unique_ptr<Ort::SessionOptions> sessionOptions;
        std::vector<std::string> inputNames;
        std::vector<std::string> outputNames;
        std::vector<std::vector<int64_t>> inputShapes;
        std::vector<std::vector<int64_t>> outputShapes;
    #endif
    
    std::vector<int> tokenizeInput(const juce::String& text);
    std::vector<uint8_t> embeddingsToMIDI(const std::vector<float>& embeddings,
                                        int lengthInBeats, int tempo);
    void updatePerformanceMetrics(float inferenceTime);
    
    // Internal processing methods
    std::vector<float> preprocessParameters(const GenerationParameters& params);
    bool runInference(const std::vector<float>& inputData, std::vector<float>& outputData);
    bool postprocessOutput(const std::vector<float>& outputData, MIDIPattern& pattern, const GenerationParameters& params);
    std::vector<uint8_t> convertPatternToMIDI(const MIDIPattern& pattern, int tempo);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ONNXModelManager)
};

// Epic 7 Integration Helper
class Epic7ONNXIntegration
{
public:
    static bool setupWeek2Environment();
    static bool validateONNXModel();
    static juce::String getWeek2StatusReport();
    static bool testLocalInference();
    
private:
    Epic7ONNXIntegration() = delete;
};
