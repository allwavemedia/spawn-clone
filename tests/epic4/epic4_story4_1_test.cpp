/*
  ==============================================================================

    Epic4_Story4_1_Test.cpp
    Created: 29 Jul 2025
    Author:  Epic 4 Implementation Test

    Comprehensive test suite for Epic 4 Story 4.1: Advanced Music Theory Engine
    Tests all music theory functionality including scales, chords, progressions,
    voice leading, harmonic analysis, and AI integration.

  ==============================================================================
*/

#include <iostream>
#include <vector>
#include <string>
#include <cassert>

// Include the Epic 4 implementation
#include "Source/ai/MusicTheoryEngine.h"
#include "Source/GenerationParameters.h"
#include "Source/MIDIPattern.h"

class Epic4Story41Test
{
public:
    void runAllTests()
    {
        std::cout << "🎼 EPIC 4 STORY 4.1: Advanced Music Theory Engine Tests\n";
        std::cout << "========================================================\n\n";
        
        testScaleDefinitions();
        testChordGeneration();
        testChordProgressions();
        testVoiceLeading();
        testHarmonicTension();
        testMelodyGeneration();
        testBassLineGeneration();
        testPatternAnalysis();
        testAIIntegration();
        
        std::cout << "\n✅ All Epic 4 Story 4.1 tests completed successfully!\n";
        std::cout << "🎯 Advanced Music Theory Engine is production ready\n\n";
    }

private:
    MusicTheoryEngine theoryEngine;
    
    void testScaleDefinitions()
    {
        std::cout << "🎵 Testing Task 4.1.1: Scale and Mode Definitions...\n";
        
        // Test major scale
        auto majorScale = theoryEngine.getScaleNotes(0, MusicTheoryEngine::ScaleType::Major);
        std::vector<int> expectedMajor = {0, 2, 4, 5, 7, 9, 11};
        assert(majorScale == expectedMajor);
        
        // Test natural minor scale
        auto minorScale = theoryEngine.getScaleNotes(0, MusicTheoryEngine::ScaleType::NaturalMinor);
        std::vector<int> expectedMinor = {0, 2, 3, 5, 7, 8, 10};
        assert(minorScale == expectedMinor);
        
        // Test pentatonic scale
        auto pentatonicScale = theoryEngine.getScaleNotes(0, MusicTheoryEngine::ScaleType::Pentatonic);
        std::vector<int> expectedPentatonic = {0, 2, 4, 7, 9};
        assert(pentatonicScale == expectedPentatonic);
        
        // Test note in scale detection
        assert(theoryEngine.isNoteInScale(4, 0, MusicTheoryEngine::ScaleType::Major)); // E in C major
        assert(!theoryEngine.isNoteInScale(1, 0, MusicTheoryEngine::ScaleType::Major)); // C# not in C major
        
        // Test scale degree calculation
        assert(theoryEngine.getScaleDegree(7, 0, MusicTheoryEngine::ScaleType::Major) == 5); // G is 5th degree
        assert(theoryEngine.getScaleDegree(1, 0, MusicTheoryEngine::ScaleType::Major) == -1); // C# not in scale
        
        std::cout << "   ✅ Scale definitions working correctly\n";
    }
    
    void testChordGeneration()
    {
        std::cout << "🎵 Testing Task 4.1.2: Chord Generation and Analysis...\n";
        
        // Test major chord generation
        auto cMajor = theoryEngine.generateChord(0, MusicTheoryEngine::ChordType::Major);
        assert(cMajor.root == 0);
        assert(cMajor.type == MusicTheoryEngine::ChordType::Major);
        assert(cMajor.notes.size() == 3);
        assert(cMajor.notes[0] == 0 && cMajor.notes[1] == 4 && cMajor.notes[2] == 7); // C-E-G
        
        // Test minor chord generation
        auto cMinor = theoryEngine.generateChord(0, MusicTheoryEngine::ChordType::Minor);
        assert(cMinor.notes[0] == 0 && cMinor.notes[1] == 3 && cMinor.notes[2] == 7); // C-Eb-G
        
        // Test dominant 7 chord
        auto cDom7 = theoryEngine.generateChord(0, MusicTheoryEngine::ChordType::Dominant7);
        assert(cDom7.notes.size() == 4);
        assert(cDom7.notes[3] == 10); // Bb
        
        // Test chord naming
        assert(cMajor.getName() == "C");
        assert(cMinor.getName() == "Cm");
        assert(cDom7.getName() == "C7");
        
        // Test chord inversion
        auto cMajorFirstInv = theoryEngine.generateChord(0, MusicTheoryEngine::ChordType::Major, 1);
        assert(cMajorFirstInv.notes[0] == 16); // E moved to next octave
        
        std::cout << "   ✅ Chord generation working correctly\n";
    }
    
    void testChordProgressions()
    {
        std::cout << "🎵 Testing Task 4.1.2: Chord Progressions...\n";
        
        // Test diatonic progression generation
        std::vector<int> romanNumerals = {1, 5, 6, 4}; // I-V-vi-IV
        auto progression = theoryEngine.generateDiatonicProgression(
            0, MusicTheoryEngine::ScaleType::Major, romanNumerals);
        
        assert(progression.size() == 4);
        assert(progression[0].root == 0); // I = C
        assert(progression[1].root == 7); // V = G
        assert(progression[2].root == 9); // vi = Am
        assert(progression[3].root == 5); // IV = F
        
        // Test common progressions retrieval
        auto popProgressions = theoryEngine.getCommonProgressions("pop");
        assert(!popProgressions.empty());
        
        auto jazzProgressions = theoryEngine.getCommonProgressions("jazz");
        assert(!jazzProgressions.empty());
        
        std::cout << "   ✅ Chord progressions working correctly\n";
    }
    
    void testVoiceLeading()
    {
        std::cout << "🎵 Testing Task 4.1.3: Voice Leading Analysis...\n";
        
        // Create two chords for voice leading test
        auto chord1 = theoryEngine.generateChord(0, MusicTheoryEngine::ChordType::Major); // C major
        auto chord2 = theoryEngine.generateChord(5, MusicTheoryEngine::ChordType::Major); // F major
        
        // Test voice leading quality calculation
        float quality = theoryEngine.calculateVoiceLeadingQuality(chord1, chord2);
        assert(quality >= 0.0f && quality <= 1.0f);
        
        // Test voice leading distance calculation
        int distance = theoryEngine.getVoiceLeadingDistance(chord1, chord2);
        assert(distance >= 0);
        
        // Test parallel fifths/octaves detection
        bool hasParallels = theoryEngine.hasParallelFifthsOctaves(chord1, chord2);
        // This should be false for C major to F major
        assert(!hasParallels);
        
        // Test voice leading optimization
        auto optimizedChord2 = theoryEngine.optimizeVoiceLeading(chord1, chord2);
        float originalQuality = theoryEngine.calculateVoiceLeadingQuality(chord1, chord2);
        float optimizedQuality = theoryEngine.calculateVoiceLeadingQuality(chord1, optimizedChord2);
        assert(optimizedQuality >= originalQuality);
        
        std::cout << "   ✅ Voice leading analysis working correctly\n";
    }
    
    void testHarmonicTension()
    {
        std::cout << "🎵 Testing Task 4.1.4: Harmonic Tension Analysis...\n";
        
        // Test interval detection
        auto interval = theoryEngine.getInterval(0, 7); // Perfect fifth
        assert(interval == MusicTheoryEngine::IntervalType::PerfectFifth);
        
        auto tritone = theoryEngine.getInterval(0, 6); // Tritone
        assert(tritone == MusicTheoryEngine::IntervalType::Tritone);
        
        // Test interval tension values
        float fifthTension = theoryEngine.getIntervalTension(MusicTheoryEngine::IntervalType::PerfectFifth);
        float tritoneTension = theoryEngine.getIntervalTension(MusicTheoryEngine::IntervalType::Tritone);
        assert(tritoneTension > fifthTension); // Tritone more tense than fifth
        
        // Test chord tension calculation
        auto majorChord = theoryEngine.generateChord(0, MusicTheoryEngine::ChordType::Major);
        auto diminishedChord = theoryEngine.generateChord(0, MusicTheoryEngine::ChordType::Diminished);
        
        float majorTension = theoryEngine.calculateHarmonicTension(majorChord);
        float diminishedTension = theoryEngine.calculateHarmonicTension(diminishedChord);
        assert(diminishedTension > majorTension); // Diminished more tense than major
        
        // Test tension resolution
        auto resolutions = theoryEngine.getTensionResolutions(
            diminishedChord, 0, MusicTheoryEngine::ScaleType::Major);
        assert(!resolutions.empty());
        
        std::cout << "   ✅ Harmonic tension analysis working correctly\n";
    }
    
    void testMelodyGeneration()
    {
        std::cout << "🎵 Testing Task 4.1.5: Melody Generation...\n";
        
        // Create chord progression for melody generation
        std::vector<int> romanNumerals = {1, 5, 6, 4};
        auto progression = theoryEngine.generateDiatonicProgression(
            0, MusicTheoryEngine::ScaleType::Major, romanNumerals);
        
        // Generate melody
        auto melody = theoryEngine.generateMelody(progression, 0, 
            MusicTheoryEngine::ScaleType::Major, 4.0, 120.0);
        
        assert(!melody.empty());
        
        // Verify melody notes are in the correct scale
        for (const auto& note : melody)
        {
            assert(theoryEngine.isNoteInScale(note.pitch, 0, MusicTheoryEngine::ScaleType::Major));
        }
        
        // Test melody harmonization
        auto harmonization = theoryEngine.harmonizeMelody(melody, 0, MusicTheoryEngine::ScaleType::Major);
        assert(!harmonization.empty());
        
        std::cout << "   ✅ Melody generation working correctly\n";
    }
    
    void testBassLineGeneration()
    {
        std::cout << "🎵 Testing Task 4.1.5: Bass Line Generation...\n";
        
        // Create chord progression for bass line generation
        std::vector<int> romanNumerals = {1, 5, 6, 4};
        auto progression = theoryEngine.generateDiatonicProgression(
            0, MusicTheoryEngine::ScaleType::Major, romanNumerals);
        
        // Generate bass line
        auto bassLine = theoryEngine.generateBassLine(progression, 4.0, 120.0);
        
        assert(!bassLine.empty());
        assert(bassLine.size() >= progression.size()); // At least one note per chord
        
        // Verify bass notes are in bass register (lower octaves)
        for (const auto& note : bassLine)
        {
            assert(note.pitch <= 48); // Should be below middle C
        }
        
        std::cout << "   ✅ Bass line generation working correctly\n";
    }
    
    void testPatternAnalysis()
    {
        std::cout << "🎵 Testing Task 4.1.5: Pattern Analysis...\n";
        
        // Create test pattern
        MIDIPattern pattern;
        pattern.notes.push_back({60, 0.0, 1.0, 80}); // C4
        pattern.notes.push_back({64, 1.0, 1.0, 80}); // E4
        pattern.notes.push_back({67, 2.0, 1.0, 80}); // G4
        
        // Create test parameters
        GenerationParameters params;
        params.key = 0; // C major
        params.scale = GenerationParameters::ScaleType::Major;
        
        // Analyze pattern
        auto analysis = theoryEngine.analyzePattern(pattern, params);
        
        assert(analysis.scaleCompliance > 0.8f); // Should be high for C major triad
        assert(!analysis.suggestions.empty());
        
        std::cout << "   ✅ Pattern analysis working correctly\n";
    }
    
    void testAIIntegration()
    {
        std::cout << "🎵 Testing Task 4.1.5: AI Generation Pipeline Integration...\n";
        
        // Create test pattern with some out-of-scale notes
        MIDIPattern pattern;
        pattern.notes.push_back({60, 0.0, 1.0, 80}); // C4 (in scale)
        pattern.notes.push_back({61, 1.0, 1.0, 80}); // C#4 (out of scale)
        pattern.notes.push_back({64, 2.0, 1.0, 80}); // E4 (in scale)
        
        // Create test parameters
        GenerationParameters params;
        params.key = 0; // C major
        params.scale = GenerationParameters::ScaleType::Major;
        
        // Apply theory constraints
        theoryEngine.applyTheoryConstraints(pattern, params);
        
        // Verify all notes are now in scale
        for (const auto& note : pattern.notes)
        {
            assert(theoryEngine.isNoteInScale(note.pitch, 0, MusicTheoryEngine::ScaleType::Major));
        }
        
        std::cout << "   ✅ AI integration working correctly\n";
    }
};

int main()
{
    try
    {
        Epic4Story41Test test;
        test.runAllTests();
        
        std::cout << "🎼 EPIC 4 STORY 4.1 IMPLEMENTATION COMPLETE\n";
        std::cout << "==========================================\n";
        std::cout << "✅ Advanced Music Theory Engine fully operational\n";
        std::cout << "✅ Scale definitions and mode analysis\n";
        std::cout << "✅ Chord generation and progression analysis\n";
        std::cout << "✅ Voice leading optimization\n";
        std::cout << "✅ Harmonic tension and resolution\n";
        std::cout << "✅ Intelligent melody and bass generation\n";
        std::cout << "✅ Pattern theory analysis and validation\n";
        std::cout << "✅ Full AI generation pipeline integration\n\n";
        std::cout << "🚀 Ready for Epic 4 Story 4.2: Genre Intelligence System\n";
        
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "❌ Test failed: " << e.what() << std::endl;
        return 1;
    }
}
