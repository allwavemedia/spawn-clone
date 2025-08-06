//==============================================================================
// ONNXModelManager.h
// ONNX Model Management for Epic 7 Week 2 Local Inference
//==============================================================================

#pragma once

#include <juce_core/juce_core.h>
#include <memory>
#include <vector>
#include <string>
#include <functional>

// Forward declarations
class ModelCacheManager;
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
