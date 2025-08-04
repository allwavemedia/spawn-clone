#include <gtest/gtest.h>
#include "ui/ExperimentPadComponent.h"

/**
 * Epic 8 Story 8.1 Tests: SPAWN-style Experiment Pad XY Controller
 * 
 * Test suite for the ExperimentPadComponent to ensure proper:
 * - Position handling and coordinate mapping
 * - Pitch shift calculations
 * - Effects parameter mapping
 * - Visual feedback and interaction
 */
class ExperimentPadComponentTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Create test component with default settings
        experimentPad = std::make_unique<ExperimentPadComponent>();
    }

    void TearDown() override
    {
        experimentPad.reset();
    }

    std::unique_ptr<ExperimentPadComponent> experimentPad;
};

//==============================================================================
// Task 8.1.1: Test basic XY position handling
TEST_F(ExperimentPadComponentTest, PositionHandling)
{
    // Test center position (default)
    auto centerPosition = experimentPad->getPadPosition();
    EXPECT_FLOAT_EQ(centerPosition.x, 0.5f);
    EXPECT_FLOAT_EQ(centerPosition.y, 0.5f);
    
    // Test setting various positions
    ExperimentPadComponent::PadPosition testPosition;
    
    // Test corner positions
    testPosition = { 0.0f, 0.0f };
    experimentPad->setPadPosition(testPosition);
    auto retrievedPosition = experimentPad->getPadPosition();
    EXPECT_FLOAT_EQ(retrievedPosition.x, 0.0f);
    EXPECT_FLOAT_EQ(retrievedPosition.y, 0.0f);
    
    testPosition = { 1.0f, 1.0f };
    experimentPad->setPadPosition(testPosition);
    retrievedPosition = experimentPad->getPadPosition();
    EXPECT_FLOAT_EQ(retrievedPosition.x, 1.0f);
    EXPECT_FLOAT_EQ(retrievedPosition.y, 1.0f);
    
    // Test clamping (out of range values should be clamped)
    testPosition = { -0.5f, 1.5f };
    experimentPad->setPadPosition(testPosition);
    retrievedPosition = experimentPad->getPadPosition();
    EXPECT_FLOAT_EQ(retrievedPosition.x, 0.0f);
    EXPECT_FLOAT_EQ(retrievedPosition.y, 1.0f);
}

//==============================================================================
// Task 8.1.2: Test Y-axis pitch shifting calculations
TEST_F(ExperimentPadComponentTest, PitchShiftCalculations)
{
    // Set default SPAWN-style pitch range (-12 to +12 semitones)
    experimentPad->setPitchRange(-12.0f, 12.0f);
    
    // Test bottom position (should be minimum pitch)
    ExperimentPadComponent::PadPosition bottomPosition = { 0.5f, 0.0f };
    experimentPad->setPadPosition(bottomPosition);
    EXPECT_FLOAT_EQ(experimentPad->getCurrentPitchShift(), -12.0f);
    
    // Test center position (should be no pitch shift)
    ExperimentPadComponent::PadPosition centerPosition = { 0.5f, 0.5f };
    experimentPad->setPadPosition(centerPosition);
    EXPECT_FLOAT_EQ(experimentPad->getCurrentPitchShift(), 0.0f);
    
    // Test top position (should be maximum pitch)
    ExperimentPadComponent::PadPosition topPosition = { 0.5f, 1.0f };
    experimentPad->setPadPosition(topPosition);
    EXPECT_FLOAT_EQ(experimentPad->getCurrentPitchShift(), 12.0f);
    
    // Test quarter position
    ExperimentPadComponent::PadPosition quarterPosition = { 0.5f, 0.25f };
    experimentPad->setPadPosition(quarterPosition);
    EXPECT_FLOAT_EQ(experimentPad->getCurrentPitchShift(), -6.0f);
}

//==============================================================================
// Task 8.1.3: Test X-axis effects morphing
TEST_F(ExperimentPadComponentTest, EffectsMorphing)
{
    // Clear default mappings and add test mappings
    experimentPad->clearEffectMappings();
    experimentPad->addEffectMapping("Filter Cutoff", 200.0f, 8000.0f);
    experimentPad->addEffectMapping("Reverb Mix", 0.0f, 1.0f);
    
    // Test left position (minimum effect values)
    ExperimentPadComponent::PadPosition leftPosition = { 0.0f, 0.5f };
    experimentPad->setPadPosition(leftPosition);
    
    auto effectValues = experimentPad->getCurrentEffectValues();
    ASSERT_EQ(effectValues.size(), 2);
    
    EXPECT_EQ(effectValues[0].effectName, "Filter Cutoff");
    EXPECT_FLOAT_EQ(effectValues[0].currentValue, 200.0f);
    
    EXPECT_EQ(effectValues[1].effectName, "Reverb Mix");
    EXPECT_FLOAT_EQ(effectValues[1].currentValue, 0.0f);
    
    // Test right position (maximum effect values)
    ExperimentPadComponent::PadPosition rightPosition = { 1.0f, 0.5f };
    experimentPad->setPadPosition(rightPosition);
    
    effectValues = experimentPad->getCurrentEffectValues();
    EXPECT_FLOAT_EQ(effectValues[0].currentValue, 8000.0f);
    EXPECT_FLOAT_EQ(effectValues[1].currentValue, 1.0f);
    
    // Test center position (middle effect values)
    ExperimentPadComponent::PadPosition centerPosition = { 0.5f, 0.5f };
    experimentPad->setPadPosition(centerPosition);
    
    effectValues = experimentPad->getCurrentEffectValues();
    EXPECT_FLOAT_EQ(effectValues[0].currentValue, 4100.0f); // (200 + 8000) / 2
    EXPECT_FLOAT_EQ(effectValues[1].currentValue, 0.5f);    // (0 + 1) / 2
}

//==============================================================================
// Task 8.1.4: Test callback functionality
TEST_F(ExperimentPadComponentTest, CallbackFunctionality)
{
    // Test variables to track callback invocations
    bool pitchCallbackInvoked = false;
    bool effectsCallbackInvoked = false;
    bool positionCallbackInvoked = false;
    
    float receivedPitchShift = 0.0f;
    std::vector<ExperimentPadComponent::EffectMapping> receivedEffects;
    ExperimentPadComponent::PadPosition receivedPosition;
    
    // Set up callbacks
    experimentPad->onPitchChanged = [&](float pitchShift) {
        pitchCallbackInvoked = true;
        receivedPitchShift = pitchShift;
    };
    
    experimentPad->onEffectsChanged = [&](const std::vector<ExperimentPadComponent::EffectMapping>& effects) {
        effectsCallbackInvoked = true;
        receivedEffects = effects;
    };
    
    experimentPad->onPositionChanged = [&](const ExperimentPadComponent::PadPosition& position) {
        positionCallbackInvoked = true;
        receivedPosition = position;
    };
    
    // Trigger callbacks by changing position
    ExperimentPadComponent::PadPosition testPosition = { 0.75f, 0.25f };
    experimentPad->setPadPosition(testPosition);
    
    // Verify callbacks were invoked
    EXPECT_TRUE(pitchCallbackInvoked);
    EXPECT_TRUE(effectsCallbackInvoked);
    EXPECT_TRUE(positionCallbackInvoked);
    
    // Verify received values
    EXPECT_FLOAT_EQ(receivedPosition.x, 0.75f);
    EXPECT_FLOAT_EQ(receivedPosition.y, 0.25f);
    EXPECT_FLOAT_EQ(receivedPitchShift, -6.0f); // 25% up from bottom = -6 semitones
}

//==============================================================================
// Task 8.1.6: Test preset save/load functionality
TEST_F(ExperimentPadComponentTest, PresetManagement)
{
    // Set up a custom configuration
    experimentPad->setPitchRange(-24.0f, 24.0f);
    experimentPad->clearEffectMappings();
    experimentPad->addEffectMapping("Custom Effect", 100.0f, 500.0f);
    
    ExperimentPadComponent::PadPosition testPosition = { 0.3f, 0.7f };
    experimentPad->setPadPosition(testPosition);
    
    // Test preset callback
    bool presetCallbackInvoked = false;
    ExperimentPadComponent::PadPreset receivedPreset;
    
    experimentPad->setPresetCallback([&](const ExperimentPadComponent::PadPreset& preset) {
        presetCallbackInvoked = true;
        receivedPreset = preset;
    });
    
    // Save preset
    experimentPad->savePreset("Test Preset");
    
    // Verify preset callback was invoked
    EXPECT_TRUE(presetCallbackInvoked);
    EXPECT_EQ(receivedPreset.name, "Test Preset");
    EXPECT_FLOAT_EQ(receivedPreset.position.x, 0.3f);
    EXPECT_FLOAT_EQ(receivedPreset.position.y, 0.7f);
    EXPECT_FLOAT_EQ(receivedPreset.pitchMin, -24.0f);
    EXPECT_FLOAT_EQ(receivedPreset.pitchMax, 24.0f);
    EXPECT_EQ(receivedPreset.effectMappings.size(), 1);
    EXPECT_EQ(receivedPreset.effectMappings[0].effectName, "Custom Effect");
    
    // Test loading preset (change configuration first)
    experimentPad->setPitchRange(-6.0f, 6.0f);
    experimentPad->clearEffectMappings();
    experimentPad->setPadPosition({ 0.0f, 0.0f });
    
    // Load the preset
    experimentPad->loadPreset(receivedPreset);
    
    // Verify configuration was restored
    auto restoredPosition = experimentPad->getPadPosition();
    EXPECT_FLOAT_EQ(restoredPosition.x, 0.3f);
    EXPECT_FLOAT_EQ(restoredPosition.y, 0.7f);
    
    auto restoredEffects = experimentPad->getCurrentEffectValues();
    EXPECT_EQ(restoredEffects.size(), 1);
    EXPECT_EQ(restoredEffects[0].effectName, "Custom Effect");
}
