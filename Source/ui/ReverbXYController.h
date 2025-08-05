//==============================================================================
/*
  ReverbXYController.h  
  Epic 6 Phase 2: XY Controller Integration for Professional Reverb
  
  Connects ExperimentPad XY controller to professional ReverbEngine
  providing intuitive real-time reverb control
*/
//==============================================================================

#pragma once

#include "../audio/ReverbEngine.h"
#include "../audio/EffectsChain.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <memory>

namespace spawnclone::ui
{
    /**
     * Professional reverb controller that maps XY pad coordinates to reverb parameters
     * 
     * Mapping Strategy:
     * - X-axis: Room Size (0.0 = small room, 1.0 = large hall)  
     * - Y-axis: Reverb Time (0.0 = short decay, 1.0 = long decay)
     * - Modifier keys: Damping, Stereo Width, Early Reflections
     */
    class ReverbXYController
    {
    public:
        ReverbXYController();
        ~ReverbXYController();
        
        //==============================================================================
        // Setup and Configuration
        
        /** Initialize with effects chain containing ReverbEngine */
        void setEffectsChain(spawnclone::audio::EffectsChain* chain, int reverbSlotIndex = 0);
        
        /** Set which reverb algorithm to use */
        void setReverbAlgorithm(spawnclone::audio::ReverbEngine::Algorithm algorithm);
        
        //==============================================================================
        // XY Pad Integration
        
        /** Update reverb parameters from XY pad position */
        void updateFromXYPosition(float x, float y);
        
        /** Update with modifier keys for advanced control */
        void updateFromXYPosition(float x, float y, bool shiftHeld, bool ctrlHeld, bool altHeld);
        
        /** Get current parameter values for display */
        struct ReverbState
        {
            float roomSize = 0.5f;
            float reverbTime = 2.0f;
            float damping = 0.3f;
            float stereoWidth = 1.0f;
            float earlyReflectionsMix = 0.3f;
            float preDelay = 20.0f;
            spawnclone::audio::ReverbEngine::Algorithm algorithm = 
                spawnclone::audio::ReverbEngine::Algorithm::Plate;
        };
        
        ReverbState getCurrentState() const { return currentState; }
        
        //==============================================================================
        // Parameter Mapping Configuration
        
        /** Configure X-axis parameter mapping */
        void setXAxisMapping(const juce::String& parameterName, float minValue, float maxValue);
        
        /** Configure Y-axis parameter mapping */  
        void setYAxisMapping(const juce::String& parameterName, float minValue, float maxValue);
        
        /** Set preset mapping configurations */
        void loadPresetMapping(const juce::String& presetName);
        
        //==============================================================================
        // Preset Configurations
        
        /** Load "Studio" mapping preset - optimized for music production */
        void loadStudioMapping();
        
        /** Load "Performance" mapping preset - optimized for live performance */
        void loadPerformanceMapping();
        
        /** Load "Creative" mapping preset - wide parameter ranges for sound design */
        void loadCreativeMapping();
        
        /** Load "Vintage" mapping preset - classic spring/plate reverb character */
        void loadVintageMapping();
        
        //==============================================================================
        // Real-time Features
        
        /** Enable smooth parameter interpolation (default: true) */
        void setSmoothingEnabled(bool enabled) { smoothingEnabled = enabled; }
        
        /** Set parameter smoothing time in seconds */
        void setSmoothingTime(float seconds) { smoothingTimeSeconds = seconds; }
        
        /** Enable parameter locking - prevents accidental changes */
        void setParameterLocked(const juce::String& paramName, bool locked);
        
        //==============================================================================
        // Callbacks for UI Updates
        
        /** Called when reverb parameters change - for updating UI displays */
        std::function<void(const ReverbState&)> onStateChanged;
        
        /** Called when algorithm changes - for updating UI algorithm selector */
        std::function<void(spawnclone::audio::ReverbEngine::Algorithm)> onAlgorithmChanged;
        
    private:
        //==============================================================================
        // Internal State
        
        spawnclone::audio::EffectsChain* effectsChain = nullptr;
        int reverbSlotIndex = 0;
        ReverbState currentState;
        ReverbState targetState;
        
        // Parameter mapping configuration
        struct AxisMapping
        {
            juce::String parameterName = "roomSize";
            float minValue = 0.0f;
            float maxValue = 1.0f;
            bool isLocked = false;
        };
        
        AxisMapping xAxisMapping;
        AxisMapping yAxisMapping;
        
        // Modifier key mappings
        AxisMapping shiftXMapping;  // Shift+X controls damping
        AxisMapping shiftYMapping;  // Shift+Y controls stereo width
        AxisMapping ctrlXMapping;   // Ctrl+X controls early reflections
        AxisMapping ctrlYMapping;   // Ctrl+Y controls pre-delay
        
        // Smoothing parameters
        bool smoothingEnabled = true;
        float smoothingTimeSeconds = 0.05f; // 50ms smoothing
        
        //==============================================================================
        // Helper Methods
        
        /** Get ReverbEngine from effects chain */
        spawnclone::audio::ReverbEngine* getReverbEngine();
        
        /** Apply parameter value with smoothing */
        void setReverbParameter(const juce::String& paramName, float value);
        
        /** Map normalized position (0-1) to parameter range */
        float mapToParameterRange(float normalizedValue, float minVal, float maxVal);
        
        /** Update target state from XY position and modifiers */
        void calculateTargetState(float x, float y, bool shift, bool ctrl, bool alt);
        
        /** Apply current state to reverb engine */
        void applyStateToReverb();
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ReverbXYController)
    };

} // namespace spawnclone::ui
