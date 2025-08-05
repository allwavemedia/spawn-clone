//==============================================================================
/*
  SynthVoice.cpp
  Author:  BMad Dev Agent
*/
//==============================================================================

#include "SynthVoice.h"
#include "AdvancedSynthesisEngine.h"  // For complete SynthesisParameters definition
#include <cmath>

namespace spawnclone::audio
{
    // Lightweight wavetable oscillator for SynthVoice
    class SynthVoice::WavetableOscillator
    {
    public:
        WavetableOscillator() = default;
        
        void setSampleRate(double sampleRate) 
        { 
            this->sampleRate = sampleRate; 
        }
        
        void setFrequency(float frequency) 
        { 
            this->frequency = frequency;
            if (sampleRate > 0.0)
                phaseIncrement = frequency / sampleRate;
        }
        
        void setWavetable(const juce::AudioBuffer<float>* wavetable)
        {
            this->wavetable = wavetable;
        }
        
        void reset() 
        { 
            phase = 0.0; 
        }
        
        float getNextSample()
        {
            if (!wavetable || wavetable->getNumSamples() == 0)
                return 0.0f;
            
            const float* samples = wavetable->getReadPointer(0);
            int tableSize = wavetable->getNumSamples();
            
            // Linear interpolation
            float exactPos = phase * (tableSize - 1);
            int index0 = static_cast<int>(exactPos);
            int index1 = (index0 + 1) % tableSize;
            float fraction = exactPos - index0;
            
            float sample = samples[index0] * (1.0f - fraction) + samples[index1] * fraction;
            
            phase += phaseIncrement;
            if (phase >= 1.0)
                phase -= 1.0;
            
            return sample;
        }
        
    private:
        double sampleRate = 44100.0;
        double phase = 0.0;
        double phaseIncrement = 0.0;
        float frequency = 440.0f;
        const juce::AudioBuffer<float>* wavetable = nullptr;
    };

    // LFO oscillator for modulation
    class SynthVoice::LFOscillator
    {
    public:
        LFOscillator() = default;
        
        void setSampleRate(double sampleRate)
        {
            this->sampleRate = sampleRate;
            updatePhaseIncrement();
        }
        
        void setFrequency(float frequency)
        {
            this->frequency = frequency;
            updatePhaseIncrement();
        }
        
        void setWaveform(int waveform)
        {
            this->waveform = juce::jlimit(0, 4, waveform);
        }
        
        void reset()
        {
            phase = 0.0;
        }
        
        float getNextSample()
        {
            float sample = 0.0f;
            
            switch (waveform)
            {
                case 0: // Sine
                    sample = std::sin(phase * 2.0 * juce::MathConstants<double>::pi);
                    break;
                case 1: // Triangle
                    sample = phase < 0.5 ? (4.0f * phase - 1.0f) : (3.0f - 4.0f * phase);
                    break;
                case 2: // Sawtooth
                    sample = 2.0f * phase - 1.0f;
                    break;
                case 3: // Square
                    sample = phase < 0.5f ? -1.0f : 1.0f;
                    break;
                case 4: // Random (noise)
                    sample = random.nextFloat() * 2.0f - 1.0f;
                    break;
            }
            
            phase += phaseIncrement;
            if (phase >= 1.0)
                phase -= 1.0;
            
            return sample;
        }
        
    private:
        double sampleRate = 44100.0;
        double phase = 0.0;
        double phaseIncrement = 0.0;
        float frequency = 2.0f;
        int waveform = 0; // 0=Sine, 1=Triangle, 2=Sawtooth, 3=Square, 4=Random
        juce::Random random;
        
        void updatePhaseIncrement()
        {
            if (sampleRate > 0.0)
                phaseIncrement = frequency / sampleRate;
        }
    };
    SynthVoice::SynthVoice(SampleCache& cache)
        : sampleEngine(cache)
    {
        wavetableOscillator = std::make_unique<WavetableOscillator>();
        lfo1 = std::make_unique<LFOscillator>();
        lfo2 = std::make_unique<LFOscillator>();
    }

    SynthVoice::~SynthVoice() = default;

    void SynthVoice::prepareToPlay(double sampleRate, int samplesPerBlock)
    {
        voiceSampleRate = sampleRate;
        adsr.setSampleRate(sampleRate);
        sampleEngine.prepareToPlay(sampleRate, samplesPerBlock);
        wavetableOscillator->setSampleRate(sampleRate);
        lfo1->setSampleRate(sampleRate);
        lfo2->setSampleRate(sampleRate);
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

        // Use default envelope parameters for now
        adsrParams = {
            0.1f,  // attack
            0.3f,  // decay
            0.7f,  // sustain
            0.5f   // release
        };
        adsr.setParameters(adsrParams);
        adsr.noteOn();

        switch (currentSynthesisType)
        {
            case SynthesisType::Sample:
                sampleEngine.startNote(midiNoteNumber, velocity, sampleParams, samplePool);
                break;
            case SynthesisType::Wavetable:
            case SynthesisType::Subtractive:
            case SynthesisType::Hybrid:
                // Configure wavetable oscillator
                {
                    double frequency = 440.0 * std::pow(2.0, (midiNoteNumber - 69) / 12.0);
                    wavetableOscillator->setFrequency(static_cast<float>(frequency));
                    wavetableOscillator->reset();
                    lfo1->reset();
                    lfo2->reset();
                }
                break;
            default:
                break;
        }
        active = true;
    }

    void SynthVoice::setWavetable(const juce::AudioBuffer<float>* wavetable)
    {
        wavetableOscillator->setWavetable(wavetable);
    }

    void SynthVoice::setModulationParameters(const AdvancedSynthesisEngine::ModulationParams& modParams)
    {
        currentModulationParams = modParams;
        
        // Configure LFO1
        lfo1->setFrequency(modParams.lfoRate);
        lfo1->setWaveform(modParams.lfoWaveform);
        
        // Configure LFO2
        lfo2->setFrequency(modParams.lfo2Rate);
        lfo2->setWaveform(modParams.lfo2Waveform);
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
            case SynthesisType::Wavetable:
            case SynthesisType::Subtractive:
            case SynthesisType::Hybrid:
                // Use wavetable oscillator for high-quality synthesis
                generateWavetableAudio(tempBuffer, numSamples);
                break;
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
        oscillatorPhase = 0.0;
    }

    void SynthVoice::generateWavetableAudio(juce::AudioBuffer<float>& buffer, int numSamples)
    {
        if (currentMidiNote < 0) return;
        
        // Base frequency from MIDI note
        double baseFrequency = 440.0 * std::pow(2.0, (currentMidiNote - 69) / 12.0);
        
        // Generate wavetable audio with modulation
        for (int sample = 0; sample < numSamples; ++sample)
        {
            // Get LFO values
            float lfo1Value = lfo1->getNextSample();
            float lfo2Value = lfo2->getNextSample();
            
            // Apply modulation based on target
            float modulatedFrequency = static_cast<float>(baseFrequency);
            
            // LFO1 modulation
            if (currentModulationParams.modulationTarget == 1) // Pitch modulation
            {
                float pitchMod = lfo1Value * currentModulationParams.lfoDepth;
                if (!currentModulationParams.bipolar)
                    pitchMod = (pitchMod + 1.0f) * 0.5f * currentModulationParams.lfoDepth;
                    
                // Convert to frequency modulation (±1 semitone max)
                float semitoneShift = pitchMod * 1.0f; // Max ±1 semitone
                modulatedFrequency *= std::pow(2.0f, semitoneShift / 12.0f);
            }
            
            // Update oscillator frequency if modulated
            if (currentModulationParams.modulationTarget == 1)
            {
                wavetableOscillator->setFrequency(modulatedFrequency);
            }
            
            // Generate base wavetable sample
            float wavetableSample = wavetableOscillator->getNextSample();
            
            // Apply amplitude modulation if needed
            if (currentModulationParams.modulationTarget == 3) // Amplitude modulation
            {
                float ampMod = lfo1Value * currentModulationParams.lfoDepth;
                if (!currentModulationParams.bipolar)
                    ampMod = (ampMod + 1.0f) * 0.5f;
                else
                    ampMod = ampMod * 0.5f + 0.5f; // Convert bipolar to 0-1 range
                    
                wavetableSample *= (1.0f - currentModulationParams.lfoDepth + ampMod * currentModulationParams.lfoDepth);
            }
            
            // Apply velocity and final scaling
            wavetableSample *= noteVelocity * 0.3f;
            
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            {
                buffer.setSample(channel, sample, wavetableSample);
            }
        }
    }

    void SynthVoice::generateBasicOscillator(juce::AudioBuffer<float>& buffer, int numSamples)
    {
        if (currentMidiNote < 0) return;
        
        // Calculate frequency from MIDI note
        double frequency = 440.0 * std::pow(2.0, (currentMidiNote - 69) / 12.0);
        double phaseIncrement = frequency / voiceSampleRate;
        
        // Generate sine wave
        for (int sample = 0; sample < numSamples; ++sample)
        {
            float sineValue = std::sin(oscillatorPhase * 2.0 * juce::MathConstants<double>::pi) * noteVelocity * 0.3f;
            
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            {
                buffer.setSample(channel, sample, sineValue);
            }
            
            oscillatorPhase += phaseIncrement;
            if (oscillatorPhase >= 1.0)
                oscillatorPhase -= 1.0;
        }
    }
}
