/*
  Minimal debug to trace each audio component
*/

#include "Source/audio/AdvancedSynthesisEngine.h"
#include <iostream>

int main()
{
    spawnclone::audio::AdvancedSynthesisEngine engine;
    engine.prepareToPlay(44100.0, 512, 2);
    
    std::cout << "Testing with default parameters (no explicit setSynthesisParameters)" << std::endl;
    
    juce::AudioBuffer<float> buffer(2, 512);
    juce::MidiBuffer midiBuffer;
    
    // Add note-on
    auto noteOnMessage = juce::MidiMessage::noteOn(1, 60, 0.8f);
    midiBuffer.addEvent(noteOnMessage, 0);
    
    // Check parameters before processing
    auto params = engine.getSynthesisParameters();
    std::cout << "masterVolume: " << params.masterVolume << std::endl;
    std::cout << "attack: " << params.envelope.attack << std::endl;
    std::cout << "sustain: " << params.envelope.sustain << std::endl;
    std::cout << "wavetableIndex: " << params.wavetable.wavetableIndex << std::endl;
    std::cout << "wavetables count: " << engine.getNumWavetables() << std::endl;
    std::cout << "filter enabled: " << (params.filter.enabled ? "true" : "false") << std::endl;
    
    buffer.clear();
    engine.processBlock(buffer, midiBuffer);
    
    // Process multiple blocks like AudioQualityBasic test
    std::cout << "\nProcessing 5 more blocks..." << std::endl;
    for (int block = 0; block < 5; ++block)
    {
        buffer.clear();
        midiBuffer.clear(); // No more MIDI events
        engine.processBlock(buffer, midiBuffer);
        
        float blockMax = 0.0f;
        for (int ch = 0; ch < 2; ++ch)
        {
            for (int i = 0; i < 512; ++i)
            {
                blockMax = std::max(blockMax, std::abs(buffer.getSample(ch, i)));
            }
        }
        std::cout << "Block " << (block+1) << " max: " << blockMax << std::endl;
    }
    
    // Check results
    std::cout << "Active voices: " << engine.getCurrentVoiceCount() << std::endl;
    
    float maxSample = 0.0f;
    for (int ch = 0; ch < 2; ++ch)
    {
        for (int i = 0; i < 512; ++i)
        {
            maxSample = std::max(maxSample, std::abs(buffer.getSample(ch, i)));
        }
    }
    std::cout << "Max amplitude: " << maxSample << std::endl;
    
    return 0;
}
