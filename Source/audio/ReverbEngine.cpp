//==============================================================================
/*
  ReverbEngine.cpp
  Author: BMad Dev Agent - Epic 6 Story 6.2
  
  Professional algorithmic reverb implementation
*/
//==============================================================================

#include "ReverbEngine.h"

namespace spawnclone::audio
{
    ReverbEngine::ReverbEngine()
    {
        // Initialize parameter smoothers
        preDelaySmoother.setRampLength(0.05); // 50ms ramp
        reverbTimeSmoother.setRampLength(0.1); // 100ms ramp
        roomSizeSmoother.setRampLength(0.05);
        dampingSmoother.setRampLength(0.05);
        stereoWidthSmoother.setRampLength(0.05);
        earlyMixSmoother.setRampLength(0.05);
    }

    ReverbEngine::~ReverbEngine() = default;

    //==============================================================================
    void ReverbEngine::prepareToPlay(double sampleRate, int samplesPerBlock)
    {
        startCPUMonitoring();
        
        currentSampleRate = sampleRate;
        currentBlockSize = samplesPerBlock;
        
        // Prepare parameter smoothers
        preDelaySmoother.setSampleRate(sampleRate);
        reverbTimeSmoother.setSampleRate(sampleRate);
        roomSizeSmoother.setSampleRate(sampleRate);
        dampingSmoother.setSampleRate(sampleRate);
        stereoWidthSmoother.setSampleRate(sampleRate);
        earlyMixSmoother.setSampleRate(sampleRate);
        
        // Reset current values
        preDelaySmoother.reset(preDelayMs);
        reverbTimeSmoother.reset(reverbTimeSeconds);
        roomSizeSmoother.reset(roomSize);
        dampingSmoother.reset(damping);
        stereoWidthSmoother.reset(stereoWidth);
        earlyMixSmoother.reset(earlyReflectionsMix);
        
        // Prepare components
        earlyReflections.prepareToPlay(sampleRate, samplesPerBlock);
        convolutionProcessor.prepareToPlay(sampleRate, samplesPerBlock);
        
        // Prepare all-pass delays for plate reverb (Dattorro-style)
        for (auto& allpass : inputAllPasses)
        {
            allpass.prepareToPlay(sampleRate, static_cast<int>(sampleRate * 0.1)); // 100ms max
        }
        
        for (auto& allpass : outputAllPasses)
        {
            allpass.prepareToPlay(sampleRate, static_cast<int>(sampleRate * 0.05)); // 50ms max
        }
        
        // Prepare comb filters
        for (auto& comb : combFilters)
        {
            comb.prepareToPlay(sampleRate, static_cast<int>(sampleRate * 0.2)); // 200ms max
        }
        
        // Prepare pre-delay
        int maxPreDelayMs = 100;
        preDelayLine.resize(static_cast<size_t>(sampleRate * maxPreDelayMs / 1000.0), 0.0f);
        preDelayWriteIndex = 0;
        
        // Prepare temporary buffers
        tempBuffer.setSize(2, samplesPerBlock);
        earlyBuffer.setSize(2, samplesPerBlock);
        lateBuffer.setSize(2, samplesPerBlock);
        
        endCPUMonitoring();
    }

    void ReverbEngine::releaseResources()
    {
        tempBuffer.setSize(0, 0);
        earlyBuffer.setSize(0, 0);
        lateBuffer.setSize(0, 0);
        preDelayLine.clear();
    }

    void ReverbEngine::reset()
    {
        earlyReflections.reset();
        convolutionProcessor.reset();
        
        for (auto& allpass : inputAllPasses)
            allpass.reset();
        for (auto& allpass : outputAllPasses)
            allpass.reset();
        for (auto& comb : combFilters)
            comb.reset();
        
        std::fill(preDelayLine.begin(), preDelayLine.end(), 0.0f);
        preDelayWriteIndex = 0;
    }

    //==============================================================================
    void ReverbEngine::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& /*midiMessages*/)
    {
        if (isBypassed())
            return;
            
        startCPUMonitoring();
        
        // Store dry signal for wet/dry mixing
        tempBuffer.makeCopyOf(buffer);
        
        // Process based on current algorithm
        switch (currentAlgorithm)
        {
            case Algorithm::Plate:
                processPlateReverb(buffer);
                break;
            case Algorithm::Hall:
                processHallReverb(buffer);
                break;
            case Algorithm::Room:
                processRoomReverb(buffer);
                break;
            case Algorithm::Spring:
                processSpringReverb(buffer);
                break;
            case Algorithm::Convolution:
                processConvolutionReverb(buffer);
                break;
            case Algorithm::Shimmer:
                processShimmerReverb(buffer);
                break;
            case Algorithm::Reverse:
                processReverseReverb(buffer);
                break;
            case Algorithm::Gated:
                processGatedReverb(buffer);
                break;
        }
        
        // Apply wet/dry mixing
        applyWetDryMix(buffer, tempBuffer);
        
        endCPUMonitoring();
    }

    //==============================================================================
    void ReverbEngine::setParameter(const juce::String& parameterName, float value, bool useInterpolation)
    {
        if (parameterName == "preDelay")
        {
            preDelayMs = juce::jlimit(0.0f, 100.0f, value);
            if (useInterpolation)
                preDelaySmoother.setTargetValue(preDelayMs);
        }
        else if (parameterName == "reverbTime")
        {
            reverbTimeSeconds = juce::jlimit(0.1f, 10.0f, value);
            if (useInterpolation)
                reverbTimeSmoother.setTargetValue(reverbTimeSeconds);
        }
        else if (parameterName == "roomSize")
        {
            roomSize = juce::jlimit(0.0f, 1.0f, value);
            if (useInterpolation)
                roomSizeSmoother.setTargetValue(roomSize);
            earlyReflections.setRoomSize(roomSize);
        }
        else if (parameterName == "damping")
        {
            damping = juce::jlimit(0.0f, 1.0f, value);
            if (useInterpolation)
                dampingSmoother.setTargetValue(damping);
        }
        else if (parameterName == "stereoWidth")
        {
            stereoWidth = juce::jlimit(0.0f, 2.0f, value);
            if (useInterpolation)
                stereoWidthSmoother.setTargetValue(stereoWidth);
        }
        else if (parameterName == "earlyReflectionsMix")
        {
            earlyReflectionsMix = juce::jlimit(0.0f, 1.0f, value);
            if (useInterpolation)
                earlyMixSmoother.setTargetValue(earlyReflectionsMix);
            earlyReflections.setMix(earlyReflectionsMix);
        }
    }

    float ReverbEngine::getParameter(const juce::String& parameterName) const
    {
        if (parameterName == "preDelay") return preDelayMs;
        if (parameterName == "reverbTime") return reverbTimeSeconds;
        if (parameterName == "roomSize") return roomSize;
        if (parameterName == "damping") return damping;
        if (parameterName == "stereoWidth") return stereoWidth;
        if (parameterName == "earlyReflectionsMix") return earlyReflectionsMix;
        return 0.0f;
    }

    juce::StringArray ReverbEngine::getParameterNames() const
    {
        return {
            "preDelay",
            "reverbTime", 
            "roomSize",
            "damping",
            "stereoWidth",
            "earlyReflectionsMix"
        };
    }

    //==============================================================================
    void ReverbEngine::setAlgorithm(Algorithm algorithm)
    {
        if (currentAlgorithm != algorithm)
        {
            currentAlgorithm = algorithm;
            reset(); // Clear state when switching algorithms
        }
    }

    void ReverbEngine::setPreDelay(float preDelayMs)
    {
        setParameter("preDelay", preDelayMs, true);
    }

    void ReverbEngine::setReverbTime(float timeSeconds)
    {
        setParameter("reverbTime", timeSeconds, true);
    }

    void ReverbEngine::setRoomSize(float size)
    {
        setParameter("roomSize", size, true);
    }

    void ReverbEngine::setDamping(float dampingValue)
    {
        setParameter("damping", dampingValue, true);
    }

    void ReverbEngine::setStereoWidth(float width)
    {
        setParameter("stereoWidth", width, true);
    }

    void ReverbEngine::setEarlyReflectionsMix(float mix)
    {
        setParameter("earlyReflectionsMix", mix, true);
    }

    bool ReverbEngine::loadImpulseResponse(const juce::AudioBuffer<float>& impulseResponse)
    {
        return convolutionProcessor.loadImpulseResponse(impulseResponse);
    }

    //==============================================================================
    void ReverbEngine::processPlateReverb(juce::AudioBuffer<float>& buffer)
    {
        // Simplified plate reverb using Dattorro-inspired structure
        
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            // Get smoothed parameter values
            float currentPreDelay = preDelaySmoother.getNextValue();
            float currentReverbTime = reverbTimeSmoother.getNextValue();
            float currentRoomSize = roomSizeSmoother.getNextValue();
            float currentDamping = dampingSmoother.getNextValue();
            
            // Process each channel
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            {
                float input = buffer.getSample(channel, sample);
                
                // Apply pre-delay
                int preDelayLength = static_cast<int>(currentPreDelay * currentSampleRate / 1000.0f);
                preDelayLength = juce::jlimit(0, static_cast<int>(preDelayLine.size() - 1), preDelayLength);
                
                int readIndex = (preDelayWriteIndex - preDelayLength + static_cast<int>(preDelayLine.size())) % static_cast<int>(preDelayLine.size());
                float preDelayedInput = preDelayLine[readIndex];
                preDelayLine[preDelayWriteIndex] = input;
                
                // Simple plate reverb algorithm
                float output = preDelayedInput;
                
                // All-pass chain for diffusion
                int allPassDelay1 = static_cast<int>(142 * currentRoomSize + 20);
                int allPassDelay2 = static_cast<int>(107 * currentRoomSize + 15);
                output = inputAllPasses[channel * 2].processSample(output, allPassDelay1, 0.7f);
                output = inputAllPasses[channel * 2 + 1].processSample(output, allPassDelay2, 0.7f);
                
                // Comb filter tank
                float combSum = 0.0f;
                for (int i = 0; i < 4; ++i)
                {
                    int combDelay = static_cast<int>((1557 + i * 47) * currentRoomSize + 100);
                    float feedback = 0.84f * currentReverbTime / 4.0f; // Scale with reverb time
                    feedback = juce::jlimit(0.0f, 0.95f, feedback);
                    
                    float combOutput = combFilters[channel * 4 + i].processSample(output, combDelay, feedback, currentDamping);
                    combSum += combOutput;
                }
                
                output = combSum * 0.25f; // Average the comb outputs
                
                buffer.setSample(channel, sample, output);
            }
            
            preDelayWriteIndex = (preDelayWriteIndex + 1) % static_cast<int>(preDelayLine.size());
        }
    }

    void ReverbEngine::processHallReverb(juce::AudioBuffer<float>& buffer)
    {
        // Larger, more spacious parameters for hall
        processPlateReverb(buffer); // Use plate as base, with different parameters applied via roomSize
    }

    void ReverbEngine::processRoomReverb(juce::AudioBuffer<float>& buffer)
    {
        // Smaller, more intimate parameters for room
        processPlateReverb(buffer); // Use plate as base, with different parameters applied via roomSize
    }

    void ReverbEngine::processSpringReverb(juce::AudioBuffer<float>& buffer)
    {
        // Spring reverb emulation with more metallic character
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            {
                float input = buffer.getSample(channel, sample);
                
                // Spring tank emulation with shorter delays and higher feedback
                float output = input;
                
                // Multiple short all-pass delays for spring character
                output = inputAllPasses[channel].processSample(output, 23, 0.9f);
                output = inputAllPasses[channel].processSample(output, 47, -0.8f);
                output = inputAllPasses[channel].processSample(output, 71, 0.7f);
                
                // Add some metallic resonance
                output = combFilters[channel].processSample(output, 179, 0.6f, 0.1f); // Less damping for metallic sound
                
                buffer.setSample(channel, sample, output * 0.7f);
            }
        }
    }

    void ReverbEngine::processConvolutionReverb(juce::AudioBuffer<float>& buffer)
    {
        convolutionProcessor.processBlock(buffer);
    }

    void ReverbEngine::processShimmerReverb(juce::AudioBuffer<float>& buffer)
    {
        // Start with plate reverb
        processPlateReverb(buffer);
        
        // Add pitch-shifted feedback (simplified shimmer effect)
        // This would need a proper pitch shifter implementation
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            {
                float input = buffer.getSample(channel, sample);
                // Placeholder for pitch shifting - would implement octave up mixing
                buffer.setSample(channel, sample, input);
            }
        }
    }

    void ReverbEngine::processReverseReverb(juce::AudioBuffer<float>& buffer)
    {
        // Placeholder for reverse reverb algorithm
        processPlateReverb(buffer);
    }

    void ReverbEngine::processGatedReverb(juce::AudioBuffer<float>& buffer)
    {
        // Placeholder for gated reverb algorithm
        processPlateReverb(buffer);
    }

    //==============================================================================
    // EarlyReflections Implementation
    
    void ReverbEngine::EarlyReflections::prepareToPlay(double sampleRate, int /*samplesPerBlock*/)
    {
        this->sampleRate = sampleRate;
        
        // Initialize delay taps with typical early reflection patterns
        float delayTimes[] = { 7.0f, 11.0f, 13.0f, 17.0f, 19.0f, 23.0f, 29.0f, 31.0f }; // ms
        float gains[] = { 0.8f, 0.7f, 0.6f, 0.5f, 0.4f, 0.35f, 0.3f, 0.25f };
        
        for (int i = 0; i < NUM_TAPS; ++i)
        {
            int delayLength = static_cast<int>(delayTimes[i] * sampleRate / 1000.0);
            taps[i].delayLine.resize(delayLength, 0.0f);
            taps[i].delayLength = delayLength;
            taps[i].writeIndex = 0;
            taps[i].gain = gains[i];
        }
    }

    void ReverbEngine::EarlyReflections::reset()
    {
        for (auto& tap : taps)
        {
            std::fill(tap.delayLine.begin(), tap.delayLine.end(), 0.0f);
            tap.writeIndex = 0;
        }
    }

    void ReverbEngine::EarlyReflections::processBlock(juce::AudioBuffer<float>& buffer)
    {
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            {
                float input = buffer.getSample(channel, sample);
                float output = 0.0f;
                
                // Process each delay tap
                for (auto& tap : taps)
                {
                    // Write input to delay line
                    tap.delayLine[tap.writeIndex] = input;
                    
                    // Read delayed sample
                    output += tap.delayLine[tap.writeIndex] * tap.gain;
                    
                    // Advance write index
                    tap.writeIndex = (tap.writeIndex + 1) % tap.delayLength;
                }
                
                buffer.setSample(channel, sample, output * mix);
            }
        }
    }

    void ReverbEngine::EarlyReflections::setRoomSize(float size)
    {
        // Scale delay times based on room size
        // This is a simplified implementation
        juce::ignoreUnused(size);
    }

    void ReverbEngine::EarlyReflections::setMix(float mixAmount)
    {
        mix = juce::jlimit(0.0f, 1.0f, mixAmount);
    }

    //==============================================================================
    // AllPassDelay Implementation
    
    void ReverbEngine::AllPassDelay::prepareToPlay(double /*sampleRate*/, int maxDelaySamples)
    {
        delayLine.resize(maxDelaySamples, 0.0f);
        writeIndex = 0;
    }

    void ReverbEngine::AllPassDelay::reset()
    {
        std::fill(delayLine.begin(), delayLine.end(), 0.0f);
        writeIndex = 0;
    }

    float ReverbEngine::AllPassDelay::processSample(float input, int delaySamples, float feedback, float gain)
    {
        delaySamples = juce::jlimit(1, static_cast<int>(delayLine.size()), delaySamples);
        
        int readIndex = (writeIndex - delaySamples + static_cast<int>(delayLine.size())) % static_cast<int>(delayLine.size());
        float delayedSample = delayLine[readIndex];
        
        float output = -input * feedback + delayedSample * gain;
        delayLine[writeIndex] = input + delayedSample * feedback;
        
        writeIndex = (writeIndex + 1) % static_cast<int>(delayLine.size());
        
        return output;
    }

    //==============================================================================
    // CombFilter Implementation
    
    void ReverbEngine::CombFilter::prepareToPlay(double /*sampleRate*/, int maxDelaySamples)
    {
        delayLine.resize(maxDelaySamples, 0.0f);
        writeIndex = 0;
        lastOutput = 0.0f;
    }

    void ReverbEngine::CombFilter::reset()
    {
        std::fill(delayLine.begin(), delayLine.end(), 0.0f);
        writeIndex = 0;
        lastOutput = 0.0f;
    }

    float ReverbEngine::CombFilter::processSample(float input, int delaySamples, float feedback, float damping)
    {
        delaySamples = juce::jlimit(1, static_cast<int>(delayLine.size()), delaySamples);
        
        int readIndex = (writeIndex - delaySamples + static_cast<int>(delayLine.size())) % static_cast<int>(delayLine.size());
        float delayedSample = delayLine[readIndex];
        
        // Apply damping (one-pole lowpass filter)
        lastOutput = delayedSample * (1.0f - damping) + lastOutput * damping;
        
        float output = lastOutput;
        delayLine[writeIndex] = input + output * feedback;
        
        writeIndex = (writeIndex + 1) % static_cast<int>(delayLine.size());
        
        return output;
    }

    //==============================================================================
    // ConvolutionProcessor Implementation
    
    void ReverbEngine::ConvolutionProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
    {
        juce::dsp::ProcessSpec spec;
        spec.sampleRate = sampleRate;
        spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
        spec.numChannels = 2;
        
        convolution.prepare(spec);
    }

    void ReverbEngine::ConvolutionProcessor::reset()
    {
        convolution.reset();
    }

    void ReverbEngine::ConvolutionProcessor::processBlock(juce::AudioBuffer<float>& buffer)
    {
        if (impulseLoaded)
        {
            juce::dsp::AudioBlock<float> block(buffer);
            juce::dsp::ProcessContextReplacing<float> context(block);
            convolution.process(context);
        }
    }

    bool ReverbEngine::ConvolutionProcessor::loadImpulseResponse(const juce::AudioBuffer<float>& impulseResponse)
    {
        try
        {
            // Create a copy of the buffer since JUCE needs a non-const reference
            juce::AudioBuffer<float> impulseBuffer;
            impulseBuffer.makeCopyOf(impulseResponse);
            
            convolution.loadImpulseResponse(std::move(impulseBuffer), 44100.0, 
                                          juce::dsp::Convolution::Stereo::yes, 
                                          juce::dsp::Convolution::Trim::yes, 
                                          juce::dsp::Convolution::Normalise::yes);
            impulseLoaded = true;
            return true;
        }
        catch (...)
        {
            impulseLoaded = false;
            return false;
        }
    }

} // namespace spawnclone::audio
