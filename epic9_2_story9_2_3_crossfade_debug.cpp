//==============================================================================
/*
  Epic 9.2 Story 9.2.3: Crossfade Debug Program
  Author:  BMad Dev Agent
*/
//==============================================================================

#include <iostream>
#include <iomanip>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include "audio/AdvancedSynthesisEngine.h"

using namespace spawnclone::audio;

void fillBuffer(juce::AudioBuffer<float>& buffer, float value)
{
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* ptr = buffer.getWritePointer(ch);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
            ptr[i] = value;
    }
}

float getBufferRMS(const juce::AudioBuffer<float>& buffer)
{
    return buffer.getRMSLevel(0, 0, buffer.getNumSamples());
}

int main()
{
    std::cout << "=== Epic 9.2 Story 9.2.3: Crossfade Debug Program ===" << std::endl;
    
    // Create engine
    auto engine = std::make_unique<AdvancedSynthesisEngine>();
    engine->prepareToPlay(44100.0, 512, 2);
    
    // Create test samples
    juce::AudioBuffer<float> soft(1, 1024), loud(1, 1024);
    fillBuffer(soft, 0.2f);
    fillBuffer(loud, 0.8f);
    
    engine->loadSample(0, soft);
    engine->loadSample(1, loud);
    
    // Setup parameters
    AdvancedSynthesisEngine::SynthesisParameters params;
    params.synthesisType = AdvancedSynthesisEngine::SynthesisType::Sample;
    params.sample.sampleMap.clear();
    params.sample.sampleMap.push_back({0, 0, 60});    // Soft sample 0-60
    params.sample.sampleMap.push_back({1, 70, 127});  // Loud sample 70-127
    params.sample.velocityCrossfadeWidth = 10.0f;
    params.masterVolume = 1.0f;
    params.envelope.attack = 0.001f;
    params.envelope.decay = 0.0f;
    params.envelope.sustain = 1.0f;
    params.envelope.release = 0.001f;
    engine->setSynthesisParameters(params);
    
    juce::AudioBuffer<float> audioBuffer(2, 512);
    juce::MidiBuffer midiBuffer;
    
    std::cout << "\nTesting velocity crossfade..." << std::endl;
    std::cout << "Soft sample (0-60): 0.2 RMS" << std::endl;
    std::cout << "Loud sample (70-127): 0.8 RMS" << std::endl;
    std::cout << "Crossfade width: 10" << std::endl;
    std::cout << "Gap between layers: 61-69" << std::endl;
    std::cout << std::endl;
    
    // Test various velocities
    std::vector<float> testVelocities = {0.4f, 0.47f, 0.511f, 0.55f, 0.6f};
    
    for (float vel : testVelocities)
    {
        int midiVel = static_cast<int>(vel * 127.0f);
        
        audioBuffer.clear();
        engine->noteOn(60, vel);
        engine->processBlock(audioBuffer, midiBuffer);
        engine->noteOff(60);
        
        float rms = getBufferRMS(audioBuffer);
        
        std::cout << "Velocity: " << std::fixed << std::setprecision(3) << vel 
                  << " (" << midiVel << ") -> RMS: " << rms << std::endl;
    }
    
    std::cout << std::endl;
    std::cout << "Expected behavior:" << std::endl;
    std::cout << "- Velocity 51 (0.4): Should use soft sample -> ~0.2" << std::endl;
    std::cout << "- Velocity 60 (0.47): Edge of soft layer, start crossfade -> mix" << std::endl;
    std::cout << "- Velocity 65 (0.511): In gap, should crossfade -> ~0.5" << std::endl;
    std::cout << "- Velocity 70 (0.55): Edge of loud layer -> mix" << std::endl;
    std::cout << "- Velocity 76 (0.6): Should use loud sample -> ~0.8" << std::endl;
    
    return 0;
}
