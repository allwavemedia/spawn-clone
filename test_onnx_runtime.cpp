//==============================================================================
// test_onnx_runtime.cpp
// Basic ONNX Runtime integration test
//==============================================================================

#include <iostream>
#include <onnxruntime_cxx_api.h>

int main()
{
    std::cout << "Testing basic ONNX Runtime initialization..." << std::endl;
    
    try {
        std::cout << "1. Creating ONNX Runtime environment..." << std::endl;
        Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "test");
        std::cout << "   ✓ Environment created" << std::endl;
        
        std::cout << "2. Creating session options..." << std::endl;
        Ort::SessionOptions session_options;
        std::cout << "   ✓ Session options created" << std::endl;
        
        std::cout << "3. Testing with a real model file..." << std::endl;
        std::string modelPath = "models/midi-model/onnx/model_base.onnx";
        std::cout << "   Model path: " << modelPath << std::endl;
        
        try {
            Ort::Session session(env, modelPath.c_str(), session_options);
            std::cout << "   ✓ Model loaded successfully!" << std::endl;
            
            // Get model info
            Ort::AllocatorWithDefaultOptions allocator;
            size_t numInputs = session.GetInputCount();
            size_t numOutputs = session.GetOutputCount();
            
            std::cout << "   Model inputs: " << numInputs << std::endl;
            std::cout << "   Model outputs: " << numOutputs << std::endl;
            
            // Print input names and shapes
            for (size_t i = 0; i < numInputs; i++) {
                auto inputName = session.GetInputNameAllocated(i, allocator);
                std::cout << "   Input " << i << ": " << inputName.get() << std::endl;
                
                auto inputTypeInfo = session.GetInputTypeInfo(i);
                auto inputTensorInfo = inputTypeInfo.GetTensorTypeAndShapeInfo();
                auto inputShape = inputTensorInfo.GetShape();
                
                std::cout << "     Shape: [";
                for (size_t j = 0; j < inputShape.size(); j++) {
                    if (j > 0) std::cout << ", ";
                    std::cout << inputShape[j];
                }
                std::cout << "]" << std::endl;
            }
            
            std::cout << std::endl << "🎉 ONNX Runtime integration successful!" << std::endl;
            
        } catch (const Ort::Exception& e) {
            std::cout << "   ❌ Model loading failed: " << e.what() << std::endl;
            return 1;
        }
        
    } catch (const std::exception& e) {
        std::cout << "   ❌ ONNX Runtime initialization failed: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
