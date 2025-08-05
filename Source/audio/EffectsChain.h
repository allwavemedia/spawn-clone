//==============================================================================
/*
  EffectsChain.h
  Author: BMad Dev Agent - Epic 6 Story 6.1
  
  Professional effects chain system with flexible routing and real-time control
*/
//==============================================================================

#pragma once

#include "EffectProcessor.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <memory>
#include <vector>

namespace spawnclone::audio
{
    /**
     * Manages a chain of up to 8 effect processors with flexible routing
     * Supports real-time insertion/removal without audio dropouts
     */
    class EffectsChain
    {
    public:
        static constexpr int MAX_EFFECT_SLOTS = 8;
        
        EffectsChain();
        ~EffectsChain();

        //==============================================================================
        // Setup and Configuration
        
        /** Prepare the effects chain for processing */
        void prepareToPlay(double sampleRate, int samplesPerBlock);
        
        /** Release all resources */
        void releaseResources();
        
        /** Reset all effects in the chain */
        void reset();

        //==============================================================================
        // Effect Management
        
        /** Insert effect at specified slot (0-7) */
        bool insertEffect(int slotIndex, std::unique_ptr<EffectProcessor> effect);
        
        /** Remove effect from specified slot */
        void removeEffect(int slotIndex);
        
        /** Get effect at specified slot (returns nullptr if empty) */
        EffectProcessor* getEffect(int slotIndex) const;
        
        /** Check if slot is occupied */
        bool isSlotOccupied(int slotIndex) const;
        
        /** Move effect from one slot to another */
        bool moveEffect(int fromSlot, int toSlot);

        //==============================================================================
        // Processing
        
        /** Process audio through the effects chain */
        void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages);

        //==============================================================================
        // Chain Control
        
        /** Enable/disable the entire effects chain */
        void setChainEnabled(bool enabled) { chainEnabled = enabled; }
        bool isChainEnabled() const { return chainEnabled; }
        
        /** Set master wet/dry mix for entire chain */
        void setMasterMix(float mix) { masterMix = juce::jlimit(0.0f, 1.0f, mix); }
        float getMasterMix() const { return masterMix; }

        //==============================================================================
        // Performance Monitoring
        
        /** Get total CPU usage for all effects */
        double getTotalCPUUsage() const;
        
        /** Get CPU usage for specific slot */
        double getSlotCPUUsage(int slotIndex) const;
        
        /** Get total latency introduced by chain */
        int getTotalLatencySamples() const;

        //==============================================================================
        // Preset Management
        
        /** Save current chain configuration */
        juce::ValueTree saveChainState() const;
        
        /** Load chain configuration */
        void loadChainState(const juce::ValueTree& state);
        
        /** Clear all effects from chain */
        void clearChain();

        //==============================================================================
        // Effect Factory Methods
        
        /** Create common effects by name */
        static std::unique_ptr<EffectProcessor> createEffect(const juce::String& effectName);
        
        /** Get list of available effect types */
        static juce::StringArray getAvailableEffectTypes();

    private:
        //==============================================================================
        // Internal State
        
        struct EffectSlot
        {
            std::unique_ptr<EffectProcessor> effect;
            bool isEnabled = true;
            float slotMix = 1.0f;
            
            EffectSlot() = default;
            EffectSlot(EffectSlot&& other) noexcept
                : effect(std::move(other.effect))
                , isEnabled(other.isEnabled)
                , slotMix(other.slotMix)
            {}
            
            EffectSlot& operator=(EffectSlot&& other) noexcept
            {
                if (this != &other)
                {
                    effect = std::move(other.effect);
                    isEnabled = other.isEnabled;
                    slotMix = other.slotMix;
                }
                return *this;
            }
        };
        
        std::array<EffectSlot, MAX_EFFECT_SLOTS> effectSlots;
        
        bool chainEnabled = true;
        float masterMix = 1.0f;
        
        // Audio processing state
        double currentSampleRate = 44100.0;
        int currentBlockSize = 512;
        
        // Temporary buffers for chain processing
        juce::AudioBuffer<float> tempBuffer;
        juce::AudioBuffer<float> dryBuffer;
        
        // Thread safety for real-time insertion/removal
        juce::SpinLock effectsLock;
        
        //==============================================================================
        // Internal Methods
        
        /** Validate slot index */
        bool isValidSlotIndex(int slotIndex) const;
        
        /** Thread-safe effect slot access */
        EffectSlot* getSlotSafe(int slotIndex);
        const EffectSlot* getSlotSafe(int slotIndex) const;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EffectsChain)
    };

} // namespace spawnclone::audio
