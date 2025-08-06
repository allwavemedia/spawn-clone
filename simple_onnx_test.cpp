//==============================================================================
// simple_onnx_test.cpp
// Simple test to debug ONNX performance optimization issues
//==============================================================================

#include <iostream>
#include "../Source/ai/ONNXModelManager.h"
#include "../Source/ai/PerformanceProfiler.h"
#include "../Source/GenerationParameters.h"

int main()
{
    std::cout << "Starting simple ONNX test..." << std::endl;
    
    try {
        std::cout << "1. Creating ONNXModelManager..." << std::endl;
        ONNXModelManager manager;
        std::cout << "   ✓ ONNXModelManager created" << std::endl;
        
        std::cout << "2. Loading simulation model..." << std::endl;
        bool loaded = manager.loadModel("test_model.onnx");
        std::cout << "   Model loaded: " << (loaded ? "YES" : "NO") << std::endl;
        if (!loaded) {
            std::cout << "   Error: " << manager.getLastError().toStdString() << std::endl;
        }
        
        std::cout << "3. Checking model status..." << std::endl;
        std::cout << "   Is loaded: " << (manager.isModelLoaded() ? "YES" : "NO") << std::endl;
        
        if (manager.isModelLoaded()) {
            std::cout << "4. Creating test parameters..." << std::endl;
            GenerationParameters params;
            params.key = 0;
            params.scale = GenerationParameters::ScaleType::Major;
            params.tempo = 120.0f;
            params.rhythmicComplexity = 0.5f;
            params.generationType = GenerationParameters::GenerationType::Melody;
            params.patternLengthBeats = 16.0f;
            params.generationSeed = 12345;
            std::cout << "   ✓ Parameters created" << std::endl;
            
            std::cout << "5. Testing single pattern generation..." << std::endl;
            std::vector<uint8_t> pattern;
            bool success = manager.generatePattern(pattern, params);
            std::cout << "   Generation success: " << (success ? "YES" : "NO") << std::endl;
            if (success) {
                std::cout << "   Generated pattern size: " << pattern.size() << " bytes" << std::endl;
            } else {
                std::cout << "   Error: " << manager.getLastError().toStdString() << std::endl;
            }
        }
        
        std::cout << "6. Performance profiler test..." << std::endl;
        auto& profiler = PerformanceProfiler::getInstance();
        auto metrics = profiler.getCurrentMetrics();
        std::cout << "   Total inferences: " << metrics.totalInferences << std::endl;
        
        std::cout << "\n✅ Simple test completed successfully!" << std::endl;
        return 0;
    }
    catch (const std::exception& e) {
        std::cout << "❌ Exception: " << e.what() << std::endl;
        return 1;
    }
    catch (...) {
        std::cout << "❌ Unknown exception occurred" << std::endl;
        return 1;
    }
}
