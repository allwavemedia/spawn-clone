/*
  ==============================================================================

    MusicTheoryEngine.h
    Created: 29 Jul 2025
    Author:  Epic 4 Story 4.1 Implementation

    Advanced music theory engine for intelligent pattern generation with
    proper harmonic analysis, scale theory, and voice leading.

  ==============================================================================
*/

#pragma once

#include <juce_core/juce_core.h>
#include "MIDIPattern.h"
#include "GenerationParameters.h"
#include <vector>
#include <unordered_map>
#include <string>

//==============================================================================
/**
 * Epic 4 Story 4.1: Advanced Music Theory Engine
 * 
 * Provides sophisticated music theory analysis and generation capabilities
 * including scales, chord progressions, voice leading, and harmonic analysis.
 */
class MusicTheoryEngine
{
public:
    //==============================================================================
    MusicTheoryEngine();
    ~MusicTheoryEngine();
    
    //==============================================================================
    // Task 4.1.1: Scale and Mode Definitions
    
    enum class ScaleType
    {
        Major = 0,
        NaturalMinor,
        HarmonicMinor,
        MelodicMinor,
        Dorian,
        Phrygian,
        Lydian,
        Mixolydian,
        Aeolian,
        Locrian,
        Pentatonic,
        BluesScale,
        WholeTone,
        Chromatic
    };
    
    /** Get scale intervals for a given scale type */
    std::vector<int> getScaleIntervals(ScaleType scaleType) const;
    
    /** Get scale notes for a given key and scale type */
    std::vector<int> getScaleNotes(int rootNote, ScaleType scaleType) const;
    
    /** Check if a note is in the given scale */
    bool isNoteInScale(int note, int rootNote, ScaleType scaleType) const;
    
    /** Get the scale degree of a note (1-7, or -1 if not in scale) */
    int getScaleDegree(int note, int rootNote, ScaleType scaleType) const;
    
    //==============================================================================
    // Task 4.1.2: Chord Progression Analysis and Generation
    
    enum class ChordType
    {
        Major,
        Minor,
        Diminished,
        Augmented,
        Major7,
        Minor7,
        Dominant7,
        Diminished7,
        HalfDiminished7,
        Major9,
        Minor9,
        Dominant9,
        Sus2,
        Sus4,
        Add9
    };
    
    struct Chord
    {
        int root;                    // Root note (0-11)
        ChordType type;              // Chord quality
        std::vector<int> notes;      // Chord tones
        int inversion = 0;           // 0=root position, 1=first inversion, etc.
        
        juce::String getName() const;
    };
    
    /** Generate chord from root and type */
    Chord generateChord(int root, ChordType type, int inversion = 0) const;
    
    /** Analyze chord from a set of notes */
    Chord analyzeChord(const std::vector<int>& notes) const;
    
    /** Generate diatonic chord progression for a key */
    std::vector<Chord> generateDiatonicProgression(int key, ScaleType scale, 
                                                   const std::vector<int>& romanNumerals) const;
    
    /** Get common chord progressions for different genres */
    std::vector<std::vector<int>> getCommonProgressions(const juce::String& genre) const;
    
    //==============================================================================
    // Task 4.1.3: Voice Leading and Harmonic Movement
    
    /** Calculate voice leading quality between two chords (0.0-1.0, higher = better) */
    float calculateVoiceLeadingQuality(const Chord& from, const Chord& to) const;
    
    /** Generate smooth voice leading between chords */
    Chord optimizeVoiceLeading(const Chord& previousChord, const Chord& targetChord) const;
    
    /** Check for parallel fifths/octaves (returns true if found) */
    bool hasParallelFifthsOctaves(const Chord& chord1, const Chord& chord2) const;
    
    /** Get the voice leading distance between two chords */
    int getVoiceLeadingDistance(const Chord& from, const Chord& to) const;
    
    //==============================================================================
    // Task 4.1.4: Interval and Tension Resolution
    
    enum class IntervalType
    {
        Unison, MinorSecond, MajorSecond, MinorThird, MajorThird,
        PerfectFourth, Tritone, PerfectFifth, MinorSixth, MajorSixth,
        MinorSeventh, MajorSeventh, Octave
    };
    
    /** Get interval type between two notes */
    IntervalType getInterval(int note1, int note2) const;
    
    /** Get interval quality (consonant, mildly dissonant, highly dissonant) */
    float getIntervalTension(IntervalType interval) const;
    
    /** Calculate harmonic tension of a chord (0.0-1.0) */
    float calculateHarmonicTension(const Chord& chord) const;
    
    /** Get resolution suggestions for high-tension chords */
    std::vector<Chord> getTensionResolutions(const Chord& tensionChord, int key, ScaleType scale) const;
    
    //==============================================================================
    // Task 4.1.5: AI Generation Pipeline Integration
    
    /** Apply music theory constraints to a generated pattern */
    void applyTheoryConstraints(MIDIPattern& pattern, const GenerationParameters& params) const;
    
    /** Generate harmonically aware bass line for a chord progression */
    std::vector<Note> generateBassLine(const std::vector<Chord>& progression, 
                                       double patternLength, double tempo) const;
    
    /** Generate melody that follows harmonic context */
    std::vector<Note> generateMelody(const std::vector<Chord>& progression,
                                     int key, ScaleType scale,
                                     double patternLength, double tempo) const;
    
    /** Harmonize a melody with appropriate chords */
    std::vector<Chord> harmonizeMelody(const std::vector<Note>& melody, 
                                       int key, ScaleType scale) const;
    
    /** Validate pattern for music theory compliance */
    struct TheoryAnalysis
    {
        float harmonicCoherence = 0.0f;      // 0.0-1.0
        float voiceLeadingQuality = 0.0f;    // 0.0-1.0
        float scaleCompliance = 0.0f;        // 0.0-1.0
        std::vector<juce::String> suggestions;
        bool isTheoricallySound = false;
    };
    
    TheoryAnalysis analyzePattern(const MIDIPattern& pattern, 
                                  const GenerationParameters& params) const;

private:
    //==============================================================================
    // Internal data structures
    
    std::unordered_map<ScaleType, std::vector<int>> scaleIntervals;
    std::unordered_map<ChordType, std::vector<int>> chordIntervals;
    std::unordered_map<juce::String, std::vector<std::vector<int>>> genreProgressions;
    
    // Circle of fifths and harmonic function tables
    std::vector<int> circleOfFifths;
    std::unordered_map<int, juce::String> harmonicFunctions;
    
    //==============================================================================
    // Helper methods
    
    /** Initialize scale interval definitions */
    void initializeScales();
    
    /** Initialize chord interval definitions */
    void initializeChords();
    
    /** Initialize common progressions by genre */
    void initializeProgressions();
    
    /** Normalize note to 0-11 range */
    int normalizeNote(int note) const { return ((note % 12) + 12) % 12; }
    
    /** Calculate interval in semitones */
    int calculateInterval(int note1, int note2) const;
    
    /** Get chord notes in a specific voicing */
    std::vector<int> getChordVoicing(const Chord& chord, int octave = 4) const;
    
    /** Find the best inversion for voice leading */
    int findBestInversion(const Chord& previousChord, const Chord& targetChord) const;
    
    /** Generate scale-appropriate melody contour */
    std::vector<int> generateMelodyContour(int length, const std::vector<int>& scaleNotes) const;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MusicTheoryEngine)
};
