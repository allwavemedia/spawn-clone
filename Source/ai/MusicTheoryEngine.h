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
#include <random>

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
    
    enum class Key
    {
        C_MAJOR = 0,     // C Major
        Db_MAJOR = 1,    // Db Major
        D_MAJOR = 2,     // D Major
        Eb_MAJOR = 3,    // Eb Major
        E_MAJOR = 4,     // E Major
        F_MAJOR = 5,     // F Major
        Gb_MAJOR = 6,    // Gb Major
        G_MAJOR = 7,     // G Major
        Ab_MAJOR = 8,    // Ab Major
        A_MAJOR = 9,     // A Major
        Bb_MAJOR = 10,   // Bb Major
        B_MAJOR = 11,    // B Major
        
        C_MINOR = 12,    // C Minor
        Db_MINOR = 13,   // Db Minor
        D_MINOR = 14,    // D Minor
        Eb_MINOR = 15,   // Eb Minor
        E_MINOR = 16,    // E Minor
        F_MINOR = 17,    // F Minor
        Gb_MINOR = 18,   // Gb Minor
        G_MINOR = 19,    // G Minor
        Ab_MINOR = 20,   // Ab Minor
        A_MINOR = 21,    // A Minor
        Bb_MINOR = 22,   // Bb Minor
        B_MINOR = 23     // B Minor
    };
    
    /** Convert Key enum to root note (0-11) */
    static int getKeyRootNote(Key key) 
    {
        return static_cast<int>(key) % 12;
    }
    
    /** Convert Key enum to ScaleType */
    static ScaleType getKeyScaleType(Key key) 
    {
        return static_cast<int>(key) < 12 ? ScaleType::Major : ScaleType::NaturalMinor;
    }
    
    /** Create Key from root note and scale type */
    static Key createKey(int rootNote, ScaleType scaleType)
    {
        int offset = (scaleType == ScaleType::Major) ? 0 : 12;
        return static_cast<Key>((rootNote % 12) + offset);
    }
    
    /** Get scale intervals for a given scale type */
    std::vector<int> getScaleIntervals(ScaleType scaleType) const;
    
    /** Get scale notes for a given key and scale type */
    std::vector<int> getScaleNotes(int rootNote, ScaleType scaleType) const;
    
    /** Get scale notes for a given key */
    std::vector<int> getScaleNotes(Key key) const
    {
        return getScaleNotes(getKeyRootNote(key), getKeyScaleType(key));
    }
    
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

    /** Find the best inversion for voice leading */
    int findBestInversion(const Chord& previousChord, const Chord& targetChord) const;
    
    /** Generate scale-appropriate melody contour */
    std::vector<int> generateMelodyContour(int length, const std::vector<int>& scaleNotes) const;

    // Epic 6 Story 6.1: Advanced Generation Functions
    std::vector<int> generateMelodicContour(const std::vector<int>& scale,
                                            int numNotes,
                                            GenerationParameters::MelodicContour contourShape,
                                            float intervalVariety);
                                            
    std::vector<double> generateRhythmicPattern(int numNotes,
                                                double totalBeats,
                                                float complexity,
                                                float density,
                                                float swing);
                                                
    // Epic 6 Story 6.2: Dynamic Pattern Variation
    struct PatternGenome
    {
        std::vector<Note> notes;
        float fitness = 0.0f;           // Quality score for evolutionary selection
        int generation = 0;             // Which generation this pattern belongs to
        juce::String id;                // Unique identifier for this pattern
        double timestamp = 0.0;         // When this pattern was created
    };
    
    /** Evolve a pattern using genetic algorithm principles */
    PatternGenome evolvePattern(const PatternGenome& parent1, 
                               const PatternGenome& parent2,
                               const GenerationParameters& params);
    
    /** Mutate a pattern based on mutation rate and scope */
    PatternGenome mutatePattern(const PatternGenome& pattern,
                               const GenerationParameters& params);
    
    /** Apply crossover between two patterns */
    PatternGenome crossoverPatterns(const PatternGenome& parent1,
                                   const PatternGenome& parent2,
                                   const GenerationParameters& params);
    
    /** Calculate fitness score for a pattern */
    float calculatePatternFitness(const PatternGenome& pattern,
                                 const GenerationParameters& params);
    
    /** Morph smoothly between two patterns */
    PatternGenome morphPatterns(const PatternGenome& source,
                               const PatternGenome& target,
                               float morphAmount);  // 0.0 = source, 1.0 = target
    
    /** Generate variations of an existing pattern */
    std::vector<PatternGenome> generateVariations(const PatternGenome& basePattern,
                                                 int numVariations,
                                                 const GenerationParameters& params);
    
    /** Select best patterns from a population */
    std::vector<PatternGenome> selectElitePatterns(const std::vector<PatternGenome>& population,
                                                  int numElite);
    
    /** Convert MIDIPattern to PatternGenome */
    PatternGenome convertToGenome(const MIDIPattern& pattern);
    
    /** Convert PatternGenome to MIDIPattern */
    MIDIPattern convertFromGenome(const PatternGenome& genome);
    
    class PatternEvolutionEngine
    {
    public:
        PatternEvolutionEngine(MusicTheoryEngine* engine);
        
        /** Initialize with seed patterns */
        void initializePopulation(const std::vector<MIDIPattern>& seedPatterns,
                                const GenerationParameters& params);
        
        /** Evolve population for one generation */
        void evolveGeneration(const GenerationParameters& params);
        
        /** Get current best patterns */
        std::vector<PatternGenome> getCurrentElite(int count = 5);
        
        /** Get pattern at specific generation */
        PatternGenome getPatternFromHistory(int generation, int index = 0);
        
        /** Revert to previous generation */
        bool revertToGeneration(int targetGeneration);
        
        /** Get evolution statistics */
        struct EvolutionStats
        {
            int currentGeneration = 0;
            float averageFitness = 0.0f;
            float bestFitness = 0.0f;
            int populationSize = 0;
            std::vector<float> fitnessHistory;
        };
        EvolutionStats getStats() const;
        
    private:
        MusicTheoryEngine* engine;
        std::vector<std::vector<PatternGenome>> generationHistory;
        std::vector<PatternGenome> currentPopulation;
        int currentGeneration = 0;
        std::mt19937 evolutionRng;
        
        void maintainDiversity();
        float calculateDiversity() const;
    };
    
    // Epic 6 Story 6.3: Advanced Chord Progressions
    
    /** Extended chord types beyond basic triads */
    enum class ExtendedChordType
    {
        // Basic triads
        Major, Minor, Diminished, Augmented,
        
        // 7th chords
        Major7, Minor7, Dominant7, Diminished7, HalfDiminished7,
        
        // Extended chords
        Major9, Minor9, Dominant9, Major11, Minor11, Dominant11,
        Major13, Minor13, Dominant13,
        
        // Altered dominants
        Dominant7b5, Dominant7sharp5, Dominant7b9, Dominant7sharp9,
        Dominant7sharp11, Dominant7b13,
        
        // Sus chords
        Sus2, Sus4, Dominant7Sus4,
        
        // Add chords
        Add9, Add11, MinorAdd9,
        
        // Special chords
        Neapolitan, AugmentedSixth, French6, German6, Italian6
    };
    
    struct AdvancedChord
    {
        int root;                           // Root note (0-11)
        ExtendedChordType type;            // Extended chord quality
        std::vector<int> notes;            // All chord tones
        std::vector<int> extensions;       // 9th, 11th, 13th extensions
        std::vector<int> alterations;      // b5, #5, b9, #9, etc.
        int bass = -1;                     // Bass note for slash chords (-1 = root in bass)
        int inversion = 0;                 // 0=root, 1=first, 2=second, etc.
        
        // Harmonic function analysis
        enum class Function { Tonic, Subdominant, Dominant, Other };
        Function function = Function::Other;
        
        // Voice leading information
        std::vector<int> voicing;          // Specific voicing with octave info
        float tension = 0.0f;              // Harmonic tension level (0.0-1.0)
        
        juce::String getName() const;
        juce::String getRomanNumeral(int key, ScaleType scale) const;
    };
    
    /** Generate advanced chord from root and extended type */
    AdvancedChord generateAdvancedChord(int root, ExtendedChordType type, 
                                       const GenerationParameters& params) const;
    
    /** Analyze advanced chord from notes */
    AdvancedChord analyzeAdvancedChord(const std::vector<int>& notes) const;
    
    /** Generate sophisticated chord progression */
    std::vector<AdvancedChord> generateAdvancedProgression(
        int key, ScaleType scale,
        const GenerationParameters& params,
        int numChords = 8) const;
    
    /** Apply reharmonization to existing progression */
    std::vector<AdvancedChord> reharmonizeProgression(
        const std::vector<AdvancedChord>& originalProgression,
        const GenerationParameters& params) const;
    
    /** Generate secondary dominants for a key */
    std::vector<AdvancedChord> generateSecondaryDominants(int key, ScaleType scale) const;
    
    /** Apply modal interchange (borrowed chords) */
    std::vector<AdvancedChord> applyModalInterchange(
        const std::vector<AdvancedChord>& progression,
        int key, ScaleType scale,
        float interchangeAmount) const;
    
    /** Calculate harmonic tension curve for progression */
    std::vector<float> calculateTensionCurve(
        const std::vector<AdvancedChord>& progression,
        const GenerationParameters& params) const;
    
    /** Optimize voice leading between advanced chords */
    std::vector<AdvancedChord> optimizeAdvancedVoiceLeading(
        const std::vector<AdvancedChord>& progression,
        const GenerationParameters& params) const;
    
    /** Generate chord substitutions */
    std::vector<AdvancedChord> generateSubstitutions(
        const AdvancedChord& originalChord,
        const GenerationParameters& params) const;
    
    /** Apply genre-specific harmonic patterns */
    std::vector<AdvancedChord> applyGenreHarmony(
        const std::vector<AdvancedChord>& baseProgression,
        GenerationParameters::HarmonicSettings::ProgressionStyle style,
        const GenerationParameters& params) const;
    
    class AdvancedHarmonyEngine
    {
    public:
        AdvancedHarmonyEngine(MusicTheoryEngine* engine);
        
        /** Generate contextually aware chord progression */
        std::vector<AdvancedChord> generateContextualProgression(
            int key, ScaleType scale,
            const GenerationParameters& params);
        
        /** Evolve harmony over time with increasing complexity */
        std::vector<AdvancedChord> evolveHarmony(
            const std::vector<AdvancedChord>& seedProgression,
            const GenerationParameters& params,
            float evolutionAmount);
        
        /** Analyze harmonic function and provide suggestions */
        struct HarmonicAnalysis
        {
            std::vector<AdvancedChord::Function> functions;
            std::vector<float> tensions;
            float overallCoherence = 0.0f;
            std::vector<juce::String> suggestions;
            bool hasStrongCadence = false;
            float harmonicRhythmScore = 0.0f;
        };
        
        HarmonicAnalysis analyzeProgression(
            const std::vector<AdvancedChord>& progression,
            int key, ScaleType scale) const;
        
        /** Generate variations of chord progression */
        std::vector<std::vector<AdvancedChord>> generateProgressionVariations(
            const std::vector<AdvancedChord>& baseProgression,
            const GenerationParameters& params,
            int numVariations = 5);
        
        /** Initialize progression templates by genre */
        void initializeProgressionTemplates();
        
        /** Analyze harmonic function of individual chord */
        AdvancedChord::Function analyzeHarmonicFunction(
            const AdvancedChord& chord, int key, ScaleType scale) const;
        
    private:
        MusicTheoryEngine* engine;
        
        // Internal progression templates by genre
        std::unordered_map<GenerationParameters::HarmonicSettings::ProgressionStyle,
                          std::vector<std::vector<int>>> progressionTemplates;
        
    };
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
    
    // Epic 6 Story 6.1: Helper for melodic contour
    int calculateNextStep(int noteIndex, int totalNotes, GenerationParameters::MelodicContour contourShape, float intervalVariety, std::mt19937& gen);
    
    // Epic 6 Story 6.3: Style-specific progression generators
    std::vector<AdvancedChord> generateClassicalProgression(int key, ScaleType scale, const GenerationParameters& params, int numChords) const;
    std::vector<AdvancedChord> generateJazzProgression(int key, ScaleType scale, const GenerationParameters& params, int numChords) const;
    std::vector<AdvancedChord> generatePopProgression(int key, ScaleType scale, const GenerationParameters& params, int numChords) const;
    std::vector<AdvancedChord> generateElectronicProgression(int key, ScaleType scale, const GenerationParameters& params, int numChords) const;
    
    std::vector<AdvancedChord> addSecondaryDominants(const std::vector<AdvancedChord>& progression, int key, ScaleType scale, float amount) const;
    float calculateChordTension(const AdvancedChord& chord, int key, MusicTheoryEngine::ScaleType scale) const;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MusicTheoryEngine)
};
