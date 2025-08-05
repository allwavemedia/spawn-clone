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
        
        void setPhaseOffset(float offset)
        {
            phaseOffset = juce::jlimit(0.0f, 1.0f, offset);
            phase = phaseOffset;
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
        float phaseOffset = 0.0f;
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

    // State Variable Filter for audio processing
    class SynthVoice::StateVariableFilter
    {
    public:
        StateVariableFilter() = default;
        
        void setSampleRate(double sampleRate)
        {
            this->sampleRate = sampleRate;
            updateCoefficients();
        }
        
        void setCutoff(float cutoff)
        {
            this->cutoff = juce::jlimit(20.0f, 20000.0f, cutoff);
            updateCoefficients();
        }
        
        void setResonance(float resonance)
        {
            this->resonance = juce::jlimit(0.0f, 0.99f, resonance);
            updateCoefficients();
        }
        
        void setFilterType(int type)
        {
            this->filterType = juce::jlimit(0, 5, type);
        }
        
        void reset()
        {
            lowpass = 0.0f;
            bandpass = 0.0f;
            highpass = 0.0f;
        }
        
        float processSample(float input)
        {
            // State variable filter implementation
            lowpass = lowpass + f * bandpass;
            highpass = input - lowpass - q * bandpass;
            bandpass = f * highpass + bandpass;
            
            // Return appropriate output based on filter type
            switch (filterType)
            {
                case 0: return lowpass;     // LowPass
                case 1: return highpass;    // HighPass
                case 2: return bandpass;    // BandPass
                case 3: return input - bandpass; // Notch
                case 4: // MoogLadder (simplified)
                    return lowpass * (1.0f + resonance * 0.5f);
                case 5: // StateVariable (mix of all)
                    return lowpass * 0.5f + bandpass * 0.3f + highpass * 0.2f;
                default:
                    return input;
            }
        }
        
        void processBlock(juce::AudioBuffer<float>& buffer, int startSample, int numSamples)
        {
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            {
                auto* channelData = buffer.getWritePointer(channel);
                
                for (int sample = startSample; sample < startSample + numSamples; ++sample)
                {
                    channelData[sample] = processSample(channelData[sample]);
                }
            }
        }
        
    private:
        double sampleRate = 44100.0;
        float cutoff = 1000.0f;
        float resonance = 0.1f;
        int filterType = 0; // 0=LowPass
        
        // Filter coefficients
        float f = 0.0f;     // Frequency coefficient
        float q = 0.0f;     // Resonance coefficient
        
        // Filter state variables
        float lowpass = 0.0f;
        float bandpass = 0.0f;
        float highpass = 0.0f;
        
        void updateCoefficients()
        {
            if (sampleRate > 0.0)
            {
                f = 2.0f * std::sin(juce::MathConstants<float>::pi * cutoff / static_cast<float>(sampleRate));
                q = resonance;
            }
        }
    };

    // Advanced envelope with curve shapes
    class SynthVoice::AdvancedEnvelope
    {
    public:
        AdvancedEnvelope() = default;
        
        void setSampleRate(double sampleRate)
        {
            this->sampleRate = sampleRate;
        }
        
        void setParameters(const AdvancedSynthesisEngine::EnvelopeParams& params)
        {
            this->params = params;
        }
        
        void noteOn()
        {
            state = Attack;
            level = 0.0f;
            phasePosition = 0.0f;
        }
        
        void noteOff()
        {
            if (state != Idle)
            {
                state = Release;
                phasePosition = 0.0f;
                releaseStartLevel = level;
            }
        }
        
        bool isActive() const
        {
            return state != Idle;
        }
        
        float getNextSample()
        {
            if (state == Idle)
                return 0.0f;
            
            float phaseDelta = 1.0f / static_cast<float>(sampleRate);
            
            switch (state)
            {
                case Idle:
                    return 0.0f;
                case Attack:
                {
                    if (params.attack > 0.001f)
                    {
                        phasePosition += phaseDelta / params.attack;
                        if (phasePosition >= 1.0f)
                        {
                            phasePosition = 0.0f;
                            state = Decay;
                            level = 1.0f;
                        }
                        else
                        {
                            level = applyCurve(phasePosition, params.attackShape);
                        }
                    }
                    else
                    {
                        level = 1.0f;
                        state = Decay;
                        phasePosition = 0.0f;
                    }
                    break;
                }
                case Decay:
                {
                    if (params.decay > 0.001f)
                    {
                        phasePosition += phaseDelta / params.decay;
                        if (phasePosition >= 1.0f)
                        {
                            state = Sustain;
                            level = params.sustain;
                        }
                        else
                        {
                            float decayProgress = applyCurve(phasePosition, params.decayShape);
                            level = 1.0f - decayProgress * (1.0f - params.sustain);
                        }
                    }
                    else
                    {
                        level = params.sustain;
                        state = Sustain;
                    }
                    break;
                }
                case Sustain:
                    level = params.sustain;
                    break;
                case Release:
                {
                    if (params.release > 0.001f)
                    {
                        phasePosition += phaseDelta / params.release;
                        if (phasePosition >= 1.0f)
                        {
                            state = Idle;
                            level = 0.0f;
                        }
                        else
                        {
                            float releaseProgress = applyCurve(phasePosition, params.releaseShape);
                            level = releaseStartLevel * (1.0f - releaseProgress);
                        }
                    }
                    else
                    {
                        state = Idle;
                        level = 0.0f;
                    }
                    break;
                }
            }
            
            return level;
        }
        
        void processBlock(juce::AudioBuffer<float>& buffer, int startSample, int numSamples)
        {
            for (int sample = startSample; sample < startSample + numSamples; ++sample)
            {
                float envLevel = getNextSample();
                for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
                {
                    buffer.setSample(channel, sample, buffer.getSample(channel, sample) * envLevel);
                }
            }
        }
        
    private:
        enum State { Idle, Attack, Decay, Sustain, Release };
        State state = Idle;
        
        double sampleRate = 44100.0;
        AdvancedSynthesisEngine::EnvelopeParams params;
        float level = 0.0f;
        float phasePosition = 0.0f;
        float releaseStartLevel = 0.0f;
        
        float applyCurve(float position, AdvancedSynthesisEngine::EnvelopeParams::Shape shape)
        {
            switch (shape)
            {
                case AdvancedSynthesisEngine::EnvelopeParams::Linear:
                    return position;
                case AdvancedSynthesisEngine::EnvelopeParams::Exponential:
                    return 1.0f - std::exp(-5.0f * position);
                case AdvancedSynthesisEngine::EnvelopeParams::Logarithmic:
                    return std::log(1.0f + 9.0f * position) / std::log(10.0f);
                case AdvancedSynthesisEngine::EnvelopeParams::SCurve:
                    return 0.5f * (1.0f + std::sin(juce::MathConstants<float>::pi * (position - 0.5f)));
                default:
                    return position;
            }
        }
    };
    SynthVoice::SynthVoice(SampleCache& cache)
        : sampleEngine(cache)
    {
        wavetableOscillator = std::make_unique<WavetableOscillator>();
        lfo1 = std::make_unique<LFOscillator>();
        lfo2 = std::make_unique<LFOscillator>();
        filter = std::make_unique<StateVariableFilter>();
        advancedEnvelope = std::make_unique<AdvancedEnvelope>();
        
        // Initialize unison oscillators
        for (int i = 0; i < MAX_UNISON_VOICES; ++i)
        {
            unisonOscillators[i] = std::make_unique<WavetableOscillator>();
            unisonPanning[i] = 0.0f;
            unisonDetune[i] = 0.0f;
        }
        
        // Set default unison parameters
        currentUnisonParams.enabled = false;
        currentUnisonParams.voiceCount = 2;
        currentUnisonParams.detune = 0.1f;
        currentUnisonParams.stereoSpread = 0.5f;
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
        filter->setSampleRate(sampleRate);
        advancedEnvelope->setSampleRate(sampleRate);
        
        // Initialize unison oscillators
        for (int i = 0; i < MAX_UNISON_VOICES; ++i)
        {
            unisonOscillators[i]->setSampleRate(sampleRate);
        }
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

        // Use advanced envelope if available, otherwise fall back to ADSR
        if (currentEnvelopeParams.attack > 0.0f || currentEnvelopeParams.decay > 0.0f)
        {
            advancedEnvelope->setParameters(currentEnvelopeParams);
            advancedEnvelope->noteOn();
        }
        else
        {
            // Use default envelope parameters for compatibility
            adsrParams = {
                0.1f,  // attack
                0.3f,  // decay
                0.7f,  // sustain
                0.5f   // release
            };
            adsr.setParameters(adsrParams);
            adsr.noteOn();
        }

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
                    double baseFrequency = 440.0 * std::pow(2.0, (midiNoteNumber - 69) / 12.0);
                    wavetableOscillator->setFrequency(static_cast<float>(baseFrequency));
                    wavetableOscillator->reset();
                    lfo1->reset();
                    lfo2->reset();
                    
                    // Initialize unison oscillators if enabled
                    if (currentUnisonParams.enabled && currentUnisonParams.voiceCount > 1)
                    {
                        int activeVoices = juce::jlimit(2, MAX_UNISON_VOICES, currentUnisonParams.voiceCount);
                        
                        for (int i = 0; i < activeVoices; ++i)
                        {
                            // Apply detuning to each unison voice
                            float detuneMultiplier = std::pow(2.0f, unisonDetune[i] / 12.0f);
                            float detunedFrequency = static_cast<float>(baseFrequency) * detuneMultiplier;
                            
                            unisonOscillators[i]->setFrequency(detunedFrequency);
                            unisonOscillators[i]->reset();
                            
                            // Add phase offset for thickness
                            if (currentUnisonParams.phaseOffset > 0.0f)
                            {
                                float randomPhase = static_cast<float>(i) / static_cast<float>(activeVoices) * currentUnisonParams.phaseOffset;
                                unisonOscillators[i]->setPhaseOffset(randomPhase);
                            }
                        }
                    }
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
        
        // Also set wavetable for all unison oscillators
        for (int i = 0; i < MAX_UNISON_VOICES; ++i)
        {
            unisonOscillators[i]->setWavetable(wavetable);
        }
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

    void SynthVoice::setFilterParameters(const AdvancedSynthesisEngine::FilterParams& filterParams)
    {
        currentFilterParams = filterParams;
        
        // Configure the filter
        filter->setCutoff(filterParams.cutoff);
        filter->setResonance(filterParams.resonance);
        filter->setFilterType(static_cast<int>(filterParams.filterType));
    }

    void SynthVoice::setUnisonParameters(const AdvancedSynthesisEngine::UnisonParams& unisonParams)
    {
        currentUnisonParams = unisonParams;
        
        // Calculate unison voice detuning and panning
        if (unisonParams.enabled && unisonParams.voiceCount > 1)
        {
            int activeVoices = juce::jlimit(2, MAX_UNISON_VOICES, unisonParams.voiceCount);
            
            for (int i = 0; i < activeVoices; ++i)
            {
                if (activeVoices == 1)
                {
                    unisonDetune[i] = 0.0f;
                    unisonPanning[i] = 0.0f;
                }
                else
                {
                    // Spread detune values symmetrically around center
                    float voicePosition = static_cast<float>(i) / static_cast<float>(activeVoices - 1);
                    unisonDetune[i] = (voicePosition - 0.5f) * 2.0f * unisonParams.detune;
                    
                    // Spread panning across stereo field
                    unisonPanning[i] = (voicePosition - 0.5f) * unisonParams.stereoSpread;
                }
            }
            
            // Clear unused voices
            for (int i = activeVoices; i < MAX_UNISON_VOICES; ++i)
            {
                unisonDetune[i] = 0.0f;
                unisonPanning[i] = 0.0f;
            }
        }
    }

    void SynthVoice::setEnvelopeParameters(const AdvancedSynthesisEngine::EnvelopeParams& envParams)
    {
        currentEnvelopeParams = envParams;
        advancedEnvelope->setParameters(envParams);
    }

    void SynthVoice::stopNote(float /*velocity*/, bool allowTailOff)
    {
        if (allowTailOff)
        {
            adsr.noteOff();
            advancedEnvelope->noteOff();
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

        // Apply envelope (advanced or standard ADSR)
        if (advancedEnvelope->isActive())
        {
            advancedEnvelope->processBlock(tempBuffer, 0, numSamples);
        }
        else
        {
            adsr.applyEnvelopeToBuffer(tempBuffer, 0, numSamples);
        }

        // Apply filter processing
        filter->processBlock(tempBuffer, 0, numSamples);

        for (int channel = 0; channel < outputBuffer.getNumChannels(); ++channel)
        {
            outputBuffer.addFrom(channel, startSample, tempBuffer, channel, 0, numSamples);
        }

        if (!adsr.isActive() && !advancedEnvelope->isActive())
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
        
        // Performance optimization: pre-calculate LFO values if needed
        bool hasLFOModulation = (currentModulationParams.modulationTarget != 0 && currentModulationParams.lfoDepth > 0.0f);
        
        if (currentUnisonParams.enabled && currentUnisonParams.voiceCount > 1)
        {
            // Generate unison audio with multiple detuned oscillators
            generateUnisonAudio(buffer, numSamples, baseFrequency);
        }
        else
        {
            // Generate single oscillator audio (optimized path)
            generateSingleOscillatorAudio(buffer, numSamples, baseFrequency, hasLFOModulation);
        }
    }
    
    void SynthVoice::generateUnisonAudio(juce::AudioBuffer<float>& buffer, int numSamples, double baseFrequency)
    {
        int activeVoices = juce::jlimit(1, MAX_UNISON_VOICES, currentUnisonParams.voiceCount);
        float voiceGain = 1.0f / std::sqrt(static_cast<float>(activeVoices)); // Compensate for loudness
        
        // Temporary buffers for each unison voice
        juce::AudioBuffer<float> unisonBuffer(buffer.getNumChannels(), numSamples);
        
        for (int voice = 0; voice < activeVoices; ++voice)
        {
            unisonBuffer.clear();
            
            // Calculate detuned frequency for this voice
            float detuneMultiplier = std::pow(2.0f, unisonDetune[voice] / 12.0f);
            float detunedFrequency = static_cast<float>(baseFrequency) * detuneMultiplier;
            
            // Generate audio for this unison voice
            for (int sample = 0; sample < numSamples; ++sample)
            {
                // Get LFO values for modulation
                float lfo1Value = lfo1->getNextSample();
                float lfo2Value = lfo2->getNextSample();
                
                // Apply modulation
                float modulatedFrequency = detunedFrequency;
                if (currentModulationParams.modulationTarget == 1) // Pitch modulation
                {
                    float pitchMod = lfo1Value * currentModulationParams.lfoDepth;
                    if (!currentModulationParams.bipolar)
                        pitchMod = (pitchMod + 1.0f) * 0.5f * currentModulationParams.lfoDepth;
                    
                    float semitoneShift = pitchMod * 1.0f;
                    modulatedFrequency *= std::pow(2.0f, semitoneShift / 12.0f);
                }
                
                // Update oscillator frequency
                unisonOscillators[voice]->setFrequency(modulatedFrequency);
                
                // Generate sample
                float wavetableSample = unisonOscillators[voice]->getNextSample();
                
                // Apply amplitude modulation if needed
                if (currentModulationParams.modulationTarget == 3) // Amplitude modulation
                {
                    float ampMod = lfo1Value * currentModulationParams.lfoDepth;
                    if (!currentModulationParams.bipolar)
                        ampMod = (ampMod + 1.0f) * 0.5f;
                    else
                        ampMod = ampMod * 0.5f + 0.5f;
                    
                    wavetableSample *= (1.0f - currentModulationParams.lfoDepth + ampMod * currentModulationParams.lfoDepth);
                }
                
                // Apply voice gain and velocity
                wavetableSample *= voiceGain * noteVelocity * 0.3f;
                
                // Store in temporary buffer
                for (int channel = 0; channel < unisonBuffer.getNumChannels(); ++channel)
                {
                    unisonBuffer.setSample(channel, sample, wavetableSample);
                }
            }
            
            // Apply stereo panning and add to main buffer
            float panPosition = unisonPanning[voice]; // -1 to 1
            float leftGain = std::cos((panPosition + 1.0f) * juce::MathConstants<float>::pi * 0.25f);
            float rightGain = std::sin((panPosition + 1.0f) * juce::MathConstants<float>::pi * 0.25f);
            
            if (buffer.getNumChannels() >= 2)
            {
                buffer.addFrom(0, 0, unisonBuffer, 0, 0, numSamples, leftGain);
                buffer.addFrom(1, 0, unisonBuffer, 0, 0, numSamples, rightGain);
            }
            else
            {
                buffer.addFrom(0, 0, unisonBuffer, 0, 0, numSamples);
            }
        }
    }

    void SynthVoice::generateSingleOscillatorAudio(juce::AudioBuffer<float>& buffer, int numSamples, double baseFrequency, bool hasLFOModulation)
    {
        // Optimized single oscillator path
        if (!hasLFOModulation)
        {
            // No modulation - most efficient path
            wavetableOscillator->setFrequency(static_cast<float>(baseFrequency));
            
            for (int sample = 0; sample < numSamples; ++sample)
            {
                float wavetableSample = wavetableOscillator->getNextSample() * noteVelocity * 0.3f;
                
                for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
                {
                    buffer.setSample(channel, sample, wavetableSample);
                }
            }
        }
        else
        {
            // With modulation - calculate per sample
            for (int sample = 0; sample < numSamples; ++sample)
            {
                float lfo1Value = lfo1->getNextSample();
                float modulatedFrequency = static_cast<float>(baseFrequency);
                
                if (currentModulationParams.modulationTarget == 1) // Pitch modulation
                {
                    float pitchMod = lfo1Value * currentModulationParams.lfoDepth;
                    if (!currentModulationParams.bipolar)
                        pitchMod = (pitchMod + 1.0f) * 0.5f * currentModulationParams.lfoDepth;
                    
                    float semitoneShift = pitchMod * 1.0f;
                    modulatedFrequency *= std::pow(2.0f, semitoneShift / 12.0f);
                }
                
                wavetableOscillator->setFrequency(modulatedFrequency);
                float wavetableSample = wavetableOscillator->getNextSample();
                
                if (currentModulationParams.modulationTarget == 3) // Amplitude modulation
                {
                    float ampMod = lfo1Value * currentModulationParams.lfoDepth;
                    if (!currentModulationParams.bipolar)
                        ampMod = (ampMod + 1.0f) * 0.5f;
                    else
                        ampMod = ampMod * 0.5f + 0.5f;
                    
                    wavetableSample *= (1.0f - currentModulationParams.lfoDepth + ampMod * currentModulationParams.lfoDepth);
                }
                
                wavetableSample *= noteVelocity * 0.3f;
                
                for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
                {
                    buffer.setSample(channel, sample, wavetableSample);
                }
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
