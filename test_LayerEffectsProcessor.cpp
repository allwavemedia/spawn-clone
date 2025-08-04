/*
  ==============================================================================

    test_LayerEffectsProcessor.cpp
    Created: 3 Aug 2025
    Author:  Epic 8 Story 8.2 Testing

    Unit tests for LayerEffectsProcessor functionality.

  ==============================================================================
*/

#include <gtest/gtest.h>
#include "../Source/audio/LayerEffectsProcessor.h"

//==============================================================================
class LayerEffectsProcessorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        processor = std::make_unique<LayerEffectsProcessor>();
        sampleRate = 44100.0;
        blockSize = 512;
    }

    void TearDown() override
    {
        processor.reset();
    }

    std::unique_ptr<LayerEffectsProcessor> processor;
    double sampleRate;
    int blockSize;
};

//==============================================================================
TEST_F(LayerEffectsProcessorTest, Constructor)
{
    EXPECT_TRUE(processor != nullptr);
    
    // Test default parameters for each layer
    auto melodyParams = processor->getLayerParameters(LayerEffectsProcessor::LayerType::Melody);
    auto chordsParams = processor->getLayerParameters(LayerEffectsProcessor::LayerType::Chords);
    auto bassParams = processor->getLayerParameters(LayerEffectsProcessor::LayerType::Bass);
    
    // Melody should have brighter filter cutoff
    EXPECT_GT(melodyParams.filterCutoff, chordsParams.filterCutoff);
    EXPECT_GT(chordsParams.filterCutoff, bassParams.filterCutoff);
    
    // Chords should have more chorus
    EXPECT_GT(chordsParams.chorusMix, melodyParams.chorusMix);
    EXPECT_GT(chordsParams.chorusMix, bassParams.chorusMix);
}

TEST_F(LayerEffectsProcessorTest, PrepareToPlay)
{
    // Should not crash when preparing
    EXPECT_NO_THROW(processor->prepareToPlay(sampleRate, blockSize, 2));
    
    // Should be able to release resources without crashing
    EXPECT_NO_THROW(processor->releaseResources());
}

TEST_F(LayerEffectsProcessorTest, ParameterUpdates)
{
    processor->prepareToPlay(sampleRate, blockSize, 2);
    
    // Test filter cutoff update
    float testCutoff = 1500.0f;
    processor->setFilterCutoff(LayerEffectsProcessor::LayerType::Melody, testCutoff);
    
    auto params = processor->getLayerParameters(LayerEffectsProcessor::LayerType::Melody);
    EXPECT_FLOAT_EQ(params.filterCutoff, testCutoff);
    
    // Test reverb mix update
    float testReverb = 0.4f;
    processor->setReverbMix(LayerEffectsProcessor::LayerType::Chords, testReverb);
    
    params = processor->getLayerParameters(LayerEffectsProcessor::LayerType::Chords);
    EXPECT_FLOAT_EQ(params.reverbWetLevel, testReverb);
}

TEST_F(LayerEffectsProcessorTest, GlobalPitchShift)
{
    processor->prepareToPlay(sampleRate, blockSize, 2);
    
    // Test pitch shift range
    processor->setGlobalPitchShift(6.0f);  // +6 semitones
    processor->setGlobalPitchShift(-12.0f); // -12 semitones (should clamp)
    processor->setGlobalPitchShift(15.0f);  // +15 semitones (should clamp to +12)
    
    // No crash is success for now - actual pitch processing would need audio buffer testing
}

TEST_F(LayerEffectsProcessorTest, LayerMuteAndSolo)
{
    auto layer = LayerEffectsProcessor::LayerType::Melody;
    
    // Test muting
    EXPECT_FALSE(processor->isLayerMuted(layer));
    processor->setLayerMuted(layer, true);
    EXPECT_TRUE(processor->isLayerMuted(layer));
    
    // Test soloing
    EXPECT_FALSE(processor->isLayerSoloed(layer));
    processor->setLayerSoloed(layer, true);
    EXPECT_TRUE(processor->isLayerSoloed(layer));
    EXPECT_TRUE(processor->isAnyLayerSoloed());
    
    processor->setLayerSoloed(layer, false);
    EXPECT_FALSE(processor->isAnyLayerSoloed());
}

TEST_F(LayerEffectsProcessorTest, EffectsMorphing)
{
    processor->prepareToPlay(sampleRate, blockSize, 2);
    
    // Test effects morphing at different values
    processor->setEffectsMorph(0.0f);  // Minimum effects
    processor->setEffectsMorph(0.5f);  // Middle range
    processor->setEffectsMorph(1.0f);  // Maximum effects
    
    // Verify parameters changed for different layers
    auto melodyParams = processor->getLayerParameters(LayerEffectsProcessor::LayerType::Melody);
    auto chordsParams = processor->getLayerParameters(LayerEffectsProcessor::LayerType::Chords);
    auto bassParams = processor->getLayerParameters(LayerEffectsProcessor::LayerType::Bass);
    
    // At maximum morph, filters should be more open for melody
    EXPECT_GT(melodyParams.filterCutoff, 2000.0f);
}

TEST_F(LayerEffectsProcessorTest, AudioProcessing)
{
    processor->prepareToPlay(sampleRate, blockSize, 2);
    
    // Create test audio buffer
    juce::AudioBuffer<float> testBuffer(2, blockSize);
    testBuffer.clear();
    
    // Add some test signal (sine wave)
    for (int channel = 0; channel < testBuffer.getNumChannels(); ++channel)
    {
        auto* channelData = testBuffer.getWritePointer(channel);
        for (int sample = 0; sample < blockSize; ++sample)
        {
            channelData[sample] = std::sin(2.0 * M_PI * 440.0 * sample / sampleRate) * 0.5f;
        }
    }
    
    // Process through effects - should not crash
    EXPECT_NO_THROW(processor->processLayer(LayerEffectsProcessor::LayerType::Melody, testBuffer));
    
    // Buffer should still contain audio (not silent)
    bool hasAudio = false;
    for (int channel = 0; channel < testBuffer.getNumChannels(); ++channel)
    {
        auto* channelData = testBuffer.getReadPointer(channel);
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

TEST_F(LayerEffectsProcessorTest, CPUUsageMonitoring)
{
    processor->prepareToPlay(sampleRate, blockSize, 2);
    
    // CPU usage should be reportable
    float usage = processor->getCurrentCPUUsage();
    EXPECT_GE(usage, 0.0f);
    EXPECT_LE(usage, 1.0f);
}
