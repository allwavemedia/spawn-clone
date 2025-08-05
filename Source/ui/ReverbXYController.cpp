//==============================================================================
/*
  ReverbXYController.cpp
  Epic 6 Phase 2: XY Controller Integration Implementation
*/
//==============================================================================

#include "ReverbXYController.h"
#include <cmath>

namespace spawnclone::ui
{
    ReverbXYController::ReverbXYController()
    {
        // Load default studio mapping on construction
        loadStudioMapping();
    }
    
    ReverbXYController::~ReverbXYController() = default;
    
    //==============================================================================
    void ReverbXYController::setEffectsChain(spawnclone::audio::EffectsChain* chain, int reverbSlotIndex)
    {
        effectsChain = chain;
        this->reverbSlotIndex = reverbSlotIndex;
        
        // Ensure reverb exists in the specified slot
        if (effectsChain && !effectsChain->isSlotOccupied(reverbSlotIndex))
        {
            auto reverb = spawnclone::audio::EffectsChain::createEffect("Reverb");
            if (reverb)
            {
                effectsChain->insertEffect(reverbSlotIndex, std::move(reverb));
            }
        }
        
        // Initialize current state from reverb if available
        if (auto* reverb = getReverbEngine())
        {
            currentState.roomSize = reverb->getParameter("roomSize");
            currentState.reverbTime = reverb->getParameter("reverbTime");
            currentState.damping = reverb->getParameter("damping");
            currentState.stereoWidth = reverb->getParameter("stereoWidth");
            currentState.earlyReflectionsMix = reverb->getParameter("earlyReflectionsMix");
            currentState.preDelay = reverb->getParameter("preDelay");
        }
        
        targetState = currentState;
    }
    
    void ReverbXYController::setReverbAlgorithm(spawnclone::audio::ReverbEngine::Algorithm algorithm)
    {
        currentState.algorithm = algorithm;
        
        if (auto* reverb = getReverbEngine())
        {
            reverb->setAlgorithm(algorithm);
        }
        
        if (onAlgorithmChanged)
            onAlgorithmChanged(algorithm);
    }
    
    //==============================================================================
    void ReverbXYController::updateFromXYPosition(float x, float y)
    {
        updateFromXYPosition(x, y, false, false, false);
    }
    
    void ReverbXYController::updateFromXYPosition(float x, float y, bool shiftHeld, bool ctrlHeld, bool altHeld)
    {
        // Clamp input values
        x = juce::jlimit(0.0f, 1.0f, x);
        y = juce::jlimit(0.0f, 1.0f, y);
        
        // Calculate target state based on current mapping and modifiers
        calculateTargetState(x, y, shiftHeld, ctrlHeld, altHeld);
        
        // Apply changes to reverb engine
        applyStateToReverb();
        
        // Notify listeners
        if (onStateChanged)
            onStateChanged(currentState);
    }
    
    //==============================================================================
    void ReverbXYController::setXAxisMapping(const juce::String& parameterName, float minValue, float maxValue)
    {
        xAxisMapping.parameterName = parameterName;
        xAxisMapping.minValue = minValue;
        xAxisMapping.maxValue = maxValue;
    }
    
    void ReverbXYController::setYAxisMapping(const juce::String& parameterName, float minValue, float maxValue)
    {
        yAxisMapping.parameterName = parameterName;
        yAxisMapping.minValue = minValue;
        yAxisMapping.maxValue = maxValue;
    }
    
    void ReverbXYController::loadPresetMapping(const juce::String& presetName)
    {
        if (presetName == "Studio")
            loadStudioMapping();
        else if (presetName == "Performance")
            loadPerformanceMapping();
        else if (presetName == "Creative")
            loadCreativeMapping();
        else if (presetName == "Vintage")
            loadVintageMapping();
    }
    
    //==============================================================================
    void ReverbXYController::loadStudioMapping()
    {
        // Studio preset: Professional mixing-friendly ranges
        setXAxisMapping("roomSize", 0.2f, 0.8f);    // Moderate room sizes
        setYAxisMapping("reverbTime", 0.8f, 4.0f);  // Usable reverb times
        
        // Modifier mappings for fine control
        shiftXMapping = {"damping", 0.1f, 0.6f, false};
        shiftYMapping = {"stereoWidth", 0.5f, 1.2f, false};
        ctrlXMapping = {"earlyReflectionsMix", 0.1f, 0.5f, false};
        ctrlYMapping = {"preDelay", 10.0f, 50.0f, false};
        
        // Default to Plate reverb for studio work
        setReverbAlgorithm(spawnclone::audio::ReverbEngine::Algorithm::Plate);
    }
    
    void ReverbXYController::loadPerformanceMapping()
    {
        // Performance preset: Wide, dramatic ranges for live use
        setXAxisMapping("roomSize", 0.0f, 1.0f);    // Full range
        setYAxisMapping("reverbTime", 0.5f, 6.0f);  // Wide reverb times
        
        // Performance-oriented modifier mappings
        shiftXMapping = {"damping", 0.0f, 0.8f, false};
        shiftYMapping = {"stereoWidth", 0.3f, 1.5f, false};
        ctrlXMapping = {"earlyReflectionsMix", 0.0f, 0.8f, false};
        ctrlYMapping = {"preDelay", 0.0f, 80.0f, false};
        
        // Default to Hall reverb for performance
        setReverbAlgorithm(spawnclone::audio::ReverbEngine::Algorithm::Hall);
    }
    
    void ReverbXYController::loadCreativeMapping()
    {
        // Creative preset: Extreme ranges for sound design
        setXAxisMapping("roomSize", 0.0f, 1.0f);    // Full range
        setYAxisMapping("reverbTime", 0.1f, 10.0f); // Extreme reverb times
        
        // Creative modifier mappings with wide ranges
        shiftXMapping = {"damping", 0.0f, 1.0f, false};
        shiftYMapping = {"stereoWidth", 0.0f, 2.0f, false};
        ctrlXMapping = {"earlyReflectionsMix", 0.0f, 1.0f, false};
        ctrlYMapping = {"preDelay", 0.0f, 100.0f, false};
        
        // Default to Shimmer for creative work
        setReverbAlgorithm(spawnclone::audio::ReverbEngine::Algorithm::Shimmer);
    }
    
    void ReverbXYController::loadVintageMapping()
    {
        // Vintage preset: Classic reverb character
        setXAxisMapping("roomSize", 0.1f, 0.6f);    // Smaller, vintage spaces
        setYAxisMapping("reverbTime", 0.3f, 2.5f);  // Classic reverb times
        
        // Vintage-appropriate modifier mappings
        shiftXMapping = {"damping", 0.3f, 0.9f, false}; // More damped, vintage sound
        shiftYMapping = {"stereoWidth", 0.7f, 1.1f, false}; // Narrower stereo field
        ctrlXMapping = {"earlyReflectionsMix", 0.2f, 0.6f, false}; // Prominent early reflections
        ctrlYMapping = {"preDelay", 5.0f, 30.0f, false}; // Shorter pre-delays
        
        // Default to Spring reverb for vintage character
        setReverbAlgorithm(spawnclone::audio::ReverbEngine::Algorithm::Spring);
    }
    
    //==============================================================================
    void ReverbXYController::setParameterLocked(const juce::String& paramName, bool locked)
    {
        if (paramName == xAxisMapping.parameterName)
            xAxisMapping.isLocked = locked;
        else if (paramName == yAxisMapping.parameterName)
            yAxisMapping.isLocked = locked;
        else if (paramName == shiftXMapping.parameterName)
            shiftXMapping.isLocked = locked;
        else if (paramName == shiftYMapping.parameterName)
            shiftYMapping.isLocked = locked;
        else if (paramName == ctrlXMapping.parameterName)
            ctrlXMapping.isLocked = locked;
        else if (paramName == ctrlYMapping.parameterName)
            ctrlYMapping.isLocked = locked;
    }
    
    //==============================================================================
    spawnclone::audio::ReverbEngine* ReverbXYController::getReverbEngine()
    {
        if (!effectsChain || !effectsChain->isSlotOccupied(reverbSlotIndex))
            return nullptr;
            
        auto* effect = effectsChain->getEffect(reverbSlotIndex);
        return dynamic_cast<spawnclone::audio::ReverbEngine*>(effect);
    }
    
    void ReverbXYController::setReverbParameter(const juce::String& paramName, float value)
    {
        if (auto* reverb = getReverbEngine())
        {
            reverb->setParameter(paramName, value, smoothingEnabled);
        }
    }
    
    float ReverbXYController::mapToParameterRange(float normalizedValue, float minVal, float maxVal)
    {
        return minVal + normalizedValue * (maxVal - minVal);
    }
    
    void ReverbXYController::calculateTargetState(float x, float y, bool shift, bool ctrl, bool alt)
    {
        // Primary X/Y axis mappings
        if (!xAxisMapping.isLocked)
        {
            float mappedValue = mapToParameterRange(x, xAxisMapping.minValue, xAxisMapping.maxValue);
            
            if (xAxisMapping.parameterName == "roomSize")
                targetState.roomSize = mappedValue;
            else if (xAxisMapping.parameterName == "reverbTime")
                targetState.reverbTime = mappedValue;
            else if (xAxisMapping.parameterName == "damping")
                targetState.damping = mappedValue;
            else if (xAxisMapping.parameterName == "stereoWidth")
                targetState.stereoWidth = mappedValue;
            else if (xAxisMapping.parameterName == "earlyReflectionsMix")
                targetState.earlyReflectionsMix = mappedValue;
            else if (xAxisMapping.parameterName == "preDelay")
                targetState.preDelay = mappedValue;
        }
        
        if (!yAxisMapping.isLocked)
        {
            float mappedValue = mapToParameterRange(y, yAxisMapping.minValue, yAxisMapping.maxValue);
            
            if (yAxisMapping.parameterName == "roomSize")
                targetState.roomSize = mappedValue;
            else if (yAxisMapping.parameterName == "reverbTime")
                targetState.reverbTime = mappedValue;
            else if (yAxisMapping.parameterName == "damping")
                targetState.damping = mappedValue;
            else if (yAxisMapping.parameterName == "stereoWidth")
                targetState.stereoWidth = mappedValue;
            else if (yAxisMapping.parameterName == "earlyReflectionsMix")
                targetState.earlyReflectionsMix = mappedValue;
            else if (yAxisMapping.parameterName == "preDelay")
                targetState.preDelay = mappedValue;
        }
        
        // Modifier key mappings
        if (shift)
        {
            if (!shiftXMapping.isLocked)
            {
                float mappedValue = mapToParameterRange(x, shiftXMapping.minValue, shiftXMapping.maxValue);
                if (shiftXMapping.parameterName == "damping")
                    targetState.damping = mappedValue;
                else if (shiftXMapping.parameterName == "stereoWidth")
                    targetState.stereoWidth = mappedValue;
                else if (shiftXMapping.parameterName == "earlyReflectionsMix")
                    targetState.earlyReflectionsMix = mappedValue;
                else if (shiftXMapping.parameterName == "preDelay")
                    targetState.preDelay = mappedValue;
            }
            
            if (!shiftYMapping.isLocked)
            {
                float mappedValue = mapToParameterRange(y, shiftYMapping.minValue, shiftYMapping.maxValue);
                if (shiftYMapping.parameterName == "damping")
                    targetState.damping = mappedValue;
                else if (shiftYMapping.parameterName == "stereoWidth")
                    targetState.stereoWidth = mappedValue;
                else if (shiftYMapping.parameterName == "earlyReflectionsMix")
                    targetState.earlyReflectionsMix = mappedValue;
                else if (shiftYMapping.parameterName == "preDelay")
                    targetState.preDelay = mappedValue;
            }
        }
        
        if (ctrl)
        {
            if (!ctrlXMapping.isLocked)
            {
                float mappedValue = mapToParameterRange(x, ctrlXMapping.minValue, ctrlXMapping.maxValue);
                if (ctrlXMapping.parameterName == "earlyReflectionsMix")
                    targetState.earlyReflectionsMix = mappedValue;
                else if (ctrlXMapping.parameterName == "preDelay")
                    targetState.preDelay = mappedValue;
                else if (ctrlXMapping.parameterName == "damping")
                    targetState.damping = mappedValue;
                else if (ctrlXMapping.parameterName == "stereoWidth")
                    targetState.stereoWidth = mappedValue;
            }
            
            if (!ctrlYMapping.isLocked)
            {
                float mappedValue = mapToParameterRange(y, ctrlYMapping.minValue, ctrlYMapping.maxValue);
                if (ctrlYMapping.parameterName == "preDelay")
                    targetState.preDelay = mappedValue;
                else if (ctrlYMapping.parameterName == "earlyReflectionsMix")
                    targetState.earlyReflectionsMix = mappedValue;
                else if (ctrlYMapping.parameterName == "damping")
                    targetState.damping = mappedValue;
                else if (ctrlYMapping.parameterName == "stereoWidth")
                    targetState.stereoWidth = mappedValue;
            }
        }
        
        // Copy target to current (in real implementation, this would be smoothed)
        currentState = targetState;
    }
    
    void ReverbXYController::applyStateToReverb()
    {
        setReverbParameter("roomSize", currentState.roomSize);
        setReverbParameter("reverbTime", currentState.reverbTime);
        setReverbParameter("damping", currentState.damping);
        setReverbParameter("stereoWidth", currentState.stereoWidth);
        setReverbParameter("earlyReflectionsMix", currentState.earlyReflectionsMix);
        setReverbParameter("preDelay", currentState.preDelay);
    }

} // namespace spawnclone::ui
