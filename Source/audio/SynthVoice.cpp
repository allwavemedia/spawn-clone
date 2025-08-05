//==============================================================================
/*
  SynthVoice.cpp
  Author:  BMad Dev Agent
*/
//==============================================================================

#include "SynthVoice.h"
#include "AdvancedSynthesisEngine.h"  // For complete SynthesisParameters definition

namespace spawnclone::audio
{
    SynthVoice::SynthVoice(SampleCache& cache)
        : sampleEngine(cache)
    {
    }

    void SynthVoice::prepareToPlay(double sampleRate, int samplesPerBlock)
    {
        voiceSampleRate = sampleRate;
        adsr.setSampleRate(sampleRate);
        sampleEngine.prepareToPlay(sampleRate, samplesPerBlock);
    }

    void SynthVoice::releaseResources()
    {
        sampleEngine.releaseResources();
    }

    void SynthVoice::startNote(int midiNoteNumber, float velocity, SynthesisType synthType, const SampleParams& sampleParams, const std::vector<juce::AudioBuffer<float>>& samplePool)
    {
        currentMidiNote = midiNoteNumber;
        noteVelocity = velocity;
        currentSynthesisType = synthType;

        adsrParams = {
            params.envelope.attack,
            params.envelope.decay,
            params.envelope.sustain,
            params.envelope.release
        };
        adsr.setParameters(adsrParams);
        adsr.noteOn();

        switch (currentSynthesisType)
        {
            case SynthesisType::Sample:
                sampleEngine.startNote(midiNoteNumber, velocity, sampleParams, samplePool);
                break;
            // Other cases for different synthesis types
            default:
                break;
        }
        active = true;
    }

    void SynthVoice::stopNote(float /*velocity*/, bool allowTailOff)
    {
        if (allowTailOff)
        {
            adsr.noteOff();
        }
        else
        {
            clearCurrentNote();
        }
    }

    void SynthVoice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
    {
        if (!active)
        {
            return;
        }

        juce::AudioBuffer<float> tempBuffer(outputBuffer.getNumChannels(), numSamples);
        tempBuffer.clear();

        switch (currentSynthesisType)
        {
            case SynthesisType::Sample:
                sampleEngine.processBlock(tempBuffer, 0, numSamples);
                break;
            // Other cases
            default:
                break;
        }

        adsr.applyEnvelopeToBuffer(tempBuffer, 0, numSamples);

        for (int channel = 0; channel < outputBuffer.getNumChannels(); ++channel)
        {
            outputBuffer.addFrom(channel, startSample, tempBuffer, channel, 0, numSamples);
        }

        if (!adsr.isActive())
        {
            clearCurrentNote();
        }
    }

    bool SynthVoice::isVoiceActive() const
    {
        return active;
    }

    bool SynthVoice::isPlayingNote(int midiNoteNumber) const
    {
        return active && currentMidiNote == midiNoteNumber;
    }

    void SynthVoice::clearCurrentNote()
    {
        active = false;
        currentMidiNote = -1;
    }
}
