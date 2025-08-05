//==============================================================================
/*
  SynthesisParameterMapper_Fixed.cpp
  Author:  BMad Dev Agent - Phase 1B & 1C Implementation (Fixed)
  
  Implementation of AI-to-Synthesis parameter mapping with correct GenerationParameters.
*/
//==============================================================================

#include "SynthesisParameterMapper_Fixed.h"
#include <cmath>

namespace spawnclone::audio
{

SynthesisParameterMapper::SynthesisParameterMapper()
{
    lastUpdateTime = juce::Time::getCurrentTime();
}

//==============================================================================
// PHASE 1B: AI-to-Synthesis Parameter Mapping

AdvancedSynthesisEngine::SynthesisParameters 
SynthesisParameterMapper::mapAIToSynthesis(const GenerationParameters& aiParams) const
{
    // Start with style-specific preset
    auto synthParams = getStylePreset(aiParams.harmony.style, aiParams.generationType);
    
    // Apply music theory characteristics
    applyMusicTheoryToSynthesis(synthParams, aiParams);
    
    // Apply AI-specific characteristics
    applyComplexityMapping(synthParams, aiParams.rhythmicComplexity, aiParams.noteDensity);
    applyTempoMapping(synthParams, aiParams.tempo);
    applyHarmonicMapping(synthParams, aiParams.harmony);
    
    return synthParams;
}

AdvancedSynthesisEngine::SynthesisParameters 
SynthesisParameterMapper::getStylePreset(
    GenerationParameters::HarmonicSettings::ProgressionStyle style,
    GenerationParameters::GenerationType generationType) const
{
    AdvancedSynthesisEngine::SynthesisParameters params;
    auto characteristics = getStyleCharacteristics(style);
    
    // Set synthesis type based on generation type
    params.synthesisType = mapGenerationTypeToSynthesis(generationType);
    
    // Configure wavetable parameters
    params.wavetable.wavetableIndex = characteristics.preferredWavetableIndex;
    params.wavetable.wavetablePosition = (characteristics.wavetablePositionRange[0] + 
                                         characteristics.wavetablePositionRange[1]) * 0.5f;
    params.wavetable.morphRate = 2.0f * characteristics.morphRateMultiplier;
    
    // Configure filter parameters
    params.filter.enabled = true;
    params.filter.filterType = characteristics.preferredFilterType;
    params.filter.cutoff = (characteristics.filterCutoffRange[0] + 
                           characteristics.filterCutoffRange[1]) * 0.5f;
    params.filter.resonance = (characteristics.filterResonanceRange[0] + 
                              characteristics.filterResonanceRange[1]) * 0.5f;
    params.filter.keyTracking = 0.3f;
    params.filter.velocityTracking = 0.4f;
    
    // Configure envelope parameters
    params.envelope.attack = (characteristics.attackRange[0] + characteristics.attackRange[1]) * 0.5f;
    params.envelope.decay = (characteristics.decayRange[0] + characteristics.decayRange[1]) * 0.5f;
    params.envelope.sustain = (characteristics.sustainRange[0] + characteristics.sustainRange[1]) * 0.5f;
    params.envelope.release = (characteristics.releaseRange[0] + characteristics.releaseRange[1]) * 0.5f;
    
    // Configure modulation parameters
    params.modulation.lfoRate = (characteristics.lfoRateRange[0] + characteristics.lfoRateRange[1]) * 0.5f;
    params.modulation.lfoDepth = (characteristics.lfoDepthRange[0] + characteristics.lfoDepthRange[1]) * 0.5f;
    params.modulation.lfoWaveform = characteristics.preferredLfoWaveform;
    params.modulation.modulationTarget = 2; // Filter modulation
    params.modulation.bipolar = true;
    
    // Configure audio character
    params.brightness = characteristics.brightnessMultiplier;
    params.warmth = characteristics.warmthMultiplier;
    params.saturation = characteristics.saturationMultiplier;
    params.masterVolume = 0.7f;
    
    return params;
}

void SynthesisParameterMapper::applyMusicTheoryToSynthesis(
    AdvancedSynthesisEngine::SynthesisParameters& synthParams,
    const GenerationParameters& aiParams) const
{
    // Apply scale-based filter adjustments
    switch (aiParams.scale)
    {
        case GenerationParameters::ScaleType::Major:
            synthParams.brightness *= 1.1f; // Brighter for major scales
            synthParams.filter.cutoff *= 1.15f;
            break;
            
        case GenerationParameters::ScaleType::Minor:
            synthParams.warmth *= 1.2f; // Warmer for minor scales
            synthParams.filter.cutoff *= 0.9f;
            synthParams.filter.resonance *= 1.1f;
            break;
            
        case GenerationParameters::ScaleType::Pentatonic:
            synthParams.wavetable.wavetablePosition = 0.3f; // More fundamental-rich
            synthParams.saturation *= 1.15f;
            break;
            
        case GenerationParameters::ScaleType::Blues:
            synthParams.saturation *= 1.3f; // More character for blues
            synthParams.filter.resonance *= 1.2f;
            synthParams.modulation.lfoDepth *= 1.1f;
            break;
            
        case GenerationParameters::ScaleType::Dorian:
            synthParams.wavetable.morphRate *= 1.3f; // More movement
            synthParams.modulation.lfoRate *= 1.1f;
            break;
            
        case GenerationParameters::ScaleType::Mixolydian:
            synthParams.brightness *= 1.2f;
            synthParams.filter.cutoff *= 1.1f;
            break;
    }
    
    // Apply melodic contour influence
    switch (aiParams.melodicContour)
    {
        case GenerationParameters::MelodicContour::Rising:
            synthParams.filter.cutoff *= 1.1f; // Brighter for rising contours
            synthParams.envelope.attack *= 0.9f;
            break;
            
        case GenerationParameters::MelodicContour::Falling:
            synthParams.warmth *= 1.1f; // Warmer for falling contours  
            synthParams.envelope.release *= 1.2f;
            break;
            
        case GenerationParameters::MelodicContour::Wave:
            synthParams.modulation.lfoDepth *= 1.2f; // More modulation for wave-like motion
            break;
            
        case GenerationParameters::MelodicContour::Arch:
            synthParams.envelope.attack *= 1.2f;
            synthParams.envelope.release *= 1.2f;
            break;
            
        case GenerationParameters::MelodicContour::Random:
            synthParams.modulation.lfoRate *= 1.1f;
            break;
    }
}

//==============================================================================
// PHASE 1C: Real-time Parameter Control

void SynthesisParameterMapper::updateRealTimeParameters(
    AdvancedSynthesisEngine* synthesisEngine,
    const juce::String& parameterName,
    float value,
    bool shouldInterpolate) const
{
    if (!synthesisEngine)
        return;
    
    // Throttle updates for performance
    auto currentTime = juce::Time::getCurrentTime();
    if ((currentTime - lastUpdateTime).inMilliseconds() < PARAMETER_UPDATE_THROTTLE_MS)
        return;
    
    lastUpdateTime = currentTime;
    
    // Thread-safe parameter update
    {
        juce::ScopedLock lock(parameterLock);
        
        float finalValue = value;
        
        // Apply interpolation if requested
        if (shouldInterpolate && parameterCache.find(parameterName) != parameterCache.end())
        {
            float currentValue = parameterCache[parameterName];
            float interpolationFactor = calculateInterpolationFactor(currentValue, value);
            finalValue = currentValue + (value - currentValue) * interpolationFactor;
        }
        
        // Update synthesis engine
        synthesisEngine->setParameter(parameterName, finalValue);
        
        // Cache the value
        parameterCache[parameterName] = finalValue;
    }
}

void SynthesisParameterMapper::batchUpdateParameters(
    AdvancedSynthesisEngine* synthesisEngine,
    const juce::StringPairArray& parameterUpdates) const
{
    if (!synthesisEngine)
        return;
    
    juce::ScopedLock lock(parameterLock);
    
    // Process all parameter updates in a batch for efficiency
    for (int i = 0; i < parameterUpdates.size(); ++i)
    {
        auto paramName = parameterUpdates.getAllKeys()[i];
        float value = parameterUpdates.getValue(paramName, "0.0").getFloatValue();
        
        synthesisEngine->setParameter(paramName, value);
        parameterCache[paramName] = value;
    }
}

float SynthesisParameterMapper::getSynthesisParameterValue(
    const AdvancedSynthesisEngine* synthesisEngine,
    const juce::String& parameterName) const
{
    juce::ScopedLock lock(parameterLock);
    
    auto it = parameterCache.find(parameterName);
    if (it != parameterCache.end())
        return it->second;
    
    return 0.0f; // Default value if not cached
}

//==============================================================================
// Private Implementation Methods

SynthesisParameterMapper::StyleCharacteristics 
SynthesisParameterMapper::getStyleCharacteristics(GenerationParameters::HarmonicSettings::ProgressionStyle style) const
{
    StyleCharacteristics characteristics;
    
    switch (style)
    {
        case GenerationParameters::HarmonicSettings::ProgressionStyle::Pop:
            characteristics.preferredWavetableIndex = 0; // Sine-based
            characteristics.preferredFilterType = AdvancedSynthesisEngine::FilterParams::LowPass;
            characteristics.filterCutoffRange[0] = 800.0f;
            characteristics.filterCutoffRange[1] = 12000.0f;
            characteristics.attackRange[0] = 0.01f;
            characteristics.attackRange[1] = 0.1f;
            characteristics.brightnessMultiplier = 1.2f;
            characteristics.lfoRateRange[0] = 2.0f;
            characteristics.lfoRateRange[1] = 6.0f;
            break;
            
        case GenerationParameters::HarmonicSettings::ProgressionStyle::Jazz:
            characteristics.preferredWavetableIndex = 4; // Complex harmonics
            characteristics.preferredFilterType = AdvancedSynthesisEngine::FilterParams::LowPass;
            characteristics.filterCutoffRange[0] = 600.0f;
            characteristics.filterCutoffRange[1] = 8000.0f;
            characteristics.warmthMultiplier = 1.3f;
            characteristics.attackRange[0] = 0.05f;
            characteristics.attackRange[1] = 0.3f;
            characteristics.lfoRateRange[0] = 1.0f;
            characteristics.lfoRateRange[1] = 4.0f;
            break;
            
        case GenerationParameters::HarmonicSettings::ProgressionStyle::Electronic:
            characteristics.preferredWavetableIndex = 2; // Square-based
            characteristics.preferredFilterType = AdvancedSynthesisEngine::FilterParams::StateVariable;
            characteristics.filterCutoffRange[0] = 400.0f;
            characteristics.filterCutoffRange[1] = 16000.0f;
            characteristics.filterResonanceRange[0] = 0.4f;
            characteristics.filterResonanceRange[1] = 0.9f;
            characteristics.morphRateMultiplier = 2.0f;
            characteristics.brightnessMultiplier = 1.5f;
            characteristics.lfoRateRange[0] = 4.0f;
            characteristics.lfoRateRange[1] = 16.0f;
            break;
            
        case GenerationParameters::HarmonicSettings::ProgressionStyle::Classical:
            characteristics.preferredWavetableIndex = 0; // Pure tones
            characteristics.preferredFilterType = AdvancedSynthesisEngine::FilterParams::LowPass;
            characteristics.filterCutoffRange[0] = 1000.0f;
            characteristics.filterCutoffRange[1] = 10000.0f;
            characteristics.warmthMultiplier = 1.1f;
            characteristics.attackRange[0] = 0.02f;
            characteristics.attackRange[1] = 0.2f;
            characteristics.lfoRateRange[0] = 0.5f;
            characteristics.lfoRateRange[1] = 3.0f;
            break;
            
        case GenerationParameters::HarmonicSettings::ProgressionStyle::Modal:
            characteristics.preferredWavetableIndex = 3; // Rich harmonics
            characteristics.preferredFilterType = AdvancedSynthesisEngine::FilterParams::BandPass;
            characteristics.filterCutoffRange[0] = 500.0f;
            characteristics.filterCutoffRange[1] = 8000.0f;
            characteristics.warmthMultiplier = 1.2f;
            characteristics.saturationMultiplier = 1.1f;
            characteristics.lfoRateRange[0] = 0.8f;
            characteristics.lfoRateRange[1] = 5.0f;
            break;
            
        case GenerationParameters::HarmonicSettings::ProgressionStyle::Cinematic:
            characteristics.preferredWavetableIndex = 5; // Evolving textures
            characteristics.preferredFilterType = AdvancedSynthesisEngine::FilterParams::LowPass;
            characteristics.filterCutoffRange[0] = 300.0f;
            characteristics.filterCutoffRange[1] = 15000.0f;
            characteristics.morphRateMultiplier = 1.5f;
            characteristics.warmthMultiplier = 1.3f;
            characteristics.attackRange[0] = 0.1f;
            characteristics.attackRange[1] = 1.0f;
            characteristics.releaseRange[0] = 0.5f;
            characteristics.releaseRange[1] = 4.0f;
            break;
            
        case GenerationParameters::HarmonicSettings::ProgressionStyle::Experimental:
            characteristics.preferredWavetableIndex = 6; // Unusual timbres
            characteristics.preferredFilterType = AdvancedSynthesisEngine::FilterParams::StateVariable;
            characteristics.filterCutoffRange[0] = 200.0f;
            characteristics.filterCutoffRange[1] = 18000.0f;
            characteristics.filterResonanceRange[0] = 0.2f;
            characteristics.filterResonanceRange[1] = 0.8f;
            characteristics.morphRateMultiplier = 2.5f;
            characteristics.saturationMultiplier = 1.4f;
            characteristics.lfoRateRange[0] = 0.1f;
            characteristics.lfoRateRange[1] = 12.0f;
            break;
    }
    
    return characteristics;
}

AdvancedSynthesisEngine::SynthesisType 
SynthesisParameterMapper::mapGenerationTypeToSynthesis(
    GenerationParameters::GenerationType generationType) const
{
    switch (generationType)
    {
        case GenerationParameters::GenerationType::Melody:
            return AdvancedSynthesisEngine::SynthesisType::Wavetable; // Rich harmonic content
            
        case GenerationParameters::GenerationType::Chords:
            return AdvancedSynthesisEngine::SynthesisType::Hybrid; // Complex harmonic structures
            
        case GenerationParameters::GenerationType::Bassline:
            return AdvancedSynthesisEngine::SynthesisType::Subtractive; // Powerful low-end
            
        case GenerationParameters::GenerationType::Drums:
            return AdvancedSynthesisEngine::SynthesisType::Sample; // Percussive samples
            
        default:
            return AdvancedSynthesisEngine::SynthesisType::Wavetable;
    }
}

float SynthesisParameterMapper::calculateInterpolationFactor(float currentValue, float targetValue) const
{
    // Smooth interpolation factor based on difference magnitude
    float difference = std::abs(targetValue - currentValue);
    
    if (difference < 0.01f)
        return 1.0f; // Snap to target for small differences
    else if (difference < 0.1f)
        return 0.3f; // Moderate interpolation
    else
        return 0.1f; // Slow interpolation for large changes
}

void SynthesisParameterMapper::applyComplexityMapping(
    AdvancedSynthesisEngine::SynthesisParameters& synthParams,
    float rhythmicComplexity, float noteDensity) const
{
    // Higher complexity = more modulation and harmonic richness
    float avgComplexity = (rhythmicComplexity + noteDensity) * 0.5f;
    avgComplexity = juce::jlimit(0.0f, 1.0f, avgComplexity);
    
    synthParams.modulation.lfoDepth *= (0.5f + avgComplexity * 0.8f);
    synthParams.modulation.lfo2Depth *= avgComplexity;
    synthParams.wavetable.morphRate *= (0.8f + avgComplexity * 1.4f);
    synthParams.filter.resonance *= (0.8f + avgComplexity * 0.4f);
    
    // Add secondary modulation for high complexity
    if (avgComplexity > 0.7f)
    {
        synthParams.modulation.enableCrossModulation = true;
        synthParams.modulation.crossModAmount = (avgComplexity - 0.7f) * 0.3f;
    }
}

void SynthesisParameterMapper::applyTempoMapping(
    AdvancedSynthesisEngine::SynthesisParameters& synthParams,
    double tempo) const
{
    // Sync modulation rates to tempo
    float tempoFactor = static_cast<float>(tempo / 120.0); // Normalize to 120 BPM
    
    synthParams.modulation.lfoRate *= tempoFactor;
    synthParams.modulation.lfo2Rate *= tempoFactor;
    synthParams.wavetable.morphRate *= tempoFactor;
    
    // Adjust envelope timing for tempo
    if (tempo > 140.0) // Fast tempo
    {
        synthParams.envelope.attack *= 0.8f;
        synthParams.envelope.decay *= 0.9f;
        synthParams.envelope.release *= 0.8f;
    }
    else if (tempo < 90.0) // Slow tempo
    {
        synthParams.envelope.attack *= 1.3f;
        synthParams.envelope.decay *= 1.2f;
        synthParams.envelope.release *= 1.4f;
    }
}

void SynthesisParameterMapper::applyHarmonicMapping(
    AdvancedSynthesisEngine::SynthesisParameters& synthParams,
    const GenerationParameters::HarmonicSettings& harmony) const
{
    // Apply harmonic complexity
    switch (harmony.complexity)
    {
        case GenerationParameters::HarmonicSettings::HarmonicComplexity::Basic:
            synthParams.brightness *= 0.9f; // Simpler timbres
            break;
            
        case GenerationParameters::HarmonicSettings::HarmonicComplexity::Intermediate:
            synthParams.modulation.lfoDepth *= 1.1f;
            break;
            
        case GenerationParameters::HarmonicSettings::HarmonicComplexity::Advanced:
            synthParams.modulation.lfoDepth *= 1.3f;
            synthParams.filter.resonance *= 1.1f;
            break;
            
        case GenerationParameters::HarmonicSettings::HarmonicComplexity::Jazz:
            synthParams.modulation.lfoDepth *= 1.4f;
            synthParams.saturation *= 1.2f;
            synthParams.warmth *= 1.1f;
            break;
    }
    
    // Apply chromaticism
    synthParams.saturation *= (1.0f + harmony.chromaticism * 0.3f);
    synthParams.modulation.lfoDepth *= (1.0f + harmony.chromaticism * 0.2f);
    
    // Apply modal interchange
    synthParams.warmth *= (1.0f + harmony.modalInterchange * 0.2f);
    
    // Apply harmonic rhythm (faster chord changes = more modulation)
    synthParams.modulation.lfoRate *= (0.8f + harmony.harmonicRhythm * 0.4f);
}

} // namespace spawnclone::audio
