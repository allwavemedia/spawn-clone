/*
  Epic 9.2 Story 9.2.3: Velocity Layer Debug Demo
  
  This program helps debug velocity layer selection issues.
*/

#include <iostream>
#include <memory>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "Source/audio/AdvancedSynthesisEngine.h"

using namespace spawnclone::audio;

float getBufferRMS(const juce::AudioBuffer<float>& buffer)
{
    if (buffer.getNumSamples() == 0) return 0.0f;
    
    float sum = 0.0f;
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            float val = buffer.getSample(channel, sample);
            sum += val * val;
        }
    }
    return std::sqrt(sum / (buffer.getNumChannels() * buffer.getNumSamples()));
}

void fillBuffer(juce::AudioBuffer<float>& buffer, float value)
{
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            buffer.setSample(channel, sample, value);
        }
    }
}

int main()
{
    std::cout << "=== Epic 9.2 Story 9.2.3: Velocity Layer Debug Demo ===" << std::endl;
    
    // Initialize JUCE
    juce::MessageManager::getInstance();
    
    // Create synthesis engine
    auto engine = std::make_unique<AdvancedSynthesisEngine>();
    engine->prepareToPlay(44100.0, 512, 2);
    
    // Create test samples
    juce::AudioBuffer<float> soft(1, 1024), loud(1, 1024);
    fillBuffer(soft, 0.25f);  // Soft sample
    fillBuffer(loud, 0.75f);  // Loud sample
    
    std::cout << "✅ Created samples: soft=0.25, loud=0.75" << std::endl;
    std::cout << "✅ Soft sample RMS: " << getBufferRMS(soft) << std::endl;
    std::cout << "✅ Loud sample RMS: " << getBufferRMS(loud) << std::endl;
    
    // Load samples
    engine->loadSample(0, soft);
    engine->loadSample(1, loud);
    
    // Setup velocity layer parameters
    AdvancedSynthesisEngine::SynthesisParameters params;
    params.synthesisType = AdvancedSynthesisEngine::SynthesisType::Sample;
    params.sample.sampleMap.clear();
    params.sample.sampleMap.push_back({0, 0, 64});     // Soft sample: velocity 0-64
    params.sample.sampleMap.push_back({1, 65, 127});   // Loud sample: velocity 65-127
    params.sample.rootNote = 60.0f;    // Ensure no pitch shifting
    params.sample.playbackSpeed = 1.0f; // Ensure no speed change
    params.masterVolume = 1.0f;
    
    // Set envelope for testing (no release to avoid fade-out during test)
    params.envelope.attack = 0.001f;   // 1ms attack
    params.envelope.decay = 0.0f;      // No decay
    params.envelope.sustain = 1.0f;    // Full sustain
    params.envelope.release = 0.001f;  // Very fast release (1ms)
    
    engine->setSynthesisParameters(params);
    
    std::cout << "✅ Configured velocity layers: 0-64=soft, 65-127=loud" << std::endl;
    
    // Test low velocity (should trigger soft sample)
    std::cout << "\n🧪 Testing low velocity (0.4 = vel 51)..." << std::endl;
    juce::AudioBuffer<float> audioBuffer(2, 512);
    juce::MidiBuffer midiBuffer;
    
    audioBuffer.clear();
    engine->noteOn(60, 0.4f);  // Velocity 51
    
    // Check how many samples have non-zero values
    int nonZeroSamples = 0;
    for (int i = 0; i < audioBuffer.getNumSamples(); ++i)
    {
        if (std::abs(audioBuffer.getSample(0, i)) > 0.0001f)
            nonZeroSamples++;
    }
    std::cout << "🔍 Before processing: " << nonZeroSamples << " non-zero samples" << std::endl;
    
    engine->processBlock(audioBuffer, midiBuffer);
    engine->noteOff(60);
    
    // Check again after processing
    nonZeroSamples = 0;
    float maxSample = 0.0f;
    float minSample = 1000.0f;
    float totalSquared = 0.0f;
    
    for (int i = 0; i < audioBuffer.getNumSamples(); ++i)
    {
        float sample = audioBuffer.getSample(0, i);
        float absSample = std::abs(sample);
        totalSquared += sample * sample;
        
        if (absSample > 0.0001f)
            nonZeroSamples++;
        maxSample = std::max(maxSample, absSample);
        minSample = std::min(minSample, absSample);
    }
    
    float manualRMS = std::sqrt(totalSquared / audioBuffer.getNumSamples());
    
    std::cout << "🔍 After processing: " << nonZeroSamples << " non-zero samples" << std::endl;
    std::cout << "🔍 Max: " << maxSample << ", Min: " << minSample << std::endl;
    std::cout << "🔍 Manual RMS: " << manualRMS << std::endl;
    
    float lowVelRMS = getBufferRMS(audioBuffer);
    std::cout << "📊 Low velocity RMS: " << lowVelRMS << " (expected: 0.25)" << std::endl;
    std::cout << "📊 Difference: " << std::abs(lowVelRMS - 0.25f) << std::endl;
    
    // Test high velocity (should trigger loud sample)
    std::cout << "\n🧪 Testing high velocity (0.8 = vel 101)..." << std::endl;
    
    audioBuffer.clear();
    engine->noteOn(60, 0.8f);  // Velocity 101
    engine->processBlock(audioBuffer, midiBuffer);
    engine->noteOff(60);
    
    float highVelRMS = getBufferRMS(audioBuffer);
    std::cout << "📊 High velocity RMS: " << highVelRMS << " (expected: 0.75)" << std::endl;
    std::cout << "📊 Difference: " << std::abs(highVelRMS - 0.75f) << std::endl;
    
    // Results analysis
    std::cout << "\n📈 ANALYSIS:" << std::endl;
    bool lowVelPass = std::abs(lowVelRMS - 0.25f) < 0.01f;
    bool highVelPass = std::abs(highVelRMS - 0.75f) < 0.01f;
    
    std::cout << "✓ Low velocity test: " << (lowVelPass ? "PASS" : "FAIL") << std::endl;
    std::cout << "✓ High velocity test: " << (highVelPass ? "PASS" : "FAIL") << std::endl;
    
    if (lowVelPass && highVelPass)
    {
        std::cout << "\n🎉 Epic 9.2 Story 9.2.3 Velocity Layer Selection: SUCCESS!" << std::endl;
    }
    else
    {
        std::cout << "\n❌ Issues found - debugging needed" << std::endl;
    }
    
    return 0;
}
