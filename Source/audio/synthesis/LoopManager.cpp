//==============================================================================
/*
  LoopManager.cpp
  Author:  BMad Dev Agent
*/
//==============================================================================

#include "LoopManager.h"

namespace spawnclone::audio
{
    LoopManager::LoopManager()
    {
    }

    void LoopManager::setLoopParameters(const SampleParams& params, double sampleRate, int sampleLength)
    {
        currentLoopMode = params.loopMode;
        loopStartSample = params.loopStart;
        loopEndSample = (params.loopEnd > 0) ? params.loopEnd : sampleLength - 1;
        loopLength = loopEndSample - loopStartSample;
        crossfadeLengthInSamples = params.loopCrossfade * (float)sampleRate / 1000.0f;
        playDirection = 1;
    }

    double LoopManager::handleLooping(double currentPosition)
    {
        if (loopLength <= 0)
        {
            return currentPosition;
        }

        if (currentLoopMode == SampleParams::LoopMode::Forward)
        {
            if (currentPosition >= loopEndSample)
            {
                return currentPosition - loopLength;
            }
        }
        else if (currentLoopMode == SampleParams::LoopMode::PingPong)
        {
            if (playDirection == 1 && currentPosition >= loopEndSample)
            {
                playDirection = -1;
                return loopEndSample;
            }
            if (playDirection == -1 && currentPosition <= loopStartSample)
            {
                playDirection = 1;
                return loopStartSample;
            }
        }
        
        return currentPosition;
    }

    float LoopManager::getCrossfadeGain(double currentPosition)
    {
        if (crossfadeLengthInSamples <= 0.0f)
        {
            return 1.0f;
        }

        float gain = 1.0f;
        if (currentPosition > loopEndSample - crossfadeLengthInSamples)
        {
            gain = (loopEndSample - (float)currentPosition) / crossfadeLengthInSamples;
        }
        else if (currentPosition < loopStartSample + crossfadeLengthInSamples)
        {
            gain = ((float)currentPosition - loopStartSample) / crossfadeLengthInSamples;
        }

        return juce::jlimit(0.0f, 1.0f, gain);
    }
}
