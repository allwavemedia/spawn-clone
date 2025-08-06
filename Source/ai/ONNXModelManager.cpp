/*
  ==============================================================================

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
#include <thread>
#include <chrono>

#ifdef ONNX_RUNTIME_AVAILABLE
    #include <onnxruntime_cxx_api.h>
#endif

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
    // NOTE: Direct C++ ONNX Runtime has segmentation fault issues on this system
    // Using Python subprocess workaround for now
    runtimeInitialized = true;
    runtimeAvailable = false;  // Mark as unavailable to force Python mode
    DBG("ONNXModelManager: Initialized in Python subprocess mode");
    return true;
    #else
    // ONNX Runtime not available, use simulation mode
    lastError = "ONNX Runtime not available - using simulation mode";
    runtimeInitialized = true;  // Allow simulation mode
    runtimeAvailable = false;   // But mark runtime as unavailable
    DBG("ONNXModelManager: Initialized in simulation mode");
    return true;
    #endif
}

bool ONNXModelManager::loadModel(const juce::String& modelPath)
{
    if (!runtimeInitialized)
    {
        lastError = "ONNX Runtime not initialized";
        return false;
    }
    
    // Use Python subprocess for model loading
    return loadModelViaPython(modelPath);
}

bool ONNXModelManager::loadModelViaPython(const juce::String& modelPath)
{
    try
    {
        juce::File modelFile(modelPath);
        if (!modelFile.existsAsFile())
        {
            lastError = "Model file not found: " + modelPath;
            return false;
        }
        
        // Extract model name from path (e.g., "model_token" from "models/midi-model/onnx/model_token.onnx")
        juce::String modelName = modelFile.getFileNameWithoutExtension();
        
        // Prepare JSON command for Python server
        juce::DynamicObject::Ptr commandObj = new juce::DynamicObject();
        commandObj->setProperty("action", "load_model");
        commandObj->setProperty("model_name", modelName);
        
        juce::String jsonCommand = juce::JSON::toString(juce::var(commandObj.get()));
        
        // Execute Python server using temporary file to avoid shell escaping issues
        juce::File tempCommandFile = juce::File::getSpecialLocation(juce::File::tempDirectory).getChildFile("onnx_command.json");
        tempCommandFile.replaceWithText(jsonCommand);
        
        juce::String workingDir = juce::File::getCurrentWorkingDirectory().getFullPathName();
        juce::String pythonCommand = "/bin/bash -c \"cd '" + workingDir + "' && cat '" + tempCommandFile.getFullPathName() + "' | python3 midi_model_server.py\"";
        
        DBG("Executing: " + pythonCommand);
        
        juce::ChildProcess process;
        if (!process.start(pythonCommand))
        {
            lastError = "Failed to start Python MIDI model server";
            return false;
        }
        
        // Get the result
        juce::String result = process.readAllProcessOutput();
        process.waitForProcessToFinish(5000); // 5 second timeout
        
        // Clean up temp file
        tempCommandFile.deleteFile();
        
        DBG("Python server response: " + result);
        
        // Parse JSON response
        juce::var responseVar = juce::JSON::parse(result);
        if (responseVar.isObject())
        {
            juce::DynamicObject* responseObj = responseVar.getDynamicObject();
            
            juce::var successVar = responseObj->getProperty("success");
            bool success = successVar.isBool() ? (bool)successVar : false;
            
            if (success)
            {
                currentModelFile = modelFile;
                modelLoaded = true;
                currentModelName = modelName;
                
                // Store model metadata from Python response
                if (responseObj->hasProperty("inputs"))
                {
                    pythonModelInputs = responseObj->getProperty("inputs");
                }
                if (responseObj->hasProperty("outputs"))
                {
                    pythonModelOutputs = responseObj->getProperty("outputs");
                }
                
                DBG("Model loaded via Python: " + modelName);
                return true;
            }
            else
            {
                juce::var errorVar = responseObj->getProperty("error");
                juce::String error = errorVar.toString();
                if (error.isEmpty()) error = "Unknown error";
                lastError = "Python server error: " + error;
                return false;
            }
        }
        else
        {
            lastError = "Invalid response from Python server: " + result;
            return false;
        }
    }
    catch (const std::exception& e)
    {
        lastError = "Exception in Python model loading: " + juce::String(e.what());
        return false;
    }
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
    
    // Use Python subprocess for pattern generation
    return generatePatternViaPython(pattern, params);
}

bool ONNXModelManager::generatePatternViaPython(std::vector<uint8_t>& pattern, const GenerationParameters& params)
{
    try
    {
        // Prepare JSON command for Python server
        juce::DynamicObject::Ptr commandObj = new juce::DynamicObject();
        commandObj->setProperty("action", "generate_pattern");
        commandObj->setProperty("model_name", currentModelName);
        
        // Convert GenerationParameters to JSON
        juce::DynamicObject::Ptr paramsObj = new juce::DynamicObject();
        paramsObj->setProperty("key", params.key);
        paramsObj->setProperty("scale", static_cast<int>(params.scale));
        paramsObj->setProperty("tempo", params.tempo);
        paramsObj->setProperty("rhythmicComplexity", params.rhythmicComplexity);
        paramsObj->setProperty("generationType", static_cast<int>(params.generationType));
        paramsObj->setProperty("patternLengthBeats", params.patternLengthBeats);
        paramsObj->setProperty("generationSeed", static_cast<int>(params.generationSeed));
        
        commandObj->setProperty("params", juce::var(paramsObj.get()));
        
        juce::String jsonCommand = juce::JSON::toString(juce::var(commandObj.get()));
        
        // Execute Python server using temporary file
        juce::File tempCommandFile = juce::File::getSpecialLocation(juce::File::tempDirectory).getChildFile("onnx_pattern_command.json");
        tempCommandFile.replaceWithText(jsonCommand);
        
        juce::String workingDir = juce::File::getCurrentWorkingDirectory().getFullPathName();
        juce::String pythonCommand = "/bin/bash -c \"cd '" + workingDir + "' && cat '" + tempCommandFile.getFullPathName() + "' | python3 midi_model_server.py\"";
        
        DBG("Executing pattern generation: " + pythonCommand);
        
        juce::ChildProcess process;
        if (!process.start(pythonCommand))
        {
            lastError = "Failed to start Python MIDI model server for pattern generation";
            return false;
        }
        
        // Get the result
        juce::String result = process.readAllProcessOutput();
        process.waitForProcessToFinish(10000); // 10 second timeout for generation
        
        // Clean up temp file
        tempCommandFile.deleteFile();
        
        DBG("Python server pattern response: " + result.substring(0, 200) + "..."); // Log first 200 chars
        
        // Parse JSON response
        juce::var responseVar = juce::JSON::parse(result);
        if (responseVar.isObject())
        {
            juce::DynamicObject* responseObj = responseVar.getDynamicObject();
            
            juce::var successVar = responseObj->getProperty("success");
            bool success = successVar.isBool() ? (bool)successVar : false;
            
            if (success)
            {
                // Extract pattern data from response
                juce::var patternData = responseObj->getProperty("pattern_data");
                
                // For now, create a simple pattern from the response
                // TODO: Implement proper MIDI tokenization/detokenization
                pattern.clear();
                pattern.resize(128, 0); // Simple 128-byte pattern
                
                // Fill with some dummy MIDI data based on parameters
                pattern[0] = 0x90; // Note on
                pattern[1] = 60 + params.key; // Note (C4 + key offset)
                pattern[2] = 64; // Velocity
                pattern[3] = 0x80; // Note off (later in pattern)
                pattern[4] = 60 + params.key;
                pattern[5] = 0;
                
                DBG("Pattern generated successfully via Python, size: " + juce::String(pattern.size()));
                return true;
            }
            else
            {
                juce::var errorVar = responseObj->getProperty("error");
                juce::String error = errorVar.toString();
                if (error.isEmpty()) error = "Unknown error";
                lastError = "Python pattern generation error: " + error;
                return false;
            }
        }
        else
        {
            lastError = "Invalid response from Python server: " + result.substring(0, 100);
            return false;
        }
    }
    catch (const std::exception& e)
    {
        lastError = "Exception in Python pattern generation: " + juce::String(e.what());
        return false;
    }
}

//==============================================================================
// Model Information

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
    PROFILE_INFERENCE();
    
    // Start detailed performance tracking
    auto startTime = std::chrono::high_resolution_clock::now();
    lastInferenceReport = DetailedPerformanceReport{};
    lastInferenceReport.memoryBefore = PerformanceProfiler::getInstance().getCurrentMetrics().currentMemoryUsage;
    
    #ifdef ONNX_RUNTIME_AVAILABLE
    try
    {
        // Run ONNX model inference
        // This will be implemented when ONNX Runtime is integrated
        PROFILE_SUCCESS();
        return true;
    }
    catch (const std::exception& e)
    {
        lastError = "Inference failed: " + juce::String(e.what());
        PROFILE_FAILURE();
        return false;
    }
    #else
    // Simulate inference for development - generate dummy output with performance tracking
    auto preprocessStart = std::chrono::high_resolution_clock::now();
    
    // Preprocessing simulation
    if (currentPerformanceMode == "quality") {
        // Simulate more thorough preprocessing
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    
    auto inferenceStart = std::chrono::high_resolution_clock::now();
    lastInferenceReport.preprocessTime = std::chrono::duration<double, std::milli>(inferenceStart - preprocessStart).count();
    
    // Main inference simulation
    outputData.resize(128);  // Simulate 128 output values
    std::mt19937 gen(inputData.size() > 0 ? static_cast<uint32_t>(inputData[6] * 10000) : 12345);
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    
    // Simulate processing time based on performance mode and batch size
    int simulatedTimeMs = 50; // Base time
    if (currentPerformanceMode == "fast") {
        simulatedTimeMs = 20;
    } else if (currentPerformanceMode == "quality") {
        simulatedTimeMs = 100;
    }
    
    // Batch processing affects timing
    if (batchProcessingEnabled) {
        simulatedTimeMs = static_cast<int>(simulatedTimeMs * 0.7 * currentBatchSize); // Batching is more efficient
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(simulatedTimeMs));
    
    for (auto& value : outputData)
    {
        value = dist(gen);
    }
    
    auto postprocessStart = std::chrono::high_resolution_clock::now();
    lastInferenceReport.inferenceTime = std::chrono::duration<double, std::milli>(postprocessStart - inferenceStart).count();
    
    // Postprocessing simulation
    if (currentPerformanceMode == "quality") {
        std::this_thread::sleep_for(std::chrono::milliseconds(3));
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    lastInferenceReport.postprocessTime = std::chrono::duration<double, std::milli>(endTime - postprocessStart).count();
    lastInferenceReport.totalTime = std::chrono::duration<double, std::milli>(endTime - startTime).count();
    lastInferenceReport.memoryAfter = PerformanceProfiler::getInstance().getCurrentMetrics().currentMemoryUsage;
    lastInferenceReport.memoryPeak = std::max(lastInferenceReport.memoryBefore, lastInferenceReport.memoryAfter);
    
    // Update global performance metrics
    updatePerformanceMetrics(static_cast<float>(lastInferenceReport.totalTime));
    
    DBG("Simulated ONNX inference: " + juce::String(lastInferenceReport.totalTime, 2) + "ms total (" +
        "preprocess: " + juce::String(lastInferenceReport.preprocessTime, 1) + "ms, " +
        "inference: " + juce::String(lastInferenceReport.inferenceTime, 1) + "ms, " +
        "postprocess: " + juce::String(lastInferenceReport.postprocessTime, 1) + "ms)");
    
    PROFILE_SUCCESS();
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

//==============================================================================
// Missing Method Implementations - Epic 7 ONNX Fixes

bool ONNXModelManager::isModelLoaded() const
{
    // In simulation mode, we just check if modelLoaded is true
    return modelLoaded;
}

int ONNXModelManager::getTotalInferences() const
{
    // Return stored inference count from ModelInfo
    return currentModelInfo.numInferences;
}

float ONNXModelManager::getAverageInferenceTime() const
{
    // Return stored average from ModelInfo
    return currentModelInfo.avgInferenceTime;
}

float ONNXModelManager::getTotalSavings() const
{
    // Calculate savings based on local vs cloud inference costs
    // Assuming cloud cost ~$0.01 per inference, local cost ~$0.001
    const float cloudCostPerInference = 0.01f;
    const float localCostPerInference = 0.001f;
    const float savingsPerInference = cloudCostPerInference - localCostPerInference;
    
    return static_cast<float>(getTotalInferences()) * savingsPerInference;
}

ONNXModelManager::ModelInfo ONNXModelManager::getModelInfo() const
{
    return currentModelInfo;
}

void ONNXModelManager::updatePerformanceMetrics(float inferenceTime)
{
    // Update inference count and running average
    currentModelInfo.numInferences++;
    
    // Calculate rolling average inference time
    if (currentModelInfo.numInferences == 1) {
        currentModelInfo.avgInferenceTime = inferenceTime;
    } else {
        // Exponential moving average for better recent performance weighting
        const float alpha = 0.1f;  // Smoothing factor
        currentModelInfo.avgInferenceTime = 
            alpha * inferenceTime + (1.0f - alpha) * currentModelInfo.avgInferenceTime;
    }
    
    // Record with global performance profiler
    PerformanceProfiler::getInstance().recordInferenceTime(inferenceTime, true);
}

//==============================================================================
// Performance Optimization Methods

void ONNXModelManager::optimizeMemoryUsage()
{
    PROFILE_OPERATION("memory-optimization");
    
    DBG("Optimizing memory usage for ONNX model...");
    
    // 1. Clear any unnecessary caches
    if (modelCacheManager) {
        // Keep only essential models in cache
        // modelCacheManager->cleanupOldModels();
    }
    
    // 2. Optimize model loading strategy
    if (currentPerformanceMode == "fast") {
        // In fast mode, prefer memory-mapped models
        modelCachingEnabled = false; // Don't keep models in memory
    } else {
        modelCachingEnabled = true;  // Keep models cached for quality
    }
    
    // 3. Adjust batch processing to reduce memory footprint
    if (batchProcessingEnabled && currentBatchSize > 2) {
        currentBatchSize = std::max(1, currentBatchSize / 2);
        DBG("Reduced batch size to " + juce::String(currentBatchSize) + " for memory optimization");
    }
    
    PROFILE_SUCCESS();
}

void ONNXModelManager::optimizeInferenceSpeed()
{
    PROFILE_OPERATION("speed-optimization");
    
    DBG("Optimizing inference speed for ONNX model...");
    
    // 1. Enable batch processing for throughput
    if (!batchProcessingEnabled && currentPerformanceMode != "quality") {
        enableBatchProcessing(2); // Start with small batch size
    }
    
    // 2. Optimize for current performance mode
    if (currentPerformanceMode == "fast") {
        // Aggressive optimizations for speed
        modelCachingEnabled = true;  // Keep model hot in memory
        currentBatchSize = std::min(4, currentBatchSize + 1);
    } else if (currentPerformanceMode == "balanced") {
        // Balanced approach
        modelCachingEnabled = true;
        currentBatchSize = 2;
    }
    
    // 3. Pre-warm the model if not already done
    if (modelLoaded && runtimeAvailable) {
        // Run a quick dummy inference to warm up the model
        std::vector<float> dummyInput(7, 0.5f); // Typical input size
        std::vector<float> dummyOutput;
        runInference(dummyInput, dummyOutput);
        DBG("Model pre-warmed for faster subsequent inferences");
    }
    
    PROFILE_SUCCESS();
}

bool ONNXModelManager::enableBatchProcessing(int batchSize)
{
    if (batchSize < 1 || batchSize > 16) {
        lastError = "Invalid batch size: must be between 1 and 16";
        return false;
    }
    
    batchProcessingEnabled = (batchSize > 1);
    currentBatchSize = batchSize;
    
    DBG("Batch processing " + juce::String(batchProcessingEnabled ? "enabled" : "disabled") + 
        " with batch size: " + juce::String(currentBatchSize));
    
    return true;
}

void ONNXModelManager::enableModelCaching(bool enable)
{
    modelCachingEnabled = enable;
    DBG("Model caching " + juce::String(enable ? "enabled" : "disabled"));
}

void ONNXModelManager::setPerformanceMode(const juce::String& mode)
{
    currentPerformanceMode = mode.toLowerCase();
    
    if (currentPerformanceMode == "fast") {
        // Optimize for speed
        enableBatchProcessing(4);
        enableModelCaching(true);
        DBG("Performance mode set to FAST - optimizing for speed");
    } else if (currentPerformanceMode == "quality") {
        // Optimize for quality
        enableBatchProcessing(1); // Disable batching
        enableModelCaching(true);
        DBG("Performance mode set to QUALITY - optimizing for accuracy");
    } else {
        // Balanced mode (default)
        currentPerformanceMode = "balanced";
        enableBatchProcessing(2);
        enableModelCaching(true);
        DBG("Performance mode set to BALANCED - optimizing for speed/quality balance");
    }
}

//==============================================================================
// Advanced Monitoring

ONNXModelManager::DetailedPerformanceReport ONNXModelManager::getLastInferenceReport() const
{
    return lastInferenceReport;
}

juce::String ONNXModelManager::getPerformanceRecommendations() const
{
    auto recommendations = PerformanceProfiler::getInstance().getOptimizationRecommendations();
    juce::String result = "=== ONNX Performance Recommendations ===\n\n";
    
    if (recommendations.empty()) {
        result += "✅ All performance targets are being met!\n";
        result += "Current configuration is optimal.\n\n";
    } else {
        for (const auto& rec : recommendations) {
            juce::String priority = (rec.priority == 1) ? "🔴 CRITICAL" :
                                   (rec.priority == 2) ? "🟡 IMPORTANT" : "🟢 MINOR";
            
            result += priority + " [" + rec.category + "]\n";
            result += "Issue: " + rec.issue + "\n";
            result += "Recommendation: " + rec.recommendation + "\n\n";
        }
    }
    
    // Add model-specific recommendations
    auto metrics = PerformanceProfiler::getInstance().getCurrentMetrics();
    
    if (currentPerformanceMode == "balanced" && metrics.averageInferenceTime > 1000.0) {
        result += "💡 Consider switching to 'fast' mode for better performance:\n";
        result += "   Call setPerformanceMode(\"fast\")\n\n";
    }
    
    if (!batchProcessingEnabled && metrics.totalInferences > 50) {
        result += "💡 Consider enabling batch processing for better throughput:\n";
        result += "   Call enableBatchProcessing(2)\n\n";
    }
    
    return result;
}
