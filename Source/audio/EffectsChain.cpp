//==============================================================================
/*
  EffectsChain.cpp
  Author: BMad Dev Agent - Epic 6 Story 6.1
  
  Implementation of professional effects chain system
*/
//==============================================================================

#include "EffectsChain.h"
#include "ReverbEngine.h"     // Epic 6 Story 6.2
#include "DelayEngine.h"      // Epic 6 Story 6.9
#include "ModulationEngine.h" // Epic 6 Story 6.10

namespace spawnclone::audio
{
    EffectsChain::EffectsChain() = default;
    EffectsChain::~EffectsChain() = default;

    //==============================================================================
    void EffectsChain::prepareToPlay(double sampleRate, int samplesPerBlock)
    {
        currentSampleRate = sampleRate;
        currentBlockSize = samplesPerBlock;
        
        // Prepare temporary buffers
        tempBuffer.setSize(2, samplesPerBlock); // Stereo
        dryBuffer.setSize(2, samplesPerBlock);
        
        // Prepare all active effects
        juce::SpinLock::ScopedLockType lock(effectsLock);
        for (auto& slot : effectSlots)
        {
            if (slot.effect)
            {
                slot.effect->prepareToPlay(sampleRate, samplesPerBlock);
            }
        }
    }

    void EffectsChain::releaseResources()
    {
        juce::SpinLock::ScopedLockType lock(effectsLock);
        for (auto& slot : effectSlots)
        {
            if (slot.effect)
            {
                slot.effect->releaseResources();
            }
        }
    }

    void EffectsChain::reset()
    {
        juce::SpinLock::ScopedLockType lock(effectsLock);
        for (auto& slot : effectSlots)
        {
            if (slot.effect)
            {
                slot.effect->reset();
            }
        }
    }

    //==============================================================================
    bool EffectsChain::insertEffect(int slotIndex, std::unique_ptr<EffectProcessor> effect)
    {
        if (!isValidSlotIndex(slotIndex) || !effect)
            return false;
        
        juce::SpinLock::ScopedLockType lock(effectsLock);
        
        // Prepare the new effect
        effect->prepareToPlay(currentSampleRate, currentBlockSize);
        
        // Insert the effect
        effectSlots[slotIndex].effect = std::move(effect);
        effectSlots[slotIndex].isEnabled = true;
        effectSlots[slotIndex].slotMix = 1.0f;
        
        return true;
    }

    void EffectsChain::removeEffect(int slotIndex)
    {
        if (!isValidSlotIndex(slotIndex))
            return;
        
        juce::SpinLock::ScopedLockType lock(effectsLock);
        
        if (effectSlots[slotIndex].effect)
        {
            effectSlots[slotIndex].effect->releaseResources();
            effectSlots[slotIndex].effect.reset();
        }
    }

    EffectProcessor* EffectsChain::getEffect(int slotIndex) const
    {
        if (!isValidSlotIndex(slotIndex))
            return nullptr;
        
        return effectSlots[slotIndex].effect.get();
    }

    bool EffectsChain::isSlotOccupied(int slotIndex) const
    {
        if (!isValidSlotIndex(slotIndex))
            return false;
        
        return effectSlots[slotIndex].effect != nullptr;
    }

    bool EffectsChain::moveEffect(int fromSlot, int toSlot)
    {
        if (!isValidSlotIndex(fromSlot) || !isValidSlotIndex(toSlot))
            return false;
        
        if (fromSlot == toSlot)
            return true;
        
        juce::SpinLock::ScopedLockType lock(effectsLock);
        
        // Move the effect
        effectSlots[toSlot] = std::move(effectSlots[fromSlot]);
        
        return true;
    }

    //==============================================================================
    void EffectsChain::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
    {
        if (!chainEnabled)
            return;
        
        // Store dry signal for master mix
        if (masterMix < 0.999f)
        {
            dryBuffer.makeCopyOf(buffer);
        }
        
        // Process through each effect in the chain
        juce::SpinLock::ScopedLockType lock(effectsLock);
        for (auto& slot : effectSlots)
        {
            if (slot.effect && slot.isEnabled && !slot.effect->isBypassed())
            {
                // Process the effect (effect handles its own wet/dry mixing internally)
                slot.effect->processBlock(buffer, midiMessages);
            }
        }
        
        // Apply master wet/dry mix
        if (masterMix < 0.999f)
        {
            float wetGain = masterMix;
            float dryGain = 1.0f - masterMix;
            
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            {
                auto* wetData = buffer.getWritePointer(channel);
                const auto* dryData = dryBuffer.getReadPointer(channel);
                
                for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
                {
                    wetData[sample] = wetData[sample] * wetGain + dryData[sample] * dryGain;
                }
            }
        }
    }

    //==============================================================================
    double EffectsChain::getTotalCPUUsage() const
    {
        double totalCPU = 0.0;
        
        for (const auto& slot : effectSlots)
        {
            if (slot.effect && slot.isEnabled)
            {
                totalCPU += slot.effect->getCPUUsage();
            }
        }
        
        return totalCPU;
    }

    double EffectsChain::getSlotCPUUsage(int slotIndex) const
    {
        if (!isValidSlotIndex(slotIndex) || !effectSlots[slotIndex].effect)
            return 0.0;
        
        return effectSlots[slotIndex].effect->getCPUUsage();
    }

    int EffectsChain::getTotalLatencySamples() const
    {
        int totalLatency = 0;
        
        for (const auto& slot : effectSlots)
        {
            if (slot.effect && slot.isEnabled)
            {
                totalLatency += slot.effect->getLatencySamples();
            }
        }
        
        return totalLatency;
    }

    //==============================================================================
    juce::ValueTree EffectsChain::saveChainState() const
    {
        juce::ValueTree chainState("EffectsChain");
        
        chainState.setProperty("chainEnabled", chainEnabled, nullptr);
        chainState.setProperty("masterMix", masterMix, nullptr);
        
        for (int i = 0; i < MAX_EFFECT_SLOTS; ++i)
        {
            const auto& slot = effectSlots[i];
            if (slot.effect)
            {
                juce::ValueTree slotState("EffectSlot");
                slotState.setProperty("slotIndex", i, nullptr);
                slotState.setProperty("effectName", slot.effect->getEffectName(), nullptr);
                slotState.setProperty("isEnabled", slot.isEnabled, nullptr);
                slotState.setProperty("slotMix", slot.slotMix, nullptr);
                
                // Save effect-specific parameters
                auto parameterNames = slot.effect->getParameterNames();
                juce::ValueTree parametersState("Parameters");
                
                for (const auto& paramName : parameterNames)
                {
                    juce::ValueTree paramState("Parameter");
                    paramState.setProperty("name", paramName, nullptr);
                    paramState.setProperty("value", slot.effect->getParameter(paramName), nullptr);
                    parametersState.appendChild(paramState, nullptr);
                }
                
                slotState.appendChild(parametersState, nullptr);
                chainState.appendChild(slotState, nullptr);
            }
        }
        
        return chainState;
    }

    void EffectsChain::loadChainState(const juce::ValueTree& state)
    {
        if (!state.hasType("EffectsChain"))
            return;
        
        // Clear existing chain
        clearChain();
        
        // Load chain properties
        chainEnabled = state.getProperty("chainEnabled", true);
        masterMix = state.getProperty("masterMix", 1.0f);
        
        // Load effect slots
        for (int i = 0; i < state.getNumChildren(); ++i)
        {
            auto slotState = state.getChild(i);
            if (slotState.hasType("EffectSlot"))
            {
                int slotIndex = slotState.getProperty("slotIndex", -1);
                juce::String effectName = slotState.getProperty("effectName", "");
                
                if (isValidSlotIndex(slotIndex) && effectName.isNotEmpty())
                {
                    // Create the effect
                    auto effect = createEffect(effectName);
                    if (effect)
                    {
                        // Load parameters
                        auto parametersState = slotState.getChildWithName("Parameters");
                        if (parametersState.isValid())
                        {
                            for (int j = 0; j < parametersState.getNumChildren(); ++j)
                            {
                                auto paramState = parametersState.getChild(j);
                                if (paramState.hasType("Parameter"))
                                {
                                    juce::String paramName = paramState.getProperty("name", "");
                                    float paramValue = paramState.getProperty("value", 0.0f);
                                    
                                    if (paramName.isNotEmpty())
                                    {
                                        effect->setParameter(paramName, paramValue, false);
                                    }
                                }
                            }
                        }
                        
                        // Insert the effect
                        insertEffect(slotIndex, std::move(effect));
                        
                        // Set slot properties
                        effectSlots[slotIndex].isEnabled = slotState.getProperty("isEnabled", true);
                        effectSlots[slotIndex].slotMix = slotState.getProperty("slotMix", 1.0f);
                    }
                }
            }
        }
    }

    void EffectsChain::clearChain()
    {
        juce::SpinLock::ScopedLockType lock(effectsLock);
        
        for (auto& slot : effectSlots)
        {
            if (slot.effect)
            {
                slot.effect->releaseResources();
                slot.effect.reset();
            }
        }
    }

    //==============================================================================
    std::unique_ptr<EffectProcessor> EffectsChain::createEffect(const juce::String& effectName)
    {
        if (effectName.equalsIgnoreCase("Reverb"))
        {
            return std::make_unique<ReverbEngine>();
        }
        
        if (effectName.equalsIgnoreCase("Delay"))
        {
            return std::make_unique<DelayEngine>(); // Epic 6 Story 6.9 - COMPLETE
        }
        
        if (effectName.equalsIgnoreCase("Modulation") || 
            effectName.equalsIgnoreCase("Chorus") ||
            effectName.equalsIgnoreCase("Flanger") ||
            effectName.equalsIgnoreCase("Phaser"))
        {
            return std::make_unique<ModulationEngine>(); // Epic 6 Story 6.10 - COMPLETE
        }
        
        // Additional effects will be added here as they're implemented
        
        return nullptr; // Effect type not yet implemented
    }

    juce::StringArray EffectsChain::getAvailableEffectTypes()
    {
        // This will be populated as we implement each effect type
        return juce::StringArray{
            "Reverb",
            "Delay", 
            "Modulation", // Epic 6 Story 6.10 - ModulationEngine with Chorus/Flanger/Phaser/etc
            "Chorus",     // Alias for ModulationEngine
            "Flanger",    // Alias for ModulationEngine
            "Phaser",     // Alias for ModulationEngine
            "Distortion",
            "Filter",
            "Compressor",
            "EQ"
        };
    }

    //==============================================================================
    bool EffectsChain::isValidSlotIndex(int slotIndex) const
    {
        return slotIndex >= 0 && slotIndex < MAX_EFFECT_SLOTS;
    }

    EffectsChain::EffectSlot* EffectsChain::getSlotSafe(int slotIndex)
    {
        if (!isValidSlotIndex(slotIndex))
            return nullptr;
        
        return &effectSlots[slotIndex];
    }

    const EffectsChain::EffectSlot* EffectsChain::getSlotSafe(int slotIndex) const
    {
        if (!isValidSlotIndex(slotIndex))
            return nullptr;
        
        return &effectSlots[slotIndex];
    }

} // namespace spawnclone::audio
