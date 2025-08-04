/*
  ==============================================================================

    GenerationParameters.h
    Created: 28 Jul 2025
    Author:  Developer Agent

    Data structure for AI generation parameters as specified in architecture.

  ==============================================================================
*/

#pragma once

#include <juce_core/juce_core.h>

//==============================================================================
/**
 * Encapsulates all user-configurable settings that guide the AI MIDI generation process.
 * This object is passed to the AI Generation Engine to produce a pattern.
 */
struct GenerationParameters
{
    // Musical key for generation (0=C, 1=C#, 2=D, etc.)
    int key = 0;
    
    // Musical scale type
    enum class ScaleType
    {
        Major = 0,
        Minor,
        Pentatonic,
        Blues,
        Dorian,
        Mixolydian
    };
    ScaleType scale = ScaleType::Major;
    
    // Tempo in beats per minute
    float tempo = 120.0f;
    
    // Rhythmic complexity (0.0 = simple, 1.0 = complex)
    float rhythmicComplexity = 0.5f;
    
    // Note density (0.0 = sparse, 1.0 = dense)
    float noteDensity = 0.5f;

    // Syncopation level (0.0 = on-beat, 1.0 = highly syncopated)
    float syncopation = 0.2f;

    // Swing amount (0.0 = straight, 1.0 = heavy swing)
    float swing = 0.0f;

    // Melodic contour shape
    enum class MelodicContour
    {
        Rising,
        Falling,
        Wave,
        Arch,
        Random
    };
    MelodicContour melodicContour = MelodicContour::Random;

    // Variety of melodic intervals (0.0 = repetitive, 1.0 = wide variety)
    float intervalVariety = 0.5f;

    // Variety of note durations (0.0 = uniform, 1.0 = wide variety)
    float noteLengthVariety = 0.5f;

    // Epic 6 Story 6.2: Dynamic Pattern Variation
    struct VariationSettings
    {
        bool enableEvolution = false;           // Enable pattern evolution over time
        float mutationRate = 0.1f;             // 0.0-1.0: Probability of note mutations
        float crossoverRate = 0.3f;            // 0.0-1.0: Pattern crossover probability
        float variationIntensity = 0.5f;       // 0.0-1.0: How dramatic variations are
        
        enum class VariationScope
        {
            Rhythm,         // Only vary rhythmic elements
            Melody,         // Only vary melodic elements
            Harmony,        // Only vary harmonic elements
            All             // Vary all elements
        };
        VariationScope scope = VariationScope::All;
        
        enum class VariationTiming
        {
            Gradual,        // Smooth, continuous evolution
            Stepped,        // Discrete changes at intervals
            Random,         // Unpredictable timing
            OnBeat          // Changes synchronized to musical beats
        };
        VariationTiming timing = VariationTiming::Gradual;
        
        float evolutionSpeed = 0.2f;           // 0.0-1.0: How quickly patterns evolve
        int generationHistory = 10;            // Number of previous generations to remember
        bool allowReversion = true;            // Allow returning to previous patterns
    };
    
    VariationSettings variation;

    // Epic 6 Story 6.3: Advanced Chord Progressions
    struct HarmonicSettings
    {
        bool enableAdvancedHarmony = false;     // Enable sophisticated chord progressions
        
        enum class HarmonicComplexity
        {
            Basic,          // Triads only (I, IV, V, vi, etc.)
            Intermediate,   // 7th chords and inversions
            Advanced,       // Extensions, alterations, substitutions
            Jazz            // Full jazz harmony vocabulary
        };
        HarmonicComplexity complexity = HarmonicComplexity::Basic;
        
        enum class ProgressionStyle
        {
            Classical,      // Traditional functional harmony
            Jazz,           // Jazz standards and modern jazz
            Pop,            // Contemporary pop progressions
            Electronic,     // EDM and electronic music harmony
            Modal,          // Modal harmony and progressions
            Cinematic,      // Film score harmonic language
            Experimental    // Unconventional progressions
        };
        ProgressionStyle style = ProgressionStyle::Pop;
        
        float chromaticism = 0.2f;              // 0.0-1.0: Amount of chromatic harmony
        float modalInterchange = 0.1f;          // 0.0-1.0: Borrowing from parallel modes
        float secondaryDominants = 0.0f;        // 0.0-1.0: Use of secondary dominants
        float substitutionRate = 0.0f;          // 0.0-1.0: Tritone/chord substitutions
        
        enum class VoiceLeadingPriority
        {
            Smooth,         // Prioritize smooth voice leading
            Dramatic,       // Allow larger leaps for effect
            Balanced        // Balance smoothness with harmonic interest
        };
        VoiceLeadingPriority voiceLeading = VoiceLeadingPriority::Smooth;
        
        float harmonicRhythm = 0.5f;            // 0.0-1.0: How often chords change
        bool enableReharmonization = false;     // Allow chord substitutions
        bool enforceKeyCenter = true;           // Stay within established key
        
        // Tension and resolution settings
        struct TensionSettings
        {
            float tensionCurve = 0.5f;          // 0.0-1.0: How tension builds over time
            float resolutionStrength = 0.7f;   // 0.0-1.0: How strong resolutions are
            bool enableClimax = true;           // Build to harmonic climax
            float climaxPosition = 0.75f;       // 0.0-1.0: Where in progression climax occurs
        };
        TensionSettings tension;
    };
    
    HarmonicSettings harmony;

    // Type of pattern to generate
    enum class GenerationType
    {
        Melody = 0,
        Chords,
        Bassline,
        Drums
    };
    GenerationType generationType = GenerationType::Melody;
    
    // AI generation mode selection
    enum class AIMode
    {
        Fast = 0,      // Rule-based algorithms, sub-2-second generation
        Quality,       // ONNX-based local ML models, 3-5 second generation
        Cloud          // Premium cloud API, 5-8 second generation
    };
    AIMode aiMode = AIMode::Fast;
    
    // Deterministic seed for reproducible results (0 = random)
    uint32_t generationSeed = 0;
    
    // Pattern length in beats
    float patternLengthBeats = 16.0f;
    
    //==============================================================================
    // Utility methods
    
    /** Returns the musical key as a string (C, C#, D, etc.) */
    juce::String getKeyString() const
    {
        const char* keyNames[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
        return juce::String(keyNames[key % 12]);
    }
    
    /** Returns the scale type as a string */
    juce::String getScaleString() const
    {
        switch (scale)
        {
            case ScaleType::Major:      return "Major";
            case ScaleType::Minor:      return "Minor";
            case ScaleType::Pentatonic: return "Pentatonic";
            case ScaleType::Blues:      return "Blues";
            case ScaleType::Dorian:     return "Dorian";
            case ScaleType::Mixolydian: return "Mixolydian";
            default:                    return "Major";
        }
    }
    
    /** Returns the generation type as a string */
    juce::String getGenerationTypeString() const
    {
        switch (generationType)
        {
            case GenerationType::Melody:  return "Melody";
            case GenerationType::Chords:  return "Chords";
            case GenerationType::Bassline: return "Bassline";
            case GenerationType::Drums:   return "Drums";
            default:                      return "Melody";
        }
    }
    
    /** Returns the AI mode as a string */
    juce::String getAIModeString() const
    {
        switch (aiMode)
        {
            case AIMode::Fast:    return "Fast Mode";
            case AIMode::Quality: return "Quality Mode";
            case AIMode::Cloud:   return "Cloud Mode";
            default:              return "Fast Mode";
        }
    }
    
    /** Returns expected generation time in seconds for current AI mode */
    float getExpectedGenerationTime() const
    {
        switch (aiMode)
        {
            case AIMode::Fast:    return 1.5f;  // Sub-2-second target
            case AIMode::Quality: return 4.0f;  // 3-5 second target
            case AIMode::Cloud:   return 6.5f;  // 5-8 second target
            default:              return 1.5f;
        }
    }
};
