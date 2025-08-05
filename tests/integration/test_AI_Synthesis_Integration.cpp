#include <gtest/gtest.h>
#include "../../Source/audio/AudioPreviewEngine.h"
#include "../../Source/GenerationParameters.h"

class AISynthesisIntegrationTest : public ::testing::Test 
{
protected:
    void SetUp() override 
    {
        audioEngine = std::make_unique<AudioPreviewEngine>();
        audioEngine->prepareToPlay(44100.0, 512);
        audioEngine->setAdvancedSynthesisEnabled(true);
    }
    
    std::unique_ptr<AudioPreviewEngine> audioEngine;
};

TEST_F(AISynthesisIntegrationTest, ElectronicStyleParameterMapping)
{
    GenerationParameters params;
    params.harmony.style = GenerationParameters::HarmonicSettings::ProgressionStyle::Electronic;
    params.scale = GenerationParameters::ScaleType::Minor;
    params.rhythmicComplexity = 0.8f;
    params.tempo = 128.0f;
    
    // Apply AI parameters and verify synthesis engine responds
    audioEngine->applyAIGenerationParameters(params);
    
    // Verify parameter mapping worked
    float cutoff = audioEngine->getSynthesisParameterValue("filter.cutoff");
    EXPECT_GT(cutoff, 4000.0f); // Electronic style should have higher cutoff
    
    float resonance = audioEngine->getSynthesisParameterValue("filter.resonance");
    EXPECT_GT(resonance, 0.4f); // Electronic style should have higher resonance
}

TEST_F(AISynthesisIntegrationTest, RealTimeParameterUpdates)
{
    // Test smooth parameter interpolation
    audioEngine->updateSynthesisParameter("filter.cutoff", 1000.0f, false);
    float initial = audioEngine->getSynthesisParameterValue("filter.cutoff");
    
    audioEngine->updateSynthesisParameter("filter.cutoff", 8000.0f, true);
    float updated = audioEngine->getSynthesisParameterValue("filter.cutoff");
    
    EXPECT_NE(initial, updated);
    EXPECT_GT(updated, initial);
}
