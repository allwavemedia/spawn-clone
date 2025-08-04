/*
  ==============================================================================

    MultiOutputManager.h
    Created: 3 Aug 2025
    Author:  Epic 8 Story 8.3 Implementation

    Multi-output audio routing manager for layer-based mixing.
    Routes melody, chords, and bass to separate stereo pairs for 
    professional DAW integration.

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_core/juce_core.h>
#include "LayerEffectsProcessor.h"

//==============================================================================
/**
 * Epic 8 Story 8.3: Multi-Output Audio Routing
 * 
 * Manages 6-channel output routing (3 stereo pairs):
 * - Channels 1-2: Melody Layer
 * - Channels 3-4: Chords Layer  
 * - Channels 5-6: Bass Layer
 */
class MultiOutputManager
{
public:
    //==============================================================================
    /** Output channel mapping */
    enum class OutputChannel
    {
        MelodyLeft = 0,     // Channel 1
        MelodyRight = 1,    // Channel 2
        ChordsLeft = 2,     // Channel 3
        ChordsRight = 3,    // Channel 4
        BassLeft = 4,       // Channel 5
        BassRight = 5       // Channel 6
    };
    
    /** Layer to stereo pair mapping */
    struct LayerChannels
    {
        int leftChannel;
        int rightChannel;
        bool enabled = true;
        bool muted = false;
        bool soloed = false;
        float volume = 1.0f;
        float pan = 0.0f;   // -1.0 to 1.0
    };
    
    //==============================================================================
    MultiOutputManager();
    ~MultiOutputManager();
    
    //==============================================================================
    /** Prepare for audio processing */
    void prepareToPlay(double sampleRate, int samplesPerBlock);
    
    /** Release audio resources */
    void releaseResources();
    
    /** Process multi-output routing */
    void processBlock(juce::AudioBuffer<float>& outputBuffer,
                     const juce::AudioBuffer<float>& melodyBuffer,
                     const juce::AudioBuffer<float>& chordsBuffer,
                     const juce::AudioBuffer<float>& bassBuffer);
    
    //==============================================================================
    /** Layer routing control */
    void setLayerChannels(LayerEffectsProcessor::LayerType layer, int leftChannel, int rightChannel);
    LayerChannels getLayerChannels(LayerEffectsProcessor::LayerType layer) const;
    
    /** Layer mute/solo control */
    void setLayerMuted(LayerEffectsProcessor::LayerType layer, bool muted);
    void setLayerSoloed(LayerEffectsProcessor::LayerType layer, bool soloed);
    bool isLayerMuted(LayerEffectsProcessor::LayerType layer) const;
    bool isLayerSoloed(LayerEffectsProcessor::LayerType layer) const;
    bool isAnyLayerSoloed() const;
    
    /** Layer volume and pan */
    void setLayerVolume(LayerEffectsProcessor::LayerType layer, float volume);
    void setLayerPan(LayerEffectsProcessor::LayerType layer, float pan);
    float getLayerVolume(LayerEffectsProcessor::LayerType layer) const;
    float getLayerPan(LayerEffectsProcessor::LayerType layer) const;
    
    //==============================================================================
    /** Output configuration */
    void setOutputEnabled(LayerEffectsProcessor::LayerType layer, bool enabled);
    bool isOutputEnabled(LayerEffectsProcessor::LayerType layer) const;
    
    /** Get channel labels for DAW display */
    juce::String getChannelLabel(int channelIndex) const;
    
    /** Get total number of output channels */
    static constexpr int getTotalOutputChannels() { return 6; }
    
    //==============================================================================
    /** Preset management */
    struct RoutingPreset
    {
        juce::String name;
        std::array<LayerChannels, 3> layerChannels;
        bool masterEnabled = true;
    };
    
    void saveRoutingPreset(const juce::String& name);
    void loadRoutingPreset(const juce::String& name);
    juce::StringArray getAvailablePresets() const;
    
    //==============================================================================
    /** Performance monitoring */
    float getCurrentCPUUsage() const { return currentCPUUsage.load(); }
    void resetCPUUsage() { currentCPUUsage.store(0.0f); }

private:
    //==============================================================================
    /** Layer channel mappings */
    std::array<LayerChannels, 3> layerChannelMappings;
    
    /** Audio processing */
    double sampleRate = 44100.0;
    int blockSize = 512;
    
    /** Performance monitoring */
    std::atomic<float> currentCPUUsage{0.0f};
    juce::Time lastCPUMeasurement;
    
    /** Preset storage */
    std::map<juce::String, RoutingPreset> presets;
    
    //==============================================================================
    /** Helper methods */
    void initializeDefaultChannelMappings();
    void updateCPUUsage();
    void applyPanAndVolume(juce::AudioBuffer<float>& buffer, const LayerChannels& channels);
    bool shouldProcessLayer(LayerEffectsProcessor::LayerType layer) const;
    void processLayerToChannels(LayerEffectsProcessor::LayerType layer,
                               juce::AudioBuffer<float>& outputBuffer,
                               const juce::AudioBuffer<float>& inputBuffer);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MultiOutputManager)
};
