//=========================    void SampleEngine::startNote(int midiNoteNumber,
                           void SampleEngine::selectSample(float velocity,
                                   const SampleParams& params, void SampleEngine::selectSample(float velocity,
                                   const SampleParams& params,        float velocity,
                                const SampleParams& params,==================================================
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
                               const AdvancedSynthesisEngine::SampleParams& params,
                               const std::vector<juce::AudioBuffer<float>>& samplePool)
    {
        noteVelocity = velocity;
        selectSample(velocity, params, samplePool);

        if (currentSampleBuffer)
        {
            double notePitch = midiNoteNumber;
            double rootPitch = params.rootNote;
            pitchRatio = std::pow(2.0, (notePitch - rootPitch) / 12.0);

            loopManager.setLoopParameters(params, engineSampleRate, currentSampleBuffer->getNumSamples());
            
            currentPlaybackPosition = 0.0;
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

        auto* channelData = buffer.getWritePointer(0, startSample);
        auto* sampleData = currentSampleBuffer->getReadPointer(0);
        const int numSampleFrames = currentSampleBuffer->getNumSamples();

        for (int i = 0; i < numSamples; ++i)
        {
            currentPlaybackPosition = loopManager.handleLooping(currentPlaybackPosition);

            // Simple linear interpolation for now
            auto pos = currentPlaybackPosition;
            auto index1 = static_cast<int>(pos);
            auto index2 = index1 + 1;
            auto frac = pos - index1;

            if (index2 >= numSampleFrames)
            {
                if (loopManager.getCrossfadeGain(pos) > 0.0f)
                    index2 = loopManager.handleLooping(index2);
                else
                {
                    isPlaying = false;
                    break;
                }
            }

            auto sample1 = sampleData[index1];
            auto sample2 = sampleData[index2];
            auto currentSample = sample1 + frac * (sample2 - sample1);

            channelData[i] += currentSample * noteVelocity * loopManager.getCrossfadeGain(currentPlaybackPosition);

            currentPlaybackPosition += pitchRatio;
        }
    }

    bool SampleEngine::isNoteFinished() const
    {
        return !isPlaying;
    }

    void SampleEngine::selectSample(float velocity,
                                  const AdvancedSynthesisEngine::SampleParams& params,
                                  const std::vector<juce::AudioBuffer<float>>& samplePool)
    {
        if (params.enableVelocityLayers && !params.sampleMap.empty())
        {
            int vel = static_cast<int>(velocity * 127.0f);
            for (const auto& entry : params.sampleMap)
            {
                if (vel >= entry.minVelocity && vel <= entry.maxVelocity)
                {
                    if (entry.sampleIndex < samplePool.size())
                    {
                        currentSampleBuffer = &samplePool[entry.sampleIndex];
                        return;
                    }
                }
            }
        }

        // Fallback to the main sample
        if (params.sampleIndex < samplePool.size())
        {
            currentSampleBuffer = &samplePool[params.sampleIndex];
        }
        else
        {
            currentSampleBuffer = nullptr;
        }
    }
}
