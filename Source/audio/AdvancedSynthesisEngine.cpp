#include "SynthVoice.h"
#include "AdvancedSynthesisEngine.h"
#include "SampleCache.h"
#include "AsyncSampleLoader.h"
#include <cmath>

namespace spawnclone::audio
{

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
                
            // Note: MoogLadder and StateVariable are handled by separate filter classes
            default:
                // Default to low-pass for unknown types
                b0 = (1.0f - cos_omega) / 2.0f;
                b1 = 1.0f - cos_omega;
                b2 = (1.0f - cos_omega) / 2.0f;
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
class AdvancedSynthesisEngine::MoogLadderFilter
{
public:
    MoogLadderFilter()
    {
        reset();
    }
    
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
        this->resonance = juce::jlimit(0.0f, 1.0f, resonance);
        updateCoefficients();
    }
    
    void setSelfOscillation(bool enabled)
    {
        selfOscillation = enabled;
        updateCoefficients();
    }
    
    void reset()
    {
        stage1 = stage2 = stage3 = stage4 = 0.0f;
        cutoff = 1000.0f;
        resonance = 0.1f;
        selfOscillation = false;
        updateCoefficients();
    }
    
    float processSample(float input)
    {
        // Apply resonance feedback
        float feedback = (stage4 * 4.0f * resonanceAmount);
        if (selfOscillation)
            feedback *= 1.2f; // Allow self-oscillation
        
        float modifiedInput = input - feedback;
        
        // 4-stage ladder filter (each stage is a simple RC low-pass)
        stage1 += (modifiedInput - stage1) * cutoffCoeff;
        stage2 += (stage1 - stage2) * cutoffCoeff;
        stage3 += (stage2 - stage3) * cutoffCoeff;
        stage4 += (stage3 - stage4) * cutoffCoeff;
        
        // Apply soft saturation for analog character
        return std::tanh(stage4 * 0.7f);
    }

private:
    double sampleRate = 44100.0;
    float cutoff = 1000.0f;
    float resonance = 0.1f;
    bool selfOscillation = false;
    
    float stage1 = 0.0f, stage2 = 0.0f, stage3 = 0.0f, stage4 = 0.0f;
    float cutoffCoeff = 0.1f;
    float resonanceAmount = 0.0f;
    
    void updateCoefficients()
    {
        if (sampleRate <= 0.0)
            return;
        
        // Calculate cutoff coefficient (0.0 to 1.0)
        float normalizedCutoff = cutoff / (static_cast<float>(sampleRate) * 0.5f);
        cutoffCoeff = juce::jlimit(0.001f, 0.99f, normalizedCutoff);
        
        // Calculate resonance amount (0.0 to 0.95 to prevent instability)
        resonanceAmount = resonance * (selfOscillation ? 0.95f : 0.85f);
    }
};

//==============================================================================
class AdvancedSynthesisEngine::StateVariableFilter
{
public:
    StateVariableFilter()
    {
        reset();
    }
    
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
        this->resonance = juce::jlimit(0.0f, 1.0f, resonance);
        updateCoefficients();
    }
    
    void reset()
    {
        low = band = high = 0.0f;
        cutoff = 1000.0f;
        resonance = 0.1f;
        updateCoefficients();
    }
    
    float processLowPass(float input)
    {
        processInternal(input);
        return low;
    }
    
    float processHighPass(float input)
    {
        processInternal(input);
        return high;
    }
    
    float processBandPass(float input)
    {
        processInternal(input);
        return band;
    }
    
    float processNotch(float input)
    {
        processInternal(input);
        return low + high; // Notch = low + high
    }

private:
    double sampleRate = 44100.0;
    float cutoff = 1000.0f;
    float resonance = 0.1f;
    
    float low = 0.0f, band = 0.0f, high = 0.0f;
    float f = 0.1f, q = 0.5f;
    
    void updateCoefficients()
    {
        if (sampleRate <= 0.0)
            return;
        
        f = 2.0f * std::sin(juce::MathConstants<float>::pi * cutoff / static_cast<float>(sampleRate));
        q = 1.0f - resonance * 0.99f; // Invert for SVF (lower q = higher resonance)
    }
    
    void processInternal(float input)
    {
        low += f * band;
        high = input - low - q * band;
        band += f * high;
        
        // Prevent numerical instability
        if (std::abs(low) > 10.0f) low = 0.0f;
        if (std::abs(band) > 10.0f) band = 0.0f;
        if (std::abs(high) > 10.0f) high = 0.0f;
    }
};

//==============================================================================
class AdvancedSynthesisEngine::MultiFilter
{
public:
    MultiFilter()
    {
        reset();
    }
    
    void setSampleRate(double sampleRate)
    {
        this->sampleRate = sampleRate;
        biquadFilter.setSampleRate(sampleRate);
        moogFilter.setSampleRate(sampleRate);
        svFilter.setSampleRate(sampleRate);
        secondaryBiquad.setSampleRate(sampleRate);
    }
    
    void setFilterParams(const FilterParams& params)
    {
        filterParams = params;
        
        // Configure primary filter
        switch (params.filterType)
        {
            case FilterParams::MoogLadder:
                moogFilter.setCutoff(params.cutoff);
                moogFilter.setResonance(params.resonance);
                moogFilter.setSelfOscillation(params.selfOscillation);
                break;
                
            case FilterParams::StateVariable:
                svFilter.setCutoff(params.cutoff);
                svFilter.setResonance(params.resonance);
                break;
                
            default:
                biquadFilter.setType(params.filterType);
                biquadFilter.setCutoff(params.cutoff);
                biquadFilter.setResonance(params.resonance);
                break;
        }
        
        // Configure secondary filter for dual routing
        if (params.routing != FilterParams::Serial)
        {
            secondaryBiquad.setType(params.secondaryFilterType);
            secondaryBiquad.setCutoff(params.secondaryCutoff);
            secondaryBiquad.setResonance(params.secondaryResonance);
        }
    }
    
    void reset()
    {
        biquadFilter.reset();
        moogFilter.reset();
        svFilter.reset();
        secondaryBiquad.reset();
        filterParams = FilterParams{};
    }
    
    float processSample(float input, int midiNote = 60, float velocity = 1.0f)
    {
        if (!filterParams.enabled)
            return input;
        
        // Apply key tracking
        float keyTrackingAmount = (midiNote - 60) * filterParams.keyTracking * 10.0f; // 10Hz per semitone
        float adjustedCutoff = filterParams.cutoff + keyTrackingAmount;
        adjustedCutoff = juce::jlimit(20.0f, 20000.0f, adjustedCutoff);
        
        // Apply velocity tracking
        float velocityAmount = velocity * filterParams.velocityTracking;
        adjustedCutoff *= (1.0f + velocityAmount);
        adjustedCutoff = juce::jlimit(20.0f, 20000.0f, adjustedCutoff);
        
        float primaryOutput = 0.0f;
        float secondaryOutput = 0.0f;
        
        // Process primary filter
        switch (filterParams.filterType)
        {
            case FilterParams::MoogLadder:
                moogFilter.setCutoff(adjustedCutoff);
                primaryOutput = moogFilter.processSample(input);
                break;
                
            case FilterParams::StateVariable:
                svFilter.setCutoff(adjustedCutoff);
                primaryOutput = svFilter.processLowPass(input); // Default to low-pass for SVF
                break;
                
            default:
                biquadFilter.setCutoff(adjustedCutoff);
                primaryOutput = biquadFilter.processSample(input);
                break;
        }
        
        // Handle filter routing
        switch (filterParams.routing)
        {
            case FilterParams::Serial:
                return primaryOutput;
                
            case FilterParams::Parallel:
                secondaryBiquad.setCutoff(filterParams.secondaryCutoff + keyTrackingAmount);
                secondaryOutput = secondaryBiquad.processSample(input);
                return primaryOutput * (1.0f - filterParams.filterBalance) + 
                       secondaryOutput * filterParams.filterBalance;
                
            case FilterParams::Split:
                // Split based on frequency - low frequencies go to primary, high to secondary
                float splitPoint = (filterParams.cutoff + filterParams.secondaryCutoff) * 0.5f;
                if (adjustedCutoff < splitPoint)
                    return primaryOutput;
                else
                {
                    secondaryBiquad.setCutoff(filterParams.secondaryCutoff + keyTrackingAmount);
                    return secondaryBiquad.processSample(input);
                }
        }
        
        return primaryOutput;
    }

private:
    double sampleRate = 44100.0;
    FilterParams filterParams;
    
    AdvancedFilter biquadFilter;
    MoogLadderFilter moogFilter;
    StateVariableFilter svFilter;
    AdvancedFilter secondaryBiquad; // For dual filter routing
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
class AdvancedSynthesisEngine::ModulationMatrix
{
public:
    ModulationMatrix()
    {
        reset();
    }
    
    void setSampleRate(double sampleRate)
    {
        this->sampleRate = sampleRate;
        lfo1.setSampleRate(sampleRate);
        lfo2.setSampleRate(sampleRate);
        envelope2.setSampleRate(sampleRate);
    }
    
    void setModulationParams(const ModulationParams& params)
    {
        modParams = params;
        
        // Configure LFO1
        lfo1.setRate(params.lfoRate);
        lfo1.setDepth(params.lfoDepth);
        lfo1.setWaveform(params.lfoWaveform);
        lfo1.setBipolar(params.bipolar);
        
        // Configure LFO2
        lfo2.setRate(params.lfo2Rate);
        lfo2.setDepth(params.lfo2Depth);
        lfo2.setWaveform(params.lfo2Waveform);
        lfo2.setBipolar(params.lfo2Bipolar);
        
        // Configure Envelope 2
        juce::ADSR::Parameters env2Params;
        env2Params.attack = params.env2Attack;
        env2Params.decay = params.env2Decay;
        env2Params.sustain = params.env2Sustain;
        env2Params.release = params.env2Release;
        envelope2.setParameters(env2Params);
    }
    
    void noteOn()
    {
        envelope2.noteOn();
    }
    
    void noteOff()
    {
        envelope2.noteOff();
    }
    
    void reset()
    {
        lfo1.reset();
        lfo2.reset();
        envelope2.reset();
        modParams = ModulationParams{};
    }
    
    struct ModulationValues
    {
        float lfo1Value = 0.0f;
        float lfo2Value = 0.0f;
        float envelope2Value = 0.0f;
        float pitchMod = 0.0f;
        float filterMod = 0.0f;
        float amplitudeMod = 0.0f;
        float wavetableMod = 0.0f;
    };
    
    ModulationValues getModulationValues(float velocity = 1.0f)
    {
        ModulationValues values;
        
        // Get raw modulation source values
        float lfo1Raw = lfo1.getNextValue();
        float lfo2Raw = lfo2.getNextValue();
        
        // Apply cross-modulation (LFO1 modulates LFO2 rate)
        if (modParams.enableCrossModulation)
        {
            float modulatedRate = modParams.lfo2Rate * (1.0f + lfo1Raw * modParams.crossModAmount);
            lfo2.setRate(juce::jlimit(0.01f, 20.0f, modulatedRate));
        }
        
        values.lfo1Value = lfo1Raw;
        values.lfo2Value = lfo2Raw;
        values.envelope2Value = envelope2.getNextSample();
        
        // Apply velocity following to envelope
        if (modParams.enableEnvFollowing)
        {
            values.envelope2Value *= (1.0f + (velocity - 1.0f) * modParams.envFollowAmount);
        }
        
        // Route modulation to targets
        switch (modParams.modulationTarget)
        {
            case 1: // Pitch
                values.pitchMod = lfo1Raw;
                break;
            case 2: // Filter
                values.filterMod = lfo1Raw + values.envelope2Value * modParams.env2Amount;
                break;
            case 3: // Amplitude
                values.amplitudeMod = lfo1Raw;
                break;
            case 4: // Wavetable Position
                values.wavetableMod = lfo1Raw;
                break;
        }
        
        // Route LFO2 to secondary target
        switch (modParams.lfo2Target)
        {
            case 1: // Pitch
                values.pitchMod += values.lfo2Value * 0.5f; // Half depth for secondary
                break;
            case 2: // Filter
                values.filterMod += values.lfo2Value * 0.5f;
                break;
            case 3: // Amplitude
                values.amplitudeMod += values.lfo2Value * 0.5f;
                break;
            case 4: // Wavetable Position
                values.wavetableMod += values.lfo2Value * 0.5f;
                break;
        }
        
        return values;
    }

private:
    double sampleRate = 44100.0;
    ModulationParams modParams;
    
    ModulationSource lfo1;
    ModulationSource lfo2;
    juce::ADSR envelope2;
};

//==============================================================================
class AdvancedSynthesisEngine::PitchShifter
{
public:
    PitchShifter() { reset(); }

    void setPitchShift(float semitones)
    {
        pitchRatio = std::pow(2.0f, semitones / 12.0f);
    }

    void reset()
    {
        pitchRatio = 1.0f;
    }

    float getPitchRatio() const
    {
        return pitchRatio;
    }

private:
    float pitchRatio = 1.0f;
};

//==============================================================================
class AdvancedSynthesisEngine::LoopManager
{
public:
    LoopManager()
    {
        reset();
    }

    void setLoopPoints(int start, int end, int sampleLength)
    {
        if (end < 0 || end >= sampleLength)
            end = sampleLength - 1;

        loopStart = juce::jlimit(0, sampleLength - 1, start);
        loopEnd = juce::jlimit(loopStart, sampleLength - 1, end);
        loopLength = loopEnd - loopStart;
    }

    void setLoopMode(SampleParams::LoopMode mode)
    {
        loopMode = mode;
    }

    void reset()
    {
        loopStart = 0;
        loopEnd = 0;
        loopLength = 0;
        loopMode = SampleParams::LoopMode::Forward;
    }

    double getNextPosition(double currentPosition, float increment, bool isReversed, int& playDirection)
    {
        if (loopLength <= 0)
        {
            return currentPosition + (increment * (isReversed ? -1.0f : 1.0f));
        }

        double nextPosition = currentPosition + (increment * playDirection);

        if (loopMode == SampleParams::LoopMode::Forward)
        {
            if (isReversed) // Reverse looping
            {
                if (nextPosition < loopStart)
                {
                    nextPosition += loopLength;
                }
            }
            else // Forward looping
            {
                if (nextPosition >= loopEnd)
                {
                    nextPosition -= loopLength;
                }
            }
        }
        else if (loopMode == SampleParams::LoopMode::PingPong)
        {
            if (nextPosition >= loopEnd)
            {
                nextPosition = loopEnd - (nextPosition - loopEnd);
                playDirection = -1;
            }
            else if (nextPosition < loopStart)
            {
                nextPosition = loopStart + (loopStart - nextPosition);
                playDirection = 1;
            }
        }
        
        return nextPosition;
    }

private:
    int loopStart = 0;
    int loopEnd = 0;
    int loopLength = 0;
    SampleParams::LoopMode loopMode = SampleParams::LoopMode::Forward;
};


//==============================================================================
class AdvancedSynthesisEngine::SampleEngine
{
public:
    SampleEngine()
    {
        reset();
    }
    
    void setSampleRate(double rate)
    {
        sampleRate = rate;
    }

    void setSampleMap(const std::vector<juce::AudioBuffer<float>>* samples)
    {
        allSamples = samples;
    }
      
    void setSampleParams(const SampleParams& params)
    {
        sampleParams = params;
    }
      
    // Inject shared SampleCache (non-owning pointer, RT-safe reads only)
    void setSampleCache(const spawnclone::audio::SampleCache* ptr) noexcept
    {
        cache = ptr;
    }
    
    void reset()
    {
        playbackPosition = 0.0f;
        isActive = false;
        currentSample = nullptr;
        nextSample = nullptr;
        pitchShifter.reset();
        loopManager.reset();
        playDirection = 1;
    }
    
    void noteOn(int midiNote, float velocity)
    {
        if (!allSamples || allSamples->empty())
        {
            isActive = false;
            return;
        }

        currentNote = midiNote;
        currentVelocity = velocity;
        velocityCrossfade = 0.0f;
        nextSample = nullptr;

        // 1. Determine which sample(s) to use based on velocity
        if (sampleParams.enableVelocityLayers && !sampleParams.sampleMap.empty())
        {
            int noteVelocity = static_cast<int>(velocity * 127.0f);
            
            const SampleMapEntry* bestFit = nullptr;
            // Find the correct layer for the given velocity
            for(const auto& entry : sampleParams.sampleMap)
            {
                if (noteVelocity >= entry.minVelocity && noteVelocity <= entry.maxVelocity)
                {
                    bestFit = &entry;
                    break;
                }
            }

            if (bestFit)
            {
                currentSample = &(*allSamples)[bestFit->sampleIndex];

                // Find the iterator to the current entry to check for the next one
                auto it = std::find_if(sampleParams.sampleMap.begin(), sampleParams.sampleMap.end(), 
                                       [&](const auto& entry){ return &entry == bestFit; });
                
                if (it != sampleParams.sampleMap.end()) {
                    auto nextIt = std::next(it);
                    // Check if there is a next layer to crossfade with
                    if (nextIt != sampleParams.sampleMap.end())
                    {
                        float fadeStart = nextIt->minVelocity - sampleParams.velocityCrossfadeWidth;
                        if (noteVelocity > fadeStart && sampleParams.velocityCrossfadeWidth > 0)
                        {
                            velocityCrossfade = juce::jmap(static_cast<float>(noteVelocity), fadeStart, static_cast<float>(nextIt->minVelocity), 0.0f, 1.0f);
                            nextSample = &(*allSamples)[nextIt->sampleIndex];
                        }
                    }
                }
            }
            else // Fallback if no layer matches (e.g., gaps in velocity map)
            {
                currentSample = &(*allSamples)[sampleParams.sampleMap.front().sampleIndex];
            }
        } else {
            // Default to the first sample if velocity layers are disabled
            currentSample = &(*allSamples)[0];
        }

        if (!currentSample || currentSample->getNumSamples() == 0)
        {
            isActive = false;
            return;
        }
        
        sampleLength = currentSample->getNumSamples();
        
        // 2. Set initial playback position
        if (sampleParams.reversePlayback)
        {
            playbackPosition = (sampleLength - 1) - (sampleParams.startOffset * (sampleLength - 1));
            playDirection = -1;
        }
        else
        {
            playbackPosition = sampleParams.startOffset * (sampleLength - 1);
            playDirection = 1;
        }

        // 3. Configure Pitch Shifter
        pitchShiftSemitones = (midiNote - sampleParams.rootNote);
        pitchShiftSemitones = juce::jlimit(-sampleParams.pitchShiftRange, 
                                          sampleParams.pitchShiftRange, 
                                          pitchShiftSemitones);
        pitchShifter.setPitchShift(pitchShiftSemitones);
        
        // 4. Configure Loop Manager
        loopManager.setLoopPoints(sampleParams.loopStart, sampleParams.loopEnd, sampleLength);
        loopManager.setLoopMode(sampleParams.loopMode);

        isActive = true;
    }
    
    void noteOff()
    {
        if (sampleParams.loopMode != SampleParams::LoopMode::OneShot)
        {
             isActive = false;
        }
    }
    
    bool isPlaying() const
    {
        return isActive && playbackPosition >= 0 && playbackPosition < sampleLength;
    }
    
    float getNextSample()
    {
        if (!isPlaying())
            return 0.0f;

        // --- 1. Get interpolated value at current position from the main sample ---
        float mainValue = getInterpolatedSample(currentSample, sampleLength, playbackPosition);

        // --- 2. Get value from the crossfade sample if active ---
        float finalValue = mainValue;
        if (nextSample && velocityCrossfade > 0.0f)
        {
            double scaledPosition = playbackPosition * ((double)nextSample->getNumSamples() / sampleLength);
            float nextValue = getInterpolatedSample(nextSample, nextSample->getNumSamples(), scaledPosition);
            finalValue = mainValue * (1.0f - velocityCrossfade) + nextValue * velocityCrossfade;
        }

        // --- 3. Calculate playback increment for next frame ---
        float pitchRatio = pitchShifter.getPitchRatio();
        float playbackIncrement = pitchRatio * sampleParams.playbackSpeed;

        // --- 4. Update playback position for the next call ---
        if (sampleParams.enableLooping)
        {
            playbackPosition = loopManager.getNextPosition(playbackPosition, playbackIncrement, sampleParams.reversePlayback, playDirection);
        }
        else
        {
            playbackPosition += playbackIncrement * (sampleParams.reversePlayback ? -1.0f : 1.0f);
        }

        // --- 5. Check if playback has ended after position update ---
        if (!isPlaying())
        {
            isActive = false;
        }

        return finalValue;
    }

private:
    float getInterpolatedSample(const juce::AudioBuffer<float>* sampleBuffer, int bufferLength, double position)
    {
        if (!sampleBuffer || bufferLength <= 1) return 0.0f;

        const float* data = sampleBuffer->getReadPointer(0);
        
        double pos0_double = std::floor(position);
        int pos0 = static_cast<int>(pos0_double);
        double frac = position - pos0_double;

        int pos1 = pos0 + 1;

        if (pos0 < 0 || pos0 >= bufferLength) return 0.0f;
        if (pos1 >= bufferLength) return data[pos0];

        float val0 = data[pos0];
        float val1 = data[pos1];
        
        return val0 + frac * (val1 - val0);
    }

    double sampleRate = 44100.0;
    const juce::AudioBuffer<float>* currentSample = nullptr;
    const juce::AudioBuffer<float>* nextSample = nullptr;
    const std::vector<juce::AudioBuffer<float>>* allSamples = nullptr;
    const spawnclone::audio::SampleCache* cache = nullptr;
    SampleParams sampleParams;
    
    int sampleLength = 0;
    double playbackPosition = 0.0f;
    bool isActive = false;
    int currentNote = 60;
    float currentVelocity = 1.0f;
    float pitchShiftSemitones = 0.0f;
    float velocityCrossfade = 0.0f;
    int playDirection = 1; // 1 for forward, -1 for backward
    
    PitchShifter pitchShifter;
    LoopManager loopManager;
};

//==============================================================================
AdvancedSynthesisEngine::AdvancedSynthesisEngine()
{
    sampleCache = std::make_unique<spawnclone::audio::SampleCache>();
    asyncSampleLoader = std::make_unique<spawnclone::audio::AsyncSampleLoader>(*sampleCache);

    for (int i = 0; i < voices.size(); ++i)
    {
        voices[i] = std::make_unique<SynthVoice>(*sampleCache);
    }

    initializeWavetables();
    initializeSamples();
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
        if(voice) voice->prepareToPlay(newSampleRate, newSamplesPerBlock);
    
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
        if (voice) voice->releaseResources();
    
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
    
    // Validate sample parameters
    currentParams.sample.sampleIndex = juce::jlimit(0, (int)samples.size() - 1, params.sample.sampleIndex);
    currentParams.sample.rootNote = juce::jlimit(0.0f, 127.0f, params.sample.rootNote);
    currentParams.sample.pitchShiftRange = juce::jlimit(1.0f, 48.0f, params.sample.pitchShiftRange);
    currentParams.sample.loopCrossfade = juce::jlimit(1.0f, 100.0f, params.sample.loopCrossfade);
    
    // Update all voices with new wavetable if changed
    // TODO: Implement wavetable setting when SynthVoice supports it
    if (params.synthesisType == SynthesisType::Wavetable || params.synthesisType == SynthesisType::Hybrid)
    {
        // Wavetable setting will be implemented when SynthVoice API supports it
    }
    
    // Update all voices with new sample if changed  
    // TODO: Implement sample map setting when SynthVoice supports it
    if (params.synthesisType == SynthesisType::Sample || params.synthesisType == SynthesisType::Hybrid)
    {
        // Sample map setting will be implemented when SynthVoice API supports it
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
        voice->startNote(midiNoteNumber, velocity, currentParams.synthesisType, currentParams.sample, samples);
        
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
        if (voice && voice->isPlayingNote(midiNoteNumber))
        {
            voice->stopNote(0.0f, true);
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
    juce::AudioBuffer<float> voiceBuffer(buffer.getNumChannels(), buffer.getNumSamples());
    
    for (auto& voice : voices)
    {
        if (voice && voice->isVoiceActive())
        {
            voiceBuffer.clear();
            voice->renderNextBlock(voiceBuffer, 0, buffer.getNumSamples());
            
            // Mix into main buffer
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            {
                buffer.addFrom(channel, 0, voiceBuffer, channel, 0, buffer.getNumSamples());
            }
        }
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

SynthVoice* AdvancedSynthesisEngine::findAvailableVoice()
{
    // First, look for an inactive voice
    for (auto& voice : voices)
    {
        if (voice && !voice->isVoiceActive())
            return voice.get();
    }
    
    // If no inactive voice, use round-robin selection to steal a voice
    auto* voice = voices[currentVoiceIndex].get();
    currentVoiceIndex = (currentVoiceIndex + 1) % voices.size();
    return voice;
}

void AdvancedSynthesisEngine::initializeWavetables()
{
    generateBuiltinWavetables();
}

void AdvancedSynthesisEngine::initializeSamples()
{
    generateBuiltinSamples();
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
    info << "Samples: " << samples.size() << " loaded\n";
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

//==============================================================================
// Sample Management Implementation

bool AdvancedSynthesisEngine::loadSample(int index, const juce::AudioBuffer<float>& sampleData)
{
    if (index < 0 || index >= MAX_SAMPLES)
        return false;
    
    if (sampleData.getNumSamples() == 0 || sampleData.getNumChannels() == 0)
        return false;
    
    // Ensure we have enough storage
    if (samples.size() <= index)
    {
        samples.resize(index + 1);
        sampleParameters.resize(index + 1);
    }
    
    // Copy sample data (convert to mono if stereo)
    int numSamples = juce::jmin(sampleData.getNumSamples(), MAX_SAMPLE_SIZE);
    samples[index].setSize(1, numSamples);
    
    if (sampleData.getNumChannels() == 1)
    {
        samples[index].copyFrom(0, 0, sampleData, 0, 0, numSamples);
    }
    else
    {
        // Mix down stereo to mono
        samples[index].clear();
        for (int ch = 0; ch < sampleData.getNumChannels(); ++ch)
        {
            samples[index].addFrom(0, 0, sampleData, ch, 0, numSamples, 1.0f / sampleData.getNumChannels());
        }
    }
    
    // Initialize default sample parameters
    sampleParameters[index] = SampleParams();
    
    // Auto-detect loop points
    autoDetectLoopPoints(index);
    
    return true;
}

void AdvancedSynthesisEngine::generateBuiltinSamples()
{
    // Generate basic synthetic samples for testing
    const double sampleRate = 44100.0;
    const int sampleLength = static_cast<int>(sampleRate * 2.0); // 2 seconds
    
    // Sample 0: Sine wave pad
    {
        juce::AudioBuffer<float> sineBuffer(1, sampleLength);
        for (int i = 0; i < sampleLength; ++i)
        {
            float phase = static_cast<float>(i) / sampleRate * 2.0f * juce::MathConstants<float>::pi;
            sineBuffer.setSample(0, i, std::sin(phase * 220.0f) * 0.7f); // A3
        }
        loadSample(0, sineBuffer);
    }
    
    // Sample 1: Sawtooth bass
    {
        juce::AudioBuffer<float> sawBuffer(1, sampleLength);
        for (int i = 0; i < sampleLength; ++i)
        {
            float phase = std::fmod(static_cast<float>(i) / sampleRate * 110.0f, 1.0f); // A2
            sawBuffer.setSample(0, i, (phase * 2.0f - 1.0f) * 0.6f);
        }
        loadSample(1, sawBuffer);
    }
    
    // Sample 2: Pluck sound
    {
        juce::AudioBuffer<float> pluckBuffer(1, sampleLength);
        juce::Random random;
        
        for (int i = 0; i < sampleLength; ++i)
        {
            float envelope = std::exp(-static_cast<float>(i) / sampleRate * 3.0f); // Decay envelope
            float noise = (random.nextFloat() * 2.0f - 1.0f) * 0.1f;
            float tone = std::sin(static_cast<float>(i) / sampleRate * 2.0f * juce::MathConstants<float>::pi * 440.0f); // A4
            pluckBuffer.setSample(0, i, (tone + noise) * envelope * 0.8f);
        }
        loadSample(2, pluckBuffer);
    }
}

bool AdvancedSynthesisEngine::setSampleLoopPoints(int sampleIndex, int loopStart, int loopEnd)
{
    if (sampleIndex < 0 || sampleIndex >= static_cast<int>(sampleParameters.size()))
        return false;
    
    if (sampleIndex >= static_cast<int>(samples.size()) || samples[sampleIndex].getNumSamples() == 0)
        return false;
    
    int numSamples = samples[sampleIndex].getNumSamples();
    
    // Validate loop points
    loopStart = juce::jlimit(0, numSamples - 1, loopStart);
    loopEnd = (loopEnd == -1) ? numSamples - 1 : juce::jlimit(loopStart + 1, numSamples - 1, loopEnd);
    
    sampleParameters[sampleIndex].loopStart = loopStart;
    sampleParameters[sampleIndex].loopEnd = loopEnd;
    
    return true;
}

bool AdvancedSynthesisEngine::autoDetectLoopPoints(int sampleIndex)
{
    if (sampleIndex < 0 || sampleIndex >= static_cast<int>(samples.size()))
        return false;
    
    if (samples[sampleIndex].getNumSamples() == 0)
        return false;
    
    // Simple auto-detection: use last 25% of sample for loop
    int numSamples = samples[sampleIndex].getNumSamples();
    int loopStart = static_cast<int>(numSamples * 0.75f);
    int loopEnd = numSamples - 1;
    
    return setSampleLoopPoints(sampleIndex, loopStart, loopEnd);
}

} // namespace spawnclone::audio
