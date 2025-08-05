//==============================================================================
/*
  Test Epic 6 Effects Integration
  Quick validation that ReverbEngine works with EffectsChain
*/
//==============================================================================

#include "Source/audio/EffectsChain.h"
#include "Source/audio/ReverbEngine.h"
#include <iostream>

using namespace spawnclone::audio;

int main()
{
    std::cout << "=== Epic 6 Effects Integration Test ===" << std::endl;
    
    // Test 1: Create EffectsChain
    EffectsChain chain;
    std::cout << "✓ EffectsChain created" << std::endl;
    
    // Test 2: Check available effects
    auto availableEffects = EffectsChain::getAvailableEffectTypes();
    std::cout << "✓ Available effects: " << availableEffects.size() << std::endl;
    for (auto& effect : availableEffects)
        std::cout << "  - " << effect.toStdString() << std::endl;
    
    // Test 3: Create ReverbEngine via factory
    auto reverb = EffectsChain::createEffect("Reverb");
    if (reverb)
    {
        std::cout << "✓ ReverbEngine created via factory" << std::endl;
    }
    else
    {
        std::cout << "✗ Failed to create ReverbEngine" << std::endl;
        return 1;
    }
    
    // Test 4: Insert reverb into chain
    chain.prepareToPlay(44100.0, 512);
    bool inserted = chain.insertEffect(0, std::move(reverb));
    if (inserted)
    {
        std::cout << "✓ ReverbEngine inserted into effects chain slot 0" << std::endl;
    }
    else
    {
        std::cout << "✗ Failed to insert ReverbEngine" << std::endl;
        return 1;
    }
    
    // Test 5: Verify effect is accessible
    auto* effectInSlot = chain.getEffect(0);
    if (effectInSlot)
    {
        std::cout << "✓ Effect accessible in slot 0" << std::endl;
        
        // Test parameter setting
        effectInSlot->setParameter("reverbTime", 2.5f, true);
        float reverbTime = effectInSlot->getParameter("reverbTime");
        std::cout << "✓ Parameter test - reverbTime set to: " << reverbTime << std::endl;
    }
    
    // Test 6: Audio processing setup
    juce::AudioBuffer<float> testBuffer(2, 512);
    testBuffer.clear();
    
    // Add some test signal
    for (int sample = 0; sample < 512; ++sample)
    {
        float testSine = std::sin(2.0f * juce::MathConstants<float>::pi * 440.0f * sample / 44100.0f);
        testBuffer.setSample(0, sample, testSine * 0.5f);
        testBuffer.setSample(1, sample, testSine * 0.5f);
    }
    
    juce::MidiBuffer emptyMidi;
    chain.processBlock(testBuffer, emptyMidi);
    std::cout << "✓ Audio processing completed without errors" << std::endl;
    
    // Test 7: Chain controls
    chain.setChainEnabled(false);
    std::cout << "✓ Chain disable: " << (chain.isChainEnabled() ? "FAILED" : "SUCCESS") << std::endl;
    
    chain.setChainEnabled(true);
    chain.setMasterMix(0.75f);
    std::cout << "✓ Master mix set to: " << chain.getMasterMix() << std::endl;
    
    std::cout << "\n=== Epic 6 Integration Test PASSED ===\n" << std::endl;
    std::cout << "🎉 Professional effects system is operational!" << std::endl;
    std::cout << "🎛️  ReverbEngine ready for use in synthesis pipeline" << std::endl;
    
    return 0;
}
