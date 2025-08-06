//==============================================================================
// test_midi_model_proper.cpp
// Proper MIDI model loading based on ONNX Runtime C++ best practices
// and skytnt/midi-model architecture research
//==============================================================================

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <fstream>

#ifdef ONNX_RUNTIME_AVAILABLE
    #include <onnxruntime_cxx_api.h>
#endif

class ProperMIDIModelTest {
public:
    ProperMIDIModelTest() = default;
    
    bool testModelLoading(const std::string& modelPath) {
        #ifdef ONNX_RUNTIME_AVAILABLE
        try {
            std::cout << "=== Proper MIDI Model Loading Test ===" << std::endl;
            
            // 1. Initialize environment with explicit settings
            std::cout << "1. Initializing ONNX Runtime environment..." << std::endl;
            Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "MIDIModelTest");
            std::cout << "   ✓ Environment created" << std::endl;
            
            // 2. Create session options with safe defaults
            std::cout << "2. Creating session options..." << std::endl;
            Ort::SessionOptions sessionOptions;
            
            // Safe threading configuration
            sessionOptions.SetIntraOpNumThreads(1);  // Start with single thread
            sessionOptions.SetInterOpNumThreads(1);
            
            // Conservative memory settings
            sessionOptions.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_BASIC);
            
            // Disable caching for initial testing
            sessionOptions.DisableMemPattern();
            sessionOptions.DisableCpuMemArena();
            
            std::cout << "   ✓ Session options configured" << std::endl;
            
            // 3. Create session with better error handling
            std::cout << "3. Creating ONNX session..." << std::endl;
            std::cout << "   Loading model: " << modelPath << std::endl;
            
            // First, verify the file exists and get its size
            std::ifstream file(modelPath, std::ios::binary | std::ios::ate);
            if (!file.is_open()) {
                std::cout << "   ❌ Cannot open file: " << modelPath << std::endl;
                return false;
            }
            
            std::streamsize fileSize = file.tellg();
            file.close();
            std::cout << "   ✓ Model file exists, size: " << fileSize << " bytes" << std::endl;
            
            // Try to create session with additional safety
            std::cout << "   Creating session (this is where segfaults typically occur)..." << std::endl;
            
            // Use wide string on Windows, regular string on Unix
            #ifdef _WIN32
            std::wstring wideModelPath(modelPath.begin(), modelPath.end());
            Ort::Session session(env, wideModelPath.c_str(), sessionOptions);
            #else
            // Add more explicit error handling
            try {
                Ort::Session session(env, modelPath.c_str(), sessionOptions);
                std::cout << "   ✓ Session created successfully!" << std::endl;
                
                // Continue with inspection only after successful session creation
                // 4. Inspect model metadata safely
                std::cout << "4. Inspecting model metadata..." << std::endl;
                
                Ort::AllocatorWithDefaultOptions allocator;
                
                // Get input information
                size_t numInputs = session.GetInputCount();
                std::cout << "   Number of inputs: " << numInputs << std::endl;
                
                for (size_t i = 0; i < numInputs; i++) {
                    try {
                        auto inputName = session.GetInputNameAllocated(i, allocator);
                        std::cout << "   Input " << i << ": " << inputName.get() << std::endl;
                        
                        auto inputTypeInfo = session.GetInputTypeInfo(i);
                        auto inputTensorInfo = inputTypeInfo.GetTensorTypeAndShapeInfo();
                        
                        // Get element type
                        ONNXTensorElementDataType inputType = inputTensorInfo.GetElementType();
                        std::cout << "     Element type: " << (int)inputType << std::endl;
                        
                        // Get shape (handle dynamic dimensions safely)
                        auto shape = inputTensorInfo.GetShape();
                        std::cout << "     Shape: [";
                        for (size_t j = 0; j < shape.size(); j++) {
                            if (j > 0) std::cout << ", ";
                            if (shape[j] == -1) {
                                std::cout << "dynamic";
                            } else {
                                std::cout << shape[j];
                            }
                        }
                        std::cout << "]" << std::endl;
                        
                    } catch (const std::exception& e) {
                        std::cout << "   Error inspecting input " << i << ": " << e.what() << std::endl;
                    }
                }
                
                // Get output information
                size_t numOutputs = session.GetOutputCount();
                std::cout << "   Number of outputs: " << numOutputs << std::endl;
                
                for (size_t i = 0; i < numOutputs; i++) {
                    try {
                        auto outputName = session.GetOutputNameAllocated(i, allocator);
                        std::cout << "   Output " << i << ": " << outputName.get() << std::endl;
                        
                        auto outputTypeInfo = session.GetOutputTypeInfo(i);
                        auto outputTensorInfo = outputTypeInfo.GetTensorTypeAndShapeInfo();
                        
                        ONNXTensorElementDataType outputType = outputTensorInfo.GetElementType();
                        std::cout << "     Element type: " << (int)outputType << std::endl;
                        
                        auto shape = outputTensorInfo.GetShape();
                        std::cout << "     Shape: [";
                        for (size_t j = 0; j < shape.size(); j++) {
                            if (j > 0) std::cout << ", ";
                            if (shape[j] == -1) {
                                std::cout << "dynamic";
                            } else {
                                std::cout << shape[j];
                            }
                        }
                        std::cout << "]" << std::endl;
                        
                    } catch (const std::exception& e) {
                        std::cout << "   Error inspecting output " << i << ": " << e.what() << std::endl;
                    }
                }
                
                std::cout << "\n✅ Model inspection completed successfully!" << std::endl;
                std::cout << "   The model loads without segmentation faults." << std::endl;
                std::cout << "   Ready for proper input tensor creation and inference." << std::endl;
                
                return true;
                
            } catch (const Ort::Exception& ortEx) {
                std::cout << "   ❌ ONNX Session Creation Failed: " << ortEx.what() << std::endl;
                std::cout << "   Error Code: " << ortEx.GetOrtErrorCode() << std::endl;
                return false;
            } catch (const std::exception& stdEx) {
                std::cout << "   ❌ Session Creation Exception: " << stdEx.what() << std::endl;
                return false;
            }
            #endif
            
        } catch (const Ort::Exception& e) {
            std::cout << "❌ ONNX Runtime Exception: " << e.what() << std::endl;
            std::cout << "   Error Code: " << e.GetOrtErrorCode() << std::endl;
            return false;
        } catch (const std::exception& e) {
            std::cout << "❌ Standard Exception: " << e.what() << std::endl;
            return false;
        } catch (...) {
            std::cout << "❌ Unknown exception occurred" << std::endl;
            return false;
        }
        #else
        std::cout << "❌ ONNX Runtime not available at compile time" << std::endl;
        return false;
        #endif
    }
    
    void demonstrateProperTensorCreation() {
        #ifdef ONNX_RUNTIME_AVAILABLE
        std::cout << "\n=== Proper Tensor Creation Example ===" << std::endl;
        std::cout << "Based on skytnt/midi-model architecture research:" << std::endl;
        std::cout << "- Model expects: (batch_size, midi_sequence_length, token_sequence_length)" << std::endl;
        std::cout << "- Typical dimensions: [1, variable_length, max_token_seq]" << std::endl;
        std::cout << "- Token sequence contains MIDI events and parameters" << std::endl;
        std::cout << "- Use dynamic shapes with proper memory allocation" << std::endl;
        
        // Example of proper input tensor creation (commented for safety)
        /*
        try {
            Ort::MemoryInfo memoryInfo = Ort::MemoryInfo::CreateCpu(
                OrtAllocatorType::OrtArenaAllocator, 
                OrtMemType::OrtMemTypeDefault
            );
            
            // Example dimensions based on model architecture
            std::vector<int64_t> inputShape = {1, 8, 512};  // batch=1, seq=8, tokens=512
            size_t inputTensorSize = 1 * 8 * 512;
            
            std::vector<int64_t> inputData(inputTensorSize, 0);  // Initialize with zeros/pad tokens
            
            auto inputTensor = Ort::Value::CreateTensor<int64_t>(
                memoryInfo, 
                inputData.data(), 
                inputTensorSize,
                inputShape.data(), 
                inputShape.size()
            );
            
            std::cout << "✓ Example tensor created successfully" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Tensor creation example error: " << e.what() << std::endl;
        }
        */
        #endif
    }
};

int main() {
    ProperMIDIModelTest test;
    
    // Test with the actual model file
    std::string modelPath = "models/midi-model/onnx/model_base.onnx";
    
    bool success = test.testModelLoading(modelPath);
    
    if (success) {
        test.demonstrateProperTensorCreation();
        std::cout << "\n🎵 Next steps:" << std::endl;
        std::cout << "1. Implement proper MIDI tokenizer integration" << std::endl;
        std::cout << "2. Create input tensors with correct MIDI token sequences" << std::endl;
        std::cout << "3. Handle dynamic sequence lengths properly" << std::endl;
        std::cout << "4. Implement post-processing for MIDI output" << std::endl;
    }
    
    return success ? 0 : 1;
}
