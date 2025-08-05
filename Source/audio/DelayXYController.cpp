//==============================================================================
/*
  DelayXYController.cpp
  Author: BMad Dev Agent - Epic 6 Story 6.11
  
  Implementation of XY Controller for DelayEngine with multi-parameter control.
*/
//==============================================================================

#include "DelayXYController.h"
#include <cmath>

namespace spawnclone::audio
{
    //==============================================================================
    // DelayXYController Implementation
    //==============================================================================
    
    DelayXYController::DelayXYController()
    {
        initializePresets();
        setPreset(DelayPreset::Classic); // Default preset
    }

    //==============================================================================
    // XY Controller Interface
    //==============================================================================
    
    void DelayXYController::setXYPosition(float x, float y)
    {
        juce::Point<float> newXY(juce::jlimit(0.0f, 1.0f, x), juce::jlimit(0.0f, 1.0f, y));
        newXY = constrainToBounds(newXY);
        
        if (smoothingEnabled)
        {
            targetXY = newXY;
            
            // Update smoothers with new target values
            delayTimeSmoother.setTargetValue(mapXToDelayTime(newXY.x));
            feedbackSmoother.setTargetValue(mapYToFeedback(newXY.y));
        }
        else
        {
            currentXY = newXY;
            targetXY = newXY;
            
            // Reset smoothers to new values
            delayTimeSmoother.reset(mapXToDelayTime(newXY.x));
            feedbackSmoother.reset(mapYToFeedback(newXY.y));
        }
        
        updateTargetEngine();
    }

    void DelayXYController::setXPosition(float x)
    {
        setXYPosition(x, currentXY.y);
    }

    void DelayXYController::setYPosition(float y)
    {
        setXYPosition(currentXY.x, y);
    }

    //==============================================================================
    // Preset Management
    //==============================================================================
    
    void DelayXYController::setPreset(DelayPreset preset)
    {
        currentPreset = preset;
        
        // Reset XY position to center for new preset
        setXYPosition(0.5f, 0.5f);
        
        // Apply preset-specific DelayEngine settings if we have a target
        if (targetEngine)
        {
            const auto& config = presetConfigs[preset];
            targetEngine->setDelayType(config.preferredType);
            targetEngine->setLowCut(config.lowCut);
            targetEngine->setHighCut(config.highCut);
            targetEngine->setStereoSpread(config.stereoSpread);
            targetEngine->setModulationRate(config.modRate);
            targetEngine->setModulationDepth(config.modDepth);
        }
    }

    juce::String DelayXYController::getPresetName(DelayPreset preset) const
    {
        auto it = presetConfigs.find(preset);
        return (it != presetConfigs.end()) ? it->second.name : "Unknown";
    }

    std::vector<DelayXYController::DelayPreset> DelayXYController::getAvailablePresets() const
    {
        return {
            DelayPreset::Classic,
            DelayPreset::Slapback,
            DelayPreset::Echo,
            DelayPreset::Space,
            DelayPreset::Rhythmic,
            DelayPreset::Ambient,
            DelayPreset::Dub,
            DelayPreset::Experimental
        };
    }

    //==============================================================================
    // Parameter Mapping
    //==============================================================================
    
    float DelayXYController::getMappedDelayTime() const
    {
        return delayTimeSmoother.getCurrentValue();
    }

    float DelayXYController::getMappedFeedback() const
    {
        return feedbackSmoother.getCurrentValue();
    }

    float DelayXYController::getMappedLowCut() const
    {
        const auto& config = presetConfigs.at(currentPreset);
        return config.lowCut;
    }

    float DelayXYController::getMappedHighCut() const
    {
        const auto& config = presetConfigs.at(currentPreset);
        return config.highCut;
    }

    float DelayXYController::getMappedStereoSpread() const
    {
        const auto& config = presetConfigs.at(currentPreset);
        return config.stereoSpread;
    }

    float DelayXYController::getMappedModRate() const
    {
        const auto& config = presetConfigs.at(currentPreset);
        return config.modRate;
    }

    float DelayXYController::getMappedModDepth() const
    {
        const auto& config = presetConfigs.at(currentPreset);
        return config.modDepth;
    }

    //==============================================================================
    // DelayEngine Integration
    //==============================================================================
    
    void DelayXYController::applyToDelayEngine(DelayEngine& delayEngine)
    {
        delayEngine.setDelayTime(getMappedDelayTime());
        delayEngine.setFeedback(getMappedFeedback());
        delayEngine.setLowCut(getMappedLowCut());
        delayEngine.setHighCut(getMappedHighCut());
        delayEngine.setStereoSpread(getMappedStereoSpread());
        delayEngine.setModulationRate(getMappedModRate());
        delayEngine.setModulationDepth(getMappedModDepth());
    }

    void DelayXYController::setTargetDelayEngine(DelayEngine* engine)
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
    
    void DelayXYController::setSmoothingTime(float timeSeconds)
    {
        smoothingTime = juce::jlimit(0.001f, 1.0f, timeSeconds);
        
        // Update smoothers with new time
        delayTimeSmoother.prepareToPlay(sampleRate, smoothingTime);
        feedbackSmoother.prepareToPlay(sampleRate, smoothingTime);
    }

    void DelayXYController::prepareToPlay(double sr, int samplesPerBlock)
    {
        sampleRate = sr;
        blockSize = samplesPerBlock;
        
        // Prepare parameter smoothers
        delayTimeSmoother.prepareToPlay(sampleRate, smoothingTime);
        feedbackSmoother.prepareToPlay(sampleRate, smoothingTime);
        
        // Initialize with current values
        delayTimeSmoother.reset(mapXToDelayTime(currentXY.x));
        feedbackSmoother.reset(mapYToFeedback(currentXY.y));
    }

    void DelayXYController::processBlock()
    {
        if (!smoothingEnabled)
            return;
        
        // Update current XY position based on smoothing
        float smoothedDelayTime = delayTimeSmoother.getNextValue();
        float smoothedFeedback = feedbackSmoother.getNextValue();
        
        // Update target engine if available
        if (targetEngine)
        {
            targetEngine->setDelayTime(smoothedDelayTime);
            targetEngine->setFeedback(smoothedFeedback);
        }
    }

    //==============================================================================
    // Utility Methods
    //==============================================================================
    
    juce::Rectangle<float> DelayXYController::getXYBounds() const
    {
        // All presets use full 0-1 range for simplicity
        return juce::Rectangle<float>(0.0f, 0.0f, 1.0f, 1.0f);
    }

    juce::Point<float> DelayXYController::constrainToBounds(juce::Point<float> xy) const
    {
        auto bounds = getXYBounds();
        return juce::Point<float>(
            juce::jlimit(bounds.getX(), bounds.getRight(), xy.x),
            juce::jlimit(bounds.getY(), bounds.getBottom(), xy.y)
        );
    }

    juce::String DelayXYController::getParameterDescription() const
    {
        juce::String description;
        description << getCurrentPresetName() << ": ";
        description << "Delay: " << juce::String(getMappedDelayTime(), 1) << "ms, ";
        description << "Feedback: " << juce::String(getMappedFeedback() * 100.0f, 1) << "%";
        return description;
    }

    juce::ValueTree DelayXYController::saveState() const
    {
        juce::ValueTree state("DelayXYController");
        state.setProperty("x", currentXY.x, nullptr);
        state.setProperty("y", currentXY.y, nullptr);
        state.setProperty("preset", static_cast<int>(currentPreset), nullptr);
        state.setProperty("smoothingEnabled", smoothingEnabled, nullptr);
        state.setProperty("smoothingTime", smoothingTime, nullptr);
        return state;
    }

    void DelayXYController::loadState(const juce::ValueTree& state)
    {
        if (state.hasType("DelayXYController"))
        {
            float x = state.getProperty("x", 0.5f);
            float y = state.getProperty("y", 0.5f);
            int presetIndex = state.getProperty("preset", static_cast<int>(DelayPreset::Classic));
            bool smoothing = state.getProperty("smoothingEnabled", true);
            float smoothTime = state.getProperty("smoothingTime", 0.1f);
            
            setSmoothingEnabled(smoothing);
            setSmoothingTime(smoothTime);
            setPreset(static_cast<DelayPreset>(presetIndex));
            setXYPosition(x, y);
        }
    }

    //==============================================================================
    // Private Helper Methods
    //==============================================================================
    
    void DelayXYController::initializePresets()
    {
        // Classic: Balanced delay for general use
        presetConfigs[DelayPreset::Classic] = {
            "Classic Delay",
            50.0f,    // Min delay: 50ms
            1000.0f,  // Max delay: 1000ms
            0.0f,     // Min feedback: 0%
            0.7f,     // Max feedback: 70%
            100.0f,   // Low cut: 100Hz
            8000.0f,  // High cut: 8kHz
            0.3f,     // Stereo spread: 30%
            0.5f,     // Mod rate: 0.5Hz
            0.1f,     // Mod depth: 10%
            DelayEngine::DelayType::Digital
        };
        
        // Slapback: Short delays for rhythm
        presetConfigs[DelayPreset::Slapback] = {
            "Slapback Echo",
            20.0f,    // Min delay: 20ms
            200.0f,   // Max delay: 200ms
            0.0f,     // Min feedback: 0%
            0.3f,     // Max feedback: 30%
            200.0f,   // Low cut: 200Hz
            6000.0f,  // High cut: 6kHz
            0.1f,     // Stereo spread: 10%
            0.0f,     // No modulation
            0.0f,
            DelayEngine::DelayType::Analog
        };
        
        // Echo: Medium delays with character
        presetConfigs[DelayPreset::Echo] = {
            "Echo Chamber",
            100.0f,   // Min delay: 100ms
            800.0f,   // Max delay: 800ms
            0.1f,     // Min feedback: 10%
            0.6f,     // Max feedback: 60%
            80.0f,    // Low cut: 80Hz
            10000.0f, // High cut: 10kHz
            0.5f,     // Stereo spread: 50%
            1.0f,     // Mod rate: 1Hz
            0.2f,     // Mod depth: 20%
            DelayEngine::DelayType::Tape
        };
        
        // Space: Long atmospheric delays
        presetConfigs[DelayPreset::Space] = {
            "Space Delay",
            200.0f,   // Min delay: 200ms
            2000.0f,  // Max delay: 2000ms
            0.2f,     // Min feedback: 20%
            0.8f,     // Max feedback: 80%
            50.0f,    // Low cut: 50Hz
            12000.0f, // High cut: 12kHz
            0.8f,     // Stereo spread: 80%
            0.3f,     // Mod rate: 0.3Hz
            0.3f,     // Mod depth: 30%
            DelayEngine::DelayType::Modulated
        };
        
        // Rhythmic: Tempo-synced delays
        presetConfigs[DelayPreset::Rhythmic] = {
            "Rhythmic Delay",
            60.0f,    // Min delay: 60ms (16th note at 250 BPM)
            1000.0f,  // Max delay: 1000ms (quarter note at 60 BPM)
            0.0f,     // Min feedback: 0%
            0.5f,     // Max feedback: 50%
            150.0f,   // Low cut: 150Hz
            7000.0f,  // High cut: 7kHz
            0.6f,     // Stereo spread: 60%
            2.0f,     // Mod rate: 2Hz
            0.1f,     // Mod depth: 10%
            DelayEngine::DelayType::MultiTap
        };
        
        // Ambient: Long, dreamy delays
        presetConfigs[DelayPreset::Ambient] = {
            "Ambient Wash",
            300.0f,   // Min delay: 300ms
            2000.0f,  // Max delay: 2000ms
            0.3f,     // Min feedback: 30%
            0.9f,     // Max feedback: 90%
            30.0f,    // Low cut: 30Hz
            15000.0f, // High cut: 15kHz
            1.0f,     // Stereo spread: 100%
            0.2f,     // Mod rate: 0.2Hz
            0.4f,     // Mod depth: 40%
            DelayEngine::DelayType::Reverse
        };
        
        // Dub: Ping-pong delays
        presetConfigs[DelayPreset::Dub] = {
            "Dub Delay",
            120.0f,   // Min delay: 120ms
            600.0f,   // Max delay: 600ms
            0.1f,     // Min feedback: 10%
            0.7f,     // Max feedback: 70%
            100.0f,   // Low cut: 100Hz
            5000.0f,  // High cut: 5kHz
            1.0f,     // Stereo spread: 100%
            0.0f,     // No modulation for clean ping-pong
            0.0f,
            DelayEngine::DelayType::PingPong
        };
        
        // Experimental: Wide range for creativity
        presetConfigs[DelayPreset::Experimental] = {
            "Experimental",
            10.0f,    // Min delay: 10ms
            2000.0f,  // Max delay: 2000ms
            0.0f,     // Min feedback: 0%
            0.95f,    // Max feedback: 95%
            20.0f,    // Low cut: 20Hz
            20000.0f, // High cut: 20kHz
            1.0f,     // Stereo spread: 100%
            5.0f,     // Mod rate: 5Hz
            0.8f,     // Mod depth: 80%
            DelayEngine::DelayType::Granular
        };
    }

    void DelayXYController::updateTargetEngine()
    {
        if (targetEngine && !smoothingEnabled)
        {
            applyToDelayEngine(*targetEngine);
        }
    }

    float DelayXYController::mapXToDelayTime(float x) const
    {
        const auto& config = presetConfigs.at(currentPreset);
        
        // Logarithmic mapping for delay time (feels more natural)
        float logMin = std::log(config.minDelayTime);
        float logMax = std::log(config.maxDelayTime);
        float logValue = logMin + x * (logMax - logMin);
        
        return std::exp(logValue);
    }

    float DelayXYController::mapYToFeedback(float y) const
    {
        const auto& config = presetConfigs.at(currentPreset);
        
        // Linear mapping for feedback
        return config.minFeedback + y * (config.maxFeedback - config.minFeedback);
    }

    //==============================================================================
    // ParameterSmoother Implementation
    //==============================================================================
    
    void DelayXYController::ParameterSmoother::prepareToPlay(double sr, float smoothTime)
    {
        sampleRate = sr;
        smoothingTime = smoothTime;
    }

    void DelayXYController::ParameterSmoother::setTargetValue(float target)
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

    float DelayXYController::ParameterSmoother::getNextValue()
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

    void DelayXYController::ParameterSmoother::reset(float value)
    {
        currentValue = value;
        targetValue = value;
        remainingSteps = 0;
        stepSize = 0.0f;
    }

} // namespace spawnclone::audio
