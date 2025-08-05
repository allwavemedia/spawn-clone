//==============================================================================
/*
  SynthesisParameterMapper_Fixed.h
  Author:  BMad Dev Agent - Phase 1B & 1C Implementation (Fixed)
  
  Converts AI generation parameters to synthesis engine parameters.
  Maps musical characteristics from GenerationParameters to AdvancedSynthesisEngine.
  
  ARCHITECTURE:
  - Maps style/harmonic settings to synthesis characteristics
  - Converts musical theory (scales, complexity) to sound parameters
  - Provides real-time parameter updates for UI control
  - Thread-safe parameter caching for performance
*/
//==============================================================================

#pragma once

#include "../GenerationParameters.h"
#include "AdvancedSynthesisEngine.h"
#include <juce_core/juce_core.h>
#include <unordered_map>

namespace spawnclone::audio
{

//==============================================================================
/**
 * Maps AI generation parameters to synthesis engine parameters.
 * 
 * Phase 1B: Converts GenerationParameters to AdvancedSynthesisEngine::SynthesisParameters
 * Phase 1C: Provides real-time parameter control with smooth interpolation
 */
class SynthesisParameterMapper
{
public:
    //==============================================================================
    SynthesisParameterMapper();
    
    //==============================================================================
    // PHASE 1B: AI-to-Synthesis Parameter Mapping
    
    /**
     * Convert AI generation parameters to synthesis parameters.
     * Maps harmonic style, musical complexity, and generation type to synthesis settings.
     */
    AdvancedSynthesisEngine::SynthesisParameters mapAIToSynthesis(
        const GenerationParameters& aiParams) const;
    
    /**
     * Get style-specific synthesis preset.
     * Returns optimized synthesis parameters for specific harmonic progression styles.
     */
    AdvancedSynthesisEngine::SynthesisParameters getStylePreset(
        GenerationParameters::HarmonicSettings::ProgressionStyle style,
        GenerationParameters::GenerationType generationType) const;
    
    /**
     * Apply music theory characteristics to synthesis.
     * Modifies synthesis parameters based on scale, mode, and harmonic content.
     */
    void applyMusicTheoryToSynthesis(
        AdvancedSynthesisEngine::SynthesisParameters& synthParams,
        const GenerationParameters& aiParams) const;
    
    //==============================================================================
    // PHASE 1C: Real-time Parameter Control
    
    /**
     * Update a single synthesis parameter in real-time with optional interpolation.
     * Thread-safe with performance throttling.
     */
    void updateRealTimeParameters(
        AdvancedSynthesisEngine* synthesisEngine,
        const juce::String& parameterName,
        float value,
        bool shouldInterpolate = true) const;
    
    /**
     * Update multiple synthesis parameters efficiently in a single operation.
     * Optimized for UI control scenarios with many parameter changes.
     */
    void batchUpdateParameters(
        AdvancedSynthesisEngine* synthesisEngine,
        const juce::StringPairArray& parameterUpdates) const;
    
    /**
     * Get current cached value of a synthesis parameter.
     * Returns interpolated value if parameter is being smoothly updated.
     */
    float getSynthesisParameterValue(
        const AdvancedSynthesisEngine* synthesisEngine,
        const juce::String& parameterName) const;

private:
    //==============================================================================
    // Style-specific characteristic definitions
    
    struct StyleCharacteristics
    {
        int preferredWavetableIndex = 0;
        AdvancedSynthesisEngine::FilterParams::Type preferredFilterType = AdvancedSynthesisEngine::FilterParams::Type::LowPass;
        AdvancedSynthesisEngine::LFOParams::Waveform preferredLfoWaveform = AdvancedSynthesisEngine::LFOParams::Triangle;
        
        // Parameter ranges for musical style characteristics
        float filterCutoffRange[2] = {400.0f, 12000.0f};
        float filterResonanceRange[2] = {0.1f, 0.7f};
        float attackRange[2] = {0.01f, 0.2f};
        float decayRange[2] = {0.1f, 1.0f};
        float sustainRange[2] = {0.3f, 0.9f};
        float releaseRange[2] = {0.2f, 2.0f};
        float lfoRateRange[2] = {0.5f, 8.0f};
        float lfoDepthRange[2] = {0.0f, 0.4f};
        float wavetablePositionRange[2] = {0.0f, 1.0f};
        
        // Sonic characteristic multipliers
        float morphRateMultiplier = 1.0f;
        float brightnessMultiplier = 1.0f;
        float warmthMultiplier = 1.0f;
        float saturationMultiplier = 1.0f;
    };
    
    /** Get synthesis characteristics for specific progression styles */
    StyleCharacteristics getStyleCharacteristics(GenerationParameters::HarmonicSettings::ProgressionStyle style) const;
    
    /** Map generation type to synthesis type */
    AdvancedSynthesisEngine::SynthesisType mapGenerationTypeToSynthesis(
        GenerationParameters::GenerationType generationType) const;
    
    /** Calculate smooth interpolation factor based on parameter difference */
    float calculateInterpolationFactor(float currentValue, float targetValue) const;
    
    /** Apply complexity-based parameter adjustments */
    void applyComplexityMapping(
        AdvancedSynthesisEngine::SynthesisParameters& synthParams,
        float rhythmicComplexity, float noteDensity) const;
    
    /** Apply tempo-based parameter adjustments */
    void applyTempoMapping(
        AdvancedSynthesisEngine::SynthesisParameters& synthParams,
        double tempo) const;
    
    /** Apply harmonic complexity to synthesis parameters */
    void applyHarmonicMapping(
        AdvancedSynthesisEngine::SynthesisParameters& synthParams,
        const GenerationParameters::HarmonicSettings& harmony) const;
    
    //==============================================================================
    // Thread-safe parameter caching and performance optimization
    
    mutable juce::CriticalSection parameterLock;
    mutable std::unordered_map<juce::String, float> parameterCache;
    mutable juce::Time lastUpdateTime;
    
    // Performance throttling
    static constexpr int PARAMETER_UPDATE_THROTTLE_MS = 16; // ~60 FPS update rate
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SynthesisParameterMapper)
};

} // namespace spawnclone::audio
