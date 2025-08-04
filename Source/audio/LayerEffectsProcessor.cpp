/*
  ==============================================================================

    LayerEffectsProcessor.cpp
    Created: 3 Aug 2025
    Author:  Epic 8 Story 8.2 Implementation

    Per-layer effects processing implementation for melody, chords, and bass layers.

  ==============================================================================
*/

#include "LayerEffectsProcessor.h"

//==============================================================================
LayerEffectsProcessor::LayerEffectsProcessor()
{
    lastCPUMeasurement = juce::Time::getCurrentTime();
    
    // Initialize default parameters for each layer
    for (int i = 0; i < static_cast<int>(LayerType::NumLayers); ++i)
    {
        auto& chain = layerChains[i];
        
        // Set layer-specific default parameters
        switch (static_cast<LayerType>(i))
        {
            case LayerType::Melody:
                chain.parameters.filterCutoff = 2000.0f;   // Brighter for melody
                chain.parameters.reverbWetLevel = 0.3f;    // More reverb for melody
                chain.parameters.delayWetLevel = 0.25f;    // Moderate delay
                break;
                
            case LayerType::Chords:
                chain.parameters.filterCutoff = 1000.0f;   // Mid-range for chords
                chain.parameters.reverbWetLevel = 0.4f;    // Rich reverb for chords
                chain.parameters.chorusMix = 0.4f;         // Chorus for width
                break;
                
            case LayerType::Bass:
                chain.parameters.filterCutoff = 500.0f;    // Darker for bass
                chain.parameters.reverbWetLevel = 0.1f;    // Less reverb for bass
                chain.parameters.distortionMix = 0.1f;     // Slight warmth
                break;
                
            default:
                break;
        }
        
        // Initialize ADSR parameters
        chain.adsrParams.attack = chain.parameters.adsrAttack;
        chain.adsrParams.decay = chain.parameters.adsrDecay;
        chain.adsrParams.sustain = chain.parameters.adsrSustain;
        chain.adsrParams.release = chain.parameters.adsrRelease;
        chain.adsr.setParameters(chain.adsrParams);
    }
}

LayerEffectsProcessor::~LayerEffectsProcessor()
{
    releaseResources();
}

//==============================================================================
void LayerEffectsProcessor::prepareToPlay(double sampleRate, int samplesPerBlock, int numChannels)
{
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = static_cast<juce::uint32>(numChannels);
    
    // Epic 8 Story 8.4: Prepare advanced real-time processor
    realTimeProcessor.prepareToPlay(sampleRate, samplesPerBlock);
    
    // Prepare each layer's effects chain
    for (auto& chain : layerChains)
    {
        setupEffectsChain(chain);
        chain.processorChain.prepare(spec);
        chain.adsr.setSampleRate(sampleRate);
        chain.pitchShiftDelay.prepare(spec);
        chain.panner.prepare(spec);
        chain.volumeGain.prepare(spec);
        chain.prepared = true;
    }
    
    juce::Logger::writeToLog("LayerEffectsProcessor: Prepared for " + 
                           juce::String(sampleRate) + "Hz, " + 
                           juce::String(samplesPerBlock) + " samples with Story 8.4 real-time processing");
}

void LayerEffectsProcessor::releaseResources()
{
    // Epic 8 Story 8.4: Release real-time processor resources
    realTimeProcessor.releaseResources();
    
    for (auto& chain : layerChains)
    {
        chain.processorChain.reset();
        chain.adsr.reset();
        chain.pitchShiftDelay.reset();
        chain.panner.reset();
        chain.volumeGain.reset();
        chain.prepared = false;
    }
}

//==============================================================================
void LayerEffectsProcessor::processLayer(LayerType layer, juce::AudioBuffer<float>& buffer, 
                                        int startSample, int numSamples)
{
    auto layerIndex = static_cast<int>(layer);
    auto& chain = layerChains[layerIndex];
    
    if (!chain.prepared)
        return;
    
    // Determine processing range
    const int samplesToProcess = (numSamples < 0) ? buffer.getNumSamples() - startSample : numSamples;
    if (samplesToProcess <= 0)
        return;
    
    // Epic 8 Story 8.4: Process real-time parameter smoothing
    realTimeProcessor.processParameterSmoothing(samplesToProcess);
    updateLayerFromRealTimeProcessor(layer);
    
    // Check if layer is muted or if another layer is soloed
    if (isLayerMuted(layer) || (isAnyLayerSoloed() && !isLayerSoloed(layer)))
    {
        buffer.clear(startSample, samplesToProcess);
        return;
    }
    
    auto cpuStartTime = juce::Time::getCurrentTime();
    
    // Create audio block for DSP processing
    juce::dsp::AudioBlock<float> block(buffer.getArrayOfWritePointers(), 
                                      buffer.getNumChannels(), 
                                      startSample, 
                                      samplesToProcess);
    
    // Apply global pitch shift if needed
    if (std::abs(globalPitchShift.load()) > 0.01f)
    {
        // Simple pitch shifting using delay line and interpolation
        float pitchRatio = semitonesToRatio(globalPitchShift.load());
        if (std::abs(pitchRatio - chain.currentPitchRatio) > 0.001f)
        {
            chain.currentPitchRatio = pitchRatio;
            // Update delay line for pitch shifting
            float delayTimeMs = (1.0f - pitchRatio) * 50.0f; // Max 50ms delay
            chain.pitchShiftDelay.setDelay(delayTimeMs * spec.sampleRate / 1000.0f);
        }
        
        // Process through pitch shift delay
        juce::dsp::ProcessContextReplacing<float> pitchContext(block);
        chain.pitchShiftDelay.process(pitchContext);
    }
    
    // Process through effects chain
    juce::dsp::ProcessContextReplacing<float> context(block);
    chain.processorChain.process(context);
    
    // Apply ADSR envelope (simplified - would need proper note tracking)
    // For now, just apply a smooth gain curve
    float adsrGain = chain.adsr.getNextSample();
    if (adsrGain != 1.0f)
    {
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        {
            auto* channelData = buffer.getWritePointer(channel, startSample);
            for (int sample = 0; sample < samplesToProcess; ++sample)
            {
                channelData[sample] *= adsrGain;
            }
        }
    }
    
    // Apply panning
    chain.panner.process(context);
    
    // Apply volume
    chain.volumeGain.process(context);
    
    // Update CPU usage
    updateCPUUsage();
}

void LayerEffectsProcessor::processAllLayers(juce::AudioBuffer<float>& melodyBuffer,
                                           juce::AudioBuffer<float>& chordsBuffer,
                                           juce::AudioBuffer<float>& bassBuffer)
{
    processLayer(LayerType::Melody, melodyBuffer);
    processLayer(LayerType::Chords, chordsBuffer);
    processLayer(LayerType::Bass, bassBuffer);
}

//==============================================================================
LayerEffectsProcessor::EffectParameters& LayerEffectsProcessor::getLayerParameters(LayerType layer)
{
    return layerChains[static_cast<int>(layer)].parameters;
}

const LayerEffectsProcessor::EffectParameters& LayerEffectsProcessor::getLayerParameters(LayerType layer) const
{
    return layerChains[static_cast<int>(layer)].parameters;
}

void LayerEffectsProcessor::setLayerParameters(LayerType layer, const EffectParameters& params)
{
    auto layerIndex = static_cast<int>(layer);
    auto& chain = layerChains[layerIndex];
    
    chain.parameters = params;
    
    if (chain.prepared)
    {
        // Update effects chain with new parameters
        setupEffectsChain(chain);
        
        // Update ADSR parameters
        chain.adsrParams.attack = params.adsrAttack;
        chain.adsrParams.decay = params.adsrDecay;
        chain.adsrParams.sustain = params.adsrSustain;
        chain.adsrParams.release = params.adsrRelease;
        chain.adsr.setParameters(chain.adsrParams);
        
        // Update volume and pan
        chain.volumeGain.setGainLinear(params.layerVolume);
        chain.panner.setPan(params.layerPan);
    }
}

//==============================================================================
void LayerEffectsProcessor::setFilterCutoff(LayerType layer, float value)
{
    auto layerIndex = static_cast<int>(layer);
    auto& chain = layerChains[layerIndex];
    
    chain.parameters.filterCutoff = juce::jlimit(20.0f, 20000.0f, value);
    
    if (chain.prepared)
    {
        // Update low-pass filter (index 1 in processor chain)
        auto& lpFilter = chain.processorChain.template get<1>();
        lpFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(
            spec.sampleRate, chain.parameters.filterCutoff, chain.parameters.filterResonance);
    }
}

void LayerEffectsProcessor::setReverbMix(LayerType layer, float value)
{
    auto layerIndex = static_cast<int>(layer);
    auto& chain = layerChains[layerIndex];
    
    chain.parameters.reverbWetLevel = juce::jlimit(0.0f, 1.0f, value);
    chain.parameters.reverbDryLevel = 1.0f - chain.parameters.reverbWetLevel;
    
    if (chain.prepared)
    {
        // Update reverb parameters (index 2 in processor chain)
        auto& reverb = chain.processorChain.template get<2>();
        juce::Reverb::Parameters reverbParams;
        reverbParams.roomSize = chain.parameters.reverbRoomSize;
        reverbParams.damping = chain.parameters.reverbDamping;
        reverbParams.wetLevel = chain.parameters.reverbWetLevel;
        reverbParams.dryLevel = chain.parameters.reverbDryLevel;
        reverb.setParameters(reverbParams);
    }
}

void LayerEffectsProcessor::setDelayFeedback(LayerType layer, float value)
{
    auto layerIndex = static_cast<int>(layer);
    auto& chain = layerChains[layerIndex];
    
    chain.parameters.delayFeedback = juce::jlimit(0.0f, 0.95f, value);
    
    // Note: JUCE's DelayLine doesn't have built-in feedback control
    // In a full implementation, we'd need a custom delay with feedback
}

void LayerEffectsProcessor::setChorusMix(LayerType layer, float value)
{
    auto layerIndex = static_cast<int>(layer);
    auto& chain = layerChains[layerIndex];
    
    chain.parameters.chorusMix = juce::jlimit(0.0f, 1.0f, value);
    
    if (chain.prepared)
    {
        // Update chorus mix (index 4 in processor chain)
        auto& chorus = chain.processorChain.template get<4>();
        chorus.setMix(chain.parameters.chorusMix);
    }
}

void LayerEffectsProcessor::setDistortionMix(LayerType layer, float value)
{
    auto layerIndex = static_cast<int>(layer);
    auto& chain = layerChains[layerIndex];
    
    chain.parameters.distortionMix = juce::jlimit(0.0f, 1.0f, value);
    
    // Note: WaveShaper doesn't have built-in mix control
    // In a full implementation, we'd apply the mix manually
}

void LayerEffectsProcessor::setLayerVolume(LayerType layer, float value)
{
    auto layerIndex = static_cast<int>(layer);
    auto& chain = layerChains[layerIndex];
    
    chain.parameters.layerVolume = juce::jlimit(0.0f, 1.0f, value);
    
    if (chain.prepared)
    {
        chain.volumeGain.setGainLinear(chain.parameters.layerVolume);
    }
}

void LayerEffectsProcessor::setLayerPan(LayerType layer, float value)
{
    auto layerIndex = static_cast<int>(layer);
    auto& chain = layerChains[layerIndex];
    
    chain.parameters.layerPan = juce::jlimit(-1.0f, 1.0f, value);
    
    if (chain.prepared)
    {
        chain.panner.setPan(chain.parameters.layerPan);
    }
}

//==============================================================================
void LayerEffectsProcessor::setGlobalPitchShift(float semitones)
{
    globalPitchShift.store(juce::jlimit(-12.0f, 12.0f, semitones));
}

void LayerEffectsProcessor::setEffectsMorph(float morphValue)
{
    effectsMorphValue.store(juce::jlimit(0.0f, 1.0f, morphValue));
    
    // Apply morphing to all layers
    for (int i = 0; i < static_cast<int>(LayerType::NumLayers); ++i)
    {
        updateEffectsFromMorph(static_cast<LayerType>(i), morphValue);
    }
}

//==============================================================================
void LayerEffectsProcessor::setLayerMuted(LayerType layer, bool muted)
{
    layerChains[static_cast<int>(layer)].parameters.layerMuted = muted;
}

void LayerEffectsProcessor::setLayerSoloed(LayerType layer, bool soloed)
{
    layerChains[static_cast<int>(layer)].parameters.layerSoloed = soloed;
}

bool LayerEffectsProcessor::isLayerMuted(LayerType layer) const
{
    return layerChains[static_cast<int>(layer)].parameters.layerMuted;
}

bool LayerEffectsProcessor::isLayerSoloed(LayerType layer) const
{
    return layerChains[static_cast<int>(layer)].parameters.layerSoloed;
}

bool LayerEffectsProcessor::isAnyLayerSoloed() const
{
    for (const auto& chain : layerChains)
    {
        if (chain.parameters.layerSoloed)
            return true;
    }
    return false;
}

//==============================================================================
void LayerEffectsProcessor::updateEffectsFromMorph(LayerType layer, float morphValue)
{
    auto layerIndex = static_cast<int>(layer);
    auto& params = layerChains[layerIndex].parameters;
    
    // Morph different parameters based on layer type
    switch (layer)
    {
        case LayerType::Melody:
            // Melody: morph filter cutoff and reverb
            setFilterCutoff(layer, 500.0f + morphValue * 3500.0f); // 500Hz to 4kHz
            setReverbMix(layer, 0.1f + morphValue * 0.4f);         // 10% to 50%
            break;
            
        case LayerType::Chords:
            // Chords: morph chorus and delay
            setChorusMix(layer, morphValue * 0.6f);                // 0% to 60%
            setDelayFeedback(layer, morphValue * 0.5f);            // 0% to 50%
            break;
            
        case LayerType::Bass:
            // Bass: morph distortion and filter
            setDistortionMix(layer, morphValue * 0.3f);            // 0% to 30%
            setFilterCutoff(layer, 200.0f + morphValue * 800.0f);  // 200Hz to 1kHz
            break;
            
        default:
            break;
    }
}

float LayerEffectsProcessor::semitonesToRatio(float semitones)
{
    return std::pow(2.0f, semitones / 12.0f);
}

void LayerEffectsProcessor::setupEffectsChain(LayerEffectsChain& chain)
{
    auto& params = chain.parameters;
    
    // Setup high-pass filter (index 0)
    auto& hpFilter = chain.processorChain.template get<0>();
    hpFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(
        spec.sampleRate, 40.0f, 0.7f); // High-pass at 40Hz
    
    // Setup low-pass filter (index 1)
    auto& lpFilter = chain.processorChain.template get<1>();
    lpFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(
        spec.sampleRate, params.filterCutoff, params.filterResonance);
    
    // Setup reverb (index 2)
    auto& reverb = chain.processorChain.template get<2>();
    juce::Reverb::Parameters reverbParams;
    reverbParams.roomSize = params.reverbRoomSize;
    reverbParams.damping = params.reverbDamping;
    reverbParams.wetLevel = params.reverbWetLevel;
    reverbParams.dryLevel = params.reverbDryLevel;
    reverb.setParameters(reverbParams);
    
    // Setup delay (index 3)
    auto& delay = chain.processorChain.template get<3>();
    delay.setMaximumDelayInSamples(static_cast<int>(spec.sampleRate * 2.0)); // 2 second max
    delay.setDelay(params.delayTime * spec.sampleRate);
    
    // Setup chorus (index 4)
    auto& chorus = chain.processorChain.template get<4>();
    chorus.setRate(params.chorusRate);
    chorus.setDepth(params.chorusDepth);
    chorus.setCentreDelay(params.chorusCentreDelay);
    chorus.setFeedback(params.chorusFeedback);
    chorus.setMix(params.chorusMix);
    
    // Setup flanger (using phaser, index 5)
    auto& flanger = chain.processorChain.template get<5>();
    flanger.setRate(params.flangerRate);
    flanger.setDepth(params.flangerDepth);
    flanger.setCentreFrequency(1000.0f / params.flangerCentreDelay); // Approximate
    flanger.setFeedback(params.flangerFeedback);
    flanger.setMix(params.flangerMix);
    
    // Setup distortion (index 6)
    auto& waveshaper = chain.processorChain.template get<6>();
    waveshaper.functionToUse = [](float x) {
        float drive = 5.0f; // Default drive, could be made parameter-driven
        return std::tanh(x * drive) / drive;
    };
    
    // Setup volume and pan
    chain.volumeGain.setGainLinear(params.layerVolume);
    chain.panner.setPan(params.layerPan);
}

void LayerEffectsProcessor::updateCPUUsage()
{
    auto currentTime = juce::Time::getCurrentTime();
    auto timeDiff = currentTime - lastCPUMeasurement;
    
    if (timeDiff.inMilliseconds() > 100) // Update every 100ms
    {
        // Simplified CPU usage calculation
        // In a real implementation, this would measure actual processing time
        float estimatedUsage = 0.1f; // Base usage
        
        // Add usage based on active effects
        for (const auto& chain : layerChains)
        {
            if (!isLayerMuted(static_cast<LayerType>(&chain - &layerChains[0])))
            {
                estimatedUsage += 0.05f; // Each active layer adds 5%
            }
        }
        
        currentCPUUsage.store(estimatedUsage);
        lastCPUMeasurement = currentTime;
        
        // Epic 8 Story 8.4: Update real-time processor with CPU usage
        realTimeProcessor.updateCPUUsage(estimatedUsage);
    }
}

//==============================================================================
// Epic 8 Story 8.4: Advanced Real-Time Processing Implementation
//==============================================================================

void LayerEffectsProcessor::setParameterSmooth(LayerType layer, RealTimeProcessor::ParameterType paramType, 
                                              float value, float smoothingTime)
{
    // Map parameter to the appropriate layer-specific parameter
    int layerOffset = static_cast<int>(layer) * static_cast<int>(RealTimeProcessor::ParameterType::NumParameterTypes);
    
    // For simplicity, we'll use a direct mapping approach
    // In a full implementation, you might want more sophisticated layer-parameter mapping
    realTimeProcessor.setParameterTarget(paramType, value, smoothingTime);
    
    // Also update the direct parameters for compatibility
    auto& params = layerChains[static_cast<int>(layer)].parameters;
    
    switch (paramType)
    {
        case RealTimeProcessor::ParameterType::FilterCutoff:
            setFilterCutoff(layer, juce::jlimit(20.0f, 20000.0f, value * 19980.0f + 20.0f));
            break;
            
        case RealTimeProcessor::ParameterType::ReverbMix:
            setReverbMix(layer, value);
            break;
            
        case RealTimeProcessor::ParameterType::DelayFeedback:
            setDelayFeedback(layer, value);
            break;
            
        case RealTimeProcessor::ParameterType::ChorusMix:
            setChorusMix(layer, value);
            break;
            
        case RealTimeProcessor::ParameterType::DistortionMix:
            setDistortionMix(layer, value);
            break;
            
        case RealTimeProcessor::ParameterType::Volume:
            setLayerVolume(layer, value);
            break;
            
        case RealTimeProcessor::ParameterType::Pan:
            setLayerPan(layer, (value - 0.5f) * 2.0f); // Convert 0-1 to -1 to +1
            break;
            
        case RealTimeProcessor::ParameterType::PitchShift:
            // Global pitch shift from -12 to +12 semitones
            setGlobalPitchShift((value - 0.5f) * 24.0f);
            break;
            
        default:
            break;
    }
}

RealTimeProcessor::PerformanceMetrics LayerEffectsProcessor::getPerformanceMetrics() const
{
    return realTimeProcessor.getPerformanceMetrics();
}

bool LayerEffectsProcessor::isRealTimeSafe() const
{
    return realTimeProcessor.isRealTimeSafe();
}

void LayerEffectsProcessor::validateRealTimeConstraints()
{
    realTimeProcessor.validateRealTimeConstraints();
    
    // Additional layer-specific validation
    auto metrics = getPerformanceMetrics();
    if (metrics.cpuUsage > 0.8f)
    {
        juce::Logger::writeToLog("LayerEffectsProcessor: High CPU usage detected: " + 
                               juce::String(metrics.cpuUsage * 100.0f, 1) + "%");
    }
    
    if (metrics.droppedFrames > 0)
    {
        juce::Logger::writeToLog("LayerEffectsProcessor: Audio dropouts detected: " + 
                               juce::String(metrics.droppedFrames) + " frames");
    }
}

void LayerEffectsProcessor::resetPerformanceMetrics()
{
    realTimeProcessor.resetPerformanceMetrics();
    currentCPUUsage.store(0.0f);
}

void LayerEffectsProcessor::updateLayerFromRealTimeProcessor(LayerType layer)
{
    // Get smoothed parameter values from the real-time processor
    // and apply them to the layer's effects chain
    
    auto layerIndex = static_cast<int>(layer);
    auto& chain = layerChains[layerIndex];
    
    if (!chain.prepared)
        return;
    
    // Check for parameter updates and apply them smoothly
    float filterValue = realTimeProcessor.getCurrentParameterValue(RealTimeProcessor::ParameterType::FilterCutoff);
    if (realTimeProcessor.parameterNeedsUpdate(RealTimeProcessor::ParameterType::FilterCutoff))
    {
        float cutoffFreq = juce::jlimit(20.0f, 20000.0f, filterValue * 19980.0f + 20.0f);
        
        // Update low-pass filter smoothly
        auto& lpFilter = chain.processorChain.template get<1>();
        lpFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(
            spec.sampleRate, cutoffFreq, chain.parameters.filterResonance);
    }
    
    // Update reverb mix if needed
    float reverbValue = realTimeProcessor.getCurrentParameterValue(RealTimeProcessor::ParameterType::ReverbMix);
    if (realTimeProcessor.parameterNeedsUpdate(RealTimeProcessor::ParameterType::ReverbMix))
    {
        auto& reverb = chain.processorChain.template get<2>();
        juce::Reverb::Parameters reverbParams;
        reverbParams.roomSize = chain.parameters.reverbRoomSize;
        reverbParams.damping = chain.parameters.reverbDamping;
        reverbParams.wetLevel = reverbValue;
        reverbParams.dryLevel = 1.0f - reverbValue;
        reverb.setParameters(reverbParams);
        
        chain.parameters.reverbWetLevel = reverbValue;
        chain.parameters.reverbDryLevel = 1.0f - reverbValue;
    }
    
    // Update volume if needed
    float volumeValue = realTimeProcessor.getCurrentParameterValue(RealTimeProcessor::ParameterType::Volume);
    if (realTimeProcessor.parameterNeedsUpdate(RealTimeProcessor::ParameterType::Volume))
    {
        chain.volumeGain.setGainLinear(volumeValue);
        chain.parameters.layerVolume = volumeValue;
    }
    
    // Update pan if needed
    float panValue = realTimeProcessor.getCurrentParameterValue(RealTimeProcessor::ParameterType::Pan);
    if (realTimeProcessor.parameterNeedsUpdate(RealTimeProcessor::ParameterType::Pan))
    {
        float panPosition = (panValue - 0.5f) * 2.0f; // Convert 0-1 to -1 to +1
        chain.panner.setPan(panPosition);
        chain.parameters.layerPan = panPosition;
    }
}
