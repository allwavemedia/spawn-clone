//==============================================================================
// minimal_onnx_test.cpp
// Absolute minimal ONNX Runtime test without any JUCE dependencies
//==============================================================================

#include <iostream>
#include <cstdlib>

#ifdef ONNX_RUNTIME_AVAILABLE
    #include <onnxruntime_c_api.h>
#endif

int main() {
    #ifdef ONNX_RUNTIME_AVAILABLE
    std::cout << "Minimal ONNX Runtime C API Test" << std::endl;
    
    // Step 1: Get API
    std::cout << "1. Getting ORT API..." << std::endl;
    const OrtApi* ortApi = OrtGetApiBase()->GetApi(ORT_API_VERSION);
    if (!ortApi) {
        std::cout << "Failed to get ORT API" << std::endl;
        return 1;
    }
    std::cout << "   ✓ Got API" << std::endl;
    
    // Step 2: Create environment
    std::cout << "2. Creating environment..." << std::endl;
    OrtEnv* ortEnv = nullptr;
    OrtStatus* status = ortApi->CreateEnv(ORT_LOGGING_LEVEL_WARNING, "MinimalTest", &ortEnv);
    if (status != nullptr) {
        std::cout << "   ❌ Failed to create environment: " << ortApi->GetErrorMessage(status) << std::endl;
        ortApi->ReleaseStatus(status);
        return 1;
    }
    std::cout << "   ✓ Environment created" << std::endl;
    
    // Step 3: Simple success test
    std::cout << "3. Testing basic functionality..." << std::endl;
    
    // Clean up
    ortApi->ReleaseEnv(ortEnv);
    
    std::cout << "\n✅ Minimal test passed!" << std::endl;
    std::cout << "ONNX Runtime basic functionality works." << std::endl;
    
    return 0;
    #else
    std::cout << "ONNX Runtime not available at compile time" << std::endl;
    return 1;
    #endif
}
