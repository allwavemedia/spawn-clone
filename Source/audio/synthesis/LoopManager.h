//==============================================================================
/*
  LoopManager.h
  Author:  BMad Dev Agent

  Manages looping for the SampleEngine.
  Handles different loop modes (Forward, Ping-Pong) and applies crossfades
  to ensure smooth loop transitions.
*/
//==============================================================================

#pragma once

#include <juce_core/juce_core.h>
#include "../SynthesisTypes.h"

namespace spawnclone::audio
{

    class LoopManager
    {
    public:
        LoopManager();

        void setLoopParameters(const SampleParams& params, double sampleRate, int sampleLength);
        double getNextPosition(double currentPosition, float increment, bool isReversed, int& playDirection);
        double handleLooping(double currentPosition);
        float getCrossfadeGain(double currentPosition);
        void reset();

    private:
        int loopStart = 0;
        int loopEnd = 0;
        int loopLength = 0;
        SampleParams::LoopMode currentLoopMode;
        
        // Additional members used by implementation
        int loopStartSample = 0;
        int loopEndSample = 0;
        float crossfadeLengthInSamples = 0.0f;
        int playDirection = 1;
    };

} // namespace spawnclone::audio
