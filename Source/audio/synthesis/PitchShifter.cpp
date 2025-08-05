//==============================================================================
/*
  PitchShifter.cpp
  Author:  BMad Dev Agent
*/
//==============================================================================

#include "PitchShifter.h"

namespace spawnclone::audio
{
    PitchShifter::PitchShifter()
    {
    }

    void PitchShifter::prepareToPlay(double sampleRate)
    {
        shifterSampleRate = sampleRate;
    }

    double PitchShifter::process(double currentPosition, double pitchRatio)
    {
        // This is a simplified approach. The actual processing will happen
        // in the SampleEngine by advancing the read position.
        return currentPosition * pitchRatio;
    }
}
