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

using namespace spawnclone::audio;

class AdvancedSynthesisEngineTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        engine = std::make_unique<spawnclone::audio::AdvancedSynthesisEngine>();
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
    
    std::unique_ptr<spawnclone::audio::AdvancedSynthesisEngine> engine;
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
    
    EXPECT_EQ(params.synthesisType, spawnclone::audio::SynthesisType::Wavetable);
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
// Story 9.2.2: Subtractive Synthesis Enhancement Tests

TEST_F(AdvancedSynthesisEngineTest, MoogLadderFilterTest)
{
    auto params = engine->getSynthesisParameters();
    params.synthesisType = AdvancedSynthesisEngine::SynthesisType::Subtractive;
    params.filter.enabled = true;
    params.filter.filterType = AdvancedSynthesisEngine::FilterParams::MoogLadder;
    params.filter.cutoff = 800.0f;
    params.filter.resonance = 0.7f;
    
    engine->setSynthesisParameters(params);
    
    // Test that Moog ladder filter parameters are accepted
    auto retrievedParams = engine->getSynthesisParameters();
    EXPECT_EQ(retrievedParams.filter.filterType, AdvancedSynthesisEngine::FilterParams::MoogLadder);
    EXPECT_FLOAT_EQ(retrievedParams.filter.cutoff, 800.0f);
    EXPECT_FLOAT_EQ(retrievedParams.filter.resonance, 0.7f);
}

TEST_F(AdvancedSynthesisEngineTest, DualFilterRoutingTest)
{
    auto params = engine->getSynthesisParameters();
    params.filter.enabled = true;
    params.filter.routing = AdvancedSynthesisEngine::FilterParams::Parallel;
    params.filter.filterType = AdvancedSynthesisEngine::FilterParams::LowPass;
    params.filter.secondaryFilterType = AdvancedSynthesisEngine::FilterParams::HighPass;
    params.filter.cutoff = 1000.0f;
    params.filter.secondaryCutoff = 3000.0f;
    params.filter.filterBalance = 0.5f;
    
    engine->setSynthesisParameters(params);
    
    // Verify parallel routing parameters
    auto retrievedParams = engine->getSynthesisParameters();
    EXPECT_EQ(retrievedParams.filter.routing, AdvancedSynthesisEngine::FilterParams::Parallel);
    EXPECT_FLOAT_EQ(retrievedParams.filter.secondaryCutoff, 3000.0f);
    EXPECT_FLOAT_EQ(retrievedParams.filter.filterBalance, 0.5f);
}

TEST_F(AdvancedSynthesisEngineTest, AdvancedModulationMatrixTest)
{
    auto params = engine->getSynthesisParameters();
    
    // Set up dual LFO modulation
    params.modulation.lfoRate = 3.0f;
    params.modulation.lfoDepth = 0.5f;
    params.modulation.lfo2Rate = 0.2f;
    params.modulation.lfo2Depth = 0.3f;
    params.modulation.lfo2Target = 2; // Filter modulation
    params.modulation.enableCrossModulation = true;
    params.modulation.crossModAmount = 0.4f;
    
    // Set up filter envelope
    params.modulation.env2Attack = 0.05f;
    params.modulation.env2Decay = 0.2f;
    params.modulation.env2Sustain = 0.6f;
    params.modulation.env2Release = 0.8f;
    params.modulation.env2Amount = 0.7f;
    
    engine->setSynthesisParameters(params);
    
    // Verify modulation parameters
    auto retrievedParams = engine->getSynthesisParameters();
    EXPECT_FLOAT_EQ(retrievedParams.modulation.lfo2Rate, 0.2f);
    EXPECT_FLOAT_EQ(retrievedParams.modulation.env2Amount, 0.7f);
    EXPECT_TRUE(retrievedParams.modulation.enableCrossModulation);
}

TEST_F(AdvancedSynthesisEngineTest, FilterKeyTrackingTest)
{
    auto params = engine->getSynthesisParameters();
    params.filter.enabled = true;
    params.filter.keyTracking = 0.8f;
    params.filter.velocityTracking = 0.6f;
    
    engine->setSynthesisParameters(params);
    
    // Test key tracking parameter acceptance
    auto retrievedParams = engine->getSynthesisParameters();
    EXPECT_FLOAT_EQ(retrievedParams.filter.keyTracking, 0.8f);
    EXPECT_FLOAT_EQ(retrievedParams.filter.velocityTracking, 0.6f);
}

TEST_F(AdvancedSynthesisEngineTest, StateVariableFilterTest)
{
    auto params = engine->getSynthesisParameters();
    params.filter.enabled = true;
    params.filter.filterType = AdvancedSynthesisEngine::FilterParams::StateVariable;
    params.filter.cutoff = 1500.0f;
    params.filter.resonance = 0.4f;
    
    engine->setSynthesisParameters(params);
    
    // Verify state variable filter parameters
    auto retrievedParams = engine->getSynthesisParameters();
    EXPECT_EQ(retrievedParams.filter.filterType, AdvancedSynthesisEngine::FilterParams::StateVariable);
}

TEST_F(AdvancedSynthesisEngineTest, SubtractiveSynthesisAudioGeneration)
{
    auto params = engine->getSynthesisParameters();
    params.synthesisType = AdvancedSynthesisEngine::SynthesisType::Subtractive;
    params.filter.enabled = true;
    params.filter.filterType = AdvancedSynthesisEngine::FilterParams::MoogLadder;
    params.filter.cutoff = 600.0f;
    params.filter.resonance = 0.8f;
    params.modulation.lfoRate = 4.0f;
    params.modulation.lfoDepth = 0.6f;
    params.modulation.modulationTarget = 2; // Filter modulation
    
    engine->setSynthesisParameters(params);
    
    // Generate audio with subtractive synthesis
    juce::AudioBuffer<float> buffer(numChannels, blockSize);
    juce::MidiBuffer midiBuffer;
    
    // Add note on
    juce::MidiMessage noteOn = juce::MidiMessage::noteOn(1, 60, 0.8f);
    midiBuffer.addEvent(noteOn, 0);
    
    engine->processBlock(buffer, midiBuffer);
    
    // Check audio generation
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
    
    EXPECT_TRUE(hasAudio) << "Subtractive synthesis should generate audio";
    EXPECT_GT(engine->getCurrentVoiceCount(), 0) << "Should have active voices";
}

TEST_F(AdvancedSynthesisEngineTest, WavetableSwitching)
{
    // Test that different wavetables produce different audio
    juce::AudioBuffer<float> buffer1(numChannels, blockSize);
    juce::AudioBuffer<float> buffer2(numChannels, blockSize);
    juce::MidiBuffer midiBuffer;
    
    // Test with sine wave (wavetable 0)
    auto params1 = engine->getSynthesisParameters();
    params1.synthesisType = AdvancedSynthesisEngine::SynthesisType::Wavetable;
    params1.wavetable.wavetableIndex = 0; // Sine wave
    engine->setSynthesisParameters(params1);
    
    juce::MidiMessage noteOn1 = juce::MidiMessage::noteOn(1, 60, 0.8f);
    midiBuffer.addEvent(noteOn1, 0);
    
    buffer1.clear();
    engine->processBlock(buffer1, midiBuffer);
    
    // Stop the note and wait
    juce::MidiMessage noteOff1 = juce::MidiMessage::noteOff(1, 60);
    midiBuffer.clear();
    midiBuffer.addEvent(noteOff1, 0);
    engine->processBlock(buffer1, midiBuffer);
    
    // Test with sawtooth wave (wavetable 1)
    auto params2 = engine->getSynthesisParameters();
    params2.synthesisType = AdvancedSynthesisEngine::SynthesisType::Wavetable;
    params2.wavetable.wavetableIndex = 1; // Sawtooth wave
    engine->setSynthesisParameters(params2);
    
    juce::MidiMessage noteOn2 = juce::MidiMessage::noteOn(1, 60, 0.8f);
    midiBuffer.clear();
    midiBuffer.addEvent(noteOn2, 0);
    
    buffer2.clear();
    engine->processBlock(buffer2, midiBuffer);
    
    // Compare the outputs - they should be different
    bool buffersDifferent = false;
    for (int channel = 0; channel < numChannels && !buffersDifferent; ++channel)
    {
        auto* data1 = buffer1.getReadPointer(channel);
        auto* data2 = buffer2.getReadPointer(channel);
        
        for (int sample = 0; sample < blockSize; ++sample)
        {
            if (std::abs(data1[sample] - data2[sample]) > 0.001f)
            {
                buffersDifferent = true;
                break;
            }
        }
    }
    
    EXPECT_TRUE(buffersDifferent) << "Different wavetables should produce different audio";
}

TEST_F(AdvancedSynthesisEngineTest, LFOModulation)
{
    // Test that LFO modulation affects audio output
    juce::AudioBuffer<float> buffer1(numChannels, blockSize * 2); // Longer buffer for modulation to be noticeable
    juce::AudioBuffer<float> buffer2(numChannels, blockSize * 2);
    juce::MidiBuffer midiBuffer;
    
    // Test without modulation
    auto params1 = engine->getSynthesisParameters();
    params1.synthesisType = AdvancedSynthesisEngine::SynthesisType::Wavetable;
    params1.modulation.lfoDepth = 0.0f; // No modulation
    params1.modulation.modulationTarget = 1; // Pitch modulation
    engine->setSynthesisParameters(params1);
    
    juce::MidiMessage noteOn1 = juce::MidiMessage::noteOn(1, 60, 0.8f);
    midiBuffer.addEvent(noteOn1, 0);
    
    buffer1.clear();
    for (int i = 0; i < 2; ++i) // Process multiple blocks
    {
        engine->processBlock(buffer1, midiBuffer);
        midiBuffer.clear(); // Only first block has note-on
    }
    
    // Stop and reset
    juce::MidiMessage noteOff1 = juce::MidiMessage::noteOff(1, 60);
    midiBuffer.clear();
    midiBuffer.addEvent(noteOff1, 0);
    engine->processBlock(buffer1, midiBuffer);
    
    // Test with pitch modulation
    auto params2 = engine->getSynthesisParameters();
    params2.synthesisType = AdvancedSynthesisEngine::SynthesisType::Wavetable;
    params2.modulation.lfoDepth = 0.5f; // Strong modulation
    params2.modulation.lfoRate = 4.0f;  // 4 Hz LFO
    params2.modulation.modulationTarget = 1; // Pitch modulation
    params2.modulation.bipolar = true;
    engine->setSynthesisParameters(params2);
    
    juce::MidiMessage noteOn2 = juce::MidiMessage::noteOn(1, 60, 0.8f);
    midiBuffer.clear();
    midiBuffer.addEvent(noteOn2, 0);
    
    buffer2.clear();
    for (int i = 0; i < 2; ++i) // Process multiple blocks
    {
        engine->processBlock(buffer2, midiBuffer);
        midiBuffer.clear(); // Only first block has note-on
    }
    
    // Compare the outputs - they should be different due to modulation
    bool modulationDetected = false;
    float maxDifference = 0.0f;
    
    for (int channel = 0; channel < numChannels && !modulationDetected; ++channel)
    {
        auto* data1 = buffer1.getReadPointer(channel);
        auto* data2 = buffer2.getReadPointer(channel);
        
        for (int sample = 0; sample < buffer1.getNumSamples(); ++sample)
        {
            float difference = std::abs(data1[sample] - data2[sample]);
            maxDifference = std::max(maxDifference, difference);
            
            if (difference > 0.01f) // Significant difference due to modulation
            {
                modulationDetected = true;
                break;
            }
        }
    }
    
    EXPECT_TRUE(modulationDetected) << "LFO modulation should affect audio output. Max difference: " << maxDifference;
}

//==============================================================================
// Main Test Entry Point

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
