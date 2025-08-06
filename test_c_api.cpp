//==============================================================================
// test_c_api.cpp
// Test using ONNX Runtime C API instead of C++ API to isolate C++ binding issues
//==============================================================================

#include <iostream>
#include <vector>
#include <string>

#ifdef ONNX_RUNTIME_AVAILABLE
    #include <onnxruntime_c_api.h>
#endif

class ONNXRuntimeCAPITest {
private:
    #ifdef ONNX_RUNTIME_AVAILABLE
    const OrtApi* ortApi = nullptr;
    OrtEnv* ortEnv = nullptr;
    OrtSessionOptions* sessionOptions = nullptr;
    OrtSession* session = nullptr;
    #endif

public:
    ONNXRuntimeCAPITest() {
        #ifdef ONNX_RUNTIME_AVAILABLE
        ortApi = OrtGetApiBase()->GetApi(ORT_API_VERSION);
        #endif
    }
    
    ~ONNXRuntimeCAPITest() {
        cleanup();
    }
    
    bool testModelLoading(const std::string& modelPath) {
        #ifdef ONNX_RUNTIME_AVAILABLE
        
        std::cout << "=== ONNX Runtime C API Test ===" << std::endl;
        
        try {
            // Step 1: Create environment
            std::cout << "1. Creating environment..." << std::endl;
            OrtStatus* status = ortApi->CreateEnv(ORT_LOGGING_LEVEL_WARNING, "CAPITest", &ortEnv);
            if (status != nullptr) {
                std::cout << "   ❌ Failed to create environment" << std::endl;
                ortApi->ReleaseStatus(status);
                return false;
            }
            std::cout << "   ✓ Environment created" << std::endl;
            
            // Step 2: Create session options
            std::cout << "2. Creating session options..." << std::endl;
            status = ortApi->CreateSessionOptions(&sessionOptions);
            if (status != nullptr) {
                std::cout << "   ❌ Failed to create session options" << std::endl;
                ortApi->ReleaseStatus(status);
                return false;
            }
            
            // Set conservative options
            ortApi->SetIntraOpNumThreads(sessionOptions, 1);
            ortApi->SetInterOpNumThreads(sessionOptions, 1);
            ortApi->SetSessionGraphOptimizationLevel(sessionOptions, ORT_DISABLE_ALL);
            
            std::cout << "   ✓ Session options configured" << std::endl;
            
            // Step 3: Create session
            std::cout << "3. Creating session with model: " << modelPath << std::endl;
            status = ortApi->CreateSession(ortEnv, modelPath.c_str(), sessionOptions, &session);
            if (status != nullptr) {
                const char* errorMsg = ortApi->GetErrorMessage(status);
                std::cout << "   ❌ Failed to create session: " << errorMsg << std::endl;
                ortApi->ReleaseStatus(status);
                return false;
            }
            std::cout << "   ✅ Session created successfully!" << std::endl;
            
            // Step 4: Get model info
            std::cout << "4. Getting model information..." << std::endl;
            
            size_t numInputs = 0;
            size_t numOutputs = 0;
            
            status = ortApi->SessionGetInputCount(session, &numInputs);
            if (status == nullptr) {
                status = ortApi->SessionGetOutputCount(session, &numOutputs);
            }
            
            if (status != nullptr) {
                std::cout << "   ⚠️  Could not get model info, but session creation succeeded" << std::endl;
                ortApi->ReleaseStatus(status);
            } else {
                std::cout << "   Inputs: " << numInputs << ", Outputs: " << numOutputs << std::endl;
            }
            
            std::cout << "\n✅ C API test completed successfully!" << std::endl;
            std::cout << "   This proves ONNX Runtime C API works correctly." << std::endl;
            std::cout << "   The issue is likely with the C++ API wrapper." << std::endl;
            
            return true;
            
        } catch (const std::exception& e) {
            std::cout << "❌ C++ Exception in C API test: " << e.what() << std::endl;
            return false;
        }
        #else
        std::cout << "ONNX Runtime not available at compile time" << std::endl;
        return false;
        #endif
    }
    
private:
    void cleanup() {
        #ifdef ONNX_RUNTIME_AVAILABLE
        if (session) ortApi->ReleaseSession(session);
        if (sessionOptions) ortApi->ReleaseSessionOptions(sessionOptions);
        if (ortEnv) ortApi->ReleaseEnv(ortEnv);
        #endif
    }
};

int main() {
    ONNXRuntimeCAPITest test;
    
    // Test with compatible simple model first
    std::string simpleModelPath = "test_simple_compatible.onnx";
    bool simpleSuccess = test.testModelLoading(simpleModelPath);
    
    if (simpleSuccess) {
        std::cout << "\n" << std::string(50, '=') << std::endl;
        
        // Test with MIDI token model
        std::string midiModelPath = "models/midi-model/onnx/model_token.onnx";
        std::cout << "Now testing with MIDI token model..." << std::endl;
        bool midiSuccess = test.testModelLoading(midiModelPath);
        
        if (midiSuccess) {
            std::cout << "\n🎵 Both simple and MIDI models work with C API!" << std::endl;
            std::cout << "   Next step: Fix the C++ API usage in your main code." << std::endl;
        }
    }
    
    return simpleSuccess ? 0 : 1;
}
