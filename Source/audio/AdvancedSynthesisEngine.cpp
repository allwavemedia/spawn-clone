#include "AdvancedSynthesisEngine.h"
#include <cmath>

//==============================================================================
class AdvancedSynthesisEngine::WavetableOscillator
{
public:
    WavetableOscillator()
    {
        reset();
    }
    
    void setSampleRate(double sampleRate)
    {
        this->sampleRate = sampleRate;
        updateFrequency();
    }
    
    void setFrequency(float frequency)
    {
        targetFrequency = frequency;
        updateFrequency();
    }
    
    void setWavetable(const juce::AudioBuffer<float>* wavetable)
    {
        currentWavetable = wavetable;
        wavetableSize = wavetable ? wavetable->getNumSamples() : 0;
    }
    
    void setWavetablePosition(float position)
    {
        wavetablePosition = juce::jlimit(0.0f, 1.0f, position);
    }
    
    void reset()
    {
        phase = 0.0f;
        currentFrequency = 440.0f;
        targetFrequency = 440.0f;
        wavetablePosition = 0.5f;
        currentWavetable = nullptr;
        wavetableSize = 0;
    }
    
    float getNextSample()
    {
        if (!currentWavetable || wavetableSize == 0)
            return 0.0f;
        
        // Smooth frequency changes
        if (std::abs(currentFrequency - targetFrequency) > 0.1f)
        {
            currentFrequency += (targetFrequency - currentFrequency) * 0.001f;
            updateFrequency();
        }
        
        // Calculate wavetable sample with interpolation
        float sample = getInterpolatedSample();
        
        // Advance phase
        phase += phaseIncrement;
        if (phase >= 1.0f)
            phase -= 1.0f;
        
        return sample;
    }

private:
    double sampleRate = 44100.0;
    float phase = 0.0f;
    float phaseIncrement = 0.0f;
    float currentFrequency = 440.0f;
    float targetFrequency = 440.0f;
    float wavetablePosition = 0.5f;
    
    const juce::AudioBuffer<float>* currentWavetable = nullptr;
    int wavetableSize = 0;
    
    void updateFrequency()
    {
        if (sampleRate > 0.0)
            phaseIncrement = static_cast<float>(currentFrequency / sampleRate);
    }
    
    float getInterpolatedSample()
    {
        if (!currentWavetable || wavetableSize == 0)
            return 0.0f;
        
        // Calculate exact position in wavetable
        float exactPos = phase * (wavetableSize - 1);
        int index0 = static_cast<int>(exactPos);
        int index1 = (index0 + 1) % wavetableSize;
        float fraction = exactPos - index0;
        
        // Get samples from both channels (if stereo wavetable)
        const float* samples = currentWavetable->getReadPointer(0);
        
        // 4-point interpolation for high quality audio
        int indexMinus1 = (index0 - 1 + wavetableSize) % wavetableSize;
        int indexPlus2 = (index1 + 1) % wavetableSize;
        
        float y0 = samples[indexMinus1];
        float y1 = samples[index0];
        float y2 = samples[index1];
        float y3 = samples[indexPlus2];
        
        // Cubic interpolation
        float a0 = y3 - y2 - y0 + y1;
        float a1 = y0 - y1 - a0;
        float a2 = y2 - y0;
        float a3 = y1;
        
        float result = a0 * fraction * fraction * fraction + 
                      a1 * fraction * fraction + 
                      a2 * fraction + 
                      a3;
        
        return result;
    }
};

//==============================================================================
class AdvancedSynthesisEngine::AdvancedFilter
{
public:
    AdvancedFilter()
    {
        reset();
    }
    
    void setSampleRate(double sampleRate)
    {
        this->sampleRate = sampleRate;
        updateCoefficients();
    }
    
    void setType(FilterParams::Type type)
    {
        filterType = type;
        updateCoefficients();
    }
    
    void setCutoff(float cutoff)
    {
        this->cutoff = juce::jlimit(20.0f, 20000.0f, cutoff);
        updateCoefficients();
    }
    
    void setResonance(float resonance)
    {
        this->resonance = juce::jlimit(0.0f, 1.0f, resonance);
        updateCoefficients();
    }
    
    void reset()
    {
        x1 = x2 = y1 = y2 = 0.0f;
        cutoff = 1000.0f;
        resonance = 0.1f;
        filterType = FilterParams::LowPass;
        // Only update coefficients if we have a valid sample rate
        if (sampleRate > 0.0)
            updateCoefficients();
    }
    
    float processSample(float input)
    {
        float output = b0 * input + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
        
        // Shift delay line
        x2 = x1;
        x1 = input;
        y2 = y1;
        y1 = output;
        
        return output;
    }

private:
    double sampleRate = 44100.0;
    float cutoff = 1000.0f;
    float resonance = 0.1f;
    FilterParams::Type filterType = FilterParams::LowPass;
    
    // Biquad coefficients
    float b0 = 1.0f, b1 = 0.0f, b2 = 0.0f;
    float a1 = 0.0f, a2 = 0.0f;
    
    // Delay line
    float x1 = 0.0f, x2 = 0.0f;
    float y1 = 0.0f, y2 = 0.0f;
    
    void updateCoefficients()
    {
        if (sampleRate <= 0.0)
            return;
        
        float omega = 2.0f * juce::MathConstants<float>::pi * cutoff / static_cast<float>(sampleRate);
        float sin_omega = std::sin(omega);
        float cos_omega = std::cos(omega);
        float alpha = sin_omega / (2.0f * (0.5f + resonance * 9.5f)); // Q = 0.5 to 10
        
        switch (filterType)
        {
            case FilterParams::LowPass:
                b0 = (1.0f - cos_omega) / 2.0f;
                b1 = 1.0f - cos_omega;
                b2 = (1.0f - cos_omega) / 2.0f;
                break;
                
            case FilterParams::HighPass:
                b0 = (1.0f + cos_omega) / 2.0f;
                b1 = -(1.0f + cos_omega);
                b2 = (1.0f + cos_omega) / 2.0f;
                break;
                
            case FilterParams::BandPass:
                b0 = alpha;
                b1 = 0.0f;
                b2 = -alpha;
                break;
                
            case FilterParams::Notch:
                b0 = 1.0f;
                b1 = -2.0f * cos_omega;
                b2 = 1.0f;
                break;
        }
        
        float norm = 1.0f + alpha;
        b0 /= norm;
        b1 /= norm;
        b2 /= norm;
        a1 = -2.0f * cos_omega / norm;
        a2 = (1.0f - alpha) / norm;
    }
};

//==============================================================================
class AdvancedSynthesisEngine::ModulationSource
{
public:
    ModulationSource()
    {
        reset();
    }
    
    void setSampleRate(double sampleRate)
    {
        this->sampleRate = sampleRate;
        updatePhaseIncrement();
    }
    
    void setRate(float rate)
    {
        this->rate = juce::jlimit(0.01f, 20.0f, rate);
        updatePhaseIncrement();
    }
    
    void setWaveform(int waveform)
    {
        this->waveform = juce::jlimit(0, 4, waveform);
    }
    
    void setDepth(float depth)
    {
        this->depth = juce::jlimit(0.0f, 1.0f, depth);
    }
    
    void setBipolar(bool bipolar)
    {
        this->bipolar = bipolar;
    }
    
    void reset()
    {
        phase = 0.0f;
        rate = 2.0f;
        depth = 0.0f;
        waveform = 0; // Sine
        bipolar = false;
        randomValue = 0.0f;
        updatePhaseIncrement();
    }
    
    float getNextValue()
    {
        float value = 0.0f;
        
        switch (waveform)
        {
            case 0: // Sine
                value = std::sin(2.0f * juce::MathConstants<float>::pi * phase);
                break;
                
            case 1: // Triangle
                value = phase < 0.5f ? (4.0f * phase - 1.0f) : (3.0f - 4.0f * phase);
                break;
                
            case 2: // Sawtooth
                value = 2.0f * phase - 1.0f;
                break;
                
            case 3: // Square
                value = phase < 0.5f ? -1.0f : 1.0f;
                break;
                
            case 4: // Random (sample & hold)
                if (phase < phaseIncrement) // New cycle
                    randomValue = (static_cast<float>(rand()) / RAND_MAX) * 2.0f - 1.0f;
                value = randomValue;
                break;
        }
        
        // Apply depth and bipolar/unipolar scaling
        if (bipolar)
            value *= depth;
        else
            value = (value + 1.0f) * 0.5f * depth; // Convert to 0-1 range
        
        // Advance phase
        phase += phaseIncrement;
        if (phase >= 1.0f)
            phase -= 1.0f;
        
        return value;
    }

private:
    double sampleRate = 44100.0;
    float phase = 0.0f;
    float phaseIncrement = 0.0f;
    float rate = 2.0f;
    float depth = 0.0f;
    int waveform = 0;
    bool bipolar = false;
    float randomValue = 0.0f;
    
    void updatePhaseIncrement()
    {
        if (sampleRate > 0.0)
            phaseIncrement = static_cast<float>(rate / sampleRate);
    }
};

//==============================================================================
class AdvancedSynthesisEngine::SynthVoice
{
public:
    SynthVoice()
    {
        envelope.reset();
        oscillator.reset();
        filter.reset();
        lfo.reset();
        
        // Set default ADSR parameters (optimized for immediate response)
        juce::ADSR::Parameters defaultParams;
        defaultParams.attack = 0.001f;  // Very fast attack for immediate audio response
        defaultParams.decay = 0.1f;
        defaultParams.sustain = 1.0f;   // Full sustain for consistent audio output
        defaultParams.release = 0.3f;
        envelope.setParameters(defaultParams);
    }
    
    void setSampleRate(double sampleRate)
    {
        this->sampleRate = sampleRate;
        oscillator.setSampleRate(sampleRate);
        filter.setSampleRate(sampleRate);
        lfo.setSampleRate(sampleRate);
        envelope.setSampleRate(sampleRate);
        
        // Ensure envelope has valid parameters after sample rate change (optimized defaults)
        juce::ADSR::Parameters params;
        params.attack = 0.001f;   // Fast attack for immediate response
        params.decay = 0.1f;
        params.sustain = 1.0f;    // Full sustain for consistent output
        params.release = 0.3f;
        envelope.setParameters(params);
        
        // Ensure filter has proper coefficients after sample rate change
        filter.setCutoff(1000.0f);
        filter.setResonance(0.1f);
    }
    
    void noteOn(int midiNote, float velocity, const SynthesisParameters& params)
    {
        isActive = true;
        currentNote = midiNote;
        currentVelocity = velocity;
        
        // Set oscillator frequency
        float frequency = 440.0f * std::pow(2.0f, (midiNote - 69) / 12.0f);
        oscillator.setFrequency(frequency);
        
        // Apply synthesis parameters
        applySynthesisParameters(params);
        
        // Trigger envelope
        envelope.noteOn();
    }
    
    void noteOff()
    {
        envelope.noteOff();
    }
    
    bool isVoiceActive() const
    {
        return isActive && envelope.isActive();
    }
    
    float getNextSample(const SynthesisParameters& params)
    {
        if (!isActive)
            return 0.0f;
        
        // Get modulation values
        float lfoValue = lfo.getNextValue();
        float envValue = envelope.getNextSample();
        
        // Apply modulation to wavetable position
        float modulatedPosition = params.wavetable.wavetablePosition;
        if (params.modulation.modulationTarget == 4) // Wavetable position
        {
            modulatedPosition += lfoValue * params.modulation.lfoDepth;
            modulatedPosition = juce::jlimit(0.0f, 1.0f, modulatedPosition);
        }
        oscillator.setWavetablePosition(modulatedPosition);
        
        // Generate oscillator sample
        float sample = oscillator.getNextSample();
        
        // Apply filter if enabled
        if (params.filter.enabled)
        {
            float modulatedCutoff = params.filter.cutoff;
            if (params.modulation.modulationTarget == 2) // Filter cutoff
            {
                modulatedCutoff *= (1.0f + lfoValue * params.modulation.lfoDepth);
                modulatedCutoff = juce::jlimit(20.0f, 20000.0f, modulatedCutoff);
            }
            filter.setCutoff(modulatedCutoff);
            sample = filter.processSample(sample);
        }
        
        // Apply envelope
        sample *= envValue * currentVelocity * params.masterVolume;
        
        // Check if voice should be deactivated
        if (!envelope.isActive())
            isActive = false;
        
        return sample;
    }
    
    void setWavetable(const juce::AudioBuffer<float>* wavetable)
    {
        oscillator.setWavetable(wavetable);
    }
    
    int getCurrentNote() const { return currentNote; }
    
    void reset()
    {
        isActive = false;
        currentNote = -1;
        currentVelocity = 0.0f;
        envelope.reset();
        oscillator.reset();
        filter.reset();
        lfo.reset();
    }

private:
    bool isActive = false;
    int currentNote = -1;
    float currentVelocity = 0.0f;
    double sampleRate = 44100.0;
    
    WavetableOscillator oscillator;
    AdvancedFilter filter;
    ModulationSource lfo;
    juce::ADSR envelope;
    
    void applySynthesisParameters(const SynthesisParameters& params)
    {
        // Set LFO parameters
        lfo.setRate(params.modulation.lfoRate);
        lfo.setDepth(params.modulation.lfoDepth);
        lfo.setWaveform(params.modulation.lfoWaveform);
        lfo.setBipolar(params.modulation.bipolar);
        
        // Set filter parameters
        filter.setType(params.filter.filterType);
        filter.setCutoff(params.filter.cutoff);
        filter.setResonance(params.filter.resonance);
        
        // Set envelope parameters
        juce::ADSR::Parameters envParams;
        envParams.attack = params.envelope.attack;
        envParams.decay = params.envelope.decay;
        envParams.sustain = params.envelope.sustain;
        envParams.release = params.envelope.release;
        envelope.setParameters(envParams);
    }
};

//==============================================================================
AdvancedSynthesisEngine::AdvancedSynthesisEngine()
{
    // Generate built-in wavetables FIRST
    generateBuiltinWavetables();
    
    // Initialize voices AFTER wavetables are ready
    for (auto& voice : voices)
        voice = std::make_unique<SynthVoice>();
    
    // Initialize synthesis parameters with defaults
    currentParams = SynthesisParameters{}; // Use struct's default member initializers
    
    // Set default wavetable for all voices IMMEDIATELY
    if (!wavetables.empty())
    {
        const auto* defaultWavetable = &wavetables[0]; // Use sine wave as default
        for (auto& voice : voices)
        {
            if (voice)
                voice->setWavetable(defaultWavetable);
        }
    }
}

AdvancedSynthesisEngine::~AdvancedSynthesisEngine()
{
    releaseResources();
}

//==============================================================================
void AdvancedSynthesisEngine::prepareToPlay(double newSampleRate, int newSamplesPerBlock, int newNumChannels)
{
    this->sampleRate = newSampleRate;
    this->samplesPerBlock = newSamplesPerBlock;
    this->numChannels = newNumChannels;
    
    // Reset first to clear any previous state
    reset();
    
    // Prepare all voices with the correct sample rate
    for (auto& voice : voices)
        if(voice) voice->setSampleRate(newSampleRate);
    
    // Initialize synthesis parameters properly after sample rate is set
    // This ensures all components (especially filters) have correct coefficients
    setSynthesisParameters(currentParams);
}

void AdvancedSynthesisEngine::releaseResources()
{
    reset();
}

void AdvancedSynthesisEngine::reset()
{
    for (auto& voice : voices)
        if (voice) voice->reset();
    
    activeVoices.store(0);
    cpuUsage.store(0.0f);
}

void AdvancedSynthesisEngine::setSynthesisParameters(const SynthesisParameters& params)
{
    currentParams = params;
    
    // Validate parameters
    currentParams.wavetable.wavetablePosition = juce::jlimit(0.0f, 1.0f, params.wavetable.wavetablePosition);
    currentParams.wavetable.morphRate = juce::jlimit(0.01f, 10.0f, params.wavetable.morphRate);
    currentParams.wavetable.wavetableIndex = juce::jlimit(0, (int)wavetables.size() - 1, params.wavetable.wavetableIndex);
    
    // Update all voices with new wavetable if changed
    if (params.synthesisType == SynthesisType::Wavetable && 
        currentParams.wavetable.wavetableIndex >= 0 && 
        currentParams.wavetable.wavetableIndex < wavetables.size())
    {
        const auto* wavetable = &wavetables[currentParams.wavetable.wavetableIndex];
        
        for (auto& voice : voices)
        {
            if (voice)
                voice->setWavetable(wavetable);
        }
    }
    else
    {
        // Default to first wavetable if available
        if (!wavetables.empty())
        {
            const auto* wavetable = &wavetables[0];
            for (auto& voice : voices)
            {
                if (voice)
                    voice->setWavetable(wavetable);
            }
        }
    }
}

void AdvancedSynthesisEngine::setParameter(const juce::String& paramName, float value)
{
    // Thread-safe parameter updates
    if (paramName == "wavetablePosition")
        currentParams.wavetable.wavetablePosition = juce::jlimit(0.0f, 1.0f, value);
    else if (paramName == "filterCutoff")
        currentParams.filter.cutoff = juce::jlimit(20.0f, 20000.0f, value);
    else if (paramName == "filterResonance")
        currentParams.filter.resonance = juce::jlimit(0.0f, 1.0f, value);
    else if (paramName == "lfoRate")
        currentParams.modulation.lfoRate = juce::jlimit(0.01f, 20.0f, value);
    else if (paramName == "lfoDepth")
        currentParams.modulation.lfoDepth = juce::jlimit(0.0f, 1.0f, value);
    else if (paramName == "masterVolume")
        currentParams.masterVolume = juce::jlimit(0.0f, 1.0f, value);
}

void AdvancedSynthesisEngine::noteOn(int midiNoteNumber, float velocity)
{
    auto* voice = findAvailableVoice();
    
    if (voice)
    {
        voice->noteOn(midiNoteNumber, velocity, currentParams);
        
        // Update active voice count by counting currently active voices
        int count = 0;
        for (auto& v : voices)
        {
            if (v && v->isVoiceActive())
                count++;
        }
        activeVoices.store(count);
    }
}

void AdvancedSynthesisEngine::noteOff(int midiNoteNumber)
{
    for (auto& voice : voices)
    {
        if (voice && voice->getCurrentNote() == midiNoteNumber)
        {
            voice->noteOff();
            // Update active voice count by counting currently active voices
            int count = 0;
            for (auto& v : voices)
            {
                if (v && v->isVoiceActive())
                    count++;
            }
            activeVoices.store(count);
            break;
        }
    }
}

void AdvancedSynthesisEngine::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiBuffer)
{
    isProcessing.store(true);
    auto startTime = juce::Time::getCurrentTime();
    
    buffer.clear();
    
    // Process MIDI events
    for (const auto metadata : midiBuffer)
    {
        auto message = metadata.getMessage();
        if (message.isNoteOn())
            noteOn(message.getNoteNumber(), message.getFloatVelocity());
        else if (message.isNoteOff())
            noteOff(message.getNoteNumber());
    }
    
    // Generate audio samples
    int currentActiveVoices = 0;
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        float mixedSample = 0.0f;
        
        // Sum all active voices and count them
        if (sample == 0) // Only count on first sample to avoid overhead
        {
            currentActiveVoices = 0;
            for (auto& voice : voices)
            {
                if (voice->isVoiceActive())
                {
                    currentActiveVoices++;
                    mixedSample += voice->getNextSample(currentParams);
                }
            }
            activeVoices.store(currentActiveVoices);
        }
        else
        {
            // Just process samples without counting
            for (auto& voice : voices)
            {
                if (voice->isVoiceActive())
                    mixedSample += voice->getNextSample(currentParams);
            }
        }
        
        // Apply to all channels
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            buffer.setSample(channel, sample, mixedSample);
    }
    
    // Update performance statistics
    auto endTime = juce::Time::getCurrentTime();
    auto processingTime = endTime - startTime;
    float blockDuration = static_cast<float>(samplesPerBlock) / static_cast<float>(sampleRate);
    cpuUsage.store((processingTime.inSeconds() / blockDuration) * 100.0f);
    
    isProcessing.store(false);
}

bool AdvancedSynthesisEngine::loadWavetable(int index, const juce::AudioBuffer<float>& wavetableData)
{
    if (index < 0 || index >= MAX_WAVETABLES)
        return false;
    
    if (index >= wavetables.size())
        wavetables.resize(index + 1);
    
    wavetables[index] = wavetableData;
    return true;
}

void AdvancedSynthesisEngine::generateBuiltinWavetables()
{
    wavetables.clear();
    wavetables.resize(8); // 8 built-in wavetables
    
    // Create basic waveforms
    for (int i = 0; i < 8; ++i)
    {
        wavetables[i].setSize(1, WAVETABLE_SIZE);
        auto* samples = wavetables[i].getWritePointer(0);
        
        for (int sample = 0; sample < WAVETABLE_SIZE; ++sample)
        {
            float phase = static_cast<float>(sample) / WAVETABLE_SIZE;
            
            switch (i)
            {
                case 0: // Sine
                    samples[sample] = std::sin(2.0f * juce::MathConstants<float>::pi * phase);
                    break;
                case 1: // Sawtooth
                    samples[sample] = 2.0f * phase - 1.0f;
                    break;
                case 2: // Square
                    samples[sample] = phase < 0.5f ? -1.0f : 1.0f;
                    break;
                case 3: // Triangle
                    samples[sample] = phase < 0.5f ? (4.0f * phase - 1.0f) : (3.0f - 4.0f * phase);
                    break;
                case 4: // Formant-like wave
                    samples[sample] = std::sin(2.0f * juce::MathConstants<float>::pi * phase) * 
                                    std::sin(6.0f * juce::MathConstants<float>::pi * phase);
                    break;
                case 5: // Harmonic series
                    samples[sample] = std::sin(2.0f * juce::MathConstants<float>::pi * phase) * 0.5f +
                                    std::sin(4.0f * juce::MathConstants<float>::pi * phase) * 0.25f +
                                    std::sin(6.0f * juce::MathConstants<float>::pi * phase) * 0.125f;
                    break;
                case 6: // Noise-modulated sine
                    samples[sample] = std::sin(2.0f * juce::MathConstants<float>::pi * phase) * 
                                    (0.8f + 0.2f * std::sin(16.0f * juce::MathConstants<float>::pi * phase));
                    break;
                case 7: // Complex wave
                    samples[sample] = std::sin(2.0f * juce::MathConstants<float>::pi * phase) * 0.4f +
                                    std::sin(3.0f * juce::MathConstants<float>::pi * phase) * 0.3f +
                                    std::sin(5.0f * juce::MathConstants<float>::pi * phase) * 0.2f +
                                    std::sin(7.0f * juce::MathConstants<float>::pi * phase) * 0.1f;
                    break;
            }
        }
    }
}

AdvancedSynthesisEngine::SynthVoice* AdvancedSynthesisEngine::findAvailableVoice()
{
    // First, look for an inactive voice
    for (auto& voice : voices)
    {
        if (voice && !voice->isVoiceActive())
            return voice.get();
    }
    
    // If no inactive voice, use round-robin selection
    auto* voice = voices[currentVoiceIndex].get();
    currentVoiceIndex = (currentVoiceIndex + 1) % voices.size();
    return voice;
}

void AdvancedSynthesisEngine::initializeWavetables()
{
    generateBuiltinWavetables();
}

void AdvancedSynthesisEngine::createBasicWavetable(int index, const juce::String& name)
{
    // This is handled by generateBuiltinWavetables()
}

void AdvancedSynthesisEngine::updateCPUUsage()
{
    // CPU usage is updated in processBlock()
}

juce::String AdvancedSynthesisEngine::getEngineInfo() const
{
    juce::String info;
    info << "Advanced Synthesis Engine v1.0\n";
    info << "Sample Rate: " << sampleRate << " Hz\n";
    info << "Block Size: " << samplesPerBlock << " samples\n";
    info << "Active Voices: " << activeVoices.load() << "/16\n";
    info << "CPU Usage: " << juce::String(cpuUsage.load(), 1) << "%\n";
    info << "Wavetables: " << wavetables.size() << " loaded\n";
    info << "Synthesis Type: ";
    
    switch (currentParams.synthesisType)
    {
        case SynthesisType::Wavetable: info << "Wavetable"; break;
        case SynthesisType::Subtractive: info << "Subtractive"; break;
        case SynthesisType::Sample: info << "Sample-based"; break;
        case SynthesisType::Hybrid: info << "Hybrid"; break;
    }
    
    return info;
}
