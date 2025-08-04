/*
  ==============================================================================

    test_AdvancedSynthesisEngine.cpp
    Created: 4 Aug 2025
    Author:  Epic 9.2 Story 9.2.1 Implementation

    Unit tests for Advanced Synthesis Engine - Wavetable Synthesis

  ==============================================================================
*/

#include <gtest/gtest.h>
#include "../Source/audio/AdvancedSynthesisEngine.h"
#include <juce_audio_basics/juce_audio_basics.h>

class AdvancedSynthesisEngineTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        engine = std::make_unique<AdvancedSynthesisEngine>();
        sampleRate = 44100.0;
        blockSize = 512;
        numChannels = 2;
        
        // Prepare the engine
        engine->prepareToPlay(sampleRate, blockSize, numChannels);
    }
    
    void TearDown() override
    {
        engine->releaseResources();
        engine.reset();
    }
    
    std::unique_ptr<AdvancedSynthesisEngine> engine;
    double sampleRate;
    int blockSize;
    int numChannels;
};

//==============================================================================
// Basic Engine Tests

TEST_F(AdvancedSynthesisEngineTest, EngineInitialization)
{
    EXPECT_NE(engine.get(), nullptr);
    EXPECT_EQ(engine->getCurrentVoiceCount(), 0);
    EXPECT_GE(engine->getNumWavetables(), 8); // Should have built-in wavetables
}

TEST_F(AdvancedSynthesisEngineTest, SynthesisParametersDefaults)
{
    auto params = engine->getSynthesisParameters();
    
    EXPECT_EQ(params.synthesisType, AdvancedSynthesisEngine::SynthesisType::Wavetable);
    EXPECT_FLOAT_EQ(params.wavetable.wavetablePosition, 0.5f);
    EXPECT_FLOAT_EQ(params.envelope.attack, 0.001f);    // Optimized for immediate response
    EXPECT_FLOAT_EQ(params.envelope.sustain, 1.0f);     // Full sustain for consistent output
    EXPECT_FLOAT_EQ(params.masterVolume, 0.7f);
}

//==============================================================================
// Wavetable Tests

TEST_F(AdvancedSynthesisEngineTest, WavetableGeneration)
{
    // Built-in wavetables should be generated on construction
    EXPECT_GE(engine->getNumWavetables(), 8);
    
    // Test wavetable loading
    juce::AudioBuffer<float> testWavetable(1, 2048);
    auto* samples = testWavetable.getWritePointer(0);
    
    // Generate sine wave wavetable
    for (int i = 0; i < 2048; ++i)
    {
        float phase = static_cast<float>(i) / 2048.0f;
        samples[i] = std::sin(2.0f * juce::MathConstants<float>::pi * phase);
    }
    
    EXPECT_TRUE(engine->loadWavetable(10, testWavetable));
    EXPECT_GE(engine->getNumWavetables(), 11);
}

TEST_F(AdvancedSynthesisEngineTest, WavetableParameterControl)
{
    // Test basic parameter setting without triggering complex updates
    engine->setParameter("wavetablePosition", 0.25f);
    
    auto retrievedParams = engine->getSynthesisParameters();
    EXPECT_FLOAT_EQ(retrievedParams.wavetable.wavetablePosition, 0.25f);
}

//==============================================================================
// Voice Management Tests

TEST_F(AdvancedSynthesisEngineTest, NoteOnOffBasic)
{
    // Initially no voices active
    EXPECT_EQ(engine->getCurrentVoiceCount(), 0);
    
    // Trigger a note
    engine->noteOn(60, 0.8f); // Middle C
    EXPECT_EQ(engine->getCurrentVoiceCount(), 1);
    
    // Release the note
    engine->noteOff(60);
    
    // Voice count may not immediately go to 0 due to release envelope
    // But should eventually reach 0 after processing
}

TEST_F(AdvancedSynthesisEngineTest, MultipleVoicesPolyphony)
{
    // Test polyphonic capability
    for (int note = 60; note < 68; ++note)
    {
        engine->noteOn(note, 0.7f);
    }
    
    EXPECT_EQ(engine->getCurrentVoiceCount(), 8);
    
    // Release all notes
    for (int midiNote = 60; midiNote < 68; ++midiNote)
    {
        engine->noteOff(midiNote);
    }
}

//==============================================================================
// Audio Processing Tests

TEST_F(AdvancedSynthesisEngineTest, AudioProcessingBasic)
{
    juce::AudioBuffer<float> buffer(numChannels, blockSize);
    juce::MidiBuffer midiBuffer;
    
    // Add a note-on message
    auto noteOnMessage = juce::MidiMessage::noteOn(1, 60, 0.8f);
    midiBuffer.addEvent(noteOnMessage, 0);
    
    // Process the block
    buffer.clear();
    engine->processBlock(buffer, midiBuffer);
    
    // Should have generated some audio
    bool hasAudio = false;
    for (int channel = 0; channel < numChannels; ++channel)
    {
        auto* channelData = buffer.getReadPointer(channel);
        for (int sample = 0; sample < blockSize; ++sample)
        {
            if (std::abs(channelData[sample]) > 0.001f)
            {
                hasAudio = true;
                break;
            }
        }
        if (hasAudio) break;
    }
    
    EXPECT_TRUE(hasAudio);
}

TEST_F(AdvancedSynthesisEngineTest, AudioProcessingSilenceWhenNoNotes)
{
    juce::AudioBuffer<float> buffer(numChannels, blockSize);
    juce::MidiBuffer midiBuffer; // Empty MIDI buffer
    
    buffer.clear();
    engine->processBlock(buffer, midiBuffer);
    
    // Should be silent
    for (int channel = 0; channel < numChannels; ++channel)
    {
        auto* channelData = buffer.getReadPointer(channel);
        for (int sample = 0; sample < blockSize; ++sample)
        {
            EXPECT_NEAR(channelData[sample], 0.0f, 0.001f);
        }
    }
}

//==============================================================================
// Filter Tests

TEST_F(AdvancedSynthesisEngineTest, FilterParameters)
{
    AdvancedSynthesisEngine::SynthesisParameters params;
    params.filter.enabled = true;
    params.filter.filterType = AdvancedSynthesisEngine::FilterParams::LowPass;
    params.filter.cutoff = 1000.0f;
    params.filter.resonance = 0.5f;
    
    engine->setSynthesisParameters(params);
    
    auto retrievedParams = engine->getSynthesisParameters();
    EXPECT_TRUE(retrievedParams.filter.enabled);
    EXPECT_EQ(retrievedParams.filter.filterType, AdvancedSynthesisEngine::FilterParams::LowPass);
    EXPECT_FLOAT_EQ(retrievedParams.filter.cutoff, 1000.0f);
    EXPECT_FLOAT_EQ(retrievedParams.filter.resonance, 0.5f);
}

//==============================================================================
// Modulation Tests

TEST_F(AdvancedSynthesisEngineTest, LFOParameters)
{
    AdvancedSynthesisEngine::SynthesisParameters params;
    params.modulation.lfoRate = 5.0f;
    params.modulation.lfoDepth = 0.3f;
    params.modulation.lfoWaveform = 1; // Triangle
    params.modulation.modulationTarget = 4; // Wavetable position
    params.modulation.bipolar = true;
    
    engine->setSynthesisParameters(params);
    
    auto retrievedParams = engine->getSynthesisParameters();
    EXPECT_FLOAT_EQ(retrievedParams.modulation.lfoRate, 5.0f);
    EXPECT_FLOAT_EQ(retrievedParams.modulation.lfoDepth, 0.3f);
    EXPECT_EQ(retrievedParams.modulation.lfoWaveform, 1);
    EXPECT_EQ(retrievedParams.modulation.modulationTarget, 4);
    EXPECT_TRUE(retrievedParams.modulation.bipolar);
}

//==============================================================================
// Performance Tests

TEST_F(AdvancedSynthesisEngineTest, CPUUsageMonitoring)
{
    // Generate some audio to get CPU usage
    juce::AudioBuffer<float> buffer(numChannels, blockSize);
    juce::MidiBuffer midiBuffer;
    
    // Add multiple notes
    for (int note = 60; note < 68; ++note)
    {
        auto noteOnMessage = juce::MidiMessage::noteOn(1, note, 0.7f);
        midiBuffer.addEvent(noteOnMessage, 0);
    }
    
    // Process multiple blocks
    for (int block = 0; block < 10; ++block)
    {
        buffer.clear();
        engine->processBlock(buffer, midiBuffer);
        midiBuffer.clear(); // Only send MIDI on first block
    }
    
    // CPU usage should be reported (may be 0 for fast processing)
    float cpuUsage = engine->getCPUUsage();
    EXPECT_GE(cpuUsage, 0.0f);
    EXPECT_LE(cpuUsage, 100.0f);
}

TEST_F(AdvancedSynthesisEngineTest, VoiceStealingUnderLoad)
{
    // Try to trigger more than 16 voices (engine limit)
    for (int note = 40; note < 80; ++note)
    {
        engine->noteOn(note, 0.7f);
    }
    
    // Should not exceed maximum voice count
    EXPECT_LE(engine->getCurrentVoiceCount(), 16);
}

//==============================================================================
// Parameter Validation Tests

TEST_F(AdvancedSynthesisEngineTest, ParameterRangeValidation)
{
    // Test parameter clamping
    engine->setParameter("wavetablePosition", 2.0f); // Over range
    auto params = engine->getSynthesisParameters();
    EXPECT_LE(params.wavetable.wavetablePosition, 1.0f);
    
    engine->setParameter("wavetablePosition", -1.0f); // Under range  
    params = engine->getSynthesisParameters();
    EXPECT_GE(params.wavetable.wavetablePosition, 0.0f);
    
    engine->setParameter("filterCutoff", 50000.0f); // Over range
    params = engine->getSynthesisParameters();
    EXPECT_LE(params.filter.cutoff, 20000.0f);
    
    engine->setParameter("filterCutoff", 5.0f); // Under range
    params = engine->getSynthesisParameters();
    EXPECT_GE(params.filter.cutoff, 20.0f);
}

//==============================================================================
// Integration Tests

TEST_F(AdvancedSynthesisEngineTest, EngineInfoReporting)
{
    juce::String info = engine->getEngineInfo();
    
    EXPECT_FALSE(info.isEmpty());
    EXPECT_TRUE(info.contains("Advanced Synthesis Engine"));
    EXPECT_TRUE(info.contains("Sample Rate"));
    EXPECT_TRUE(info.contains("Wavetable"));
}

TEST_F(AdvancedSynthesisEngineTest, ResetFunctionality)
{
    // Trigger some notes
    engine->noteOn(60, 0.8f);
    engine->noteOn(64, 0.7f);
    
    EXPECT_GT(engine->getCurrentVoiceCount(), 0);
    
    // Reset engine
    engine->reset();
    
    // Should clear all voices
    EXPECT_EQ(engine->getCurrentVoiceCount(), 0);
}

//==============================================================================
// Quality Assurance Tests

TEST_F(AdvancedSynthesisEngineTest, AudioQualityBasic)
{
    juce::AudioBuffer<float> buffer(numChannels, blockSize);
    juce::MidiBuffer midiBuffer;
    
    // Add a note
    auto noteOnMessage = juce::MidiMessage::noteOn(1, 69, 1.0f); // A4 = 440Hz
    midiBuffer.addEvent(noteOnMessage, 0);
    
    // Process several blocks to let envelope attack complete
    for (int block = 0; block < 5; ++block)
    {
        buffer.clear();
        engine->processBlock(buffer, midiBuffer);
        midiBuffer.clear();
    }
    
    // Check for reasonable amplitude levels
    float maxAmplitude = 0.0f;
    for (int channel = 0; channel < numChannels; ++channel)
    {
        auto* channelData = buffer.getReadPointer(channel);
        for (int sample = 0; sample < blockSize; ++sample)
        {
            maxAmplitude = juce::jmax(maxAmplitude, std::abs(channelData[sample]));
        }
    }
    
    // Should have reasonable output level (not clipping, not too quiet)
    EXPECT_GT(maxAmplitude, 0.01f);  // Not too quiet
    EXPECT_LT(maxAmplitude, 1.0f);   // Not clipping
}

//==============================================================================
// Main Test Entry Point

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
