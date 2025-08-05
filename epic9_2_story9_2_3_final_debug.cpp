//==============================================================================
/*
  Epic 9.2 Story 9.2.3: Final Test Debug Program
  Author:  BMad Dev Agent
*/
//==============================================================================

#include <iostream>
#include <iomanip>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include "audio/AdvancedSynthesisEngine.h"

using namespace spawnclone::audio;

void fillBuffer(juce::AudioBuffer<float>& buffer, float value, int startSample = 0, int numSamples = -1)
{
    if (numSamples == -1)
        numSamples = buffer.getNumSamples() - startSample;

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* ptr = buffer.getWritePointer(ch, startSample);
        for (int i = 0; i < numSamples; ++i)
            ptr[i] = value;
    }
}

float getBufferRMS(const juce::AudioBuffer<float>& buffer)
{
    return buffer.getRMSLevel(0, 0, buffer.getNumSamples());
}

int main()
{
    std::cout << "=== Epic 9.2 Story 9.2.3: Final Test Debug Program ===" << std::endl;
    
    // Create engine exactly like the test
    auto engine = std::make_unique<AdvancedSynthesisEngine>();
    engine->prepareToPlay(44100.0, 512, 2);
    
    // Create test sample exactly like the test SetUp method
    const int sampleLength = 44100;
    juce::AudioBuffer<float> testSample;
    testSample.setSize(1, sampleLength);
    
    for (int i = 0; i < sampleLength; ++i)
    {
        float phase = static_cast<float>(i) / 44100.0f * 2.0f * juce::MathConstants<float>::pi;
        testSample.setSample(0, i, std::sin(phase * 440.0f) * 0.7f); // A4 (440Hz)
    }
    
    std::cout << "\n=== REVERSE PLAYBACK TEST REPLICATION ===" << std::endl;
    
    // Create a ramp-up sample exactly like the test
    juce::AudioBuffer<float> rampUp(1, 512);
    for(int i = 0; i < 512; ++i) rampUp.setSample(0, i, static_cast<float>(i) / 511.0f);
    
    engine->loadSample(0, rampUp);
    
    AdvancedSynthesisEngine::SynthesisParameters params;
    params.synthesisType = AdvancedSynthesisEngine::SynthesisType::Sample;
    params.sample.sampleIndex = 0;
    params.sample.reversePlayback = true;
    params.masterVolume = 1.0f;
    params.envelope.attack = 0.001f;
    params.envelope.decay = 0.0f;
    params.envelope.sustain = 1.0f;
    params.envelope.release = 0.001f;
    engine->setSynthesisParameters(params);

    juce::AudioBuffer<float> audioBuffer(2, 512);
    juce::MidiBuffer midiBuffer;

    // Trigger note exactly like the test
    audioBuffer.clear();
    engine->noteOn(60, 1.0f);
    engine->processBlock(audioBuffer, midiBuffer);
    engine->noteOff(60);

    std::cout << "Reverse playback test replication:" << std::endl;
    std::cout << "  audioBuffer.getSample(0, 0) = " << audioBuffer.getSample(0, 0) << " (expected > 0.9)" << std::endl;
    std::cout << "  audioBuffer.getSample(0, 511) = " << audioBuffer.getSample(0, 511) << " (expected near 0)" << std::endl;
    
    // Check if any audio was generated at all
    bool hasAudio = false;
    for (int i = 0; i < 512; ++i) {
        if (std::abs(audioBuffer.getSample(0, i)) > 0.001f) {
            hasAudio = true;
            break;
        }
    }
    std::cout << "  Has any audio: " << (hasAudio ? "YES" : "NO") << std::endl;
    
    if (hasAudio) {
        // Find max sample
        float maxSample = 0.0f;
        int maxIndex = 0;
        for (int i = 0; i < 512; ++i) {
            float sample = std::abs(audioBuffer.getSample(0, i));
            if (sample > maxSample) {
                maxSample = sample;
                maxIndex = i;
            }
        }
        std::cout << "  Max sample: " << maxSample << " at index " << maxIndex << std::endl;
    }
    
    std::cout << "\n=== PLAYBACK SPEED TEST REPLICATION ===" << std::endl;
    
    // Load the 1s test sample
    engine->loadSample(0, testSample);
    
    AdvancedSynthesisEngine::SynthesisParameters speedParams;
    speedParams.synthesisType = AdvancedSynthesisEngine::SynthesisType::Sample;
    speedParams.sample.sampleIndex = 0;
    speedParams.masterVolume = 1.0f;
    speedParams.envelope.attack = 0.001f;
    speedParams.envelope.decay = 0.0f;
    speedParams.envelope.sustain = 1.0f;
    speedParams.envelope.release = 0.001f;
    
    juce::AudioBuffer<float> speedBuffer(2, 44100); // Process 1 second
    juce::MidiBuffer speedMidiBuffer;

    // Test double speed
    speedParams.sample.playbackSpeed = 2.0f;
    speedParams.sample.enableLooping = false;
    engine->setSynthesisParameters(speedParams);
    speedBuffer.clear();
    engine->noteOn(60, 1.0f);
    engine->processBlock(speedBuffer, speedMidiBuffer);
    engine->noteOff(60);
    
    float firstHalfRMS = speedBuffer.getRMSLevel(0, 0, 22050);
    float secondHalfRMS = speedBuffer.getRMSLevel(0, 22050, 22050);
    
    std::cout << "Double speed test replication:" << std::endl;
    std::cout << "  First half RMS: " << firstHalfRMS << " (expected > 0.1)" << std::endl;
    std::cout << "  Second half RMS: " << secondHalfRMS << " (expected < 0.05)" << std::endl;
    std::cout << "  Test result: " << (secondHalfRMS < 0.05f ? "PASS" : "FAIL") << std::endl;
    
    return 0;
}
