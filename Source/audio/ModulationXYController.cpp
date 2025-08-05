//==============================================================================
/*
  ModulationXYController.cpp
  Author: BMad Dev Agent - Epic 6 Story 6.11
  
  Implementation of XY Controller for ModulationEngine with multi-parameter control.
*/
//==============================================================================

#include "ModulationXYController.h"
#include <cmath>

namespace spawnclone::audio
{
    //==============================================================================
    // ModulationXYController Implementation
    //==============================================================================
    
    ModulationXYController::ModulationXYController()
    {
        initializePresets();
        setPreset(ModulationPreset::SubtleChorus); // Default preset
    }

    //==============================================================================
    // XY Controller Interface
    //==============================================================================
    
    void ModulationXYController::setXYPosition(float x, float y)
    {
        juce::Point<float> newXY(juce::jlimit(0.0f, 1.0f, x), juce::jlimit(0.0f, 1.0f, y));
        newXY = constrainToBounds(newXY);
        
        if (smoothingEnabled)
        {
            targetXY = newXY;
            
            // Update smoothers with new target values
            rateSmoother.setTargetValue(mapXToRate(newXY.x));
            depthSmoother.setTargetValue(mapYToDepth(newXY.y));
        }
        else
        {
            currentXY = newXY;
            targetXY = newXY;
            
            // Reset smoothers to new values
            rateSmoother.reset(mapXToRate(newXY.x));
            depthSmoother.reset(mapYToDepth(newXY.y));
        }
        
        updateTargetEngine();
    }

    void ModulationXYController::setXPosition(float x)
    {
        setXYPosition(x, currentXY.y);
    }

    void ModulationXYController::setYPosition(float y)
    {
        setXYPosition(currentXY.x, y);
    }

    //==============================================================================
    // Preset Management
    //==============================================================================
    
    void ModulationXYController::setPreset(ModulationPreset preset)
    {
        currentPreset = preset;
        
        // Reset XY position to center for new preset
        setXYPosition(0.5f, 0.5f);
        
        // Apply preset-specific ModulationEngine settings if we have a target
        if (targetEngine)
        {
            const auto& config = presetConfigs[preset];
            targetEngine->setModulationType(config.modulationType);
            targetEngine->setFeedback(config.feedback);
            targetEngine->setStereoSpread(config.stereoSpread);
            targetEngine->setMix(config.mix);
        }
    }

    juce::String ModulationXYController::getPresetName(ModulationPreset preset) const
    {
        auto it = presetConfigs.find(preset);
        return (it != presetConfigs.end()) ? it->second.name : "Unknown";
    }

    std::vector<ModulationXYController::ModulationPreset> ModulationXYController::getAvailablePresets() const
    {
        return {
            ModulationPreset::SubtleChorus,
            ModulationPreset::WideChorus,
            ModulationPreset::ClassicFlanger,
            ModulationPreset::JetFlanger,
            ModulationPreset::VintagePhaser,
            ModulationPreset::IntensePhaser,
            ModulationPreset::SlowTremolo,
            ModulationPreset::ChoppyTremolo,
            ModulationPreset::SmoothVibrato,
            ModulationPreset::WideVibrato,
            ModulationPreset::SlowAutoPan,
            ModulationPreset::FastAutoPan,
            ModulationPreset::WarmRingMod,
            ModulationPreset::MetallicRingMod,
            ModulationPreset::SlowRotary,
            ModulationPreset::FastRotary
        };
    }

    std::vector<ModulationXYController::ModulationPreset> ModulationXYController::getPresetsForType(ModulationEngine::ModulationType type) const
    {
        std::vector<ModulationPreset> typePresets;
        
        for (const auto& [preset, config] : presetConfigs)
        {
            if (config.modulationType == type)
            {
                typePresets.push_back(preset);
            }
        }
        
        return typePresets;
    }

    //==============================================================================
    // Parameter Mapping
    //==============================================================================
    
    float ModulationXYController::getMappedRate() const
    {
        return rateSmoother.getCurrentValue();
    }

    float ModulationXYController::getMappedDepth() const
    {
        return depthSmoother.getCurrentValue();
    }

    float ModulationXYController::getMappedFeedback() const
    {
        const auto& config = presetConfigs.at(currentPreset);
        return config.feedback;
    }

    float ModulationXYController::getMappedStereoSpread() const
    {
        const auto& config = presetConfigs.at(currentPreset);
        return config.stereoSpread;
    }

    float ModulationXYController::getMappedMix() const
    {
        const auto& config = presetConfigs.at(currentPreset);
        return config.mix;
    }

    ModulationEngine::ModulationType ModulationXYController::getMappedModulationType() const
    {
        const auto& config = presetConfigs.at(currentPreset);
        return config.modulationType;
    }

    //==============================================================================
    // ModulationEngine Integration
    //==============================================================================
    
    void ModulationXYController::applyToModulationEngine(ModulationEngine& modulationEngine)
    {
        modulationEngine.setModulationType(getMappedModulationType());
        modulationEngine.setRate(getMappedRate());
        modulationEngine.setDepth(getMappedDepth());
        modulationEngine.setFeedback(getMappedFeedback());
        modulationEngine.setStereoSpread(getMappedStereoSpread());
        modulationEngine.setMix(getMappedMix());
    }

    void ModulationXYController::setTargetModulationEngine(ModulationEngine* engine)
    {
        targetEngine = engine;
        if (targetEngine)
        {
            // Apply current preset settings
            setPreset(currentPreset);
            updateTargetEngine();
        }
    }

    //==============================================================================
    // Smoothing and Interpolation
    //==============================================================================
    
    void ModulationXYController::setSmoothingTime(float timeSeconds)
    {
        smoothingTime = juce::jlimit(0.001f, 1.0f, timeSeconds);
        
        // Update smoothers with new time
        rateSmoother.prepareToPlay(sampleRate, smoothingTime);
        depthSmoother.prepareToPlay(sampleRate, smoothingTime);
    }

    void ModulationXYController::prepareToPlay(double sr, int samplesPerBlock)
    {
        sampleRate = sr;
        blockSize = samplesPerBlock;
        
        // Prepare parameter smoothers
        rateSmoother.prepareToPlay(sampleRate, smoothingTime);
        depthSmoother.prepareToPlay(sampleRate, smoothingTime);
        
        // Initialize with current values
        rateSmoother.reset(mapXToRate(currentXY.x));
        depthSmoother.reset(mapYToDepth(currentXY.y));
    }

    void ModulationXYController::processBlock()
    {
        if (!smoothingEnabled)
            return;
        
        // Update current parameters based on smoothing
        float smoothedRate = rateSmoother.getNextValue();
        float smoothedDepth = depthSmoother.getNextValue();
        
        // Update target engine if available
        if (targetEngine)
        {
            targetEngine->setRate(smoothedRate);
            targetEngine->setDepth(smoothedDepth);
        }
    }

    //==============================================================================
    // Utility Methods
    //==============================================================================
    
    juce::Rectangle<float> ModulationXYController::getXYBounds() const
    {
        // All presets use full 0-1 range for simplicity
        return juce::Rectangle<float>(0.0f, 0.0f, 1.0f, 1.0f);
    }

    juce::Point<float> ModulationXYController::constrainToBounds(juce::Point<float> xy) const
    {
        auto bounds = getXYBounds();
        return juce::Point<float>(
            juce::jlimit(bounds.getX(), bounds.getRight(), xy.x),
            juce::jlimit(bounds.getY(), bounds.getBottom(), xy.y)
        );
    }

    juce::String ModulationXYController::getParameterDescription() const
    {
        juce::String description;
        description << getCurrentPresetName() << ": ";
        description << "Rate: " << juce::String(getMappedRate(), 2) << "Hz, ";
        description << "Depth: " << juce::String(getMappedDepth() * 100.0f, 1) << "%";
        return description;
    }

    juce::ValueTree ModulationXYController::saveState() const
    {
        juce::ValueTree state("ModulationXYController");
        state.setProperty("x", currentXY.x, nullptr);
        state.setProperty("y", currentXY.y, nullptr);
        state.setProperty("preset", static_cast<int>(currentPreset), nullptr);
        state.setProperty("smoothingEnabled", smoothingEnabled, nullptr);
        state.setProperty("smoothingTime", smoothingTime, nullptr);
        return state;
    }

    void ModulationXYController::loadState(const juce::ValueTree& state)
    {
        if (state.hasType("ModulationXYController"))
        {
            float x = state.getProperty("x", 0.5f);
            float y = state.getProperty("y", 0.5f);
            int presetIndex = state.getProperty("preset", static_cast<int>(ModulationPreset::SubtleChorus));
            bool smoothing = state.getProperty("smoothingEnabled", true);
            float smoothTime = state.getProperty("smoothingTime", 0.1f);
            
            setSmoothingEnabled(smoothing);
            setSmoothingTime(smoothTime);
            setPreset(static_cast<ModulationPreset>(presetIndex));
            setXYPosition(x, y);
        }
    }

    //==============================================================================
    // Private Helper Methods
    //==============================================================================
    
    void ModulationXYController::initializePresets()
    {
        using ModType = ModulationEngine::ModulationType;
        
        // Chorus Presets
        presetConfigs[ModulationPreset::SubtleChorus] = {
            "Subtle Chorus", 0.5f, 2.0f, 0.1f, 0.6f, 0.0f, 0.3f, 0.4f, ModType::Chorus
        };
        presetConfigs[ModulationPreset::WideChorus] = {
            "Wide Chorus", 0.8f, 3.0f, 0.3f, 0.9f, 0.1f, 0.8f, 0.6f, ModType::Chorus
        };
        
        // Flanger Presets
        presetConfigs[ModulationPreset::ClassicFlanger] = {
            "Classic Flanger", 0.2f, 1.5f, 0.3f, 0.8f, 0.4f, 0.5f, 0.5f, ModType::Flanger
        };
        presetConfigs[ModulationPreset::JetFlanger] = {
            "Jet Flanger", 0.1f, 2.0f, 0.5f, 1.0f, 0.7f, 0.3f, 0.7f, ModType::Flanger
        };
        
        // Phaser Presets
        presetConfigs[ModulationPreset::VintagePhaser] = {
            "Vintage Phaser", 0.3f, 1.2f, 0.4f, 0.8f, 0.3f, 0.4f, 0.6f, ModType::Phaser
        };
        presetConfigs[ModulationPreset::IntensePhaser] = {
            "Intense Phaser", 0.5f, 3.0f, 0.6f, 1.0f, 0.5f, 0.6f, 0.8f, ModType::Phaser
        };
        
        // Tremolo Presets
        presetConfigs[ModulationPreset::SlowTremolo] = {
            "Slow Tremolo", 1.0f, 4.0f, 0.2f, 0.7f, 0.0f, 0.2f, 1.0f, ModType::Tremolo
        };
        presetConfigs[ModulationPreset::ChoppyTremolo] = {
            "Choppy Tremolo", 3.0f, 8.0f, 0.5f, 1.0f, 0.0f, 0.1f, 1.0f, ModType::Tremolo
        };
        
        // Vibrato Presets
        presetConfigs[ModulationPreset::SmoothVibrato] = {
            "Smooth Vibrato", 2.0f, 6.0f, 0.1f, 0.4f, 0.0f, 0.0f, 1.0f, ModType::Vibrato
        };
        presetConfigs[ModulationPreset::WideVibrato] = {
            "Wide Vibrato", 3.0f, 8.0f, 0.3f, 0.8f, 0.0f, 0.0f, 1.0f, ModType::Vibrato
        };
        
        // Auto-Pan Presets
        presetConfigs[ModulationPreset::SlowAutoPan] = {
            "Slow Auto-Pan", 0.2f, 1.0f, 0.4f, 1.0f, 0.0f, 0.0f, 1.0f, ModType::AutoPan
        };
        presetConfigs[ModulationPreset::FastAutoPan] = {
            "Fast Auto-Pan", 1.0f, 4.0f, 0.6f, 1.0f, 0.0f, 0.0f, 1.0f, ModType::AutoPan
        };
        
        // Ring Modulation Presets
        presetConfigs[ModulationPreset::WarmRingMod] = {
            "Warm Ring Mod", 20.0f, 100.0f, 0.2f, 0.7f, 0.0f, 0.3f, 0.4f, ModType::RingMod
        };
        presetConfigs[ModulationPreset::MetallicRingMod] = {
            "Metallic Ring Mod", 50.0f, 300.0f, 0.5f, 1.0f, 0.0f, 0.5f, 0.6f, ModType::RingMod
        };
        
        // Rotary Speaker Presets
        presetConfigs[ModulationPreset::SlowRotary] = {
            "Slow Rotary", 0.5f, 2.0f, 0.3f, 0.8f, 0.0f, 0.7f, 0.8f, ModType::Rotary
        };
        presetConfigs[ModulationPreset::FastRotary] = {
            "Fast Rotary", 2.0f, 6.0f, 0.5f, 1.0f, 0.0f, 0.9f, 0.9f, ModType::Rotary
        };
    }

    void ModulationXYController::updateTargetEngine()
    {
        if (targetEngine && !smoothingEnabled)
        {
            applyToModulationEngine(*targetEngine);
        }
    }

    float ModulationXYController::mapXToRate(float x) const
    {
        const auto& config = presetConfigs.at(currentPreset);
        
        // Logarithmic mapping for rate (feels more musical)
        float logMin = std::log(config.minRate);
        float logMax = std::log(config.maxRate);
        float logValue = logMin + x * (logMax - logMin);
        
        return std::exp(logValue);
    }

    float ModulationXYController::mapYToDepth(float y) const
    {
        const auto& config = presetConfigs.at(currentPreset);
        
        // Linear mapping for depth
        return config.minDepth + y * (config.maxDepth - config.minDepth);
    }

    //==============================================================================
    // ParameterSmoother Implementation
    //==============================================================================
    
    void ModulationXYController::ParameterSmoother::prepareToPlay(double sr, float smoothTime)
    {
        sampleRate = sr;
        smoothingTime = smoothTime;
    }

    void ModulationXYController::ParameterSmoother::setTargetValue(float target)
    {
        targetValue = target;
        
        if (smoothingTime > 0.0f)
        {
            int numSteps = static_cast<int>(smoothingTime * sampleRate);
            remainingSteps = numSteps;
            stepSize = (targetValue - currentValue) / static_cast<float>(numSteps);
        }
        else
        {
            currentValue = targetValue;
            remainingSteps = 0;
            stepSize = 0.0f;
        }
    }

    float ModulationXYController::ParameterSmoother::getNextValue()
    {
        if (remainingSteps > 0)
        {
            currentValue += stepSize;
            --remainingSteps;
            
            if (remainingSteps == 0)
                currentValue = targetValue; // Ensure exact final value
        }
        
        return currentValue;
    }

    void ModulationXYController::ParameterSmoother::reset(float value)
    {
        currentValue = value;
        targetValue = value;
        remainingSteps = 0;
        stepSize = 0.0f;
    }

} // namespace spawnclone::audio
