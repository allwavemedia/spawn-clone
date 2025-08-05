//==============================================================================
/*
  Epic 9.2 Story 9.2.3: Reverse Playback Debug Program
  Author:  BMad Dev Agent
*/
//==============================================================================

#include <iostream>
#include <iomanip>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include "audio/AdvancedSynthesisEngine.h"

using namespace spawnclone::audio;

int main()
{
    std::cout << "=== Epic 9.2 Story 9.2.3: Reverse Playback Debug Program ===" << std::endl;
    
    // Create engine
    auto engine = std::make_unique<AdvancedSynthesisEngine>();
    engine->prepareToPlay(44100.0, 512, 2);
    
    // Create a ramp-up sample: 0.0 at start, 1.0 at end
    juce::AudioBuffer<float> rampUp(1, 512);
    for(int i = 0; i < 512; ++i) 
    {
        float value = static_cast<float>(i) / 511.0f;
        rampUp.setSample(0, i, value);
    }
    
    std::cout << "Created ramp sample:" << std::endl;
    std::cout << "  Sample[0] = " << rampUp.getSample(0, 0) << std::endl;
    std::cout << "  Sample[255] = " << rampUp.getSample(0, 255) << std::endl;
    std::cout << "  Sample[511] = " << rampUp.getSample(0, 511) << std::endl;
    
    engine->loadSample(0, rampUp);
    
    // Test forward playback first
    std::cout << "\n--- Testing Forward Playback ---" << std::endl;
    AdvancedSynthesisEngine::SynthesisParameters forwardParams;
    forwardParams.synthesisType = AdvancedSynthesisEngine::SynthesisType::Sample;
    forwardParams.sample.sampleIndex = 0; 
    forwardParams.sample.reversePlayback = false;
    forwardParams.masterVolume = 1.0f;
    forwardParams.envelope.attack = 0.001f;
    forwardParams.envelope.decay = 0.0f;
    forwardParams.envelope.sustain = 1.0f;
    forwardParams.envelope.release = 0.001f;
    engine->setSynthesisParameters(forwardParams);
    
    juce::AudioBuffer<float> audioBuffer(2, 512);
    juce::MidiBuffer midiBuffer;
    
    audioBuffer.clear();
    engine->noteOn(60, 1.0f);
    engine->processBlock(audioBuffer, midiBuffer);
    engine->noteOff(60);
    
    std::cout << "Forward playback results:" << std::endl;
    std::cout << "  Output[0] = " << audioBuffer.getSample(0, 0) << std::endl;
    std::cout << "  Output[255] = " << audioBuffer.getSample(0, 255) << std::endl;
    std::cout << "  Output[511] = " << audioBuffer.getSample(0, 511) << std::endl;
    
    // Test reverse playback
    std::cout << "\n--- Testing Reverse Playback ---" << std::endl;
    AdvancedSynthesisEngine::SynthesisParameters reverseParams;
    reverseParams.synthesisType = AdvancedSynthesisEngine::SynthesisType::Sample;
    reverseParams.sample.sampleIndex = 0; 
    reverseParams.sample.reversePlayback = true;
    reverseParams.masterVolume = 1.0f;
    reverseParams.envelope.attack = 0.001f;
    reverseParams.envelope.decay = 0.0f;
    reverseParams.envelope.sustain = 1.0f;
    reverseParams.envelope.release = 0.001f;
    engine->setSynthesisParameters(reverseParams);
    
    audioBuffer.clear();
    engine->noteOn(60, 1.0f);
    engine->processBlock(audioBuffer, midiBuffer);
    engine->noteOff(60);
    
    std::cout << "Reverse playback results:" << std::endl;
    std::cout << "  Output[0] = " << audioBuffer.getSample(0, 0) << std::endl;
    std::cout << "  Output[255] = " << audioBuffer.getSample(0, 255) << std::endl;
    std::cout << "  Output[511] = " << audioBuffer.getSample(0, 511) << std::endl;
    
    std::cout << "\nExpected for reverse:" << std::endl;
    std::cout << "  Output[0] should be ~1.0 (sample[511])" << std::endl;
    std::cout << "  Output[255] should be ~0.5 (sample[255])" << std::endl;
    std::cout << "  Output[511] should be ~0.0 (sample[0])" << std::endl;
    
    return 0;
}
