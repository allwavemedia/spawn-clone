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
    
    // Type of pattern to generate
    enum class GenerationType
    {
        Melody = 0,
        Chords,
        Bassline,
        Drums
    };
    GenerationType generationType = GenerationType::Melody;
    
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
};
