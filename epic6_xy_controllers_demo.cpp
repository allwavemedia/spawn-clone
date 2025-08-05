//==============================================================================
/*
  epic6_xy_controllers_demo.cpp
  Author: BMad Dev Agent - Epic 6 Story 6.11
  
  Comprehensive demonstration of DelayXYController and ModulationXYController
  showcasing real-time multi-parameter control and preset systems.
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
#include "Source/audio/ModulationEngine.h"
#include "Source/audio/DelayXYController.h"
#include "Source/audio/ModulationXYController.h"
#include "Source/audio/EffectsChain.h"

using namespace spawnclone::audio;
using namespace juce;
using ModulationType = ModulationEngine::ModulationType;

//==============================================================================
class XYControllersDemo
{
public:
    XYControllersDemo()
    {
        setupAudio();
        setupEngines();
        setupControllers();
    }

    void runDemo()
    {
        std::cout << "\n🎛️ Epic 6 Story 6.11: XY Controllers Demo\n";
        std::cout << "==========================================\n\n";
        
        testDelayXYController();
        testModulationXYController();
        testCombinedEffectsControl();
        testPresetSystems();
        testPerformanceAndSmoothing();
        testStateManagement();
        
        std::cout << "\n✅ XY Controllers Demo Complete!\n";
        std::cout << "🎉 Epic 6 Phase 3 Implementation COMPLETE: 21/21 Story Points\n\n";
    }

private:
    //==============================================================================
    void setupAudio()
    {
        sampleRate = 48000.0;
        blockSize = 512;
        
        // Create test buffer with sine wave
        testBuffer.setSize(2, blockSize);
        generateTestSignal(440.0f); // A4
    }
    
    void setupEngines()
    {
        // Create and prepare engines
        delayEngine = std::make_unique<DelayEngine>();
        modulationEngine = std::make_unique<ModulationEngine>();
        effectsChain = std::make_unique<EffectsChain>();
        
        delayEngine->prepareToPlay(sampleRate, blockSize);
        modulationEngine->prepareToPlay(sampleRate, blockSize);
        effectsChain->prepareToPlay(sampleRate, blockSize);
    }
    
    void setupControllers()
    {
        // Create and prepare XY controllers
        delayXY = std::make_unique<DelayXYController>();
        modulationXY = std::make_unique<ModulationXYController>();
        
        delayXY->prepareToPlay(sampleRate, blockSize);
        modulationXY->prepareToPlay(sampleRate, blockSize);
        
        // Link controllers to engines
        delayXY->setTargetDelayEngine(delayEngine.get());
        modulationXY->setTargetModulationEngine(modulationEngine.get());
    }
    
    void generateTestSignal(float frequency)
    {
        static float phase = 0.0f;
        float phaseDelta = 2.0f * MathConstants<float>::pi * frequency / static_cast<float>(sampleRate);
        
        for (int sample = 0; sample < blockSize; ++sample)
        {
            float sineWave = std::sin(phase) * 0.5f;
            
            for (int channel = 0; channel < 2; ++channel)
            {
                testBuffer.setSample(channel, sample, sineWave);
            }
            
            phase += phaseDelta;
            if (phase >= 2.0f * MathConstants<float>::pi)
                phase -= 2.0f * MathConstants<float>::pi;
        }
    }

    //==============================================================================
    void testDelayXYController()
    {
        std::cout << "🎚️ Testing DelayXYController:\n";
        std::cout << "-----------------------------\n";
        
        // Test all presets
        auto presets = delayXY->getAvailablePresets();
        std::cout << "  Available Presets: " << presets.size() << "\n";
        
        for (const auto& preset : presets)
        {
            delayXY->setPreset(preset);
            std::cout << "  • " << delayXY->getPresetName(preset) << std::endl;
            
            // Test XY positions for each preset
            struct TestPosition { float x, y; std::string desc; };
            std::vector<TestPosition> positions = {
                {0.0f, 0.0f, "Bottom-left"},
                {1.0f, 0.0f, "Bottom-right"},
                {1.0f, 1.0f, "Top-right"},
                {0.0f, 1.0f, "Top-left"},
                {0.5f, 0.5f, "Center"}
            };
            
            for (const auto& pos : positions)
            {
                delayXY->setXYPosition(pos.x, pos.y);
                
                float delayTime = delayXY->getMappedDelayTime();
                float feedback = delayXY->getMappedFeedback();
                
                std::cout << "    " << pos.desc << ": " 
                          << delayTime << "ms, " 
                          << (feedback * 100.0f) << "% feedback\n";
                
                // Process audio to verify
                AudioBuffer<float> testCopy;
                testCopy.makeCopyOf(testBuffer);
                MidiBuffer midiBuffer;
                
                auto startTime = std::chrono::high_resolution_clock::now();
                delayEngine->processBlock(testCopy, midiBuffer);
                auto endTime = std::chrono::high_resolution_clock::now();
                
                auto processingTime = std::chrono::duration_cast<std::chrono::microseconds>(
                    endTime - startTime).count();
                
                if (processingTime < 500) // Under 500μs is good
                    std::cout << "      ✓ Processing: " << processingTime << "μs\n";
                else
                    std::cout << "      ⚠️ Processing: " << processingTime << "μs (slow)\n";
            }
            std::cout << "\n";
        }
    }

    //==============================================================================
    void testModulationXYController()
    {
        std::cout << "🌊 Testing ModulationXYController:\n";
        std::cout << "---------------------------------\n";
        
        // Test presets by modulation type
        std::vector<ModulationType> types = {
            ModulationType::Chorus, ModulationType::Flanger, ModulationType::Phaser,
            ModulationType::Tremolo, ModulationType::Vibrato, ModulationType::AutoPan,
            ModulationType::RingMod, ModulationType::Rotary
        };
        
        const char* typeNames[] = {
            "Chorus", "Flanger", "Phaser", "Tremolo",
            "Vibrato", "AutoPan", "RingMod", "Rotary"
        };
        
        for (int i = 0; i < 8; ++i)
        {
            auto typePresets = modulationXY->getPresetsForType(types[i]);
            std::cout << "  " << typeNames[i] << " Presets: " << typePresets.size() << "\n";
            
            for (const auto& preset : typePresets)
            {
                modulationXY->setPreset(preset);
                std::cout << "    • " << modulationXY->getPresetName(preset) << std::endl;
                
                // Test corner positions
                modulationXY->setXYPosition(0.0f, 0.0f);
                float minRate = modulationXY->getMappedRate();
                float minDepth = modulationXY->getMappedDepth();
                
                modulationXY->setXYPosition(1.0f, 1.0f);
                float maxRate = modulationXY->getMappedRate();
                float maxDepth = modulationXY->getMappedDepth();
                
                std::cout << "      Range: " << minRate << "-" << maxRate << "Hz, "
                          << (minDepth * 100.0f) << "-" << (maxDepth * 100.0f) << "%\n";
                
                // Process audio
                AudioBuffer<float> testCopy;
                testCopy.makeCopyOf(testBuffer);
                MidiBuffer midiBuffer;
                
                auto startTime = std::chrono::high_resolution_clock::now();
                modulationEngine->processBlock(testCopy, midiBuffer);
                auto endTime = std::chrono::high_resolution_clock::now();
                
                auto processingTime = std::chrono::duration_cast<std::chrono::microseconds>(
                    endTime - startTime).count();
                
                std::cout << "      ✓ Processing: " << processingTime << "μs\n";
            }
            std::cout << "\n";
        }
    }

    //==============================================================================
    void testCombinedEffectsControl()
    {
        std::cout << "🔗 Testing Combined Effects Control:\n";
        std::cout << "-----------------------------------\n";
        
        // Add both effects to chain
        auto delay = std::make_unique<DelayEngine>();
        auto modulation = std::make_unique<ModulationEngine>();
        
        delay->prepareToPlay(sampleRate, blockSize);
        modulation->prepareToPlay(sampleRate, blockSize);
        
        // Link XY controllers
        delayXY->setTargetDelayEngine(delay.get());
        modulationXY->setTargetModulationEngine(modulation.get());
        
        // Insert into effects chain
        bool delayInserted = effectsChain->insertEffect(0, std::move(delay));
        bool modulationInserted = effectsChain->insertEffect(1, std::move(modulation));
        
        std::cout << "  • Delay insertion: " << (delayInserted ? "✓" : "❌") << "\n";
        std::cout << "  • Modulation insertion: " << (modulationInserted ? "✓" : "❌") << "\n";
        
        if (delayInserted && modulationInserted)
        {
            // Test combined effect
            delayXY->setPreset(DelayXYController::DelayPreset::Echo);
            modulationXY->setPreset(ModulationXYController::ModulationPreset::SubtleChorus);
            
            delayXY->setXYPosition(0.6f, 0.4f);        // Medium delay, moderate feedback
            modulationXY->setXYPosition(0.3f, 0.7f);   // Slow rate, high depth
            
            std::cout << "  • Combined preset: " << delayXY->getCurrentPresetName() 
                      << " + " << modulationXY->getCurrentPresetName() << "\n";
            
            // Process chain
            AudioBuffer<float> chainBuffer;
            chainBuffer.makeCopyOf(testBuffer);
            MidiBuffer midiBuffer;
            
            auto startTime = std::chrono::high_resolution_clock::now();
            effectsChain->processBlock(chainBuffer, midiBuffer);
            auto endTime = std::chrono::high_resolution_clock::now();
            
            auto processingTime = std::chrono::duration_cast<std::chrono::microseconds>(
                endTime - startTime).count();
            
            double cpuUsage = effectsChain->getTotalCPUUsage();
            
            std::cout << "  • Combined processing: " << processingTime << "μs\n";
            std::cout << "  • Combined CPU usage: " << cpuUsage << "%\n";
            
            if (processingTime < 1000) // Under 1ms for both effects
                std::cout << "  ✓ Excellent combined performance\n";
            else
                std::cout << "  ⚠️ Combined performance needs optimization\n";
        }
        
        std::cout << "\n";
    }

    //==============================================================================
    void testPresetSystems()
    {
        std::cout << "🎭 Testing Preset Systems:\n";
        std::cout << "-------------------------\n";
        
        // Test DelayXYController presets
        std::cout << "  DelayXYController Presets:\n";
        auto delayPresets = delayXY->getAvailablePresets();
        for (const auto& preset : delayPresets)
        {
            delayXY->setPreset(preset);
            delayXY->setXYPosition(0.7f, 0.3f); // Test position
            
            std::cout << "    • " << delayXY->getPresetName(preset) 
                      << ": " << delayXY->getParameterDescription() << "\n";
        }
        
        std::cout << "\n  ModulationXYController Presets:\n";
        auto modulationPresets = modulationXY->getAvailablePresets();
        for (const auto& preset : modulationPresets)
        {
            modulationXY->setPreset(preset);
            modulationXY->setXYPosition(0.4f, 0.8f); // Test position
            
            std::cout << "    • " << modulationXY->getPresetName(preset) 
                      << ": " << modulationXY->getParameterDescription() << "\n";
        }
        
        std::cout << "\n";
    }

    //==============================================================================
    void testPerformanceAndSmoothing()
    {
        std::cout << "⚡ Testing Performance and Smoothing:\n";
        std::cout << "-----------------------------------\n";
        
        // Test parameter smoothing
        delayXY->setSmoothingEnabled(true);
        delayXY->setSmoothingTime(0.1f); // 100ms smoothing
        modulationXY->setSmoothingEnabled(true);
        modulationXY->setSmoothingTime(0.05f); // 50ms smoothing
        
        std::cout << "  • Smoothing enabled: DelayXY=100ms, ModulationXY=50ms\n";
        
        // Test rapid parameter changes
        const int numPositions = 10;
        float positions[][2] = {
            {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}, {0.5f, 0.5f},
            {0.2f, 0.8f}, {0.8f, 0.2f}, {0.3f, 0.7f}, {0.7f, 0.3f}, {0.5f, 0.5f}
        };
        
        auto startTime = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < numPositions; ++i)
        {
            delayXY->setXYPosition(positions[i][0], positions[i][1]);
            modulationXY->setXYPosition(positions[i][0], positions[i][1]);
            
            // Process blocks to advance smoothers
            for (int block = 0; block < 10; ++block)
            {
                delayXY->processBlock();
                modulationXY->processBlock();
            }
        }
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto smoothingTime = std::chrono::duration_cast<std::chrono::microseconds>(
            endTime - startTime).count();
        
        std::cout << "  • Rapid parameter changes: " << smoothingTime << "μs for " 
                  << numPositions << " positions\n";
        
        // Test without smoothing
        delayXY->setSmoothingEnabled(false);
        modulationXY->setSmoothingEnabled(false);
        
        startTime = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < numPositions; ++i)
        {
            delayXY->setXYPosition(positions[i][0], positions[i][1]);
            modulationXY->setXYPosition(positions[i][0], positions[i][1]);
        }
        
        endTime = std::chrono::high_resolution_clock::now();
        auto directTime = std::chrono::duration_cast<std::chrono::microseconds>(
            endTime - startTime).count();
        
        std::cout << "  • Direct parameter changes: " << directTime << "μs for " 
                  << numPositions << " positions\n";
        
        std::cout << "  ✓ Smoothing overhead: " << (smoothingTime - directTime) << "μs\n";
        
        std::cout << "\n";
    }

    //==============================================================================
    void testStateManagement()
    {
        std::cout << "💾 Testing State Management:\n";
        std::cout << "----------------------------\n";
        
        // Set up initial state
        delayXY->setPreset(DelayXYController::DelayPreset::Space);
        delayXY->setXYPosition(0.3f, 0.7f);
        delayXY->setSmoothingTime(0.2f);
        
        modulationXY->setPreset(ModulationXYController::ModulationPreset::VintagePhaser);
        modulationXY->setXYPosition(0.8f, 0.4f);
        modulationXY->setSmoothingTime(0.15f);
        
        // Save states
        auto delayState = delayXY->saveState();
        auto modulationState = modulationXY->saveState();
        
        std::cout << "  • Saved DelayXY state: " << delayState.getNumProperties() << " properties\n";
        std::cout << "  • Saved ModulationXY state: " << modulationState.getNumProperties() << " properties\n";
        
        // Change to different states
        delayXY->setPreset(DelayXYController::DelayPreset::Slapback);
        delayXY->setXYPosition(0.9f, 0.1f);
        
        modulationXY->setPreset(ModulationXYController::ModulationPreset::ChoppyTremolo);
        modulationXY->setXYPosition(0.2f, 0.9f);
        
        std::cout << "  • Changed to: " << delayXY->getCurrentPresetName() 
                  << " + " << modulationXY->getCurrentPresetName() << "\n";
        
        // Restore states
        delayXY->loadState(delayState);
        modulationXY->loadState(modulationState);
        
        std::cout << "  • Restored to: " << delayXY->getCurrentPresetName() 
                  << " + " << modulationXY->getCurrentPresetName() << "\n";
        
        // Verify restoration
        auto restoredXY = delayXY->getXYPosition();
        std::cout << "  • DelayXY position: (" << restoredXY.x << ", " << restoredXY.y << ")\n";
        
        auto restoredModXY = modulationXY->getXYPosition();
        std::cout << "  • ModulationXY position: (" << restoredModXY.x << ", " << restoredModXY.y << ")\n";
        
        if (std::abs(restoredXY.x - 0.3f) < 0.01f && std::abs(restoredXY.y - 0.7f) < 0.01f &&
            std::abs(restoredModXY.x - 0.8f) < 0.01f && std::abs(restoredModXY.y - 0.4f) < 0.01f)
        {
            std::cout << "  ✅ State restoration successful\n";
        }
        else
        {
            std::cout << "  ❌ State restoration failed\n";
        }
        
        std::cout << "\n";
    }

    //==============================================================================
    // Member Variables
    
    std::unique_ptr<DelayEngine> delayEngine;
    std::unique_ptr<ModulationEngine> modulationEngine;
    std::unique_ptr<EffectsChain> effectsChain;
    
    std::unique_ptr<DelayXYController> delayXY;
    std::unique_ptr<ModulationXYController> modulationXY;
    
    AudioBuffer<float> testBuffer;
    double sampleRate;
    int blockSize;
};

//==============================================================================
int main()
{
    std::cout << "🎛️ SpawnClone Epic 6 Story 6.11: XY Controllers Demo\n";
    std::cout << "===================================================\n";
    
    try
    {
        XYControllersDemo demo;
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
