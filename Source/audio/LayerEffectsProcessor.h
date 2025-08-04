/*
  ==============================================================================

    LayerEffectsProcessor.h
    Created: 3 Aug 2025
    Author:  Epic 8 Story 8.2 Implementation

    Per-layer effects processing for melody, chords, and bass layers.
    Provides independent effects chains for each pattern layer with
    real-time parameter control from the ExperimentPad.

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_core/juce_core.h>

//==============================================================================
/**
 * Epic 8 Story 8.2: Per-Layer Effects Processing
 * 
 * Provides independent effects chains for melody, chords, and bass layers.
 * Each layer has its own reverb, delay, chorus, flanger, distortion, and ADSR.
 */
class LayerEffectsProcessor
{
public:
    //==============================================================================
    enum class LayerType
    {
        Melody = 0,
        Chords = 1,
        Bass = 2,
        NumLayers = 3
    };
    
    //==============================================================================
    /** Effect parameters structure for each layer */
    struct EffectParameters
    {
        // Filter
        float filterCutoff = 1000.0f;     // 20Hz to 20kHz
        float filterResonance = 0.5f;     // 0.0 to 1.0
        
        // Reverb
        float reverbRoomSize = 0.3f;      // 0.0 to 1.0
        float reverbDamping = 0.5f;       // 0.0 to 1.0
        float reverbWetLevel = 0.2f;      // 0.0 to 1.0
        float reverbDryLevel = 0.8f;      // 0.0 to 1.0
        
        // Delay
        float delayTime = 0.25f;          // 0.0 to 2.0 seconds
        float delayFeedback = 0.3f;       // 0.0 to 0.95
        float delayWetLevel = 0.2f;       // 0.0 to 1.0
        
        // Chorus
        float chorusRate = 1.0f;          // 0.1 to 10.0 Hz
        float chorusDepth = 0.3f;         // 0.0 to 1.0
        float chorusCentreDelay = 7.0f;   // 1.0 to 100.0 ms
        float chorusFeedback = 0.1f;      // 0.0 to 0.95
        float chorusMix = 0.3f;           // 0.0 to 1.0
        
        // Flanger
        float flangerRate = 0.5f;         // 0.1 to 10.0 Hz
        float flangerDepth = 0.8f;        // 0.0 to 1.0
        float flangerCentreDelay = 5.0f;  // 1.0 to 20.0 ms
        float flangerFeedback = 0.3f;     // 0.0 to 0.95
        float flangerMix = 0.2f;          // 0.0 to 1.0
        
        // Distortion
        float distortionDrive = 5.0f;     // 1.0 to 50.0
        float distortionMix = 0.0f;       // 0.0 to 1.0
        
        // ADSR Envelope
        float adsrAttack = 0.01f;         // 0.001 to 5.0 seconds
        float adsrDecay = 0.3f;           // 0.001 to 5.0 seconds
        float adsrSustain = 0.7f;         // 0.0 to 1.0
        float adsrRelease = 0.5f;         // 0.001 to 10.0 seconds
        
        // Global
        float layerVolume = 0.8f;         // 0.0 to 1.0
        float layerPan = 0.0f;            // -1.0 to 1.0
        bool layerMuted = false;
        bool layerSoloed = false;
    };
    
    //==============================================================================
    LayerEffectsProcessor();
    ~LayerEffectsProcessor();
    
    //==============================================================================
    /** Prepare all effects for playback */
    void prepareToPlay(double sampleRate, int samplesPerBlock, int numChannels);
    
    /** Release all effects resources */
    void releaseResources();
    
    /** Process audio for a specific layer */
    void processLayer(LayerType layer, juce::AudioBuffer<float>& buffer, 
                     int startSample = 0, int numSamples = -1);
    
    /** Process all layers with proper routing */
    void processAllLayers(juce::AudioBuffer<float>& melodyBuffer,
                         juce::AudioBuffer<float>& chordsBuffer,
                         juce::AudioBuffer<float>& bassBuffer);
    
    //==============================================================================
    /** Get/Set parameters for a specific layer */
    EffectParameters& getLayerParameters(LayerType layer);
    const EffectParameters& getLayerParameters(LayerType layer) const;
    void setLayerParameters(LayerType layer, const EffectParameters& params);
    
    /** Update a specific parameter for real-time control */
    void setFilterCutoff(LayerType layer, float value);
    void setReverbMix(LayerType layer, float value);
    void setDelayFeedback(LayerType layer, float value);
    void setChorusMix(LayerType layer, float value);
    void setDistortionMix(LayerType layer, float value);
    void setLayerVolume(LayerType layer, float value);
    void setLayerPan(LayerType layer, float value);
    
    //==============================================================================
    /** Global pitch shift control (from ExperimentPad Y-axis) */
    void setGlobalPitchShift(float semitones); // -12.0 to +12.0
    
    /** Effects morphing control (from ExperimentPad X-axis) */
    void setEffectsMorph(float morphValue); // 0.0 to 1.0
    
    //==============================================================================
    /** Layer muting and soloing */
    void setLayerMuted(LayerType layer, bool muted);
    void setLayerSoloed(LayerType layer, bool soloed);
    bool isLayerMuted(LayerType layer) const;
    bool isLayerSoloed(LayerType layer) const;
    bool isAnyLayerSoloed() const;
    
    //==============================================================================
    /** Get current CPU usage for performance monitoring */
    float getCurrentCPUUsage() const { return currentCPUUsage.load(); }

private:
    //==============================================================================
    /** Individual layer effects chain */
    struct LayerEffectsChain
    {
        // DSP Processing Chain
        juce::dsp::ProcessorChain<
            juce::dsp::IIR::Filter<float>,           // High-pass filter
            juce::dsp::IIR::Filter<float>,           // Low-pass filter
            juce::dsp::Reverb,                       // Reverb
            juce::dsp::DelayLine<float>,             // Delay
            juce::dsp::Chorus<float>,                // Chorus
            juce::dsp::Phaser<float>,                // Flanger (using Phaser)
            juce::dsp::WaveShaper<float>             // Distortion
        > processorChain;
        
        // ADSR Envelope
        juce::ADSR adsr;
        juce::ADSR::Parameters adsrParams;
        
        // Pitch shifting (using ring buffer and interpolation)
        juce::dsp::DelayLine<float> pitchShiftDelay;
        float currentPitchRatio = 1.0f;
        
        // Pan and Volume
        juce::dsp::Panner<float> panner;
        juce::dsp::Gain<float> volumeGain;
        
        // Parameters
        EffectParameters parameters;
        
        // State
        bool prepared = false;
    };
    
    //==============================================================================
    /** Array of effects chains for each layer */
    std::array<LayerEffectsChain, static_cast<int>(LayerType::NumLayers)> layerChains;
    
    /** Audio spec for DSP preparation */
    juce::dsp::ProcessSpec spec;
    
    /** Global controls */
    std::atomic<float> globalPitchShift{0.0f};  // -12.0 to +12.0 semitones
    std::atomic<float> effectsMorphValue{0.0f}; // 0.0 to 1.0
    
    /** Performance monitoring */
    std::atomic<float> currentCPUUsage{0.0f};
    juce::Time lastCPUMeasurement;
    
    //==============================================================================
    /** Helper methods */
    void updateEffectsFromMorph(LayerType layer, float morphValue);
    void updateGlobalPitchShift();
    float semitonesToRatio(float semitones);
    void setupEffectsChain(LayerEffectsChain& chain);
    void updateCPUUsage();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LayerEffectsProcessor)
};
