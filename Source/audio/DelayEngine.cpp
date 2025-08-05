//==============================================================================
/*
  DelayEngine.cpp
  Author: BMad Dev Agent - Epic 6 Story 6.9
  
  Professional delay/echo processor with multiple delay types and advanced features
  
  Features:
  - 8 delay algorithms: Digital, Analog, Tape, PingPong, MultiTap, Reverse, Granular, Modulated
  - Tempo synchronization with DAW transport  
  - Professional parameter smoothing
  - Real-time safe processing
  - CPU monitoring and optimization
*/
//==============================================================================

#include "DelayEngine.h"
#include <cmath>

namespace spawnclone::audio
{
    //==============================================================================
    // DelayEngine Implementation
    //==============================================================================
    
    DelayEngine::DelayEngine()
    {
        // Initialize delay taps for multi-tap configuration
        delayTaps[0] = {125.0f, 0.8f, -0.5f};   // 1/8 note left
        delayTaps[1] = {250.0f, 0.6f, 0.5f};    // 1/4 note right
        delayTaps[2] = {375.0f, 0.4f, -0.3f};   // 3/8 note left
        delayTaps[3] = {500.0f, 0.3f, 0.3f};    // 1/2 note right
        delayTaps[4] = {750.0f, 0.2f, -0.1f};   // 3/4 note left
        delayTaps[5] = {1000.0f, 0.15f, 0.1f};  // 1 bar right
        delayTaps[6] = {1500.0f, 0.1f, 0.0f};   // 1.5 bars center
        delayTaps[7] = {2000.0f, 0.05f, 0.0f};  // 2 bars center
    }

    DelayEngine::~DelayEngine() = default;

    //==============================================================================
    void DelayEngine::prepareToPlay(double sampleRate, int samplesPerBlock)
    {
        currentSampleRate = sampleRate;
        currentBlockSize = samplesPerBlock;
        
        // Prepare delay lines (2 seconds maximum delay)
        for (auto& delayLine : delayLines)
        {
            delayLine.prepareToPlay(sampleRate, 2.0f);
        }
        
        // Prepare tone filters
        for (auto& filter : toneFilters)
        {
            filter.setSampleRate(sampleRate);
        }
        
        // Set initial filter frequencies
        toneFilters[0].setFrequency(lowCutFreq);    // Left low-cut
        toneFilters[1].setFrequency(highCutFreq);   // Left high-cut
        toneFilters[2].setFrequency(lowCutFreq);    // Right low-cut
        toneFilters[3].setFrequency(highCutFreq);   // Right high-cut
        
        // Prepare LFOs
        for (auto& lfo : modulationLFOs)
        {
            lfo.setSampleRate(sampleRate);
            lfo.setFrequency(modulationRate);
        }
        
        // Initialize parameter smoothers
        delayTimeSmoother.setSampleRate(sampleRate);
        delayTimeSmoother.setRampLength(0.05); // 50ms smoothing
        delayTimeSmoother.reset(delayTimeMs);
        
        feedbackSmoother.setSampleRate(sampleRate);
        feedbackSmoother.setRampLength(0.05);
        feedbackSmoother.reset(feedback);
        
        lowCutSmoother.setSampleRate(sampleRate);
        lowCutSmoother.setRampLength(0.1); // Filter changes can be slower
        lowCutSmoother.reset(lowCutFreq);
        
        highCutSmoother.setSampleRate(sampleRate);
        highCutSmoother.setRampLength(0.1);
        highCutSmoother.reset(highCutFreq);
        
        stereoSpreadSmoother.setSampleRate(sampleRate);
        stereoSpreadSmoother.setRampLength(0.05);
        stereoSpreadSmoother.reset(stereoSpread);
        
        modRateSmoother.setSampleRate(sampleRate);
        modRateSmoother.setRampLength(0.1);
        modRateSmoother.reset(modulationRate);
        
        modDepthSmoother.setSampleRate(sampleRate);
        modDepthSmoother.setRampLength(0.05);
        modDepthSmoother.reset(modulationDepth);
        
        // Prepare temporary buffers
        feedbackBuffer.setSize(2, samplesPerBlock);
        tempBuffer.setSize(2, samplesPerBlock);
        
        feedbackBuffer.clear();
        tempBuffer.clear();
    }

    void DelayEngine::releaseResources()
    {
        feedbackBuffer.setSize(0, 0);
        tempBuffer.setSize(0, 0);
    }

    void DelayEngine::reset()
    {
        for (auto& delayLine : delayLines)
        {
            delayLine.reset();
        }
        
        for (auto& filter : toneFilters)
        {
            filter.reset();
        }
        
        for (auto& lfo : modulationLFOs)
        {
            lfo.reset();
        }
        
        feedbackBuffer.clear();
        tempBuffer.clear();
        
        // Reset parameter smoothers to current values
        delayTimeSmoother.reset(delayTimeMs);
        feedbackSmoother.reset(feedback);
        lowCutSmoother.reset(lowCutFreq);
        highCutSmoother.reset(highCutFreq);
        stereoSpreadSmoother.reset(stereoSpread);
        modRateSmoother.reset(modulationRate);
        modDepthSmoother.reset(modulationDepth);
    }

    void DelayEngine::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
    {
        juce::ignoreUnused(midiMessages);
        
        if (isBypassed())
            return;
            
        startCPUMonitoring();
        
        const int numSamples = buffer.getNumSamples();
        const int numChannels = juce::jmin(buffer.getNumChannels(), 2);
        
        // Ensure temporary buffers are the right size
        if (feedbackBuffer.getNumSamples() != numSamples)
        {
            feedbackBuffer.setSize(2, numSamples, false, false, true);
            tempBuffer.setSize(2, numSamples, false, false, true);
        }
        
        // Store dry signal for wet/dry mixing
        juce::AudioBuffer<float> dryBuffer;
        dryBuffer.makeCopyOf(buffer);
        
        // Process according to current delay type
        switch (currentDelayType)
        {
            case DelayType::Digital:
                processDigitalDelay(buffer);
                break;
            case DelayType::Analog:
                processAnalogDelay(buffer);
                break;
            case DelayType::Tape:
                processTapeDelay(buffer);
                break;
            case DelayType::PingPong:
                processPingPongDelay(buffer);
                break;
            case DelayType::MultiTap:
                processMultiTapDelay(buffer);
                break;
            case DelayType::Reverse:
                processReverseDelay(buffer);
                break;
            case DelayType::Granular:
                processGranularDelay(buffer);
                break;
            case DelayType::Modulated:
                processModulatedDelay(buffer);
                break;
        }
        
        // Apply wet/dry mixing
        applyWetDryMix(buffer, dryBuffer);
        
        endCPUMonitoring();
    }

    //==============================================================================
    // Parameter Control
    //==============================================================================
    
    void DelayEngine::setParameter(const juce::String& parameterName, float value, bool useInterpolation)
    {
        if (parameterName == "delayTime")
        {
            delayTimeMs = juce::jlimit(1.0f, 2000.0f, value);
            if (useInterpolation)
                delayTimeSmoother.setTargetValue(delayTimeMs);
            else
                delayTimeSmoother.reset(delayTimeMs);
        }
        else if (parameterName == "feedback")
        {
            feedback = juce::jlimit(0.0f, 0.95f, value);
            if (useInterpolation)
                feedbackSmoother.setTargetValue(feedback);
            else
                feedbackSmoother.reset(feedback);
        }
        else if (parameterName == "lowCut")
        {
            lowCutFreq = juce::jlimit(20.0f, 2000.0f, value);
            if (useInterpolation)
                lowCutSmoother.setTargetValue(lowCutFreq);
            else
                lowCutSmoother.reset(lowCutFreq);
        }
        else if (parameterName == "highCut")
        {
            highCutFreq = juce::jlimit(1000.0f, 20000.0f, value);
            if (useInterpolation)
                highCutSmoother.setTargetValue(highCutFreq);
            else
                highCutSmoother.reset(highCutFreq);
        }
        else if (parameterName == "stereoSpread")
        {
            stereoSpread = juce::jlimit(0.0f, 1.0f, value);
            if (useInterpolation)
                stereoSpreadSmoother.setTargetValue(stereoSpread);
            else
                stereoSpreadSmoother.reset(stereoSpread);
        }
        else if (parameterName == "modRate")
        {
            modulationRate = juce::jlimit(0.1f, 20.0f, value);
            if (useInterpolation)
                modRateSmoother.setTargetValue(modulationRate);
            else
                modRateSmoother.reset(modulationRate);
        }
        else if (parameterName == "modDepth")
        {
            modulationDepth = juce::jlimit(0.0f, 1.0f, value);
            if (useInterpolation)
                modDepthSmoother.setTargetValue(modulationDepth);
            else
                modDepthSmoother.reset(modulationDepth);
        }
    }

    float DelayEngine::getParameter(const juce::String& parameterName) const
    {
        if (parameterName == "delayTime") return delayTimeMs;
        if (parameterName == "feedback") return feedback;
        if (parameterName == "lowCut") return lowCutFreq;
        if (parameterName == "highCut") return highCutFreq;
        if (parameterName == "stereoSpread") return stereoSpread;
        if (parameterName == "modRate") return modulationRate;
        if (parameterName == "modDepth") return modulationDepth;
        
        return 0.0f;
    }

    juce::StringArray DelayEngine::getParameterNames() const
    {
        return {"delayTime", "feedback", "lowCut", "highCut", "stereoSpread", "modRate", "modDepth"};
    }

    //==============================================================================
    // Delay-Specific Interface
    //==============================================================================
    
    void DelayEngine::setDelayType(DelayType type)
    {
        if (currentDelayType != type)
        {
            currentDelayType = type;
            reset(); // Clear any artifacts when switching types
        }
    }

    void DelayEngine::setDelayTime(float timeMs)
    {
        setParameter("delayTime", timeMs, true);
    }

    void DelayEngine::setFeedback(float feedbackAmount)
    {
        setParameter("feedback", feedbackAmount, true);
    }

    void DelayEngine::setLowCut(float frequency)
    {
        setParameter("lowCut", frequency, true);
    }

    void DelayEngine::setHighCut(float frequency)
    {
        setParameter("highCut", frequency, true);
    }

    void DelayEngine::setStereoSpread(float spread)
    {
        setParameter("stereoSpread", spread, true);
    }

    void DelayEngine::setModulationRate(float rateHz)
    {
        setParameter("modRate", rateHz, true);
    }

    void DelayEngine::setModulationDepth(float depth)
    {
        setParameter("modDepth", depth, true);
    }

    void DelayEngine::setTempoSyncEnabled(bool enabled)
    {
        tempoSyncEnabled = enabled;
        if (enabled)
        {
            updateDelayTimesFromTempo();
        }
    }

    void DelayEngine::setNoteDivision(float division)
    {
        noteDivision = juce::jlimit(0.0625f, 4.0f, division); // 1/16 to 4 bars
        if (tempoSyncEnabled)
        {
            updateDelayTimesFromTempo();
        }
    }

    //==============================================================================
    // Delay Algorithm Implementations
    //==============================================================================
    
    void DelayEngine::processDigitalDelay(juce::AudioBuffer<float>& buffer)
    {
        const int numSamples = buffer.getNumSamples();
        const int numChannels = juce::jmin(buffer.getNumChannels(), 2);
        
        for (int sample = 0; sample < numSamples; ++sample)
        {
            // Update smoothed parameters
            float currentDelayTime = delayTimeSmoother.getNextValue();
            float currentFeedback = feedbackSmoother.getNextValue();
            float currentLowCut = lowCutSmoother.getNextValue();
            float currentHighCut = highCutSmoother.getNextValue();
            
            // Update filter frequencies
            toneFilters[0].setFrequency(currentLowCut);
            toneFilters[1].setFrequency(currentHighCut);
            toneFilters[2].setFrequency(currentLowCut);
            toneFilters[3].setFrequency(currentHighCut);
            
            float delaySamples = millisecondsToSamples(currentDelayTime);
            
            for (int channel = 0; channel < numChannels; ++channel)
            {
                float* channelData = buffer.getWritePointer(channel);
                float input = channelData[sample];
                
                // Read delayed signal
                float delayed = delayLines[channel].readSample(delaySamples, true);
                
                // Apply feedback filtering
                delayed = toneFilters[channel * 2].processHighpass(delayed);     // High-pass (low-cut)
                delayed = toneFilters[channel * 2 + 1].processLowpass(delayed);  // Low-pass (high-cut)
                
                // Calculate feedback signal
                float feedback_signal = delayed * currentFeedback;
                
                // Write to delay line (input + feedback)
                delayLines[channel].writeSample(input + feedback_signal);
                
                // Output is input + delayed signal (wet level controlled by base class)
                channelData[sample] = input + delayed;
            }
        }
    }

    void DelayEngine::processAnalogDelay(juce::AudioBuffer<float>& buffer)
    {
        const int numSamples = buffer.getNumSamples();
        const int numChannels = juce::jmin(buffer.getNumChannels(), 2);
        
        for (int sample = 0; sample < numSamples; ++sample)
        {
            float currentDelayTime = delayTimeSmoother.getNextValue();
            float currentFeedback = feedbackSmoother.getNextValue();
            float currentLowCut = lowCutSmoother.getNextValue();
            float currentHighCut = highCutSmoother.getNextValue();
            
            // Update filter frequencies
            toneFilters[0].setFrequency(currentLowCut);
            toneFilters[1].setFrequency(currentHighCut);
            toneFilters[2].setFrequency(currentLowCut);
            toneFilters[3].setFrequency(currentHighCut);
            
            float delaySamples = millisecondsToSamples(currentDelayTime);
            
            for (int channel = 0; channel < numChannels; ++channel)
            {
                float* channelData = buffer.getWritePointer(channel);
                float input = channelData[sample];
                
                // Read delayed signal
                float delayed = delayLines[channel].readSample(delaySamples, true);
                
                // Apply analog-style saturation to feedback path
                delayed = saturation.processSample(delayed, 1.5f);
                
                // Apply feedback filtering (more colored than digital)
                delayed = toneFilters[channel * 2].processHighpass(delayed);
                delayed = toneFilters[channel * 2 + 1].processLowpass(delayed);
                
                // Feedback with slight compression
                float feedback_signal = delayed * currentFeedback;
                feedback_signal = saturation.processSample(feedback_signal, 0.8f);
                
                // Write to delay line
                delayLines[channel].writeSample(input + feedback_signal);
                
                // Output with subtle analog warmth
                float output = input + saturation.processSample(delayed, 0.3f);
                channelData[sample] = output;
            }
        }
    }

    void DelayEngine::processTapeDelay(juce::AudioBuffer<float>& buffer)
    {
        const int numSamples = buffer.getNumSamples();
        const int numChannels = juce::jmin(buffer.getNumChannels(), 2);
        
        for (int sample = 0; sample < numSamples; ++sample)
        {
            float currentDelayTime = delayTimeSmoother.getNextValue();
            float currentFeedback = feedbackSmoother.getNextValue();
            
            // Add slight modulation for tape wow/flutter
            float lfoMod = modulationLFOs[0].getNextSample() * 0.002f; // Very subtle
            float delaySamples = millisecondsToSamples(currentDelayTime * (1.0f + lfoMod));
            
            for (int channel = 0; channel < numChannels; ++channel)
            {
                float* channelData = buffer.getWritePointer(channel);
                float input = channelData[sample];
                
                // Read delayed signal with tape-style interpolation
                float delayed = delayLines[channel].readSample(delaySamples, true);
                
                // Heavy tape saturation and filtering
                delayed = saturation.processSample(delayed, 2.0f);
                
                // Tape-style high-frequency rolloff
                delayed = toneFilters[channel * 2 + 1].processLowpass(delayed);
                delayed = toneFilters[channel * 2 + 1].processLowpass(delayed); // Double filtering
                
                // Feedback with tape compression
                float feedback_signal = delayed * currentFeedback;
                feedback_signal = saturation.processSample(feedback_signal, 1.2f);
                
                delayLines[channel].writeSample(input + feedback_signal);
                
                // Output with tape character
                channelData[sample] = input + delayed * 0.9f; // Slightly lower level
            }
        }
    }

    void DelayEngine::processPingPongDelay(juce::AudioBuffer<float>& buffer)
    {
        const int numSamples = buffer.getNumSamples();
        const int numChannels = juce::jmin(buffer.getNumChannels(), 2);
        
        if (numChannels < 2)
        {
            // Fall back to digital delay for mono
            processDigitalDelay(buffer);
            return;
        }
        
        for (int sample = 0; sample < numSamples; ++sample)
        {
            float currentDelayTime = delayTimeSmoother.getNextValue();
            float currentFeedback = feedbackSmoother.getNextValue();
            float currentSpread = stereoSpreadSmoother.getNextValue();
            
            float leftDelay = millisecondsToSamples(currentDelayTime);
            float rightDelay = millisecondsToSamples(currentDelayTime * (1.0f + currentSpread * 0.1f));
            
            float* leftData = buffer.getWritePointer(0);
            float* rightData = buffer.getWritePointer(1);
            
            float leftInput = leftData[sample];
            float rightInput = rightData[sample];
            
            // Read delayed signals
            float leftDelayed = delayLines[0].readSample(leftDelay, true);
            float rightDelayed = delayLines[1].readSample(rightDelay, true);
            
            // Apply filtering
            leftDelayed = toneFilters[0].processHighpass(leftDelayed);
            leftDelayed = toneFilters[1].processLowpass(leftDelayed);
            rightDelayed = toneFilters[2].processHighpass(rightDelayed);
            rightDelayed = toneFilters[3].processLowpass(rightDelayed);
            
            // Ping-pong: left delay goes to right, right delay goes to left
            float leftFeedback = rightDelayed * currentFeedback;
            float rightFeedback = leftDelayed * currentFeedback;
            
            // Write to delay lines
            delayLines[0].writeSample(leftInput + leftFeedback);
            delayLines[1].writeSample(rightInput + rightFeedback);
            
            // Output
            leftData[sample] = leftInput + leftDelayed;
            rightData[sample] = rightInput + rightDelayed;
        }
    }

    void DelayEngine::processMultiTapDelay(juce::AudioBuffer<float>& buffer)
    {
        const int numSamples = buffer.getNumSamples();
        const int numChannels = juce::jmin(buffer.getNumChannels(), 2);
        
        // Clear temp buffer for accumulating taps
        tempBuffer.clear();
        
        for (int sample = 0; sample < numSamples; ++sample)
        {
            float currentFeedback = feedbackSmoother.getNextValue();
            
            for (int channel = 0; channel < numChannels; ++channel)
            {
                float* channelData = buffer.getWritePointer(channel);
                float* tempData = tempBuffer.getWritePointer(channel);
                float input = channelData[sample];
                
                float tappedOutput = 0.0f;
                
                // Process all delay taps
                for (const auto& tap : delayTaps)
                {
                    if (tap.gain > 0.0f)
                    {
                        float tapDelay = millisecondsToSamples(tap.delayTime);
                        float tapOutput = delayLines[channel].readSample(tapDelay, true);
                        
                        // Apply pan to tap (simple pan law)
                        float panGain = (channel == 0) ? 
                            std::sqrt(0.5f * (1.0f - tap.pan)) : 
                            std::sqrt(0.5f * (1.0f + tap.pan));
                        
                        tappedOutput += tapOutput * tap.gain * panGain;
                    }
                }
                
                // Apply feedback filtering
                tappedOutput = toneFilters[channel * 2].processHighpass(tappedOutput);
                tappedOutput = toneFilters[channel * 2 + 1].processLowpass(tappedOutput);
                
                // Write to delay line with feedback
                delayLines[channel].writeSample(input + tappedOutput * currentFeedback);
                
                // Accumulate output
                tempData[sample] = tappedOutput;
                channelData[sample] = input + tappedOutput;
            }
        }
    }

    void DelayEngine::processReverseDelay(juce::AudioBuffer<float>& buffer)
    {
        // Note: This is a simplified reverse delay - full implementation would require
        // buffering and reversing chunks of audio
        const int numSamples = buffer.getNumSamples();
        const int numChannels = juce::jmin(buffer.getNumChannels(), 2);
        
        for (int sample = 0; sample < numSamples; ++sample)
        {
            float currentDelayTime = delayTimeSmoother.getNextValue();
            float currentFeedback = feedbackSmoother.getNextValue();
            
            float delaySamples = millisecondsToSamples(currentDelayTime);
            
            for (int channel = 0; channel < numChannels; ++channel)
            {
                float* channelData = buffer.getWritePointer(channel);
                float input = channelData[sample];
                
                // Read delayed signal
                float delayed = delayLines[channel].readSample(delaySamples, true);
                
                // Apply reverse envelope (fade in effect)
                float envelopePhase = fmod((sample / float(numSamples)) + 
                                         (juce::Time::getCurrentTime().toMilliseconds() * 0.001), 1.0);
                float envelope = std::sin(envelopePhase * juce::MathConstants<float>::pi);
                
                delayed *= envelope;
                
                // Feedback
                float feedback_signal = delayed * currentFeedback;
                delayLines[channel].writeSample(input + feedback_signal);
                
                channelData[sample] = input + delayed;
            }
        }
    }

    void DelayEngine::processGranularDelay(juce::AudioBuffer<float>& buffer)
    {
        // Simplified granular delay - full implementation would be much more complex
        const int numSamples = buffer.getNumSamples();
        const int numChannels = juce::jmin(buffer.getNumChannels(), 2);
        
        for (int sample = 0; sample < numSamples; ++sample)
        {
            float currentDelayTime = delayTimeSmoother.getNextValue();
            float currentFeedback = feedbackSmoother.getNextValue();
            float currentModDepth = modDepthSmoother.getNextValue();
            
            // Granular-style pitch modulation
            float pitchMod = modulationLFOs[0].getNextSample() * currentModDepth * 0.2f; // ±20% pitch
            float delaySamples = millisecondsToSamples(currentDelayTime * (1.0f + pitchMod));
            
            for (int channel = 0; channel < numChannels; ++channel)
            {
                float* channelData = buffer.getWritePointer(channel);
                float input = channelData[sample];
                
                // Read with granular-style windowing
                float delayed = delayLines[channel].readSample(delaySamples, true);
                
                // Apply windowing function for grain-like effect
                float windowPhase = fmod((sample / 32.0f), 1.0f); // 32-sample grains
                float window = 0.5f * (1.0f - std::cos(2.0f * juce::MathConstants<float>::pi * windowPhase));
                delayed *= window;
                
                // Feedback
                float feedback_signal = delayed * currentFeedback;
                delayLines[channel].writeSample(input + feedback_signal);
                
                channelData[sample] = input + delayed;
            }
        }
    }

    void DelayEngine::processModulatedDelay(juce::AudioBuffer<float>& buffer)
    {
        const int numSamples = buffer.getNumSamples();
        const int numChannels = juce::jmin(buffer.getNumChannels(), 2);
        
        for (int sample = 0; sample < numSamples; ++sample)
        {
            float currentDelayTime = delayTimeSmoother.getNextValue();
            float currentFeedback = feedbackSmoother.getNextValue();
            float currentModRate = modRateSmoother.getNextValue();
            float currentModDepth = modDepthSmoother.getNextValue();
            
            // Update LFO rates
            for (auto& lfo : modulationLFOs)
            {
                lfo.setFrequency(currentModRate);
            }
            
            for (int channel = 0; channel < numChannels; ++channel)
            {
                float* channelData = buffer.getWritePointer(channel);
                float input = channelData[sample];
                
                // Add LFO modulation to delay time (chorus-style)
                float lfoValue = modulationLFOs[channel].getNextSample();
                float modAmount = currentModDepth * 5.0f; // Max 5ms modulation
                float modulatedDelay = currentDelayTime + (lfoValue * modAmount);
                float delaySamples = millisecondsToSamples(modulatedDelay);
                
                // Read delayed signal
                float delayed = delayLines[channel].readSample(delaySamples, true);
                
                // Apply feedback filtering
                delayed = toneFilters[channel * 2].processHighpass(delayed);
                delayed = toneFilters[channel * 2 + 1].processLowpass(delayed);
                
                // Feedback
                float feedback_signal = delayed * currentFeedback;
                delayLines[channel].writeSample(input + feedback_signal);
                
                channelData[sample] = input + delayed;
            }
        }
    }

    //==============================================================================
    // Helper Methods
    //==============================================================================
    
    float DelayEngine::millisecondsToSamples(float ms) const
    {
        return ms * 0.001f * static_cast<float>(currentSampleRate);
    }

    float DelayEngine::samplesToMilliseconds(float samples) const
    {
        return samples / static_cast<float>(currentSampleRate) * 1000.0f;
    }

    float DelayEngine::noteDivisionToMilliseconds(float division, float tempo) const
    {
        // Convert note division to milliseconds
        // 1.0 = quarter note, 0.5 = eighth note, etc.
        float beatLength = 60000.0f / tempo; // Quarter note in milliseconds
        return beatLength * division;
    }

    void DelayEngine::updateDelayTimesFromTempo()
    {
        if (tempoSyncEnabled)
        {
            float syncedTime = noteDivisionToMilliseconds(noteDivision, currentTempo);
            setDelayTime(syncedTime);
        }
    }

    //==============================================================================
    // Inner Class Implementations
    //==============================================================================
    
    // DelayLine Implementation
    void DelayEngine::DelayLine::prepareToPlay(double sr, float maxDelaySeconds)
    {
        sampleRate = sr;
        int maxSamples = static_cast<int>(maxDelaySeconds * sr) + 1;
        buffer.resize(maxSamples);
        reset();
    }

    void DelayEngine::DelayLine::reset()
    {
        std::fill(buffer.begin(), buffer.end(), 0.0f);
        writeIndex = 0;
        currentDelay = 0.0f;
    }

    float DelayEngine::DelayLine::readSample(float delaySamples, bool interpolate) const
    {
        if (buffer.empty())
            return 0.0f;
            
        float readPosition = writeIndex - delaySamples;
        
        // Wrap around buffer
        while (readPosition < 0.0f)
            readPosition += buffer.size();
        while (readPosition >= buffer.size())
            readPosition -= buffer.size();
        
        if (!interpolate)
        {
            return buffer[static_cast<int>(readPosition)];
        }
        
        // Hermite interpolation for high quality
        int x0 = static_cast<int>(readPosition);
        float fraction = readPosition - x0;
        
        int x1 = (x0 + 1) % buffer.size();
        int x_1 = (x0 - 1 + buffer.size()) % buffer.size();
        int x2 = (x0 + 2) % buffer.size();
        
        return interpolateHermite(fraction, buffer[x_1], buffer[x0], buffer[x1], buffer[x2]);
    }

    void DelayEngine::DelayLine::writeSample(float sample)
    {
        if (!buffer.empty())
        {
            buffer[writeIndex] = sample;
            writeIndex = (writeIndex + 1) % buffer.size();
        }
    }

    void DelayEngine::DelayLine::setDelayTime(float delaySamples)
    {
        currentDelay = juce::jlimit(0.0f, static_cast<float>(buffer.size() - 1), delaySamples);
    }

    float DelayEngine::DelayLine::interpolateHermite(float x, float y0, float y1, float y2, float y3) const
    {
        // Hermite interpolation for smooth delay line reading
        float c0 = y1;
        float c1 = 0.5f * (y2 - y0);
        float c2 = y0 - 2.5f * y1 + 2.0f * y2 - 0.5f * y3;
        float c3 = 0.5f * (y3 - y0) + 1.5f * (y1 - y2);
        
        return ((c3 * x + c2) * x + c1) * x + c0;
    }

    // OnePoleFilter Implementation
    void DelayEngine::OnePoleFilter::setSampleRate(double sr)
    {
        sampleRate = sr;
    }

    void DelayEngine::OnePoleFilter::setFrequency(float frequency)
    {
        float omega = 2.0f * juce::MathConstants<float>::pi * frequency / static_cast<float>(sampleRate);
        coefficient = std::exp(-omega);
    }

    void DelayEngine::OnePoleFilter::reset()
    {
        state = 0.0f;
    }

    float DelayEngine::OnePoleFilter::processLowpass(float input)
    {
        state = input * (1.0f - coefficient) + state * coefficient;
        return state;
    }

    float DelayEngine::OnePoleFilter::processHighpass(float input)
    {
        float lowpass = processLowpass(input);
        return input - lowpass;
    }

    // LFO Implementation
    void DelayEngine::LFO::setSampleRate(double sr)
    {
        sampleRate = sr;
    }

    void DelayEngine::LFO::setFrequency(float freq)
    {
        frequency = freq;
    }

    void DelayEngine::LFO::reset()
    {
        phase = 0.0f;
    }

    float DelayEngine::LFO::getNextSample()
    {
        float output = std::sin(phase);
        phase += 2.0f * juce::MathConstants<float>::pi * frequency / static_cast<float>(sampleRate);
        
        if (phase >= 2.0f * juce::MathConstants<float>::pi)
            phase -= 2.0f * juce::MathConstants<float>::pi;
            
        return output;
    }

    // AnalogSaturation Implementation
    float DelayEngine::AnalogSaturation::processSample(float input, float drive)
    {
        float driven = input * drive;
        return tanhApprox(driven) / drive;
    }

    float DelayEngine::AnalogSaturation::tanhApprox(float x)
    {
        // Fast tanh approximation for analog-style saturation
        if (x > 3.0f) return 1.0f;
        if (x < -3.0f) return -1.0f;
        
        float x2 = x * x;
        return x * (27.0f + x2) / (27.0f + 9.0f * x2);
    }

} // namespace spawnclone::audio
