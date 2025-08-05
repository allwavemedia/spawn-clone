//==============================================================================
/*
  SampleEngine.cpp
  Author:  BMad Dev Agent
*/
//==============================================================================

#include "SampleEngine.h"

namespace spawnclone::audio
{
    SampleEngine::SampleEngine(SampleCache& cache)
        : sampleCache(cache)
    {
    }

    void SampleEngine::prepareToPlay(double sampleRate, int /*samplesPerBlock*/)
    {
        engineSampleRate = sampleRate;
        pitchShifter.prepareToPlay(sampleRate);
    }

    void SampleEngine::releaseResources()
    {
        // Nothing to release for now
    }

    void SampleEngine::startNote(int midiNoteNumber,
                               float velocity,
                               const SampleParams& params,
                               const std::vector<juce::AudioBuffer<float>>& samplePool)
    {
        noteVelocity = velocity;
        
        // Store parameters for processing
        currentParams = params;
        
        selectSample(velocity, params, samplePool);

        if (currentSampleBuffer)
        {
            double notePitch = midiNoteNumber;
            double rootPitch = params.rootNote;
            pitchRatio = std::pow(2.0, (notePitch - rootPitch) / 12.0);
            
            // Apply playback speed modifier separately to preserve pitch accuracy
            playbackSpeed = params.playbackSpeed;
            effectivePlaybackRate = pitchRatio * playbackSpeed;

            loopManager.setLoopParameters(params, engineSampleRate, currentSampleBuffer->getNumSamples());
            
            // Apply start offset with proper reverse playback handling
            if (params.reversePlayback)
            {
                // For reverse playback, start from the end and apply offset backwards
                double endPosition = currentSampleBuffer->getNumSamples() - 1;
                double offsetAmount = params.startOffset * currentSampleBuffer->getNumSamples();
                currentPlaybackPosition = endPosition - offsetAmount;
            }
            else
            {
                // For forward playback, start from beginning with offset
                currentPlaybackPosition = params.startOffset * currentSampleBuffer->getNumSamples();
            }
            
            isPlaying = true;
        }
    }

    void SampleEngine::stopNote()
    {
        isPlaying = false;
    }

    void SampleEngine::processBlock(juce::AudioBuffer<float>& buffer, int startSample, int numSamples)
    {
        if (!isPlaying || !currentSampleBuffer)
        {
            return;
        }

        const int numChannels = buffer.getNumChannels();
        auto* sampleData = currentSampleBuffer->getReadPointer(0);
        const int numSampleFrames = currentSampleBuffer->getNumSamples();
        
        // Secondary sample data for velocity crossfading
        const float* secondarySampleData = nullptr;
        int secondaryNumFrames = 0;
        if (secondarySampleBuffer && velocityBlendAmount > 0.0f)
        {
            secondarySampleData = secondarySampleBuffer->getReadPointer(0);
            secondaryNumFrames = secondarySampleBuffer->getNumSamples();
        }

        for (int i = 0; i < numSamples; ++i)
        {
            double pos = currentPlaybackPosition;
            
            // Bounds checking for primary sample
            if (currentParams.reversePlayback)
            {
                if (pos < 0.0)
                {
                    if (currentParams.enableLooping)
                    {
                        pos = numSampleFrames - 1;
                        currentPlaybackPosition = pos;
                    }
                    else
                    {
                        isPlaying = false;
                        break;
                    }
                }
            }
            else
            {
                if (pos >= numSampleFrames)
                {
                    if (currentParams.enableLooping)
                    {
                        pos = 0.0;
                        currentPlaybackPosition = pos;
                    }
                    else
                    {
                        isPlaying = false;
                        break;
                    }
                }
            }
            
            // Get primary sample
            float primarySample = getSampleAtPosition(sampleData, numSampleFrames, pos);
            float finalSample = primarySample;
            
            // Apply velocity crossfading if secondary sample exists
            if (secondarySampleData && velocityBlendAmount > 0.0f)
            {
                float secondarySample = getSampleAtPosition(secondarySampleData, secondaryNumFrames, pos);
                finalSample = primarySample * (1.0f - velocityBlendAmount) + 
                             secondarySample * velocityBlendAmount;
            }

            // Apply velocity scaling 
            if (!currentParams.sampleMap.empty())
            {
                // For velocity layers, the layer selection is the primary volume control
                // No additional velocity scaling needed
                // finalSample is used as-is
            }
            else
            {
                // For single samples, full velocity scaling
                finalSample *= noteVelocity;
            }
            
            // Write to all channels
            for (int channel = 0; channel < numChannels; ++channel)
            {
                buffer.addSample(channel, startSample + i, finalSample);
            }

            // Update playback position
            double increment = currentParams.reversePlayback ? -effectivePlaybackRate : effectivePlaybackRate;
            currentPlaybackPosition += increment;
        }
    }

    bool SampleEngine::isNoteFinished() const
    {
        return !isPlaying;
    }

    void SampleEngine::selectSample(float velocity,
                                  const SampleParams& params,
                                  const std::vector<juce::AudioBuffer<float>>& samplePool)
    {
        currentSampleBuffer = nullptr;
        secondarySampleBuffer = nullptr;
        velocityBlendAmount = 0.0f;
        
        if ((params.enableVelocityLayers || !params.sampleMap.empty()) && !params.sampleMap.empty())
        {
            int vel = static_cast<int>(velocity * 127.0f);
            
            // Find the primary sample layer
            int primaryIndex = -1;
            
            for (size_t i = 0; i < params.sampleMap.size(); ++i)
            {
                const auto& entry = params.sampleMap[i];
                if (vel >= entry.minVelocity && vel <= entry.maxVelocity)
                {
                    primaryIndex = static_cast<int>(i);
                    break;
                }
            }
            
            // If no layer contains this velocity, check if we're in a gap that should trigger crossfading
            if (primaryIndex == -1)
            {
                for (size_t i = 0; i < params.sampleMap.size() - 1; ++i)
                {
                    const auto& currentEntry = params.sampleMap[i];
                    const auto& nextEntry = params.sampleMap[i + 1];
                    
                    // Check if velocity is in the gap between layers
                    if (vel > currentEntry.maxVelocity && vel < nextEntry.minVelocity)
                    {
                        // Use the lower layer as primary for gap crossfading
                        primaryIndex = static_cast<int>(i);
                        
                        // Set up crossfading between the two layers
                        if (currentEntry.sampleIndex < samplePool.size() && nextEntry.sampleIndex < samplePool.size())
                        {
                            currentSampleBuffer = &samplePool[currentEntry.sampleIndex];
                            secondarySampleBuffer = &samplePool[nextEntry.sampleIndex];
                            
                            // Calculate blend amount based on position in gap
                            float gapSize = nextEntry.minVelocity - currentEntry.maxVelocity - 1;
                            float positionInGap = vel - currentEntry.maxVelocity - 1;
                            velocityBlendAmount = positionInGap / gapSize;
                            velocityBlendAmount = juce::jlimit(0.0f, 1.0f, velocityBlendAmount);
                        }
                        return;
                    }
                }
            }
            
            if (primaryIndex >= 0 && params.sampleMap[primaryIndex].sampleIndex < samplePool.size())
            {
                currentSampleBuffer = &samplePool[params.sampleMap[primaryIndex].sampleIndex];
                
                // Check for velocity crossfading
                if (params.velocityCrossfadeWidth > 0.0f)
                {
                    // Look for adjacent layer for crossfading
                    const auto& primaryEntry = params.sampleMap[primaryIndex];
                    float crossfadeRange = params.velocityCrossfadeWidth;
                    
                    // Check if we're near the boundary with the next layer
                    if (primaryIndex + 1 < params.sampleMap.size())
                    {
                        const auto& nextEntry = params.sampleMap[primaryIndex + 1];
                        
                        // Calculate distance from the upper edge of primary layer
                        float distanceFromUpperEdge = primaryEntry.maxVelocity - vel;
                        
                        // If we're within crossfade range of the upper boundary
                        if (distanceFromUpperEdge < crossfadeRange && nextEntry.sampleIndex < samplePool.size())
                        {
                            secondarySampleBuffer = &samplePool[nextEntry.sampleIndex];
                            // Blend amount increases as we get closer to next layer
                            velocityBlendAmount = (crossfadeRange - distanceFromUpperEdge) / crossfadeRange;
                            velocityBlendAmount = juce::jlimit(0.0f, 1.0f, velocityBlendAmount);
                        }
                    }
                    
                    // Also check if we're near the boundary with the previous layer
                    if (primaryIndex > 0 && secondarySampleBuffer == nullptr)
                    {
                        const auto& prevEntry = params.sampleMap[primaryIndex - 1];
                        
                        // Calculate distance from the lower edge of primary layer
                        float distanceFromLowerEdge = vel - primaryEntry.minVelocity;
                        
                        // If we're within crossfade range of the lower boundary
                        if (distanceFromLowerEdge < crossfadeRange && prevEntry.sampleIndex < samplePool.size())
                        {
                            secondarySampleBuffer = &samplePool[prevEntry.sampleIndex];
                            // Blend amount increases as we get closer to previous layer
                            velocityBlendAmount = (crossfadeRange - distanceFromLowerEdge) / crossfadeRange;
                            velocityBlendAmount = juce::jlimit(0.0f, 1.0f, velocityBlendAmount);
                        }
                    }
                }
                return;
            }
        }

        // Fallback to the main sample
        if (params.sampleIndex < samplePool.size())
        {
            currentSampleBuffer = &samplePool[params.sampleIndex];
        }
    }
    
    float SampleEngine::getSampleAtPosition(const float* sampleData, int numFrames, double position) const
    {
        if (numFrames <= 0 || !sampleData)
            return 0.0f;
            
        // Linear interpolation with bounds safety
        int index1 = static_cast<int>(position);
        int index2 = index1 + (currentParams.reversePlayback ? -1 : 1);
        
        // Ensure indices are within bounds
        index1 = juce::jlimit(0, numFrames - 1, index1);
        index2 = juce::jlimit(0, numFrames - 1, index2);
        
        float frac = static_cast<float>(position - static_cast<int>(position));
        if (currentParams.reversePlayback)
            frac = 1.0f - frac;
        
        float sample1 = sampleData[index1];
        float sample2 = sampleData[index2];
        return sample1 + frac * (sample2 - sample1);
    }
}
