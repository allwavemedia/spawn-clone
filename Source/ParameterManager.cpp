#include "ParameterManager.h"

//==============================================================================
// Parameter ID constants
const juce::String ParameterManager::KEY_PARAM_ID = "key";
const juce::String ParameterManager::SCALE_PARAM_ID = "scale";
const juce::String ParameterManager::TEMPO_PARAM_ID = "tempo";
const juce::String ParameterManager::COMPLEXITY_PARAM_ID = "complexity";
const juce::String ParameterManager::GENERATION_TYPE_PARAM_ID = "generationType";
const juce::String ParameterManager::PATTERN_LENGTH_PARAM_ID = "patternLength";
const juce::String ParameterManager::AI_MODE_PARAM_ID = "aiMode";
const juce::String ParameterManager::GENERATION_SEED_PARAM_ID = "generationSeed";

// Epic 6 Feature Parameter Constants  
const juce::String ParameterManager::INSTRUMENT_MODE_PARAM_ID = "instrumentMode";
const juce::String ParameterManager::AUTO_PLAY_PARAM_ID = "autoPlay";

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
    atomicAIMode.store(0);
    atomicGenerationSeed.store(0);
    atomicInstrumentMode.store(false);
    atomicAutoPlay.store(true);
}

juce::AudioProcessorValueTreeState& ParameterManager::getState()
{
    return valueTreeState;
}

//==============================================================================
GenerationParameters ParameterManager::getCurrentParameters() const
{
    GenerationParameters params;
    
    // Use atomic values for thread-safe access
    params.key = atomicKey.load();
    params.scale = static_cast<GenerationParameters::ScaleType>(atomicScale.load());
    params.tempo = atomicTempo.load();
    params.rhythmicComplexity = atomicComplexity.load();
    params.generationType = static_cast<GenerationParameters::GenerationType>(atomicGenerationType.load());
    params.patternLengthBeats = atomicPatternLength.load();
    params.aiMode = static_cast<GenerationParameters::AIMode>(atomicAIMode.load());
    params.generationSeed = atomicGenerationSeed.load();
    
    return params;
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout ParameterManager::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    // Create parameter group with versioning for VST3 compatibility
    auto mainGroup = std::make_unique<juce::AudioProcessorParameterGroup>
    (
        "main", "Main", "|",
        
        // Key parameter (0-11 for C to B)
        std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID(KEY_PARAM_ID, 1),
            "Key",
            juce::StringArray { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" },
            0  // Default to C
        ),
        
        // Scale parameter
        std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID(SCALE_PARAM_ID, 1),
            "Scale",
            juce::StringArray { "Major", "Minor", "Pentatonic", "Blues", "Dorian", "Mixolydian" },
            0  // Default to Major
        ),
        
        // Tempo parameter (60-200 BPM)
        std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(TEMPO_PARAM_ID, 1),
            "Tempo",
            juce::NormalisableRange<float>(60.0f, 200.0f, 1.0f),
            120.0f  // Default to 120 BPM
        ),
        
        // Complexity parameter (0.0-1.0)
        std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(COMPLEXITY_PARAM_ID, 1),
            "Complexity",
            juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
            0.5f  // Default to 50%
        ),
        
        // Generation Type parameter
        std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID(GENERATION_TYPE_PARAM_ID, 1),
            "Generation Type",
            juce::StringArray { "Melody", "Chords", "Bassline", "Drums" },
            0  // Default to Melody
        ),
        
        // Pattern Length parameter (1-32 beats)
        std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(PATTERN_LENGTH_PARAM_ID, 1),
            "Pattern Length",
            juce::NormalisableRange<float>(1.0f, 32.0f, 0.25f),
            16.0f  // Default to 16 beats
        ),
        
        // AI Mode parameter
        std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID(AI_MODE_PARAM_ID, 1),
            "AI Mode",
            juce::StringArray { "Fast", "Balanced", "Quality" },
            1  // Default to Balanced
        ),
        
        // Generation Seed parameter
        std::make_unique<juce::AudioParameterInt>(
            juce::ParameterID(GENERATION_SEED_PARAM_ID, 1),
            "Generation Seed",
            0, 9999, 0  // Default to 0 for random
        ),
        
        // Epic 6 Feature: Instrument Mode
        std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID(INSTRUMENT_MODE_PARAM_ID, 1),
            "Instrument Mode",
            false  // Default to false
        ),
        
        // Epic 6 Feature: Auto-play on Generate
        std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID(AUTO_PLAY_PARAM_ID, 1),
            "Auto Play on Generate",
            true  // Default to true
        )
    );
    
    layout.add(std::move(mainGroup));
    return layout;
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
    else if (parameterID == AI_MODE_PARAM_ID)
        atomicAIMode.store(static_cast<int>(newValue));
    else if (parameterID == GENERATION_SEED_PARAM_ID)
        atomicGenerationSeed.store(static_cast<int>(newValue));
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

//==============================================================================
// Epic 7: AI Mode helper methods
const juce::StringArray ParameterManager::getAIModeItems() const
{
    return juce::StringArray { "Fast", "Quality", "Cloud" };
}

//==============================================================================
// Epic 6: Setter methods for new parameters
void ParameterManager::setInstrumentMode(bool enabled)
{
    atomicInstrumentMode.store(enabled);
    // Also update the parameter tree if needed
    // valueTreeState.getParameter(INSTRUMENT_MODE_PARAM_ID)->setValueNotifyingHost(enabled ? 1.0f : 0.0f);
}

void ParameterManager::setAutoPlayOnGenerate(bool enabled)
{
    atomicAutoPlay.store(enabled);
    // Also update the parameter tree if needed  
    // valueTreeState.getParameter(AUTO_PLAY_PARAM_ID)->setValueNotifyingHost(enabled ? 1.0f : 0.0f);
}
