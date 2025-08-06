//==============================================================================
// test_model_loading_no_juce.cpp
// Test ONNX model loading without JUCE to isolate JUCE interaction issues
//==============================================================================

#include <iostream>
#include <string>

#ifdef ONNX_RUNTIME_AVAILABLE
    #include <onnxruntime_c_api.h>
#endif

int main() {
    #ifdef ONNX_RUNTIME_AVAILABLE
    std::cout << "=== ONNX Model Loading Test (No JUCE) ===" << std::endl;
    
    const OrtApi* ortApi = OrtGetApiBase()->GetApi(ORT_API_VERSION);
    
    // Step 1: Create environment
    std::cout << "1. Creating environment..." << std::endl;
    OrtEnv* ortEnv = nullptr;
    OrtStatus* status = ortApi->CreateEnv(ORT_LOGGING_LEVEL_WARNING, "ModelTest", &ortEnv);
    if (status != nullptr) {
        std::cout << "   ❌ Failed: " << ortApi->GetErrorMessage(status) << std::endl;
        ortApi->ReleaseStatus(status);
        return 1;
    }
    std::cout << "   ✓ Environment created" << std::endl;
    
    // Step 2: Create session options
    std::cout << "2. Creating session options..." << std::endl;
    OrtSessionOptions* sessionOptions = nullptr;
    status = ortApi->CreateSessionOptions(&sessionOptions);
    if (status != nullptr) {
        std::cout << "   ❌ Failed: " << ortApi->GetErrorMessage(status) << std::endl;
        ortApi->ReleaseStatus(status);
        ortApi->ReleaseEnv(ortEnv);
        return 1;
    }
    
    // Set conservative options
    status = ortApi->SetIntraOpNumThreads(sessionOptions, 1);
    if (status) { ortApi->ReleaseStatus(status); }
    
    status = ortApi->SetInterOpNumThreads(sessionOptions, 1);
    if (status) { ortApi->ReleaseStatus(status); }
    
    status = ortApi->SetSessionGraphOptimizationLevel(sessionOptions, ORT_DISABLE_ALL);
    if (status) { ortApi->ReleaseStatus(status); }
    
    std::cout << "   ✓ Session options created" << std::endl;
    
    // Step 3: Try loading the simple model
    std::cout << "3. Loading simple model..." << std::endl;
    std::string modelPath = "test_simple_compatible.onnx";
    
    OrtSession* session = nullptr;
    status = ortApi->CreateSession(ortEnv, modelPath.c_str(), sessionOptions, &session);
    if (status != nullptr) {
        std::cout << "   ❌ Failed to load simple model: " << ortApi->GetErrorMessage(status) << std::endl;
        ortApi->ReleaseStatus(status);
        ortApi->ReleaseSessionOptions(sessionOptions);
        ortApi->ReleaseEnv(ortEnv);
        return 1;
    }
    std::cout << "   ✅ Simple model loaded successfully!" << std::endl;
    
    // Get model info
    size_t numInputs = 0;
    size_t numOutputs = 0;
    ortApi->SessionGetInputCount(session, &numInputs);
    ortApi->SessionGetOutputCount(session, &numOutputs);
    std::cout << "   Inputs: " << numInputs << ", Outputs: " << numOutputs << std::endl;
    
    // Clean up simple model
    ortApi->ReleaseSession(session);
    
    // Step 4: Try loading the MIDI token model
    std::cout << "4. Loading MIDI token model..." << std::endl;
    std::string midiModelPath = "models/midi-model/onnx/model_token.onnx";
    
    OrtSession* midiSession = nullptr;
    status = ortApi->CreateSession(ortEnv, midiModelPath.c_str(), sessionOptions, &midiSession);
    if (status != nullptr) {
        std::cout << "   ⚠️  MIDI model failed: " << ortApi->GetErrorMessage(status) << std::endl;
        ortApi->ReleaseStatus(status);
    } else {
        std::cout << "   ✅ MIDI token model loaded successfully!" << std::endl;
        
        size_t midiInputs = 0;
        size_t midiOutputs = 0;
        ortApi->SessionGetInputCount(midiSession, &midiInputs);
        ortApi->SessionGetOutputCount(midiSession, &midiOutputs);
        std::cout << "   MIDI Inputs: " << midiInputs << ", Outputs: " << midiOutputs << std::endl;
        
        ortApi->ReleaseSession(midiSession);
    }
    
    // Clean up
    ortApi->ReleaseSessionOptions(sessionOptions);
    ortApi->ReleaseEnv(ortEnv);
    
    std::cout << "\n✅ Test completed without JUCE interference!" << std::endl;
    
    return 0;
    #else
    std::cout << "ONNX Runtime not available" << std::endl;
    return 1;
    #endif
}
