#include "ParameterManager.h"

//==============================================================================
// Parameter ID constants
const juce::String ParameterManager::KEY_PARAM_ID = "key";
const juce::String ParameterManager::SCALE_PARAM_ID = "scale";
const juce::String ParameterManager::TEMPO_PARAM_ID = "tempo";
const juce::String ParameterManager::COMPLEXITY_PARAM_ID = "complexity";
const juce::String ParameterManager::GENERATION_TYPE_PARAM_ID = "generationType";
const juce::String ParameterManager::PATTERN_LENGTH_PARAM_ID = "patternLength";

//==============================================================================
ParameterManager::ParameterManager(juce::AudioProcessor& processor)
    : valueTreeState(processor, nullptr, "PARAMETERS", createParameterLayout())
{
    // Initialize atomic values with current parameter values
    atomicKey.store(0);
    atomicScale.store(0);
    atomicTempo.store(120.0f);
    atomicComplexity.store(0.5f);
    atomicGenerationType.store(0);
    atomicPatternLength.store(16.0f);
}

juce::AudioProcessorValueTreeState& ParameterManager::getState()
{
    return valueTreeState;
}

//==============================================================================
GenerationParameters ParameterManager::getCurrentParameters() const
{
    GenerationParameters params;
    
    // Get values from the parameter tree
    auto keyParam = valueTreeState.getRawParameterValue(KEY_PARAM_ID);
    auto scaleParam = valueTreeState.getRawParameterValue(SCALE_PARAM_ID);
    auto tempoParam = valueTreeState.getRawParameterValue(TEMPO_PARAM_ID);
    auto complexityParam = valueTreeState.getRawParameterValue(COMPLEXITY_PARAM_ID);
    auto genTypeParam = valueTreeState.getRawParameterValue(GENERATION_TYPE_PARAM_ID);
    auto lengthParam = valueTreeState.getRawParameterValue(PATTERN_LENGTH_PARAM_ID);
    
    params.key = keyParam ? static_cast<int>(keyParam->load()) : 0;
    params.scale = scaleParam ? static_cast<GenerationParameters::ScaleType>(static_cast<int>(scaleParam->load())) : GenerationParameters::ScaleType::Major;
    params.tempo = tempoParam ? tempoParam->load() : 120.0f;
    params.rhythmicComplexity = complexityParam ? complexityParam->load() : 0.5f;
    params.generationType = genTypeParam ? static_cast<GenerationParameters::GenerationType>(static_cast<int>(genTypeParam->load())) : GenerationParameters::GenerationType::Melody;
    params.patternLengthBeats = lengthParam ? lengthParam->load() : 16.0f;
    
    return params;
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout ParameterManager::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameters;
    
    // Key parameter (0-11 for C to B)
    parameters.push_back(std::make_unique<juce::AudioParameterChoice>(
        KEY_PARAM_ID,
        "Key",
        juce::StringArray { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" },
        0  // Default to C
    ));
    
    // Scale parameter
    parameters.push_back(std::make_unique<juce::AudioParameterChoice>(
        SCALE_PARAM_ID,
        "Scale",
        juce::StringArray { "Major", "Minor", "Pentatonic", "Blues", "Dorian", "Mixolydian" },
        0  // Default to Major
    ));
    
    // Tempo parameter (60-200 BPM)
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        TEMPO_PARAM_ID,
        "Tempo",
        juce::NormalisableRange<float>(60.0f, 200.0f, 1.0f),
        120.0f,  // Default tempo
        "BPM"
    ));
    
    // Rhythmic complexity (0.0 to 1.0)
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        COMPLEXITY_PARAM_ID,
        "Complexity",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.5f  // Default complexity
    ));
    
    // Generation type
    parameters.push_back(std::make_unique<juce::AudioParameterChoice>(
        GENERATION_TYPE_PARAM_ID,
        "Generation Type",
        juce::StringArray { "Melody", "Chord", "Bass", "Drum" },
        0  // Default to Melody
    ));
    
    // Pattern length (4-32 beats)
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        PATTERN_LENGTH_PARAM_ID,
        "Pattern Length",
        juce::NormalisableRange<float>(4.0f, 32.0f, 4.0f),
        16.0f,  // Default to 16 beats
        "beats"
    ));
    
    return { parameters.begin(), parameters.end() };
}

//==============================================================================
void ParameterManager::parameterChanged(const juce::String& parameterID, float newValue)
{
    if (parameterID == KEY_PARAM_ID)
        atomicKey.store(static_cast<int>(newValue));
    else if (parameterID == SCALE_PARAM_ID)
        atomicScale.store(static_cast<int>(newValue));
    else if (parameterID == TEMPO_PARAM_ID)
        atomicTempo.store(newValue);
    else if (parameterID == COMPLEXITY_PARAM_ID)
        atomicComplexity.store(newValue);
    else if (parameterID == GENERATION_TYPE_PARAM_ID)
        atomicGenerationType.store(static_cast<int>(newValue));
    else if (parameterID == PATTERN_LENGTH_PARAM_ID)
        atomicPatternLength.store(newValue);
}

//==============================================================================
const juce::StringArray ParameterManager::getKeySignatureItems() const
{
    return juce::StringArray { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
}

const juce::StringArray ParameterManager::getScaleTypeItems() const
{
    return juce::StringArray { "Major", "Minor", "Pentatonic", "Blues", "Dorian", "Mixolydian" };
}

const juce::StringArray ParameterManager::getGenerationTypeItems() const
{
    return juce::StringArray { "Melody", "Chords", "Bassline", "Drums" };
}
