//==============================================================================
// test_minimal_onnx.cpp
// Absolutely minimal ONNX Runtime test to isolate the segfault issue
//==============================================================================

#include <iostream>
#include <fstream>
#include <vector>

#ifdef ONNX_RUNTIME_AVAILABLE
    #include <onnxruntime_cxx_api.h>
#endif

int main() {
    #ifdef ONNX_RUNTIME_AVAILABLE
    std::cout << "=== Minimal ONNX Runtime Test ===" << std::endl;
    
    try {
        // Step 1: Basic environment creation
        std::cout << "1. Creating basic environment..." << std::endl;
        Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "MinimalTest");
        std::cout << "   ✓ Environment created" << std::endl;
        
        // Step 2: Minimal session options
        std::cout << "2. Creating minimal session options..." << std::endl;
        Ort::SessionOptions sessionOptions;
        
        // Very conservative settings to avoid memory issues
        sessionOptions.SetIntraOpNumThreads(1);
        sessionOptions.SetInterOpNumThreads(1);
        sessionOptions.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_DISABLE_ALL);
        
        // Disable memory optimizations that might cause issues
        sessionOptions.DisableMemPattern();
        sessionOptions.DisableCpuMemArena();
        sessionOptions.DisableProfiling();
        
        std::cout << "   ✓ Minimal session options set" << std::endl;
        
        // Step 3: Try loading the smaller token model first
        std::string tokenModelPath = "models/midi-model/onnx/model_token.onnx";
        
        std::ifstream tokenFile(tokenModelPath, std::ios::binary | std::ios::ate);
        if (tokenFile.is_open()) {
            std::streamsize tokenFileSize = tokenFile.tellg();
            tokenFile.close();
            
            std::cout << "3. Testing with smaller token model..." << std::endl;
            std::cout << "   Token model size: " << tokenFileSize << " bytes" << std::endl;
            
            std::cout << "   Creating session for token model..." << std::endl;
            try {
                Ort::Session tokenSession(env, tokenModelPath.c_str(), sessionOptions);
                std::cout << "   ✅ Token model loaded successfully!" << std::endl;
                
                // Get basic info
                size_t numInputs = tokenSession.GetInputCount();
                size_t numOutputs = tokenSession.GetOutputCount();
                std::cout << "   Token model inputs: " << numInputs << ", outputs: " << numOutputs << std::endl;
                
            } catch (const Ort::Exception& e) {
                std::cout << "   ❌ Token model failed: " << e.what() << std::endl;
                std::cout << "   Error code: " << e.GetOrtErrorCode() << std::endl;
            }
        } else {
            std::cout << "3. Token model not found, skipping..." << std::endl;
        }
        
        // Step 4: Try the main model with extreme caution
        std::string baseModelPath = "models/midi-model/onnx/model_base.onnx";
        
        std::ifstream baseFile(baseModelPath, std::ios::binary | std::ios::ate);
        if (baseFile.is_open()) {
            std::streamsize baseFileSize = baseFile.tellg();
            baseFile.close();
            
            std::cout << "4. Testing with large base model..." << std::endl;
            std::cout << "   Base model size: " << baseFileSize << " bytes (" << (baseFileSize / 1024 / 1024) << " MB)" << std::endl;
            
            if (baseFileSize > 500 * 1024 * 1024) {  // > 500MB
                std::cout << "   WARNING: Large model detected, this may cause memory issues" << std::endl;
            }
            
            std::cout << "   Creating session for base model..." << std::endl;
            try {
                // Try with even more restrictive settings for large model
                Ort::SessionOptions largeModelOptions;
                largeModelOptions.SetIntraOpNumThreads(1);
                largeModelOptions.SetInterOpNumThreads(1);
                largeModelOptions.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_DISABLE_ALL);
                largeModelOptions.DisableMemPattern();
                largeModelOptions.DisableCpuMemArena();
                largeModelOptions.DisableProfiling();
                
                // Add memory constraints if available
                // largeModelOptions.AddConfigEntry("session.memory_pattern", "0");
                
                Ort::Session baseSession(env, baseModelPath.c_str(), largeModelOptions);
                std::cout << "   ✅ Base model loaded successfully!" << std::endl;
                
                // Get basic info
                size_t numInputs = baseSession.GetInputCount();
                size_t numOutputs = baseSession.GetOutputCount();
                std::cout << "   Base model inputs: " << numInputs << ", outputs: " << numOutputs << std::endl;
                
            } catch (const Ort::Exception& e) {
                std::cout << "   ❌ Base model failed: " << e.what() << std::endl;
                std::cout << "   Error code: " << e.GetOrtErrorCode() << std::endl;
                std::cout << "   This might be due to model size or memory constraints" << std::endl;
                
                // Provide suggestions
                std::cout << "\n💡 Troubleshooting suggestions:" << std::endl;
                std::cout << "   1. Check available system memory" << std::endl;
                std::cout << "   2. Try quantized model versions" << std::endl;
                std::cout << "   3. Consider model loading in chunks" << std::endl;
                std::cout << "   4. Verify ONNX Runtime version compatibility" << std::endl;
            }
        } else {
            std::cout << "4. Base model not found" << std::endl;
        }
        
        std::cout << "\n=== Test completed ===" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cout << "❌ Fatal error: " << e.what() << std::endl;
        return 1;
    }
    #else
    std::cout << "ONNX Runtime not available at compile time" << std::endl;
    return 1;
    #endif
}
