/*
  ==============================================================================

    ONNXModelManager.h
    Created: 29 Jul 2025
    Author:  Epic 7 Story 7.2 Implementation

    Manages ONNX Runtime model loading and inference for Quality Mode AI generation.

  ==============================================================================
*/

#pragma once

#include <juce_core/juce_core.h>
#include "../GenerationParameters.h"
#include "../MIDIPattern.h"
#include <memory>

// Forward declaration
class ModelCacheManager;

//==============================================================================
/**
    Manages ONNX Runtime model loading and inference for local ML-based pattern generation.
    Provides fallback to Fast Mode when models are unavailable.
*/
class ONNXModelManager
{
public:
    ONNXModelManager();
    ~ONNXModelManager();

    //==============================================================================
    // Model Management (Task 7.2.1, 7.2.2)
    bool initializeRuntime();
    bool loadModel(const juce::String& modelPath);
    bool isModelLoaded() const { return modelLoaded; }
    bool isRuntimeAvailable() const { return runtimeInitialized; }
    
    //==============================================================================
    // Pattern Generation (Task 7.2.3)
    bool generatePattern(MIDIPattern& pattern, const GenerationParameters& params);
    
    //==============================================================================
    // Model Validation (Task 7.2.4)
    bool validateModel(const juce::File& modelFile);
    juce::String getModelVersion() const { return modelVersion; }
    
    //==============================================================================
    // Fallback Management (Task 7.2.5)
    bool requiresFallback() const;
    juce::String getLastError() const { return lastError; }
    
    //==============================================================================
    // Model Cache Integration (Task 7.4.1)
    
    /** Set model cache manager */
    void setModelCacheManager(std::shared_ptr<ModelCacheManager> cacheManager);
    
    /** Auto-load best available model */
    bool autoLoadBestModel();

private:
    //==============================================================================
    // ONNX Runtime Integration
    #ifdef ONNX_RUNTIME_AVAILABLE
    // ONNX Runtime session and environment will go here
    // For now, we'll simulate the interface
    #endif
    
    //==============================================================================
    // State management
    bool runtimeInitialized = false;
    bool modelLoaded = false;
    juce::String modelVersion = "1.0.0";
    juce::String lastError;
    juce::File currentModelFile;
    
    // Model cache integration
    std::shared_ptr<ModelCacheManager> cacheManager;
    
    //==============================================================================
    // Model inference methods
    std::vector<float> preprocessParameters(const GenerationParameters& params);
    bool runInference(const std::vector<float>& inputData, std::vector<float>& outputData);
    bool postprocessOutput(const std::vector<float>& outputData, MIDIPattern& pattern, const GenerationParameters& params);
    
    //==============================================================================
    // Validation helpers
    bool checkModelCompatibility(const juce::File& modelFile);
    bool verifyModelIntegrity(const juce::File& modelFile);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ONNXModelManager)
};
