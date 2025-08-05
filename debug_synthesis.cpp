/*
  Debug program to trace Advanced Synthesis Engine audio generation
*/

#include "Source/audio/AdvancedSynthesisEngine.h"
#include <iostream>
#include <iomanip>

int main()
{
    std::cout << "=== Advanced Synthesis Engine Debug ===" << std::endl;
    
    // Create engine
    spawnclone::audio::AdvancedSynthesisEngine engine;
    
    // Setup audio parameters
    double sampleRate = 44100.0;
    int blockSize = 512;
    int numChannels = 2;
    
    std::cout << "Preparing engine..." << std::endl;
    engine.prepareToPlay(sampleRate, blockSize, numChannels);
    
    // Test exactly like the failing test - no setSynthesisParameters call
    std::cout << "Testing exactly like unit test (no setSynthesisParameters)" << std::endl;
    
    // Check default parameters
    auto params = engine.getSynthesisParameters();
    std::cout << "Default masterVolume: " << params.masterVolume << std::endl;
    std::cout << "Default wavetablePosition: " << params.wavetable.wavetablePosition << std::endl;
    std::cout << "Default attack: " << params.envelope.attack << std::endl;
    std::cout << "Default sustain: " << params.envelope.sustain << std::endl;
    std::cout << "Default filter.enabled: " << (params.filter.enabled ? "true" : "false") << std::endl;
    std::cout << "Default lfo.depth: " << params.modulation.lfoDepth << std::endl;
    std::cout << "Default lfo.rate: " << params.modulation.lfoRate << std::endl;
    std::cout << "Wavetables count: " << engine.getNumWavetables() << std::endl;
    
    // Create audio buffer and MIDI
    juce::AudioBuffer<float> buffer(numChannels, blockSize);
    juce::MidiBuffer midiBuffer;
    
    // Add note-on
    auto noteOnMessage = juce::MidiMessage::noteOn(1, 60, 0.8f);
    midiBuffer.addEvent(noteOnMessage, 0);
    
    std::cout << "\nProcessing audio block with note-on..." << std::endl;
    
    // Process block
    buffer.clear();
    engine.processBlock(buffer, midiBuffer);
    
    // Check for audio output
    float maxSample = 0.0f;
    int nonZeroSamples = 0;
    
    for (int channel = 0; channel < numChannels; ++channel)
    {
        auto* channelData = buffer.getReadPointer(channel);
        for (int sample = 0; sample < blockSize; ++sample)
        {
            float sampleValue = std::abs(channelData[sample]);
            if (sampleValue > 0.000001f)
            {
                nonZeroSamples++;
                if (sampleValue > maxSample)
                    maxSample = sampleValue;
            }
        }
    }
    
    std::cout << "Active voices: " << engine.getCurrentVoiceCount() << std::endl;
    std::cout << "Non-zero samples: " << nonZeroSamples << std::endl;
    std::cout << "Max sample amplitude: " << std::fixed << std::setprecision(8) << maxSample << std::endl;
    
    // Process multiple blocks to allow envelope attack phase
    std::cout << "\nProcessing 5 more blocks to allow envelope attack..." << std::endl;
    for (int i = 0; i < 5; ++i)
    {
        buffer.clear();
        midiBuffer.clear(); // No more MIDI events
        engine.processBlock(buffer, midiBuffer);
        
        // Check max amplitude after each block
        float blockMax = 0.0f;
        for (int channel = 0; channel < numChannels; ++channel)
        {
            auto* channelData = buffer.getReadPointer(channel);
            for (int sample = 0; sample < blockSize; ++sample)
            {
                blockMax = std::max(blockMax, std::abs(channelData[sample]));
            }
        }
        std::cout << "Block " << (i+1) << " max amplitude: " << std::fixed << std::setprecision(8) << blockMax << std::endl;
    }
    
    std::cout << "Final active voices: " << engine.getCurrentVoiceCount() << std::endl;
    
    // Show first few samples for debugging
    std::cout << "\nFirst 10 samples channel 0:" << std::endl;
    auto* channelData = buffer.getReadPointer(0);
    for (int i = 0; i < 10; ++i)
    {
        std::cout << "  Sample " << i << ": " << std::fixed << std::setprecision(8) << channelData[i] << std::endl;
    }
    
    return 0;
}
