//==============================================================================
/*
  epic6_xy_demo.cpp
  Epic 6 Phase 2 Demo - Professional Reverb XY Controller Integration
  
  Demonstrates the ReverbXYController integration with ExperimentPad
*/
//==============================================================================

#include "Source/ui/ReverbXYController.h"
#include "Source/audio/EffectsChain.h"
#include "Source/audio/ReverbEngine.h"
#include <iostream>
#include <iomanip>

using namespace spawnclone::ui;
using namespace spawnclone::audio;

class XYReverbDemo
{
public:
    XYReverbDemo()
    {
        std::cout << "🎛️  Epic 6 XY Reverb Controller Demo" << std::endl;
        setupEffectsChain();
        setupXYController();
    }
    
    void runDemo()
    {
        std::cout << "\n=== Epic 6 Professional Reverb XY Control Demo ===" << std::endl;
        
        demonstrateMappingPresets();
        demonstrateXYControl();
        demonstrateModifierKeys();
        demonstrateRealtimePerformance();
        
        std::cout << "\n🎉 Epic 6 XY Integration Complete!" << std::endl;
        std::cout << "✨ Professional reverb now controllable via intuitive XY interface!" << std::endl;
    }

private:
    std::unique_ptr<EffectsChain> effectsChain;
    std::unique_ptr<ReverbXYController> xyController;
    
    void setupEffectsChain()
    {
        effectsChain = std::make_unique<EffectsChain>();
        effectsChain->prepareToPlay(44100.0, 512);
        
        // Insert reverb into slot 0
        auto reverb = EffectsChain::createEffect("Reverb");
        if (reverb)
        {
            effectsChain->insertEffect(0, std::move(reverb));
            std::cout << "✓ Professional ReverbEngine created and inserted" << std::endl;
        }
    }
    
    void setupXYController()
    {
        xyController = std::make_unique<ReverbXYController>();
        xyController->setEffectsChain(effectsChain.get(), 0);
        
        // Set up state change callback for demo
        xyController->onStateChanged = [this](const ReverbXYController::ReverbState& state)
        {
            displayCurrentState(state);
        };
        
        std::cout << "✓ XY Controller configured and connected to effects chain" << std::endl;
    }
    
    void demonstrateMappingPresets()
    {
        std::cout << "\n--- 1. Mapping Preset Demonstration ---" << std::endl;
        
        std::vector<std::pair<std::string, std::string>> presets = {
            {"Studio", "Professional mixing-friendly ranges"},
            {"Performance", "Wide dramatic ranges for live use"},
            {"Creative", "Extreme ranges for sound design"},
            {"Vintage", "Classic reverb character"}
        };
        
        for (const auto& [presetName, description] : presets)
        {
            std::cout << "\n🎵 " << presetName << " Preset: " << description << std::endl;
            
            xyController->loadPresetMapping(juce::String(presetName));
            
            // Test center position for each preset
            xyController->updateFromXYPosition(0.5f, 0.5f);
            
            std::cout << "   Center position (0.5, 0.5) results:" << std::endl;
            auto state = xyController->getCurrentState();
            std::cout << "   • Room Size: " << std::fixed << std::setprecision(2) << state.roomSize << std::endl;
            std::cout << "   • Reverb Time: " << state.reverbTime << "s" << std::endl;
            std::cout << "   • Damping: " << state.damping << std::endl;
        }
    }
    
    void demonstrateXYControl()
    {
        std::cout << "\n--- 2. XY Position Control Demonstration ---" << std::endl;
        
        xyController->loadStudioMapping(); // Use studio preset for demonstration
        
        std::vector<std::pair<float, float>> positions = {
            {0.0f, 0.0f},   // Bottom-left: Small room, short reverb
            {1.0f, 0.0f},   // Bottom-right: Large room, short reverb  
            {0.0f, 1.0f},   // Top-left: Small room, long reverb
            {1.0f, 1.0f},   // Top-right: Large room, long reverb
            {0.5f, 0.5f}    // Center: Balanced settings
        };
        
        for (const auto& [x, y] : positions)
        {
            std::cout << "\n🎛️  XY Position (" << x << ", " << y << "):" << std::endl;
            xyController->updateFromXYPosition(x, y);
            
            auto state = xyController->getCurrentState();
            std::cout << "   Result: Room " << std::fixed << std::setprecision(2) << state.roomSize 
                     << ", Time " << state.reverbTime << "s" << std::endl;
        }
    }
    
    void demonstrateModifierKeys()
    {
        std::cout << "\n--- 3. Modifier Key Control Demonstration ---" << std::endl;
        
        std::cout << "\n🎹 Normal XY Control (Room Size + Reverb Time):" << std::endl;
        xyController->updateFromXYPosition(0.7f, 0.3f, false, false, false);
        
        std::cout << "\n⇧ Shift + XY Control (Damping + Stereo Width):" << std::endl;
        xyController->updateFromXYPosition(0.3f, 0.8f, true, false, false);
        
        std::cout << "\n⌃ Ctrl + XY Control (Early Reflections + Pre-delay):" << std::endl;
        xyController->updateFromXYPosition(0.6f, 0.4f, false, true, false);
        
        std::cout << "\n✓ Advanced parameter control via modifier keys demonstrated" << std::endl;
    }
    
    void demonstrateRealtimePerformance()
    {
        std::cout << "\n--- 4. Real-time Performance Test ---" << std::endl;
        
        auto startTime = juce::Time::getHighResolutionTicks();
        
        // Simulate rapid XY movements
        const int numUpdates = 1000;
        for (int i = 0; i < numUpdates; ++i)
        {
            float x = 0.5f + 0.4f * std::sin(i * 0.1f);
            float y = 0.5f + 0.3f * std::cos(i * 0.07f);
            
            xyController->updateFromXYPosition(x, y);
        }
        
        auto endTime = juce::Time::getHighResolutionTicks();
        double processingTime = juce::Time::highResolutionTicksToSeconds(endTime - startTime);
        
        std::cout << "⚡ Performance Results:" << std::endl;
        std::cout << "   • " << numUpdates << " parameter updates" << std::endl;
        std::cout << "   • Total time: " << (processingTime * 1000.0) << "ms" << std::endl;
        std::cout << "   • Average per update: " << (processingTime * 1000000.0 / numUpdates) << "μs" << std::endl;
        std::cout << "   • Real-time safe: " << (processingTime < 0.01 ? "YES ✓" : "NEEDS OPTIMIZATION") << std::endl;
    }
    
    void displayCurrentState(const ReverbXYController::ReverbState& state)
    {
        // Only display for specific calls to avoid spam during performance test
        static int displayCounter = 0;
        if (++displayCounter % 100 == 1) // Display every 100th update during performance test
        {
            std::cout << "   State: Room=" << std::fixed << std::setprecision(2) << state.roomSize 
                     << " Time=" << state.reverbTime << "s"
                     << " Damp=" << state.damping 
                     << " Width=" << state.stereoWidth << std::endl;
        }
    }
};

int main()
{
    try
    {
        XYReverbDemo demo;
        demo.runDemo();
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Demo failed: " << e.what() << std::endl;
        return 1;
    }
}
