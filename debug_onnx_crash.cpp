//==============================================================================
// debug_onnx_crash.cpp
// Minimal test to debug ONNX Runtime crash
//==============================================================================

#include <iostream>
#include <onnxruntime_cxx_api.h>

int main()
{
    std::cout << "=== ONNX Runtime Crash Debugging ===" << std::endl;
    
    try {
        std::cout << "1. Creating environment with different log levels..." << std::endl;
        
        // Try with more verbose logging
        Ort::Env env(ORT_LOGGING_LEVEL_VERBOSE, "debug_test");
        std::cout << "   ✓ Environment created" << std::endl;
        
        std::cout << "2. Creating session options with careful configuration..." << std::endl;
        Ort::SessionOptions session_options;
        
        // Add some safety options
        session_options.SetIntraOpNumThreads(1);
        session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_DISABLE_ALL);
        std::cout << "   ✓ Session options configured" << std::endl;
        
        std::cout << "3. Attempting to load smaller model first..." << std::endl;
        std::string modelPath = "models/midi-model/onnx/model_token.onnx";
        
        // Use a try-catch specifically for the session creation
        std::cout << "   Creating session for: " << modelPath << std::endl;
        
        Ort::Session session(env, modelPath.c_str(), session_options);
        
        std::cout << "   ✓ SUCCESS! Model loaded without crash" << std::endl;
        
        // If we get here, try to get basic info
        size_t numInputs = session.GetInputCount();
        std::cout << "   Model has " << numInputs << " inputs" << std::endl;
        
    } catch (const Ort::Exception& e) {
        std::cout << "   ❌ ONNX Exception: " << e.what() << std::endl;
        std::cout << "   Error code: " << e.GetOrtErrorCode() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cout << "   ❌ Standard Exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cout << "   ❌ Unknown exception caught" << std::endl;
        return 1;
    }
    
    std::cout << "🎉 Test completed successfully!" << std::endl;
    return 0;
}
