//==============================================================================
/*
  PitchShifter.h
  Author:  BMad Dev Agent

  A simple pitch shifter based on resampling.
  For the MVP, this will be a basic implementation. More advanced techniques
  can be integrated later.
*/
//==============================================================================

#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

namespace spawnclone::audio
{
    class PitchShifter
    {
    public:
        PitchShifter();

        void prepareToPlay(double sampleRate);
        
        double process(double currentPosition, double pitchRatio);

    private:
        double shifterSampleRate;
        juce::LagrangeInterpolator interpolator;
    };
}
