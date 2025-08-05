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

class ONNXModelManager
{
public:
    struct InferenceResult
    {
        std::vector<float> embeddings;
        float inferenceTime;
        bool success;
        String errorMessage;
        
        InferenceResult() : inferenceTime(0.0f), success(false) {}
    };
    
    struct ModelInfo
    {
        String modelPath;
        String modelType;
        bool isLoaded;
        float avgInferenceTime;
        int numInferences;
        
        ModelInfo() : isLoaded(false), avgInferenceTime(0.0f), numInferences(0) {}
    };
    
    ONNXModelManager();
    ~ONNXModelManager();
    
    // Model Loading
    bool loadModel(const String& modelPath);
    bool isModelLoaded() const;
    ModelInfo getModelInfo() const;
    void unloadModel();
    
    // Inference Operations
    InferenceResult runInference(const std::vector<int>& inputIds);
    InferenceResult generateMIDIEmbeddings(const String& genre, 
                                         const String& style,
                                         int patternLength = 128);
    
    // Pattern Generation (Epic 7 Integration)
    std::vector<uint8_t> generateMIDIPattern(const String& genre,
                                           const String& style, 
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
    String getWeek2Status() const;
    
private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
    
    // Helper Methods
    std::vector<int> tokenizeInput(const String& text);
    std::vector<uint8_t> embeddingsToMIDI(const std::vector<float>& embeddings,
                                        int lengthInBeats, int tempo);
    void updatePerformanceMetrics(float inferenceTime);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ONNXModelManager)
};

// Epic 7 Integration Helper
class Epic7ONNXIntegration
{
public:
    static bool setupWeek2Environment();
    static bool validateONNXModel();
    static String getWeek2StatusReport();
    static bool testLocalInference();
    
private:
    Epic7ONNXIntegration() = delete;
};
