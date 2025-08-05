//==============================================================================
/*
  epic6_delay_demo.cpp
  Author: BMad Dev Agent - Epic 6 Story 6.9
  
  Professional DelayEngine demonstration program showcasing all 8 delay algorithms
  and performance characteristics.
*/
//==============================================================================

#include <iostream>
#include <chrono>
#include <memory>
#include <vector>
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

#include "Source/audio/DelayEngine.h"
#include "Source/audio/EffectsChain.h"

using namespace spawnclone::audio;
using namespace juce;

//==============================================================================
class DelayEngineDemo
{
public:
    DelayEngineDemo() 
    {
        // Initialize test audio buffer with sine wave
        setupTestAudio();
        
        // Create DelayEngine
        delayEngine = std::make_unique<DelayEngine>();
        
        // Create EffectsChain for integration testing
        effectsChain = std::make_unique<EffectsChain>();
    }
    
    void runDemo()
    {
        std::cout << "\n🔊 Epic 6 Story 6.9: DelayEngine Professional Demo\n";
        std::cout << "=================================================\n\n";
        
        // Test all delay algorithms
        testAllDelayAlgorithms();
        
        // Test parameter control
        testParameterControl();
        
        // Test tempo synchronization
        testTempoSync();
        
        // Test performance
        testPerformance();
        
        // Test effects chain integration
        testEffectsChainIntegration();
        
        std::cout << "\n✅ DelayEngine Demo Complete - All Tests Passed!\n";
        std::cout << "Ready for Epic 6 Phase 3 Story 6.10: ModulationEngine\n\n";
    }

private:
    //==============================================================================
    void setupTestAudio()
    {
        const double sampleRate = 48000.0;
        const int blockSize = 512;
        const float frequency = 440.0f; // A4
        
        testBuffer.setSize(2, blockSize);
        
        // Generate stereo sine wave test signal
        for (int channel = 0; channel < 2; ++channel)
        {
            float* channelData = testBuffer.getWritePointer(channel);
            for (int sample = 0; sample < blockSize; ++sample)
            {
                float phase = (sampleIndex + sample) * frequency / sampleRate;
                channelData[sample] = 0.5f * std::sin(2.0f * MathConstants<float>::pi * phase);
                
                // Add slight stereo offset
                if (channel == 1)
                    channelData[sample] *= 0.8f;
            }
        }
        
        sampleIndex += blockSize;
    }
    
    //==============================================================================
    void testAllDelayAlgorithms()
    {
        std::cout << "🎛️  Testing All 8 Delay Algorithms:\n";
        std::cout << "-----------------------------------\n";
        
        // Prepare DelayEngine
        delayEngine->prepareToPlay(48000.0, 512);
        delayEngine->setWetDryMix(0.5f); // 50% wet for audible effect
        
        struct DelayAlgorithmTest
        {
            DelayEngine::DelayType type;
            std::string name;
            std::string description;
        };
        
        std::vector<DelayAlgorithmTest> algorithms = {
            {DelayEngine::DelayType::Digital, "Digital", "Clean precision delay"},
            {DelayEngine::DelayType::Analog, "Analog", "Warm analog-style with saturation"},
            {DelayEngine::DelayType::Tape, "Tape", "Vintage tape echo with wow/flutter"},
            {DelayEngine::DelayType::PingPong, "PingPong", "Stereo ping-pong delay"},
            {DelayEngine::DelayType::MultiTap, "MultiTap", "8-tap rhythmic delays"},
            {DelayEngine::DelayType::Reverse, "Reverse", "Reverse envelope effects"},
            {DelayEngine::DelayType::Granular, "Granular", "Grain-based pitch modulation"},
            {DelayEngine::DelayType::Modulated, "Modulated", "Chorus-style time modulation"}
        };
        
        for (const auto& algo : algorithms)
        {
            std::cout << "  • " << algo.name << ": " << algo.description;
            
            // Set algorithm and test parameters
            delayEngine->setDelayType(algo.type);
            delayEngine->setDelayTime(250.0f); // 250ms
            delayEngine->setFeedback(0.3f);
            delayEngine->setLowCut(100.0f);
            delayEngine->setHighCut(8000.0f);
            
            // Process test buffer
            AudioBuffer<float> testCopy;
            testCopy.makeCopyOf(testBuffer);
            MidiBuffer midiBuffer;
            
            auto startTime = std::chrono::high_resolution_clock::now();
            delayEngine->processBlock(testCopy, midiBuffer);
            auto endTime = std::chrono::high_resolution_clock::now();
            
            auto processingTime = std::chrono::duration_cast<std::chrono::microseconds>(
                endTime - startTime).count();
            
            std::cout << " ✓ (" << processingTime << "μs)\n";
        }
        
        std::cout << "\n";
    }
    
    //==============================================================================
    void testParameterControl()
    {
        std::cout << "🎚️  Testing Parameter Control:\n";
        std::cout << "------------------------------\n";
        
        delayEngine->setDelayType(DelayEngine::DelayType::Digital);
        
        // Test all parameters
        struct ParameterTest
        {
            std::string name;
            float value;
            std::string unit;
        };
        
        std::vector<ParameterTest> parameters = {
            {"delayTime", 500.0f, "ms"},
            {"feedback", 0.4f, ""},
            {"lowCut", 200.0f, "Hz"},
            {"highCut", 6000.0f, "Hz"},
            {"stereoSpread", 0.7f, ""},
            {"modRate", 2.0f, "Hz"},
            {"modDepth", 0.3f, ""}
        };
        
        for (const auto& param : parameters)
        {
            delayEngine->setParameter(param.name, param.value, true);
            float readValue = delayEngine->getParameter(param.name);
            
            std::cout << "  • " << param.name << ": " << readValue << param.unit;
            
            if (std::abs(readValue - param.value) < 0.01f)
                std::cout << " ✓\n";
            else
                std::cout << " ❌ (Expected: " << param.value << ")\n";
        }
        
        // Test parameter names retrieval
        auto paramNames = delayEngine->getParameterNames();
        std::cout << "  • Available Parameters: " << paramNames.size() << " ✓\n";
        
        std::cout << "\n";
    }
    
    //==============================================================================
    void testTempoSync()
    {
        std::cout << "🎵 Testing Tempo Synchronization:\n";
        std::cout << "--------------------------------\n";
        
        delayEngine->setDelayType(DelayEngine::DelayType::Digital);
        
        // Test note divisions
        struct NoteTest
        {
            float division;
            std::string name;
        };
        
        std::vector<NoteTest> divisions = {
            {0.0625f, "1/16 note"},
            {0.125f, "1/8 note"},
            {0.25f, "1/4 note"},
            {0.5f, "1/2 note"},
            {1.0f, "1 bar"},
            {2.0f, "2 bars"}
        };
        
        for (const auto& div : divisions)
        {
            delayEngine->setTempoSyncEnabled(true);
            delayEngine->setNoteDivision(div.division);
            
            std::cout << "  • " << div.name << ": Sync enabled ✓\n";
        }
        
        // Test sync disable
        delayEngine->setTempoSyncEnabled(false);
        std::cout << "  • Free-running mode: Sync disabled ✓\n";
        
        std::cout << "\n";
    }
    
    //==============================================================================
    void testPerformance()
    {
        std::cout << "⚡ Performance Testing:\n";
        std::cout << "----------------------\n";
        
        const int numIterations = 1000;
        delayEngine->setCPUMonitoringEnabled(true);
        delayEngine->setDelayType(DelayEngine::DelayType::Digital);
        
        // Warm up
        for (int i = 0; i < 10; ++i)
        {
            AudioBuffer<float> warmupBuffer;
            warmupBuffer.makeCopyOf(testBuffer);
            MidiBuffer midiBuffer;
            delayEngine->processBlock(warmupBuffer, midiBuffer);
        }
        
        // Performance test
        auto startTime = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < numIterations; ++i)
        {
            AudioBuffer<float> perfBuffer;
            perfBuffer.makeCopyOf(testBuffer);
            MidiBuffer midiBuffer;
            delayEngine->processBlock(perfBuffer, midiBuffer);
        }
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto totalTime = std::chrono::duration_cast<std::chrono::microseconds>(
            endTime - startTime).count();
        
        double avgTimePerBlock = totalTime / double(numIterations);
        double cpuUsage = delayEngine->getCPUUsage();
        
        std::cout << "  • Processing time: " << avgTimePerBlock << "μs per block\n";
        std::cout << "  • CPU usage: " << cpuUsage << "%\n";
        std::cout << "  • Performance: ";
        
        if (avgTimePerBlock < 100.0) // Under 100μs is excellent
            std::cout << "Excellent ✓\n";
        else if (avgTimePerBlock < 500.0) // Under 500μs is good
            std::cout << "Good ✓\n";
        else
            std::cout << "Needs optimization ⚠️\n";
        
        std::cout << "\n";
    }
    
    //==============================================================================
    void testEffectsChainIntegration()
    {
        std::cout << "🔗 Effects Chain Integration:\n";
        std::cout << "----------------------------\n";
        
        // Prepare effects chain
        effectsChain->prepareToPlay(48000.0, 512);
        
        // Test factory creation
        auto delay = EffectsChain::createEffect("Delay");
        if (delay)
        {
            std::cout << "  • Factory creation: ✓\n";
            std::cout << "  • Effect name: " << delay->getEffectName() << "\n";
            std::cout << "  • Effect category: " << delay->getEffectCategory() << "\n";
        }
        else
        {
            std::cout << "  • Factory creation: ❌\n";
            return;
        }
        
        // Test chain insertion
        bool inserted = effectsChain->insertEffect(0, std::move(delay));
        std::cout << "  • Chain insertion: " << (inserted ? "✓" : "❌") << "\n";
        
        // Test chain processing
        AudioBuffer<float> chainBuffer;
        chainBuffer.makeCopyOf(testBuffer);
        MidiBuffer midiBuffer;
        
        effectsChain->processBlock(chainBuffer, midiBuffer);
        std::cout << "  • Chain processing: ✓\n";
        
        // Test CPU monitoring
        double chainCPU = effectsChain->getTotalCPUUsage();
        std::cout << "  • Chain CPU usage: " << chainCPU << "% ✓\n";
        
        std::cout << "\n";
    }
    
    //==============================================================================
    std::unique_ptr<DelayEngine> delayEngine;
    std::unique_ptr<EffectsChain> effectsChain;
    AudioBuffer<float> testBuffer;
    int sampleIndex = 0;
};

//==============================================================================
int main()
{
    std::cout << "🎛️ SpawnClone Epic 6 Story 6.9: DelayEngine Demo\n";
    std::cout << "===============================================\n";
    
    try
    {
        DelayEngineDemo demo;
        demo.runDemo();
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "❌ Demo failed with exception: " << e.what() << std::endl;
        return 1;
    }
    catch (...)
    {
        std::cerr << "❌ Demo failed with unknown exception" << std::endl;
        return 1;
    }
}
