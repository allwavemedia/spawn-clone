//==============================================================================
// test_simple_model.cpp
// Test with an extremely simple synthetic ONNX model to isolate the issue
//==============================================================================

#include <iostream>

#ifdef ONNX_RUNTIME_AVAILABLE
    #include <onnxruntime_cxx_api.h>
#endif

int main() {
    #ifdef ONNX_RUNTIME_AVAILABLE
    std::cout << "=== Testing Simple Synthetic Model ===" << std::endl;
    
    try {
        std::cout << "1. Creating environment..." << std::endl;
        Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "SimpleModelTest");
        std::cout << "   ✓ Environment created" << std::endl;
        
        std::cout << "2. Setting up session options..." << std::endl;
        Ort::SessionOptions sessionOptions;
        sessionOptions.SetIntraOpNumThreads(1);
        sessionOptions.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_DISABLE_ALL);
        std::cout << "   ✓ Session options configured" << std::endl;
        
        std::cout << "3. Loading simple synthetic model..." << std::endl;
        std::string modelPath = "test_simple_compatible.onnx";
        
        std::cout << "   Creating session..." << std::endl;
        Ort::Session session(env, modelPath.c_str(), sessionOptions);
        std::cout << "   ✅ Simple model loaded successfully!" << std::endl;
        
        // Get basic info
        size_t numInputs = session.GetInputCount();
        size_t numOutputs = session.GetOutputCount();
        std::cout << "   Inputs: " << numInputs << ", Outputs: " << numOutputs << std::endl;
        
        std::cout << "\n✅ ONNX Runtime is working correctly!" << std::endl;
        std::cout << "The issue is specific to the MIDI models, not ONNX Runtime itself." << std::endl;
        
        return 0;
        
    } catch (const Ort::Exception& e) {
        std::cout << "❌ ONNX Exception: " << e.what() << std::endl;
        std::cout << "   Error Code: " << e.GetOrtErrorCode() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cout << "❌ Standard Exception: " << e.what() << std::endl;
        return 1;
    }
    #else
    std::cout << "ONNX Runtime not available" << std::endl;
    return 1;
    #endif
}
