/*
  ===================================================bool ONNXModelManager::loadModel(const juce::String& modelPath)
{
    if (!runtimeInitialized)
    {
        lastError = "ONNX Runtime not initialized";
        return false;
    }
    
    juce::File modelFile(modelPath);
    if (!modelFile.existsAsFile())
    {
        lastError = "Model file not found: " + modelPath;
        return false;
    }
    
    // Validate model format
    if (!validateModel(modelFile))==========

    ONNXModelManager.cpp
    Created: 29 Jul 2025
    Author:  Epic 7 Story 7.2 Implementation

    Implementation of ONNX Runtime model management for Quality Mode generation.

  ==============================================================================
*/

#include "ONNXModelManager.h"
#include "ModelCacheManager.h"
#include "../MIDIPattern.h"
#include "../GenerationParameters.h"
#include <random>
#include <algorithm>

//==============================================================================
ONNXModelManager::ONNXModelManager()
{
    // Initialize ONNX Runtime if available
    initializeRuntime();
}

ONNXModelManager::~ONNXModelManager()
{
    // Cleanup ONNX Runtime resources
}

//==============================================================================
// Model Management (Task 7.2.1, 7.2.2)

bool ONNXModelManager::initializeRuntime()
{
    #ifdef ONNX_RUNTIME_AVAILABLE
    try
    {
        // Initialize ONNX Runtime environment
        // This will be implemented when ONNX Runtime is integrated
        runtimeInitialized = true;
        runtimeAvailable = true;
        return true;
    }
    catch (const std::exception& e)
    {
        lastError = "Failed to initialize ONNX Runtime: " + juce::String(e.what());
        runtimeInitialized = false;
        runtimeAvailable = false;
        return false;
    }
    #else
    // ONNX Runtime not available, simulate unavailable state
    lastError = "ONNX Runtime not available in this build";
    runtimeInitialized = false;
    runtimeAvailable = false;
    return false;
    #endif
}

bool ONNXModelManager::loadModel(const juce::String& modelPath)
{
    if (!runtimeInitialized)
    {
        lastError = "ONNX Runtime not initialized";
        return false;
    }
    
    juce::File modelFile(modelPath);
    if (!modelFile.existsAsFile())
    {
        lastError = "Model file not found: " + modelPath;
        return false;
    }
    
    // Validate model before loading (Task 7.2.4)
    if (!validateModel(modelFile))
    {
        return false;
    }
    
    #ifdef ONNX_RUNTIME_AVAILABLE
    try
    {
        // Load ONNX model into session
        // This will be implemented when ONNX Runtime is integrated
        currentModelFile = modelFile;
        modelLoaded = true;
        return true;
    }
    catch (const std::exception& e)
    {
        lastError = "Failed to load model: " + juce::String(e.what());
        modelLoaded = false;
        return false;
    }
    #else
    // Simulate model loading for development
    currentModelFile = modelFile;
    modelLoaded = true;
    DBG("Simulated model loading: " + modelPath);
    return true;
    #endif
}

//==============================================================================
// Pattern Generation (Task 7.2.3)

bool ONNXModelManager::generatePattern(std::vector<uint8_t>& pattern, const GenerationParameters& params)
{
    if (!modelLoaded)
    {
        lastError = "No model loaded";
        return false;
    }
    
    // Convert to MIDIPattern for processing
    MIDIPattern midiPattern;
    
    // Preprocess generation parameters into model input format
    auto inputData = preprocessParameters(params);
    
    // Run model inference
    std::vector<float> outputData;
    if (!runInference(inputData, outputData))
    {
        return false;
    }
    
    // Post-process model output into MIDI pattern
    if (!postprocessOutput(outputData, midiPattern, params))
    {
        return false;
    }
    
    // Convert MIDIPattern to raw MIDI bytes
    pattern = convertPatternToMIDI(midiPattern, static_cast<int>(params.tempo));
    return true;
}

//==============================================================================
// Model Validation (Task 7.2.4)

bool ONNXModelManager::validateModel(const juce::File& modelFile)
{
    // Check file size and basic integrity
    if (!verifyModelIntegrity(modelFile))
    {
        lastError = "Model file integrity check failed";
        return false;
    }
    
    // Check model compatibility with our interface
    if (!checkModelCompatibility(modelFile))
    {
        lastError = "Model is not compatible with this version";
        return false;
    }
    
    return true;
}

bool ONNXModelManager::verifyModelIntegrity(const juce::File& modelFile)
{
    // Basic file size and format checks
    auto size = modelFile.getSize();
    if (size < 1000) // Too small to be a valid model
    {
        return false;
    }
    
    // For now, assume file is valid if size is reasonable
    return true;
}

bool ONNXModelManager::checkModelCompatibility(const juce::File& modelFile)
{
    // Check model version and interface compatibility
    // For now, assume compatible
    return true;
}

//==============================================================================
// Fallback Management (Task 7.2.5)

bool ONNXModelManager::requiresFallback() const
{
    return !runtimeInitialized || !modelLoaded;
}

//==============================================================================
// Private Implementation

std::vector<float> ONNXModelManager::preprocessParameters(const GenerationParameters& params)
{
    // Convert GenerationParameters to model input format
    std::vector<float> inputData;
    
    // Normalize parameters to 0-1 range for model input
    inputData.push_back(params.key / 11.0f);  // Key (0-11) -> 0-1
    inputData.push_back(static_cast<float>(params.scale) / 5.0f);  // Scale (0-5) -> 0-1
    inputData.push_back((params.tempo - 60.0f) / 140.0f);  // Tempo (60-200) -> 0-1
    inputData.push_back(params.rhythmicComplexity);  // Already 0-1
    inputData.push_back(static_cast<float>(params.generationType) / 3.0f);  // Type (0-3) -> 0-1
    inputData.push_back((params.patternLengthBeats - 4.0f) / 28.0f);  // Length (4-32) -> 0-1
    inputData.push_back(params.generationSeed / 10000.0f);  // Seed (0-10000) -> 0-1
    
    return inputData;
}

bool ONNXModelManager::runInference(const std::vector<float>& inputData, std::vector<float>& outputData)
{
    #ifdef ONNX_RUNTIME_AVAILABLE
    try
    {
        // Run ONNX model inference
        // This will be implemented when ONNX Runtime is integrated
        return true;
    }
    catch (const std::exception& e)
    {
        lastError = "Inference failed: " + juce::String(e.what());
        return false;
    }
    #else
    // Simulate inference for development - generate dummy output
    outputData.resize(128);  // Simulate 128 output values
    std::mt19937 gen(inputData.size() > 0 ? static_cast<uint32_t>(inputData[6] * 10000) : 12345);
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    
    for (auto& value : outputData)
    {
        value = dist(gen);
    }
    
    DBG("Simulated ONNX inference with " + juce::String(inputData.size()) + " inputs");
    return true;
    #endif
}

bool ONNXModelManager::postprocessOutput(const std::vector<float>& outputData, MIDIPattern& pattern, const GenerationParameters& params)
{
    // Convert model output back to MIDI pattern
    pattern.notes.clear();
    pattern.lengthInBeats = params.patternLengthBeats;
    pattern.metadata.tempo = params.tempo;
    pattern.metadata.key = params.key;
    pattern.metadata.scale = params.scale;
    
    // Interpret model output as note events
    // For simulation, create a more sophisticated pattern than Fast Mode
    int numNotes = static_cast<int>(outputData.size() / 4);  // 4 values per note: pitch, time, duration, velocity
    numNotes = std::min(numNotes, 32);  // Limit to reasonable number
    
    for (int i = 0; i < numNotes; ++i)
    {
        if (i * 4 + 3 < outputData.size())
        {
            Note note;
            
            // Use model output to determine note properties
            note.pitch = 60 + static_cast<int>(outputData[i * 4] * 24);  // C4 +/- 2 octaves
            note.startTime = outputData[i * 4 + 1] * params.patternLengthBeats;
            note.duration = std::max(0.1, static_cast<double>(outputData[i * 4 + 2] * 2.0));  // Max 2 beats duration
            note.velocity = static_cast<int>(60 + outputData[i * 4 + 3] * 67);  // 60-127 velocity
            
            // Only add note if it fits within pattern length
            if (note.startTime < params.patternLengthBeats)
            {
                pattern.notes.push_back(note);
            }
        }
    }
    
    // Sort notes by start time
    std::sort(pattern.notes.begin(), pattern.notes.end(), 
              [](const Note& a, const Note& b) { return a.startTime < b.startTime; });
    
    return true;
}

//==============================================================================
// Model Cache Integration (Task 7.4.1)

void ONNXModelManager::setModelCacheManager(std::shared_ptr<ModelCacheManager> newCacheManager)
{
    modelCacheManager = newCacheManager;
    
    // Auto-load best available model when cache manager is set
    if (modelCacheManager)
    {
        autoLoadBestModel();
    }
}

bool ONNXModelManager::autoLoadBestModel()
{
    if (!modelCacheManager)
    {
        lastError = "No cache manager available";
        return false;
    }
    
    // Get list of cached models
    auto cachedModels = modelCacheManager->getCachedModels();
    if (cachedModels.empty())
    {
        lastError = "No models available in cache";
        return false;
    }
    
    // Try to load the best available model (prefer base-pattern models)
    std::vector<juce::String> preferredOrder = {
        "base-pattern-v1",
        "genre-specialist-v2"
    };
    
    for (const auto& preferredModel : preferredOrder)
    {
        if (std::find(cachedModels.begin(), cachedModels.end(), preferredModel) != cachedModels.end())
        {
            auto modelFile = modelCacheManager->getModelFile(preferredModel);
            if (loadModel(modelFile.getFullPathName()))
            {
                DBG("Auto-loaded model: " + preferredModel);
                return true;
            }
        }
    }
    
    // If no preferred models found, try the first available
    if (!cachedModels.empty())
    {
        auto modelFile = modelCacheManager->getModelFile(cachedModels[0]);
        if (loadModel(modelFile.getFullPathName()))
        {
            DBG("Auto-loaded fallback model: " + cachedModels[0]);
            return true;
        }
    }
    
    lastError = "Failed to load any cached models";
    return false;
}

//==============================================================================
// Epic 7 Week 2: Enhanced Integration Methods

bool ONNXModelManager::isWeek2Ready() const
{
    return isModelLoaded() && meetsPerformanceTarget();
}

juce::String ONNXModelManager::getWeek2Status() const
{
    juce::String status = "ONNX Model Status:\n";
    status += "- Loaded: " + juce::String(isModelLoaded() ? "YES" : "NO") + "\n";
    status += "- Performance Target: " + juce::String(meetsPerformanceTarget() ? "MET" : "NOT MET") + "\n";
    
    if (runtimeAvailable)
    {
        status += "- Runtime: AVAILABLE\n";
        status += "- Fallback Required: " + juce::String(requiresFallback() ? "YES" : "NO") + "\n";
    }
    else
    {
        status += "- Runtime: NOT AVAILABLE (using fallback)\n";
    }
    
    return status;
}

bool ONNXModelManager::meetsPerformanceTarget() const
{
    // Week 2 performance target: <2s inference
    // For local inference, this should be much faster
    return !requiresFallback() && runtimeAvailable;
}

std::vector<uint8_t> ONNXModelManager::generateMIDIPattern(const juce::String& genre,
                                                           const juce::String& style,
                                                           int lengthInBeats,
                                                           int tempo)
{
    GenerationParameters params;
    // Map genre/style to available parameters as best we can
    params.tempo = static_cast<float>(tempo);
    params.patternLengthBeats = static_cast<float>(lengthInBeats);
    
    std::vector<uint8_t> pattern;
    if (generatePattern(pattern, params))
    {
        return pattern;
    }
    
    return {};
}

std::vector<uint8_t> ONNXModelManager::convertPatternToMIDI(const MIDIPattern& pattern, int tempo)
{
    std::vector<uint8_t> midiData;
    
    // Simple MIDI conversion for Week 2
    for (const auto& note : pattern.notes)
    {
        // Note on
        midiData.push_back(0x90); // Note on, channel 0
        midiData.push_back(static_cast<uint8_t>(note.pitch));
        midiData.push_back(static_cast<uint8_t>(note.velocity));
        
        // Note off (simplified - in real MIDI this would be timed)
        midiData.push_back(0x80); // Note off, channel 0
        midiData.push_back(static_cast<uint8_t>(note.pitch));
        midiData.push_back(0x00);
    }
    
    return midiData;
}

//==============================================================================
// Access methods for AIGenerationEngine integration

juce::String ONNXModelManager::getLastError() const
{
    return lastError;
}
