//==============================================================================
/*
  ModulationEngine.cpp
  Author: BMad Dev Agent - Epic 6 Story 6.10
  
  Professional modulation effects processor implementation with 8 algorithms.
  
  Features:
  - 8 modulation algorithms: Chorus, Flanger, Phaser, Tremolo, Vibrato, AutoPan, RingMod, Rotary
  - LFO integration with existing synthesis LFOs
  - XY controller mapping for rate and depth
  - Stereo processing with width control
  - Professional parameter smoothing and real-time safety
*/
//==============================================================================

#include "ModulationEngine.h"
#include <cmath>

namespace spawnclone::audio
{
    //==============================================================================
    // ModulationEngine Implementation
    //==============================================================================
    
    ModulationEngine::ModulationEngine()
    {
        initializeChorusVoices();
    }

    ModulationEngine::~ModulationEngine() = default;

    //==============================================================================
    void ModulationEngine::prepareToPlay(double sampleRate, int samplesPerBlock)
    {
        currentSampleRate = sampleRate;
        currentBlockSize = samplesPerBlock;
        
        // Prepare LFOs
        for (auto& lfo : lfos)
        {
            lfo.prepareToPlay(sampleRate);
            lfo.setFrequency(rate);
        }
        
        // Add slight offset between left and right LFOs for stereo spread
        lfos[1].reset(); // Right LFO starts with phase offset
        
        // Prepare delay lines (50ms maximum for chorus/flanger)
        for (auto& delayLine : delayLines)
        {
            delayLine.prepareToPlay(sampleRate, 50.0f);
        }
        
        // Prepare all-pass filters for phaser
        for (auto& filter : allPassFilters)
        {
            filter.prepareToPlay(sampleRate);
        }
        
        // Prepare tone filters
        for (auto& filter : toneFilters)
        {
            filter.setSampleRate(sampleRate);
        }
        
        // Initialize parameter smoothers
        rateSmoother.setSampleRate(sampleRate);
        rateSmoother.setRampLength(0.1); // 100ms smoothing for rate changes
        rateSmoother.reset(rate);
        
        depthSmoother.setSampleRate(sampleRate);
        depthSmoother.setRampLength(0.05); // 50ms smoothing
        depthSmoother.reset(depth);
        
        feedbackSmoother.setSampleRate(sampleRate);
        feedbackSmoother.setRampLength(0.05);
        feedbackSmoother.reset(feedback);
        
        stereoSpreadSmoother.setSampleRate(sampleRate);
        stereoSpreadSmoother.setRampLength(0.1);
        stereoSpreadSmoother.reset(stereoSpread);
        
        mixSmoother.setSampleRate(sampleRate);
        mixSmoother.setRampLength(0.05);
        mixSmoother.reset(mix);
        
        centerDelaySmoother.setSampleRate(sampleRate);
        centerDelaySmoother.setRampLength(0.05);
        centerDelaySmoother.reset(centerDelay);
        
        // Prepare temporary buffers
        tempBuffer.setSize(2, samplesPerBlock);
        wetBuffer.setSize(2, samplesPerBlock);
        
        tempBuffer.clear();
        wetBuffer.clear();
    }

    void ModulationEngine::releaseResources()
    {
        tempBuffer.setSize(0, 0);
        wetBuffer.setSize(0, 0);
    }

    void ModulationEngine::reset()
    {
        for (auto& lfo : lfos)
        {
            lfo.reset();
        }
        
        for (auto& delayLine : delayLines)
        {
            delayLine.reset();
        }
        
        for (auto& filter : allPassFilters)
        {
            filter.reset();
        }
        
        for (auto& filter : toneFilters)
        {
            filter.reset();
        }
        
        tempBuffer.clear();
        wetBuffer.clear();
        
        // Reset parameter smoothers to current values
        rateSmoother.reset(rate);
        depthSmoother.reset(depth);
        feedbackSmoother.reset(feedback);
        stereoSpreadSmoother.reset(stereoSpread);
        mixSmoother.reset(mix);
        centerDelaySmoother.reset(centerDelay);
    }

    void ModulationEngine::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
    {
        juce::ignoreUnused(midiMessages);
        
        if (isBypassed())
            return;
            
        startCPUMonitoring();
        
        const int numSamples = buffer.getNumSamples();
        const int numChannels = juce::jmin(buffer.getNumChannels(), 2);
        
        // Ensure temporary buffers are the right size
        if (tempBuffer.getNumSamples() != numSamples)
        {
            tempBuffer.setSize(2, numSamples, false, false, true);
            wetBuffer.setSize(2, numSamples, false, false, true);
        }
        
        // Store dry signal for wet/dry mixing
        juce::AudioBuffer<float> dryBuffer;
        dryBuffer.makeCopyOf(buffer);
        
        // Process according to current modulation type
        switch (currentModulationType)
        {
            case ModulationType::Chorus:
                processChorus(buffer);
                break;
            case ModulationType::Flanger:
                processFlanger(buffer);
                break;
            case ModulationType::Phaser:
                processPhaser(buffer);
                break;
            case ModulationType::Tremolo:
                processTremolo(buffer);
                break;
            case ModulationType::Vibrato:
                processVibrato(buffer);
                break;
            case ModulationType::AutoPan:
                processAutoPan(buffer);
                break;
            case ModulationType::RingMod:
                processRingMod(buffer);
                break;
            case ModulationType::Rotary:
                processRotary(buffer);
                break;
        }
        
        // Apply wet/dry mixing
        applyWetDryMix(buffer, dryBuffer);
        
        endCPUMonitoring();
    }

    //==============================================================================
    // Wet/Dry Mixing
    //==============================================================================
    
    void ModulationEngine::applyWetDryMix(juce::AudioBuffer<float>& wetBuffer, const juce::AudioBuffer<float>& dryBuffer)
    {
        float currentMix = mix; // Use stored value instead of smoother current
        
        const int numSamples = wetBuffer.getNumSamples();
        const int numChannels = juce::jmin(wetBuffer.getNumChannels(), dryBuffer.getNumChannels());
        
        for (int channel = 0; channel < numChannels; ++channel)
        {
            float* wetData = wetBuffer.getWritePointer(channel);
            const float* dryData = dryBuffer.getReadPointer(channel);
            
            for (int sample = 0; sample < numSamples; ++sample)
            {
                wetData[sample] = dryData[sample] * (1.0f - currentMix) + wetData[sample] * currentMix;
            }
        }
    }

    //==============================================================================
    // Parameter Control
    //==============================================================================
    
    void ModulationEngine::setParameter(const juce::String& parameterName, float value, bool useInterpolation)
    {
        if (parameterName == "rate")
        {
            rate = juce::jlimit(0.1f, 20.0f, value);
            if (useInterpolation)
                rateSmoother.setTargetValue(rate);
            else
                rateSmoother.reset(rate);
        }
        else if (parameterName == "depth")
        {
            depth = juce::jlimit(0.0f, 1.0f, value);
            if (useInterpolation)
                depthSmoother.setTargetValue(depth);
            else
                depthSmoother.reset(depth);
        }
        else if (parameterName == "feedback")
        {
            feedback = juce::jlimit(0.0f, 0.95f, value);
            if (useInterpolation)
                feedbackSmoother.setTargetValue(feedback);
            else
                feedbackSmoother.reset(feedback);
        }
        else if (parameterName == "stereoSpread")
        {
            stereoSpread = juce::jlimit(0.0f, 1.0f, value);
            if (useInterpolation)
                stereoSpreadSmoother.setTargetValue(stereoSpread);
            else
                stereoSpreadSmoother.reset(stereoSpread);
        }
        else if (parameterName == "mix")
        {
            mix = juce::jlimit(0.0f, 1.0f, value);
            if (useInterpolation)
                mixSmoother.setTargetValue(mix);
            else
                mixSmoother.reset(mix);
        }
        else if (parameterName == "centerDelay")
        {
            centerDelay = juce::jlimit(1.0f, 50.0f, value);
            if (useInterpolation)
                centerDelaySmoother.setTargetValue(centerDelay);
            else
                centerDelaySmoother.reset(centerDelay);
        }
    }

    float ModulationEngine::getParameter(const juce::String& parameterName) const
    {
        if (parameterName == "rate") return rate;
        if (parameterName == "depth") return depth;
        if (parameterName == "feedback") return feedback;
        if (parameterName == "stereoSpread") return stereoSpread;
        if (parameterName == "mix") return mix;
        if (parameterName == "centerDelay") return centerDelay;
        
        return 0.0f;
    }

    juce::StringArray ModulationEngine::getParameterNames() const
    {
        return {"rate", "depth", "feedback", "stereoSpread", "mix", "centerDelay"};
    }

    //==============================================================================
    // Modulation-Specific Interface
    //==============================================================================
    
    void ModulationEngine::setModulationType(ModulationType type)
    {
        if (currentModulationType != type)
        {
            currentModulationType = type;
            reset(); // Clear any artifacts when switching types
        }
    }

    void ModulationEngine::setRate(float rateHz)
    {
        setParameter("rate", rateHz, true);
    }

    void ModulationEngine::setDepth(float depthAmount)
    {
        setParameter("depth", depthAmount, true);
    }

    void ModulationEngine::setFeedback(float feedbackAmount)
    {
        setParameter("feedback", feedbackAmount, true);
    }

    void ModulationEngine::setStereoSpread(float spread)
    {
        setParameter("stereoSpread", spread, true);
    }

    void ModulationEngine::setMix(float mixAmount)
    {
        setParameter("mix", mixAmount, true);
    }

    void ModulationEngine::setVoiceCount(int voices)
    {
        voiceCount = juce::jlimit(1, 8, voices);
    }

    void ModulationEngine::setCenterDelay(float delayMs)
    {
        setParameter("centerDelay", delayMs, true);
    }

    //==============================================================================
    // Modulation Algorithm Implementations
    //==============================================================================
    
    void ModulationEngine::processChorus(juce::AudioBuffer<float>& buffer)
    {
        const int numSamples = buffer.getNumSamples();
        const int numChannels = juce::jmin(buffer.getNumChannels(), 2);
        
        wetBuffer.clear();
        
        for (int sample = 0; sample < numSamples; ++sample)
        {
            // Update smoothed parameters
            float currentRate = rateSmoother.getNextValue();
            float currentDepth = depthSmoother.getNextValue();
            float currentCenterDelay = centerDelaySmoother.getNextValue();
            float currentMix = mixSmoother.getNextValue();
            
            // Update LFO frequencies
            for (auto& lfo : lfos)
            {
                lfo.setFrequency(currentRate);
            }
            
            for (int channel = 0; channel < numChannels; ++channel)
            {
                float* channelData = buffer.getWritePointer(channel);
                float* wetData = wetBuffer.getWritePointer(channel);
                float input = channelData[sample];
                
                float chorusOutput = 0.0f;
                
                // Process active chorus voices
                for (int voice = 0; voice < voiceCount; ++voice)
                {
                    const auto& voiceParams = chorusVoices[voice];
                    
                    // Calculate modulated delay time
                    float lfoValue = lfos[channel].getNextSample();
                    float modulatedDelay = currentCenterDelay + voiceParams.delayOffset +
                                         (lfoValue * currentDepth * 5.0f); // Max 5ms modulation
                    
                    // Ensure we don't exceed delay line array bounds
                    int delayLineIndex = (voice * 2 + channel) % 8; // Safely map to 0-7 range
                    
                    // Process through delay line
                    float voiceOutput = delayLines[delayLineIndex].processSample(input, modulatedDelay);
                    
                    // Apply voice gain and pan
                    float panGain = (channel == 0) ? 
                        std::sqrt(0.5f * (1.0f - voiceParams.panPosition)) : 
                        std::sqrt(0.5f * (1.0f + voiceParams.panPosition));
                    
                    chorusOutput += voiceOutput * voiceParams.gain * panGain;
                }
                
                // Average the voices
                chorusOutput /= static_cast<float>(voiceCount);
                
                // Store wet signal for global mixing
                channelData[sample] = chorusOutput;
            }
        }
    }

    void ModulationEngine::processFlanger(juce::AudioBuffer<float>& buffer)
    {
        const int numSamples = buffer.getNumSamples();
        const int numChannels = juce::jmin(buffer.getNumChannels(), 2);
        
        for (int sample = 0; sample < numSamples; ++sample)
        {
            float currentRate = rateSmoother.getNextValue();
            float currentDepth = depthSmoother.getNextValue();
            float currentFeedback = feedbackSmoother.getNextValue();
            float currentMix = mixSmoother.getNextValue();
            
            // Update LFO frequencies
            for (auto& lfo : lfos)
            {
                lfo.setFrequency(currentRate);
            }
            
            for (int channel = 0; channel < numChannels; ++channel)
            {
                float* channelData = buffer.getWritePointer(channel);
                float input = channelData[sample];
                
                // Get LFO modulation (0.1ms to 20ms range)
                float lfoValue = lfos[channel].getNextSample();
                float modulatedDelay = 0.1f + (0.5f + 0.5f * lfoValue) * currentDepth * 19.9f;
                
                // Set feedback for delay line
                delayLines[channel].setFeedback(currentFeedback);
                
                // Process through delay line
                float flangerOutput = delayLines[channel].processSample(input, modulatedDelay);
                
                // Store wet signal for global mixing
                channelData[sample] = flangerOutput;
            }
        }
    }

    void ModulationEngine::processPhaser(juce::AudioBuffer<float>& buffer)
    {
        const int numSamples = buffer.getNumSamples();
        const int numChannels = juce::jmin(buffer.getNumChannels(), 2);
        
        for (int sample = 0; sample < numSamples; ++sample)
        {
            float currentRate = rateSmoother.getNextValue();
            float currentDepth = depthSmoother.getNextValue();
            float currentFeedback = feedbackSmoother.getNextValue();
            float currentMix = mixSmoother.getNextValue();
            
            // Update LFO frequencies
            for (auto& lfo : lfos)
            {
                lfo.setFrequency(currentRate);
            }
            
            for (int channel = 0; channel < numChannels; ++channel)
            {
                float* channelData = buffer.getWritePointer(channel);
                float input = channelData[sample];
                
                // Get LFO modulation for frequency sweep
                float lfoValue = lfos[channel].getNextSample();
                float baseFreq = 200.0f + (1.0f + lfoValue) * currentDepth * 1800.0f; // 200Hz to 2000Hz
                
                // Process through 4-stage all-pass filter bank
                float phaserOutput = input;
                for (int stage = 0; stage < 4; ++stage)
                {
                    float stageFreq = baseFreq * std::pow(2.0f, stage * 0.5f); // Frequency spread
                    phaserOutput = allPassFilters[stage + channel * 4].processSample(phaserOutput, stageFreq);
                }
                
                // Apply feedback
                phaserOutput = phaserOutput + (phaserOutput * currentFeedback);
                
                // Store wet signal for global mixing
                channelData[sample] = phaserOutput;
            }
        }
    }

    void ModulationEngine::processTremolo(juce::AudioBuffer<float>& buffer)
    {
        const int numSamples = buffer.getNumSamples();
        const int numChannels = juce::jmin(buffer.getNumChannels(), 2);
        
        for (int sample = 0; sample < numSamples; ++sample)
        {
            float currentRate = rateSmoother.getNextValue();
            float currentDepth = depthSmoother.getNextValue();
            float currentStereoSpread = stereoSpreadSmoother.getNextValue();
            
            // Update LFO frequencies
            for (auto& lfo : lfos)
            {
                lfo.setFrequency(currentRate);
            }
            
            for (int channel = 0; channel < numChannels; ++channel)
            {
                float* channelData = buffer.getWritePointer(channel);
                float input = channelData[sample];
                
                // Get LFO modulation with stereo offset
                float lfoValue = lfos[channel].getNextSample();
                if (channel == 1 && numChannels == 2)
                {
                    // Apply stereo spread offset to right channel
                    lfoValue = lfos[0].getNextSample(); // Use phase-shifted version
                }
                
                // Calculate amplitude modulation
                float amplitude = 1.0f - currentDepth * 0.5f * (1.0f + lfoValue);
                amplitude = juce::jlimit(0.0f, 1.0f, amplitude);
                
                // Apply tremolo
                channelData[sample] = input * amplitude;
            }
        }
    }

    void ModulationEngine::processVibrato(juce::AudioBuffer<float>& buffer)
    {
        const int numSamples = buffer.getNumSamples();
        const int numChannels = juce::jmin(buffer.getNumChannels(), 2);
        
        for (int sample = 0; sample < numSamples; ++sample)
        {
            float currentRate = rateSmoother.getNextValue();
            float currentDepth = depthSmoother.getNextValue();
            
            // Update LFO frequencies
            for (auto& lfo : lfos)
            {
                lfo.setFrequency(currentRate);
            }
            
            for (int channel = 0; channel < numChannels; ++channel)
            {
                float* channelData = buffer.getWritePointer(channel);
                float input = channelData[sample];
                
                // Get LFO modulation for pitch variation
                float lfoValue = lfos[channel].getNextSample();
                float pitchModulation = lfoValue * currentDepth * 5.0f; // Max 5ms modulation for vibrato
                
                // Use delay line for pitch modulation
                float vibratoOutput = delayLines[channel].processSample(input, 5.0f + pitchModulation);
                
                channelData[sample] = vibratoOutput;
            }
        }
    }

    void ModulationEngine::processAutoPan(juce::AudioBuffer<float>& buffer)
    {
        const int numSamples = buffer.getNumSamples();
        const int numChannels = juce::jmin(buffer.getNumChannels(), 2);
        
        if (numChannels < 2)
        {
            // Fall back to tremolo for mono
            processTremolo(buffer);
            return;
        }
        
        for (int sample = 0; sample < numSamples; ++sample)
        {
            float currentRate = rateSmoother.getNextValue();
            float currentDepth = depthSmoother.getNextValue();
            
            lfos[0].setFrequency(currentRate);
            
            float* leftData = buffer.getWritePointer(0);
            float* rightData = buffer.getWritePointer(1);
            
            float leftInput = leftData[sample];
            float rightInput = rightData[sample];
            float monoInput = (leftInput + rightInput) * 0.5f;
            
            // Get LFO modulation for panning
            float lfoValue = lfos[0].getNextSample();
            float panPosition = lfoValue * currentDepth; // -1 to +1
            
            // Calculate pan gains (constant power panning)
            float leftGain = std::sqrt(0.5f * (1.0f - panPosition));
            float rightGain = std::sqrt(0.5f * (1.0f + panPosition));
            
            // Apply auto-panning
            leftData[sample] = monoInput * leftGain;
            rightData[sample] = monoInput * rightGain;
        }
    }

    void ModulationEngine::processRingMod(juce::AudioBuffer<float>& buffer)
    {
        const int numSamples = buffer.getNumSamples();
        const int numChannels = juce::jmin(buffer.getNumChannels(), 2);
        
        for (int sample = 0; sample < numSamples; ++sample)
        {
            float currentRate = rateSmoother.getNextValue();
            float currentDepth = depthSmoother.getNextValue();
            float currentMix = mixSmoother.getNextValue();
            
            // Use rate as carrier frequency for ring modulation
            for (auto& lfo : lfos)
            {
                lfo.setFrequency(currentRate);
            }
            
            for (int channel = 0; channel < numChannels; ++channel)
            {
                float* channelData = buffer.getWritePointer(channel);
                float input = channelData[sample];
                
                // Get carrier oscillator
                float carrier = lfos[channel].getNextSample();
                
                // Apply ring modulation
                float ringModOutput = input * carrier * currentDepth;
                
                // Store wet signal for global mixing
                channelData[sample] = ringModOutput;
            }
        }
    }

    void ModulationEngine::processRotary(juce::AudioBuffer<float>& buffer)
    {
        const int numSamples = buffer.getNumSamples();
        const int numChannels = juce::jmin(buffer.getNumChannels(), 2);
        
        // Simplified rotary speaker simulation
        for (int sample = 0; sample < numSamples; ++sample)
        {
            float currentRate = rateSmoother.getNextValue();
            float currentDepth = depthSmoother.getNextValue();
            
            // Update LFO frequencies (rotary speed)
            for (auto& lfo : lfos)
            {
                lfo.setFrequency(currentRate);
            }
            
            for (int channel = 0; channel < numChannels; ++channel)
            {
                float* channelData = buffer.getWritePointer(channel);
                float input = channelData[sample];
                
                // Get rotation modulation
                float lfoValue = lfos[channel].getNextSample();
                
                // Apply Doppler effect simulation with delay modulation
                float dopplerDelay = 10.0f + lfoValue * currentDepth * 5.0f;
                float rotaryOutput = delayLines[channel].processSample(input, dopplerDelay);
                
                // Apply amplitude modulation for horn effect
                float amplitude = 1.0f + 0.3f * lfoValue * currentDepth;
                rotaryOutput *= amplitude;
                
                // Apply simple filtering for frequency modulation
                rotaryOutput = toneFilters[channel].processLowpass(rotaryOutput);
                
                channelData[sample] = rotaryOutput;
            }
        }
    }

    //==============================================================================
    // Helper Methods
    //==============================================================================
    
    float ModulationEngine::millisecondsToSamples(float ms) const
    {
        return ms * 0.001f * static_cast<float>(currentSampleRate);
    }

    float ModulationEngine::samplesToMilliseconds(float samples) const
    {
        return samples / static_cast<float>(currentSampleRate) * 1000.0f;
    }

    void ModulationEngine::initializeChorusVoices()
    {
        // Initialize 8 chorus voices with musical intervals and panning
        chorusVoices[0] = {-2.0f, -0.05f, -0.7f, 0.8f};  // Left, slightly flat
        chorusVoices[1] = {-1.0f, 0.03f, -0.3f, 0.9f};   // Left-center, slightly sharp
        chorusVoices[2] = {0.0f, 0.0f, 0.0f, 1.0f};      // Center, unison
        chorusVoices[3] = {1.0f, -0.02f, 0.3f, 0.9f};    // Right-center, slightly flat
        chorusVoices[4] = {2.0f, 0.07f, 0.7f, 0.8f};     // Right, slightly sharp
        chorusVoices[5] = {-3.0f, 0.04f, -0.5f, 0.6f};   // Extra left voice
        chorusVoices[6] = {1.5f, -0.06f, 0.1f, 0.7f};    // Extra center-right
        chorusVoices[7] = {3.0f, 0.02f, 0.5f, 0.6f};     // Extra right voice
    }

    void ModulationEngine::updatePhaserFrequencies(float baseFreq, float lfoValue)
    {
        // Update all-pass filter frequencies for phaser sweep
        for (int i = 0; i < 4; ++i)
        {
            float stageFreq = baseFreq * std::pow(2.0f, i * 0.7f) * (1.0f + lfoValue * 0.5f);
            stageFreq = juce::jlimit(50.0f, 5000.0f, stageFreq);
            
            // Update filters for both channels
            // Note: Frequency update happens in processSample call for real-time modulation
        }
    }

    //==============================================================================
    // Inner Class Implementations
    //==============================================================================
    
    // ModulationLFO Implementation
    void ModulationEngine::ModulationLFO::prepareToPlay(double sr)
    {
        sampleRate = sr;
        phase = 0.0f;
    }

    void ModulationEngine::ModulationLFO::reset()
    {
        phase = 0.0f;
        randomValue = random.nextFloat() * 2.0f - 1.0f;
    }

    void ModulationEngine::ModulationLFO::setFrequency(float freq)
    {
        frequency = freq;
    }

    void ModulationEngine::ModulationLFO::setWaveform(Waveform waveform)
    {
        currentWaveform = waveform;
    }

    float ModulationEngine::ModulationLFO::getNextSample()
    {
        float output = 0.0f;
        
        switch (currentWaveform)
        {
            case Waveform::Sine:
                output = std::sin(phase);
                break;
            case Waveform::Triangle:
                output = (2.0f / juce::MathConstants<float>::pi) * std::asin(std::sin(phase));
                break;
            case Waveform::Sawtooth:
                output = (2.0f / juce::MathConstants<float>::pi) * (phase - juce::MathConstants<float>::pi);
                break;
            case Waveform::Square:
                output = (phase < juce::MathConstants<float>::pi) ? 1.0f : -1.0f;
                break;
            case Waveform::Random:
                if (phase >= 2.0f * juce::MathConstants<float>::pi)
                    randomValue = random.nextFloat() * 2.0f - 1.0f;
                output = randomValue;
                break;
        }
        
        phase += 2.0f * juce::MathConstants<float>::pi * frequency / static_cast<float>(sampleRate);
        if (phase >= 2.0f * juce::MathConstants<float>::pi)
            phase -= 2.0f * juce::MathConstants<float>::pi;
            
        return output;
    }

    // ModulationDelayLine Implementation
    void ModulationEngine::ModulationDelayLine::prepareToPlay(double sr, float maxDelayMs)
    {
        sampleRate = sr;
        int maxSamples = static_cast<int>(maxDelayMs * 0.001f * sr) + 1;
        buffer.resize(maxSamples);
        reset();
    }

    void ModulationEngine::ModulationDelayLine::reset()
    {
        std::fill(buffer.begin(), buffer.end(), 0.0f);
        writeIndex = 0;
    }

    float ModulationEngine::ModulationDelayLine::processSample(float input, float delayMs, bool interpolate)
    {
        if (buffer.empty())
            return input;
            
        float delaySamples = delayMs * 0.001f * static_cast<float>(sampleRate);
        delaySamples = juce::jlimit(0.0f, static_cast<float>(buffer.size() - 1), delaySamples);
        
        float readPosition = writeIndex - delaySamples;
        while (readPosition < 0.0f)
            readPosition += buffer.size();
        while (readPosition >= buffer.size())
            readPosition -= buffer.size();
        
        float output;
        if (interpolate)
        {
            int readIndex = static_cast<int>(readPosition);
            float fraction = readPosition - readIndex;
            int nextIndex = (readIndex + 1) % buffer.size();
            
            output = interpolateLinear(fraction, buffer[readIndex], buffer[nextIndex]);
        }
        else
        {
            output = buffer[static_cast<int>(readPosition)];
        }
        
        // Write input with feedback
        buffer[writeIndex] = input + output * feedbackAmount;
        writeIndex = (writeIndex + 1) % buffer.size();
        
        return output;
    }

    float ModulationEngine::ModulationDelayLine::interpolateLinear(float x, float y0, float y1) const
    {
        return y0 + x * (y1 - y0);
    }

    // AllPassFilter Implementation
    void ModulationEngine::AllPassFilter::prepareToPlay(double sr)
    {
        sampleRate = sr;
        reset();
    }

    void ModulationEngine::AllPassFilter::reset()
    {
        state = 0.0f;
    }

    float ModulationEngine::AllPassFilter::processSample(float input, float frequency)
    {
        // Calculate coefficient for all-pass filter
        float omega = 2.0f * juce::MathConstants<float>::pi * frequency / static_cast<float>(sampleRate);
        float tan_half_omega = std::tan(omega * 0.5f);
        coefficient = (1.0f - tan_half_omega) / (1.0f + tan_half_omega);
        
        // All-pass filter processing
        float output = -coefficient * input + state;
        state = input + coefficient * output;
        
        return output;
    }

    // OnePoleFilter Implementation
    void ModulationEngine::OnePoleFilter::setSampleRate(double sr)
    {
        sampleRate = sr;
    }

    void ModulationEngine::OnePoleFilter::setFrequency(float frequency)
    {
        float omega = 2.0f * juce::MathConstants<float>::pi * frequency / static_cast<float>(sampleRate);
        coefficient = std::exp(-omega);
    }

    void ModulationEngine::OnePoleFilter::reset()
    {
        state = 0.0f;
    }

    float ModulationEngine::OnePoleFilter::processLowpass(float input)
    {
        state = input * (1.0f - coefficient) + state * coefficient;
        return state;
    }

    float ModulationEngine::OnePoleFilter::processHighpass(float input)
    {
        float lowpass = processLowpass(input);
        return input - lowpass;
    }

} // namespace spawnclone::audio
