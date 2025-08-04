/*
  ==============================================================================

    MultiOutputManager.cpp
    Created: 3 Aug 2025
    Author:  Epic 8 Story 8.3 Implementation

    Implementation of multi-output audio routing manager.

  ==============================================================================
*/

#include "MultiOutputManager.h"

//==============================================================================
MultiOutputManager::MultiOutputManager()
{
    initializeDefaultChannelMappings();
    lastCPUMeasurement = juce::Time::getCurrentTime();
}

MultiOutputManager::~MultiOutputManager()
{
}

//==============================================================================
void MultiOutputManager::prepareToPlay(double newSampleRate, int samplesPerBlock)
{
    sampleRate = newSampleRate;
    blockSize = samplesPerBlock;
    
    // Reset CPU monitoring
    currentCPUUsage.store(0.0f);
    lastCPUMeasurement = juce::Time::getCurrentTime();
}

void MultiOutputManager::releaseResources()
{
    // Nothing specific to release
}

void MultiOutputManager::processBlock(juce::AudioBuffer<float>& outputBuffer,
                                    const juce::AudioBuffer<float>& melodyBuffer,
                                    const juce::AudioBuffer<float>& chordsBuffer,
                                    const juce::AudioBuffer<float>& bassBuffer)
{
    auto cpuStartTime = juce::Time::getCurrentTime();
    
    // Ensure output buffer has the correct number of channels
    jassert(outputBuffer.getNumChannels() >= getTotalOutputChannels());
    
    // Clear output buffer
    outputBuffer.clear();
    
    // Process each layer to its designated channels
    processLayerToChannels(LayerEffectsProcessor::LayerType::Melody, outputBuffer, melodyBuffer);
    processLayerToChannels(LayerEffectsProcessor::LayerType::Chords, outputBuffer, chordsBuffer);
    processLayerToChannels(LayerEffectsProcessor::LayerType::Bass, outputBuffer, bassBuffer);
    
    // Update CPU usage
    auto cpuEndTime = juce::Time::getCurrentTime();
    auto cpuTime = cpuEndTime - cpuStartTime;
    float usage = (float)(cpuTime.inSeconds() / (blockSize / sampleRate));
    currentCPUUsage.store(juce::jmin(1.0f, usage));
}

//==============================================================================
void MultiOutputManager::setLayerChannels(LayerEffectsProcessor::LayerType layer, int leftChannel, int rightChannel)
{
    auto layerIndex = static_cast<int>(layer);
    if (layerIndex >= 0 && layerIndex < 3)
    {
        layerChannelMappings[layerIndex].leftChannel = leftChannel;
        layerChannelMappings[layerIndex].rightChannel = rightChannel;
    }
}

MultiOutputManager::LayerChannels MultiOutputManager::getLayerChannels(LayerEffectsProcessor::LayerType layer) const
{
    auto layerIndex = static_cast<int>(layer);
    if (layerIndex >= 0 && layerIndex < 3)
        return layerChannelMappings[layerIndex];
    
    return {};
}

//==============================================================================
void MultiOutputManager::setLayerMuted(LayerEffectsProcessor::LayerType layer, bool muted)
{
    auto layerIndex = static_cast<int>(layer);
    if (layerIndex >= 0 && layerIndex < 3)
        layerChannelMappings[layerIndex].muted = muted;
}

void MultiOutputManager::setLayerSoloed(LayerEffectsProcessor::LayerType layer, bool soloed)
{
    auto layerIndex = static_cast<int>(layer);
    if (layerIndex >= 0 && layerIndex < 3)
        layerChannelMappings[layerIndex].soloed = soloed;
}

bool MultiOutputManager::isLayerMuted(LayerEffectsProcessor::LayerType layer) const
{
    auto layerIndex = static_cast<int>(layer);
    if (layerIndex >= 0 && layerIndex < 3)
        return layerChannelMappings[layerIndex].muted;
    return false;
}

bool MultiOutputManager::isLayerSoloed(LayerEffectsProcessor::LayerType layer) const
{
    auto layerIndex = static_cast<int>(layer);
    if (layerIndex >= 0 && layerIndex < 3)
        return layerChannelMappings[layerIndex].soloed;
    return false;
}

bool MultiOutputManager::isAnyLayerSoloed() const
{
    for (const auto& layer : layerChannelMappings)
    {
        if (layer.soloed)
            return true;
    }
    return false;
}

//==============================================================================
void MultiOutputManager::setLayerVolume(LayerEffectsProcessor::LayerType layer, float volume)
{
    auto layerIndex = static_cast<int>(layer);
    if (layerIndex >= 0 && layerIndex < 3)
        layerChannelMappings[layerIndex].volume = juce::jlimit(0.0f, 2.0f, volume);
}

void MultiOutputManager::setLayerPan(LayerEffectsProcessor::LayerType layer, float pan)
{
    auto layerIndex = static_cast<int>(layer);
    if (layerIndex >= 0 && layerIndex < 3)
        layerChannelMappings[layerIndex].pan = juce::jlimit(-1.0f, 1.0f, pan);
}

float MultiOutputManager::getLayerVolume(LayerEffectsProcessor::LayerType layer) const
{
    auto layerIndex = static_cast<int>(layer);
    if (layerIndex >= 0 && layerIndex < 3)
        return layerChannelMappings[layerIndex].volume;
    return 1.0f;
}

float MultiOutputManager::getLayerPan(LayerEffectsProcessor::LayerType layer) const
{
    auto layerIndex = static_cast<int>(layer);
    if (layerIndex >= 0 && layerIndex < 3)
        return layerChannelMappings[layerIndex].pan;
    return 0.0f;
}

//==============================================================================
void MultiOutputManager::setOutputEnabled(LayerEffectsProcessor::LayerType layer, bool enabled)
{
    auto layerIndex = static_cast<int>(layer);
    if (layerIndex >= 0 && layerIndex < 3)
        layerChannelMappings[layerIndex].enabled = enabled;
}

bool MultiOutputManager::isOutputEnabled(LayerEffectsProcessor::LayerType layer) const
{
    auto layerIndex = static_cast<int>(layer);
    if (layerIndex >= 0 && layerIndex < 3)
        return layerChannelMappings[layerIndex].enabled;
    return true;
}

juce::String MultiOutputManager::getChannelLabel(int channelIndex) const
{
    switch (channelIndex)
    {
        case 0: return "Melody L";
        case 1: return "Melody R";
        case 2: return "Chords L";
        case 3: return "Chords R";
        case 4: return "Bass L";
        case 5: return "Bass R";
        default: return "Unknown";
    }
}

//==============================================================================
void MultiOutputManager::saveRoutingPreset(const juce::String& name)
{
    RoutingPreset preset;
    preset.name = name;
    preset.layerChannels = layerChannelMappings;
    preset.masterEnabled = true;
    
    presets[name] = preset;
}

void MultiOutputManager::loadRoutingPreset(const juce::String& name)
{
    auto it = presets.find(name);
    if (it != presets.end())
    {
        layerChannelMappings = it->second.layerChannels;
    }
}

juce::StringArray MultiOutputManager::getAvailablePresets() const
{
    juce::StringArray presetNames;
    for (const auto& pair : presets)
        presetNames.add(pair.first);
    return presetNames;
}

//==============================================================================
void MultiOutputManager::initializeDefaultChannelMappings()
{
    // Melody: Channels 1-2 (0-1)
    layerChannelMappings[0].leftChannel = 0;
    layerChannelMappings[0].rightChannel = 1;
    layerChannelMappings[0].enabled = true;
    layerChannelMappings[0].volume = 1.0f;
    layerChannelMappings[0].pan = 0.0f;
    
    // Chords: Channels 3-4 (2-3)
    layerChannelMappings[1].leftChannel = 2;
    layerChannelMappings[1].rightChannel = 3;
    layerChannelMappings[1].enabled = true;
    layerChannelMappings[1].volume = 1.0f;
    layerChannelMappings[1].pan = 0.0f;
    
    // Bass: Channels 5-6 (4-5)
    layerChannelMappings[2].leftChannel = 4;
    layerChannelMappings[2].rightChannel = 5;
    layerChannelMappings[2].enabled = true;
    layerChannelMappings[2].volume = 1.0f;
    layerChannelMappings[2].pan = 0.0f;
}

void MultiOutputManager::processLayerToChannels(LayerEffectsProcessor::LayerType layer,
                                               juce::AudioBuffer<float>& outputBuffer,
                                               const juce::AudioBuffer<float>& inputBuffer)
{
    if (!shouldProcessLayer(layer) || inputBuffer.getNumSamples() == 0)
        return;
    
    auto layerIndex = static_cast<int>(layer);
    const auto& channels = layerChannelMappings[layerIndex];
    
    // Ensure we don't exceed output buffer channels
    if (channels.leftChannel >= outputBuffer.getNumChannels() ||
        channels.rightChannel >= outputBuffer.getNumChannels())
        return;
    
    int numSamples = juce::jmin(inputBuffer.getNumSamples(), outputBuffer.getNumSamples());
    
    // Copy input to output channels with pan and volume
    if (inputBuffer.getNumChannels() >= 2)
    {
        // Stereo input
        auto leftGain = channels.volume * (1.0f - juce::jmax(0.0f, channels.pan));
        auto rightGain = channels.volume * (1.0f + juce::jmin(0.0f, channels.pan));
        
        outputBuffer.addFrom(channels.leftChannel, 0, inputBuffer, 0, 0, numSamples, leftGain);
        outputBuffer.addFrom(channels.rightChannel, 0, inputBuffer, 1, 0, numSamples, rightGain);
    }
    else if (inputBuffer.getNumChannels() >= 1)
    {
        // Mono input - distribute with pan
        auto leftGain = channels.volume * (1.0f - juce::jmax(0.0f, channels.pan));
        auto rightGain = channels.volume * (1.0f + juce::jmin(0.0f, channels.pan));
        
        outputBuffer.addFrom(channels.leftChannel, 0, inputBuffer, 0, 0, numSamples, leftGain);
        outputBuffer.addFrom(channels.rightChannel, 0, inputBuffer, 0, 0, numSamples, rightGain);
    }
}

bool MultiOutputManager::shouldProcessLayer(LayerEffectsProcessor::LayerType layer) const
{
    auto layerIndex = static_cast<int>(layer);
    if (layerIndex < 0 || layerIndex >= 3)
        return false;
    
    const auto& channels = layerChannelMappings[layerIndex];
    
    // Don't process if disabled
    if (!channels.enabled)
        return false;
    
    // Don't process if muted
    if (channels.muted)
        return false;
    
    // If any layer is soloed, only process soloed layers
    if (isAnyLayerSoloed() && !channels.soloed)
        return false;
    
    return true;
}
