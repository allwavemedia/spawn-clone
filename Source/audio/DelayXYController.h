//==============================================================================
/*
  DelayXYController.h
  Author: BMad Dev Agent - Epic 6 Story 6.11
  
  XY Controller for DelayEngine providing real-time multi-parameter control.
  
  Features:
  - X-axis: Delay Time control (50ms to 2000ms)
  - Y-axis: Feedback control (0% to 90%)
  - Multi-effect presets with smooth parameter mapping
  - Real-time parameter interpolation for smooth control
  - Preset system for different delay styles
*/
//==============================================================================

#pragma once

#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include "DelayEngine.h"

namespace spawnclone::audio
{
    /**
     * XY Controller for DelayEngine providing intuitive real-time control
     * over delay time and feedback parameters with preset management.
     */
    class DelayXYController
    {
    public:
        //==============================================================================
        // Preset System
        
        enum class DelayPreset
        {
            Classic,        // Balanced delay and feedback
            Slapback,       // Short delay, minimal feedback
            Echo,           // Medium delay, moderate feedback
            Space,          // Long delay, high feedback
            Rhythmic,       // Tempo-synced delays
            Ambient,        // Long atmospheric delays
            Dub,            // Dub-style ping-pong delays
            Experimental    // Wide range for creative use
        };
        
        //==============================================================================
        // Constructor/Destructor
        
        DelayXYController();
        ~DelayXYController() = default;
        
        //==============================================================================
        // XY Controller Interface
        
        /** Set XY position (0.0 to 1.0 for both axes) */
        void setXYPosition(float x, float y);
        
        /** Get current XY position */
        juce::Point<float> getXYPosition() const { return currentXY; }
        
        /** Set X position only (delay time control) */
        void setXPosition(float x);
        
        /** Set Y position only (feedback control) */
        void setYPosition(float y);
        
        //==============================================================================
        // Preset Management
        
        /** Set active preset */
        void setPreset(DelayPreset preset);
        DelayPreset getPreset() const { return currentPreset; }
        
        /** Get preset name */
        juce::String getPresetName(DelayPreset preset) const;
        juce::String getCurrentPresetName() const { return getPresetName(currentPreset); }
        
        /** Get all available presets */
        std::vector<DelayPreset> getAvailablePresets() const;
        
        //==============================================================================
        // Parameter Mapping
        
        /** Get delay time for current XY position (in milliseconds) */
        float getMappedDelayTime() const;
        
        /** Get feedback amount for current XY position (0.0 to 1.0) */
        float getMappedFeedback() const;
        
        /** Get additional parameters based on preset */
        float getMappedLowCut() const;
        float getMappedHighCut() const;
        float getMappedStereoSpread() const;
        float getMappedModRate() const;
        float getMappedModDepth() const;
        
        //==============================================================================
        // DelayEngine Integration
        
        /** Apply current XY mapping to DelayEngine */
        void applyToDelayEngine(DelayEngine& delayEngine);
        
        /** Set target DelayEngine for automatic updates */
        void setTargetDelayEngine(DelayEngine* engine);
        
        //==============================================================================
        // Smoothing and Interpolation
        
        /** Enable/disable parameter smoothing */
        void setSmoothingEnabled(bool enabled) { smoothingEnabled = enabled; }
        bool isSmoothingEnabled() const { return smoothingEnabled; }
        
        /** Set smoothing time in seconds */
        void setSmoothingTime(float timeSeconds);
        float getSmoothingTime() const { return smoothingTime; }
        
        /** Prepare for audio processing */
        void prepareToPlay(double sampleRate, int samplesPerBlock);
        
        /** Process one audio block for parameter smoothing */
        void processBlock();
        
        //==============================================================================
        // Utility Methods
        
        /** Get XY bounds for current preset */
        juce::Rectangle<float> getXYBounds() const;
        
        /** Map raw XY to constrained bounds */
        juce::Point<float> constrainToBounds(juce::Point<float> xy) const;
        
        /** Get parameter description for current position */
        juce::String getParameterDescription() const;
        
        /** Save/load XY controller state */
        juce::ValueTree saveState() const;
        void loadState(const juce::ValueTree& state);

    private:
        //==============================================================================
        // Internal State
        
        juce::Point<float> currentXY{0.5f, 0.5f};  // Current XY position
        juce::Point<float> targetXY{0.5f, 0.5f};   // Target for smoothing
        DelayPreset currentPreset = DelayPreset::Classic;
        
        DelayEngine* targetEngine = nullptr;
        bool smoothingEnabled = true;
        float smoothingTime = 0.1f; // 100ms default
        double sampleRate = 48000.0;
        int blockSize = 512;
        
        //==============================================================================
        // Preset Configurations
        
        struct PresetConfig
        {
            juce::String name;
            float minDelayTime;    // Minimum delay time (ms)
            float maxDelayTime;    // Maximum delay time (ms)
            float minFeedback;     // Minimum feedback
            float maxFeedback;     // Maximum feedback
            float lowCut;          // Fixed low cut frequency
            float highCut;         // Fixed high cut frequency
            float stereoSpread;    // Fixed stereo spread
            float modRate;         // Fixed modulation rate
            float modDepth;        // Fixed modulation depth
            DelayEngine::DelayType preferredType; // Preferred delay algorithm
        };
        
        std::map<DelayPreset, PresetConfig> presetConfigs;
        
        //==============================================================================
        // Parameter Smoothing
        
        class ParameterSmoother
        {
        public:
            void prepareToPlay(double sr, float smoothTime);
            void setTargetValue(float target);
            float getNextValue();
            void reset(float value);
            float getCurrentValue() const { return currentValue; }
            
        private:
            float currentValue = 0.0f;
            float targetValue = 0.0f;
            float stepSize = 0.0f;
            int remainingSteps = 0;
            double sampleRate = 48000.0;
            float smoothingTime = 0.1f;
        };
        
        ParameterSmoother delayTimeSmoother;
        ParameterSmoother feedbackSmoother;
        
        //==============================================================================
        // Helper Methods
        
        void initializePresets();
        void updateTargetEngine();
        float mapXToDelayTime(float x) const;
        float mapYToFeedback(float y) const;
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DelayXYController)
    };

} // namespace spawnclone::audio
