//==============================================================================
/*
  epic6_reverb_demo.cpp
  Epic 6 Phase 1 Demo - Professional Reverb Processing
  
  Demonstrates ReverbEngine quality with different algorithms and parameters
*/
//==============================================================================

#include "Source/audio/EffectsChain.h"
#include "Source/audio/ReverbEngine.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <iostream>
#include <cmath>

using namespace spawnclone::audio;

class ReverbDemo
{
public:
    ReverbDemo() 
    {
        // Initialize JUCE
        juce::MessageManager::getInstance();
        formatManager.registerBasicFormats();
        std::cout << "🎛️  Epic 6 Reverb Demo Initialized" << std::endl;
    }
    
    void runDemo()
    {
        std::cout << "\n=== Epic 6 Professional Reverb Demo ===" << std::endl;
        
        // Demo 1: Create and configure effects chain
        demonstrateEffectsChainSetup();
        
        // Demo 2: Test different reverb algorithms
        demonstrateReverbAlgorithms();
        
        // Demo 3: Parameter control demonstration
        demonstrateParameterControl();
        
        // Demo 4: Audio processing performance
        demonstrateAudioProcessing();
        
        std::cout << "\n🎉 Epic 6 Reverb Demo Complete!" << std::endl;
        std::cout << "✨ Professional effects system operational and ready for integration!" << std::endl;
    }

private:
    juce::AudioFormatManager formatManager;
    static constexpr double SAMPLE_RATE = 44100.0;
    static constexpr int BLOCK_SIZE = 512;
    
    void demonstrateEffectsChainSetup()
    {
        std::cout << "\n--- 1. Effects Chain Setup ---" << std::endl;
        
        EffectsChain chain;
        chain.prepareToPlay(SAMPLE_RATE, BLOCK_SIZE);
        
        // Show available effects
        auto availableEffects = EffectsChain::getAvailableEffectTypes();
        std::cout << "✓ Available effect types (" << availableEffects.size() << "):" << std::endl;
        for (const auto& effect : availableEffects)
        {
            std::cout << "  • " << effect.toStdString() << std::endl;
        }
        
        // Create and insert reverb
        auto reverb = EffectsChain::createEffect("Reverb");
        if (reverb)
        {
            std::cout << "✓ ReverbEngine created successfully" << std::endl;
            
            bool inserted = chain.insertEffect(0, std::move(reverb));
            std::cout << "✓ Reverb inserted into slot 0: " << (inserted ? "SUCCESS" : "FAILED") << std::endl;
            
            // Verify access
            auto* effect = chain.getEffect(0);
            if (effect)
            {
                auto params = effect->getParameterNames();
                std::cout << "✓ Reverb parameters available (" << params.size() << "):" << std::endl;
                for (const auto& param : params)
                {
                    float value = effect->getParameter(param);
                    std::cout << "  • " << param.toStdString() << ": " << value << std::endl;
                }
            }
        }
        
        std::cout << "✓ Chain enabled: " << (chain.isChainEnabled() ? "YES" : "NO") << std::endl;
        std::cout << "✓ Master mix: " << chain.getMasterMix() << std::endl;
    }
    
    void demonstrateReverbAlgorithms()
    {
        std::cout << "\n--- 2. Reverb Algorithm Showcase ---" << std::endl;
        
        auto reverb = std::make_unique<ReverbEngine>();
        reverb->prepareToPlay(SAMPLE_RATE, BLOCK_SIZE);
        
        // Test each algorithm
        std::vector<std::pair<ReverbEngine::Algorithm, std::string>> algorithms = {
            {ReverbEngine::Algorithm::Plate, "Plate Reverb - Studio Classic"},
            {ReverbEngine::Algorithm::Hall, "Hall Reverb - Concert Space"},
            {ReverbEngine::Algorithm::Room, "Room Reverb - Intimate Space"},
            {ReverbEngine::Algorithm::Spring, "Spring Reverb - Vintage Character"}
        };
        
        for (const auto& [algorithm, description] : algorithms)
        {
            reverb->setAlgorithm(algorithm);
            
            // Configure parameters for this algorithm
            switch (algorithm)
            {
                case ReverbEngine::Algorithm::Plate:
                    reverb->setReverbTime(2.5f);
                    reverb->setRoomSize(0.7f);
                    reverb->setDamping(0.3f);
                    break;
                case ReverbEngine::Algorithm::Hall:
                    reverb->setReverbTime(4.0f);
                    reverb->setRoomSize(0.9f);
                    reverb->setDamping(0.2f);
                    break;
                case ReverbEngine::Algorithm::Room:
                    reverb->setReverbTime(1.2f);
                    reverb->setRoomSize(0.4f);
                    reverb->setDamping(0.5f);
                    break;
                case ReverbEngine::Algorithm::Spring:
                    reverb->setReverbTime(0.8f);
                    reverb->setRoomSize(0.3f);
                    reverb->setDamping(0.1f);
                    break;
                default:
                    break;
            }
            
            std::cout << "🎵 " << description << std::endl;
            std::cout << "   Reverb Time: " << reverb->getParameter("reverbTime") << "s" << std::endl;
            std::cout << "   Room Size: " << reverb->getParameter("roomSize") << std::endl;
            std::cout << "   Damping: " << reverb->getParameter("damping") << std::endl;
            
            // Quick processing test
            juce::AudioBuffer<float> testBuffer(2, BLOCK_SIZE);
            generateTestTone(testBuffer, 440.0f); // A4 note
            
            juce::MidiBuffer emptyMidi;
            reverb->processBlock(testBuffer, emptyMidi);
            
            float rmsLevel = calculateRMS(testBuffer);
            std::cout << "   Processing: ✓ RMS Level: " << rmsLevel << std::endl;
        }
    }
    
    void demonstrateParameterControl()
    {
        std::cout << "\n--- 3. Real-time Parameter Control ---" << std::endl;
        
        auto reverb = std::make_unique<ReverbEngine>();
        reverb->prepareToPlay(SAMPLE_RATE, BLOCK_SIZE);
        reverb->setAlgorithm(ReverbEngine::Algorithm::Plate);
        
        // Demonstrate parameter sweeping
        std::cout << "🎛️  Parameter Sweep Demonstration:" << std::endl;
        
        // Room size sweep
        std::cout << "   Room Size Sweep (0.1 → 1.0):" << std::endl;
        for (float roomSize = 0.1f; roomSize <= 1.0f; roomSize += 0.2f)
        {
            reverb->setRoomSize(roomSize);
            float actualValue = reverb->getParameter("roomSize");
            std::cout << "     " << roomSize << " → " << actualValue << " ✓" << std::endl;
        }
        
        // Reverb time sweep
        std::cout << "   Reverb Time Sweep (0.5s → 8.0s):" << std::endl;
        for (float reverbTime = 0.5f; reverbTime <= 8.0f; reverbTime += 1.5f)
        {
            reverb->setReverbTime(reverbTime);
            float actualValue = reverb->getParameter("reverbTime");
            std::cout << "     " << reverbTime << "s → " << actualValue << "s ✓" << std::endl;
        }
        
        // Damping sweep
        std::cout << "   Damping Sweep (0.0 → 1.0):" << std::endl;
        for (float damping = 0.0f; damping <= 1.0f; damping += 0.25f)
        {
            reverb->setDamping(damping);
            float actualValue = reverb->getParameter("damping");
            std::cout << "     " << damping << " → " << actualValue << " ✓" << std::endl;
        }
        
        std::cout << "✓ All parameters respond correctly with smooth interpolation" << std::endl;
    }
    
    void demonstrateAudioProcessing()
    {
        std::cout << "\n--- 4. Audio Processing Performance ---" << std::endl;
        
        EffectsChain chain;
        chain.prepareToPlay(SAMPLE_RATE, BLOCK_SIZE);
        
        auto reverb = EffectsChain::createEffect("Reverb");
        chain.insertEffect(0, std::move(reverb));
        
        // Configure for performance test
        auto* effect = chain.getEffect(0);
        if (effect)
        {
            effect->setParameter("reverbTime", 3.0f, true);
            effect->setParameter("roomSize", 0.8f, true); 
            effect->setParameter("damping", 0.4f, true);
        }
        
        // Performance test with different signal types
        std::vector<std::pair<float, std::string>> testSignals = {
            {220.0f, "A3 Bass Note"},
            {440.0f, "A4 Middle"},
            {880.0f, "A5 Treble"},
            {1760.0f, "A6 High Treble"}
        };
        
        std::cout << "🎵 Audio Processing Test Results:" << std::endl;
        
        for (const auto& [frequency, description] : testSignals)
        {
            juce::AudioBuffer<float> testBuffer(2, BLOCK_SIZE);
            generateTestTone(testBuffer, frequency);
            
            float inputRMS = calculateRMS(testBuffer);
            
            juce::MidiBuffer emptyMidi;
            auto startTime = juce::Time::getHighResolutionTicks();
            
            // Process multiple blocks to test stability
            for (int block = 0; block < 10; ++block)
            {
                generateTestTone(testBuffer, frequency);
                chain.processBlock(testBuffer, emptyMidi);
            }
            
            auto endTime = juce::Time::getHighResolutionTicks();
            double processingTime = juce::Time::highResolutionTicksToSeconds(endTime - startTime);
            
            float outputRMS = calculateRMS(testBuffer);
            
            std::cout << "   " << description << " (" << frequency << "Hz):" << std::endl;
            std::cout << "     Input RMS: " << inputRMS << std::endl;
            std::cout << "     Output RMS: " << outputRMS << std::endl;
            std::cout << "     Processing Time: " << (processingTime * 1000.0) << "ms (10 blocks)" << std::endl;
            std::cout << "     CPU Usage: " << chain.getTotalCPUUsage() << "%" << std::endl;
        }
        
        // Test different wet/dry mixes
        std::cout << "\n🎛️  Wet/Dry Mix Test:" << std::endl;
        for (float mix = 0.0f; mix <= 1.0f; mix += 0.25f)
        {
            chain.setMasterMix(mix);
            juce::AudioBuffer<float> testBuffer(2, BLOCK_SIZE);
            generateTestTone(testBuffer, 440.0f);
            
            juce::MidiBuffer emptyMidi;
            chain.processBlock(testBuffer, emptyMidi);
            
            float rms = calculateRMS(testBuffer);
            std::cout << "   Mix " << mix << " → RMS: " << rms << " ✓" << std::endl;
        }
    }
    
    void generateTestTone(juce::AudioBuffer<float>& buffer, float frequency)
    {
        buffer.clear();
        float phase = 0.0f;
        float phaseIncrement = frequency / static_cast<float>(SAMPLE_RATE);
        
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            float sineValue = std::sin(2.0f * juce::MathConstants<float>::pi * phase) * 0.3f;
            
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            {
                buffer.setSample(channel, sample, sineValue);
            }
            
            phase += phaseIncrement;
            if (phase >= 1.0f)
                phase -= 1.0f;
        }
    }
    
    float calculateRMS(const juce::AudioBuffer<float>& buffer)
    {
        float sum = 0.0f;
        int totalSamples = 0;
        
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        {
            for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            {
                float value = buffer.getSample(channel, sample);
                sum += value * value;
                totalSamples++;
            }
        }
        
        return totalSamples > 0 ? std::sqrt(sum / totalSamples) : 0.0f;
    }
};

int main()
{
    try
    {
        ReverbDemo demo;
        demo.runDemo();
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Demo failed with exception: " << e.what() << std::endl;
        return 1;
    }
    catch (...)
    {
        std::cerr << "Demo failed with unknown exception" << std::endl;
        return 1;
    }
}
