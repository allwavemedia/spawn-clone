//==============================================================================
/*
  ModulationXYController.h
  Author: BMad Dev Agent - Epic 6 Story 6.11
  
  XY Controller for ModulationEngine providing real-time multi-parameter control.
  
  Features:
  - X-axis: Rate control (0.1Hz to 10Hz)
  - Y-axis: Depth control (0% to 100%)
  - Multi-effect presets for different modulation styles
  - Real-time parameter interpolation for smooth control
  - Automatic effect type selection based on presets
*/
//==============================================================================

#pragma once

#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include "ModulationEngine.h"

namespace spawnclone::audio
{
    /**
     * XY Controller for ModulationEngine providing intuitive real-time control
     * over modulation rate and depth parameters with effect-specific presets.
     */
    class ModulationXYController
    {
    public:
        //==============================================================================
        // Preset System
        
        enum class ModulationPreset
        {
            SubtleChorus,     // Light chorus for vocals
            WideChorus,       // Wide stereo chorus
            ClassicFlanger,   // Traditional flanger sweep
            JetFlanger,       // Aggressive flanger
            VintagePhaser,    // Smooth phaser sweep
            IntensePhaser,    // Deep phaser modulation
            SlowTremolo,      // Gentle amplitude modulation
            ChoppyTremolo,    // Rhythmic tremolo
            SmoothVibrato,    // Subtle pitch modulation
            WideVibrato,      // Obvious pitch modulation
            SlowAutoPan,      // Gentle stereo movement
            FastAutoPan,      // Rapid panning
            WarmRingMod,      // Musical ring modulation
            MetallicRingMod,  // Harsh ring modulation
            SlowRotary,       // Slow Leslie simulation
            FastRotary        // Fast Leslie simulation
        };
        
        //==============================================================================
        // Constructor/Destructor
        
        ModulationXYController();
        ~ModulationXYController() = default;
        
        //==============================================================================
        // XY Controller Interface
        
        /** Set XY position (0.0 to 1.0 for both axes) */
        void setXYPosition(float x, float y);
        
        /** Get current XY position */
        juce::Point<float> getXYPosition() const { return currentXY; }
        
        /** Set X position only (rate control) */
        void setXPosition(float x);
        
        /** Set Y position only (depth control) */
        void setYPosition(float y);
        
        //==============================================================================
        // Preset Management
        
        /** Set active preset */
        void setPreset(ModulationPreset preset);
        ModulationPreset getPreset() const { return currentPreset; }
        
        /** Get preset name */
        juce::String getPresetName(ModulationPreset preset) const;
        juce::String getCurrentPresetName() const { return getPresetName(currentPreset); }
        
        /** Get all available presets */
        std::vector<ModulationPreset> getAvailablePresets() const;
        
        /** Get presets for specific modulation type */
        std::vector<ModulationPreset> getPresetsForType(ModulationEngine::ModulationType type) const;
        
        //==============================================================================
        // Parameter Mapping
        
        /** Get rate for current XY position (in Hz) */
        float getMappedRate() const;
        
        /** Get depth for current XY position (0.0 to 1.0) */
        float getMappedDepth() const;
        
        /** Get additional parameters based on preset */
        float getMappedFeedback() const;
        float getMappedStereoSpread() const;
        float getMappedMix() const;
        ModulationEngine::ModulationType getMappedModulationType() const;
        
        //==============================================================================
        // ModulationEngine Integration
        
        /** Apply current XY mapping to ModulationEngine */
        void applyToModulationEngine(ModulationEngine& modulationEngine);
        
        /** Set target ModulationEngine for automatic updates */
        void setTargetModulationEngine(ModulationEngine* engine);
        
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
        ModulationPreset currentPreset = ModulationPreset::SubtleChorus;
        
        ModulationEngine* targetEngine = nullptr;
        bool smoothingEnabled = true;
        float smoothingTime = 0.1f; // 100ms default
        double sampleRate = 48000.0;
        int blockSize = 512;
        
        //==============================================================================
        // Preset Configurations
        
        struct PresetConfig
        {
            juce::String name;
            float minRate;         // Minimum rate (Hz)
            float maxRate;         // Maximum rate (Hz)
            float minDepth;        // Minimum depth
            float maxDepth;        // Maximum depth
            float feedback;        // Fixed feedback amount
            float stereoSpread;    // Fixed stereo spread
            float mix;             // Fixed wet/dry mix
            ModulationEngine::ModulationType modulationType; // Effect type
        };
        
        std::map<ModulationPreset, PresetConfig> presetConfigs;
        
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
        
        ParameterSmoother rateSmoother;
        ParameterSmoother depthSmoother;
        
        //==============================================================================
        // Helper Methods
        
        void initializePresets();
        void updateTargetEngine();
        float mapXToRate(float x) const;
        float mapYToDepth(float y) const;
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModulationXYController)
    };

} // namespace spawnclone::audio
