# Phase 7: Advanced Synthesis Types

## Overview

Phase 7 expands SpawnClone's sonic palette by implementing advanced synthesis methods that go beyond traditional subtractive synthesis. This phase adds FM synthesis, granular synthesis, and physical modeling to create a comprehensive synthesis platform.

## Technical Specifications

### Synthesis Architecture Extension

The existing synthesis engine will be extended with new synthesis types while maintaining the current wavetable/subtractive foundation:

```cpp
enum class AdvancedSynthesisType
{
    // Existing types
    Sample,
    Wavetable, 
    Subtractive,
    Hybrid,
    
    // New advanced types
    FM,                    // Frequency Modulation
    Granular,              // Granular synthesis
    PhysicalModeling,      // Physical modeling
    Additive,              // Additive synthesis
    Spectral               // Spectral manipulation
};
```

### 1. FM (Frequency Modulation) Synthesis

#### Technical Implementation

```cpp
class FMSynthesisEngine
{
public:
    struct FMOperator
    {
        float frequency = 440.0f;
        float amplitude = 1.0f;
        float phase = 0.0f;
        float phaseIncrement = 0.0f;
        
        // Envelope for operator
        ADSREnvelope envelope;
        
        // Operator connections
        std::vector<int> modulationTargets;
        std::vector<float> modulationAmounts;
    };
    
    struct FMParams
    {
        static constexpr int MAX_OPERATORS = 6;
        std::array<FMOperator, MAX_OPERATORS> operators;
        
        // Algorithm matrix (operator routing)
        std::array<std::array<float, MAX_OPERATORS>, MAX_OPERATORS> algorithmMatrix;
        
        // Global parameters
        float feedback = 0.0f;
        int activeOperators = 4;
        int algorithm = 1; // DX7-style algorithms 1-32
    };
    
    float generateSample(const FMParams& params);
};
```

#### Key Features

- **6-Operator Architecture**: Full DX7-compatible operator count
- **32 Preset Algorithms**: Classic FM routing configurations
- **Custom Algorithm Matrix**: User-definable operator connections
- **Per-Operator Envelopes**: Independent ADSR for each operator
- **Feedback Loops**: Self-modulation capabilities
- **Velocity Sensitivity**: Per-operator velocity scaling

#### Mathematical Foundation

```cpp
// FM synthesis equation: y(t) = A * sin(ωc*t + I * sin(ωm*t))
// Where: A = amplitude, ωc = carrier frequency, ωm = modulator frequency, I = modulation index

float FMSynthesisEngine::calculateOperatorOutput(int operatorIndex, const FMParams& params)
{
    const auto& op = params.operators[operatorIndex];
    
    // Calculate modulation input from other operators
    float modulationInput = 0.0f;
    for (int i = 0; i < MAX_OPERATORS; ++i)
    {
        if (params.algorithmMatrix[i][operatorIndex] > 0.0f)
        {
            modulationInput += operatorOutputs[i] * params.algorithmMatrix[i][operatorIndex];
        }
    }
    
    // Apply feedback if configured
    if (operatorIndex == feedbackOperator)
    {
        modulationInput += previousOutput * params.feedback;
    }
    
    // Generate output
    float output = op.amplitude * std::sin(op.phase + modulationInput);
    op.phase += op.phaseIncrement;
    
    return output * op.envelope.getCurrentLevel();
}
```

### 2. Granular Synthesis

#### Core Architecture

```cpp
class GranularSynthesisEngine
{
public:
    struct Grain
    {
        bool active = false;
        float position = 0.0f;        // Position in source material
        float duration = 0.1f;        // Grain length in seconds
        float amplitude = 1.0f;       // Grain amplitude
        float pitch = 1.0f;           // Pitch multiplier
        float pan = 0.0f;             // Stereo position
        
        // Grain envelope
        float envelopePhase = 0.0f;
        WindowType windowType = WindowType::Hann;
    };
    
    struct GranularParams
    {
        // Source material
        const juce::AudioBuffer<float>* sourceBuffer = nullptr;
        
        // Grain parameters
        float grainSize = 0.1f;       // seconds
        float grainRate = 20.0f;      // grains per second
        float positionSpread = 0.1f;  // randomization
        float pitchSpread = 0.0f;     // semitones
        
        // Global controls
        float density = 1.0f;         // grain overlap
        float feedback = 0.0f;        // grain recycling
        PlaybackMode mode = Forward;
    };
    
    static constexpr int MAX_GRAINS = 64;
    std::array<Grain, MAX_GRAINS> grainPool;
};
```

#### Advanced Features

- **Multi-source Granular**: Multiple audio sources simultaneously
- **Real-time Grain Manipulation**: Live parameter control
- **Spectral Granular**: Frequency-domain grain processing
- **Grain Synthesis**: Procedural grain generation
- **Cloud Textures**: High-density grain clouds

#### Window Functions

```cpp
enum class WindowType
{
    Rectangular,
    Hann,
    Hamming,
    Blackman,
    Gaussian,
    Kaiser
};

float GranularSynthesisEngine::applyWindow(float phase, WindowType type)
{
    switch (type)
    {
        case WindowType::Hann:
            return 0.5f * (1.0f - std::cos(2.0f * juce::MathConstants<float>::pi * phase));
        case WindowType::Hamming:
            return 0.54f - 0.46f * std::cos(2.0f * juce::MathConstants<float>::pi * phase);
        case WindowType::Gaussian:
            return std::exp(-0.5f * std::pow((phase - 0.5f) / 0.2f, 2.0f));
        // Additional window types...
    }
}
```

### 3. Physical Modeling Synthesis

#### String Modeling (Karplus-Strong)

```cpp
class StringModel
{
public:
    struct StringParams
    {
        float frequency = 440.0f;
        float damping = 0.995f;       // String damping factor
        float stiffness = 0.1f;       // String stiffness
        float pickup = 0.5f;          // Pickup position
        float excitation = 1.0f;      // Pluck/bow strength
        
        ExcitationType excitationType = Pluck;
    };
    
private:
    std::vector<float> delayLine;
    float allpassCoeff = 0.0f;
    juce::Random noiseGenerator;
    
    float processKarplusStrong(float input, const StringParams& params);
};
```

#### Wind Instrument Modeling

```cpp
class WindModel
{
public:
    struct WindParams
    {
        float blowPressure = 0.5f;
        float embouchure = 0.5f;      // Lip tension
        float boreLength = 1.0f;      // Instrument length
        float toneholes = 0.0f;       // Open tonehole simulation
        
        InstrumentType type = Flute;
    };
    
    // Waveguide implementation
    std::vector<float> boreDelayLine;
    std::vector<float> reflectionFilter;
};
```

### 4. Additive Synthesis

#### Harmonic Series Control

```cpp
class AdditiveSynthesisEngine
{
public:
    struct Harmonic
    {
        float amplitude = 0.0f;
        float frequency = 0.0f;       // Relative to fundamental
        float phase = 0.0f;
        bool enabled = true;
        
        // Individual harmonic envelope
        ADSREnvelope envelope;
    };
    
    struct AdditiveParams
    {
        static constexpr int MAX_HARMONICS = 64;
        std::array<Harmonic, MAX_HARMONICS> harmonics;
        
        float fundamentalFreq = 440.0f;
        float harmonicDecay = 0.8f;   // Amplitude falloff per harmonic
        float inharmonicity = 0.0f;   // Frequency deviation from perfect harmonics
        
        // Spectral envelope
        std::vector<float> spectralEnvelope;
    };
};
```

## Integration with Existing Engine

### Parameter Management

```cpp
// Extension to AdvancedSynthesisEngine.h
struct AdvancedSynthesisParams
{
    // Existing parameters...
    
    // FM Synthesis
    FMSynthesisEngine::FMParams fmParams;
    
    // Granular Synthesis  
    GranularSynthesisEngine::GranularParams granularParams;
    
    // Physical Modeling
    StringModel::StringParams stringParams;
    WindModel::WindParams windParams;
    
    // Additive Synthesis
    AdditiveSynthesisEngine::AdditiveParams additiveParams;
    
    // Synthesis type selection
    AdvancedSynthesisType synthesisType = AdvancedSynthesisType::Wavetable;
};
```

### Voice Management Updates

```cpp
// SynthVoice.cpp additions
void SynthVoice::generateAdvancedSynthesis(juce::AudioBuffer<float>& buffer, int numSamples)
{
    switch (currentAdvancedSynthesisType)
    {
        case AdvancedSynthesisType::FM:
            generateFMSynthesis(buffer, numSamples);
            break;
        case AdvancedSynthesisType::Granular:
            generateGranularSynthesis(buffer, numSamples);
            break;
        case AdvancedSynthesisType::PhysicalModeling:
            generatePhysicalModelingSynthesis(buffer, numSamples);
            break;
        case AdvancedSynthesisType::Additive:
            generateAdditiveSynthesis(buffer, numSamples);
            break;
        default:
            // Fall back to existing synthesis types
            generateWavetableAudio(buffer, numSamples);
            break;
    }
}
```

## Implementation Timeline

### Sprint 1-2: FM Synthesis (Weeks 1-4)
- Operator architecture implementation
- Algorithm matrix system
- DX7-compatible parameter mapping
- Basic FM algorithms (1-8)

### Sprint 3-4: Granular Synthesis (Weeks 5-8)  
- Grain engine implementation
- Multi-source capability
- Real-time parameter control
- Window function library

### Sprint 5-6: Physical Modeling (Weeks 9-12)
- Karplus-Strong string synthesis
- Basic wind instrument modeling
- Excitation models (pluck, bow, blow)
- Resonator filters

### Sprint 7-8: Integration & Optimization (Weeks 13-16)
- Performance optimization
- Cross-synthesis capabilities
- Advanced parameter automation
- Preset management for new synthesis types

## Technical Challenges

### 1. CPU Performance
**Challenge**: Advanced synthesis types are computationally intensive
**Solutions**:
- SIMD optimization for harmonic calculations
- Grain pool management for granular synthesis
- Efficient delay line implementations
- Adaptive quality scaling based on polyphony

### 2. Parameter Complexity
**Challenge**: Each synthesis type has dozens of parameters
**Solutions**:
- Hierarchical parameter organization
- Macro controls for common adjustments  
- Intelligent parameter grouping
- Context-sensitive UI elements

### 3. Audio Quality vs Performance
**Challenge**: Balancing quality with real-time performance
**Solutions**:
- Multiple quality modes (draft/performance/ultra)
- Adaptive sample rate processing
- Intelligent voice allocation
- Background processing for non-critical calculations

## Quality Assurance

### Test Requirements
```cpp
class AdvancedSynthesisTest : public ::testing::Test
{
public:
    void TestFMAlgorithmAccuracy();
    void TestGranularGrainGeneration();  
    void TestPhysicalModelingStability();
    void TestAdditiveSynthesisHarmonics();
    void TestCPUPerformanceUnderLoad();
    void TestParameterAutomation();
};
```

### Performance Benchmarks
- **FM Synthesis**: 6 operators at 48kHz, <10% CPU per voice
- **Granular Synthesis**: 32 active grains, <15% CPU per voice
- **Physical Modeling**: String + body resonance, <12% CPU per voice
- **Additive Synthesis**: 32 harmonics, <8% CPU per voice

## Success Criteria

### Must Have
✅ FM synthesis with 6 operators and 8 algorithms
✅ Granular synthesis with real-time source manipulation
✅ Basic physical modeling (string synthesis)
✅ Performance within CPU budget (Voice limit: 16 voices total)

### Should Have  
✅ 32 FM algorithms (full DX7 compatibility)
✅ Advanced granular features (spectral processing)
✅ Wind instrument physical modeling
✅ Cross-synthesis capabilities (FM + Granular, etc.)

### Could Have
✅ Additive synthesis with spectral envelope control
✅ Advanced physical modeling (brass, percussion)
✅ Spectral synthesis and manipulation
✅ Morphing between synthesis types

---

**Estimated Timeline**: 16 weeks
**Team Size**: 3-4 developers (DSP specialists required)
**Dependencies**: Phase 1-6 completion
**Risk Level**: High (complex algorithms, performance optimization)
