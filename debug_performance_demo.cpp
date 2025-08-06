//==============================================================================
// debug_performance_demo.cpp
// Debug version of the performance demo to isolate the hanging issue
//==============================================================================

#include <iostream>
#include <chrono>
#include <vector>
#include "../Source/ai/ONNXModelManager.h"
#include "../Source/ai/PerformanceProfiler.h"
#include "../Source/GenerationParameters.h"

int main()
{
    std::cout << "=== Debug Performance Demo ===" << std::endl;
    
    try {
        std::cout << "1. Initializing components..." << std::endl;
        ONNXModelManager manager;
        auto& profiler = PerformanceProfiler::getInstance();
        profiler.enableDetailedLogging(true);
        profiler.reset();
        
        std::cout << "2. Loading model..." << std::endl;
        if (!manager.loadModel("simulation_model.onnx")) {
            std::cout << "Failed to load model: " << manager.getLastError().toStdString() << std::endl;
            return 1;
        }
        std::cout << "   ✓ Model loaded" << std::endl;
        
        std::cout << "3. Creating test parameters..." << std::endl;
        GenerationParameters params;
        params.key = 0;
        params.scale = GenerationParameters::ScaleType::Major;
        params.tempo = 120.0f;
        params.rhythmicComplexity = 0.5f;
        params.generationType = GenerationParameters::GenerationType::Melody;
        params.patternLengthBeats = 16.0f;
        params.generationSeed = 12345;
        
        std::cout << "4. Running 5 test inferences..." << std::endl;
        
        for (int i = 0; i < 5; ++i) {
            std::cout << "   Inference " << (i+1) << "/5..." << std::endl;
            
            auto start = std::chrono::high_resolution_clock::now();
            
            std::vector<uint8_t> pattern;
            bool success = manager.generatePattern(pattern, params);
            
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration<double, std::milli>(end - start).count();
            
            std::cout << "     Result: " << (success ? "SUCCESS" : "FAILED") << std::endl;
            std::cout << "     Time: " << duration << "ms" << std::endl;
            std::cout << "     Pattern size: " << pattern.size() << " bytes" << std::endl;
            
            // Small variation for next test
            params.generationSeed = (params.generationSeed + 1000) % 10000;
        }
        
        std::cout << "5. Getting performance metrics..." << std::endl;
        auto metrics = profiler.getCurrentMetrics();
        std::cout << "   Total inferences: " << metrics.totalInferences << std::endl;
        std::cout << "   Average time: " << metrics.averageInferenceTime << "ms" << std::endl;
        std::cout << "   Success rate: " << (metrics.successRate * 100) << "%" << std::endl;
        
        std::cout << "6. Testing performance mode changes..." << std::endl;
        manager.setPerformanceMode("fast");
        std::cout << "   ✓ Set to fast mode" << std::endl;
        
        manager.optimizeInferenceSpeed();
        std::cout << "   ✓ Speed optimization completed" << std::endl;
        
        std::cout << "7. Testing batch processing..." << std::endl;
        bool batchResult = manager.enableBatchProcessing(4);
        std::cout << "   Batch processing enabled: " << (batchResult ? "YES" : "NO") << std::endl;
        
        std::cout << "\n✅ Debug demo completed successfully!" << std::endl;
        return 0;
    }
    catch (const std::exception& e) {
        std::cout << "❌ Exception: " << e.what() << std::endl;
        return 1;
    }
}
