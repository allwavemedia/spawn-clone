/*
  ==============================================================================

    GenreSpecialist.h
    Created: 29 Jul 2025
    Author: BMad Master
    
    Epic 4 Story 4.2: Genre-Specific Intelligence Systems
    
    Base class and implementations for genre-specific pattern generation
    with authentic musical characteristics for Hip Hop, Pop, EDM, R&B.

  ==============================================================================
*/

#pragma once

#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include "../MIDIPattern.h"
#include "../GenerationParameters.h"
#include "MusicTheoryEngine.h"

//==============================================================================
/**
    Genre types supported by the intelligence system
*/
enum class GenreType
{
    HipHop,
    Pop,
    Dance,
    EDM,
    RnB,
    Rock,
    Jazz,
    Latin,
    Funk,
    Generic
};

//==============================================================================
/**
    Rhythm pattern characteristics for different genres
*/
struct RhythmPattern
{
    std::vector<float> kickPattern;        // Kick drum pattern (0.0-1.0 velocity)
    std::vector<float> snarePattern;       // Snare/clap pattern
    std::vector<float> hiHatPattern;       // Hi-hat pattern
    std::vector<float> percussionPattern;  // Additional percussion
    
    float swingAmount = 0.0f;              // Swing/shuffle amount (0.0-1.0)
    float humanization = 0.1f;             // Timing humanization
    int subdivisions = 16;                 // Pattern subdivisions (8, 16, 32)
    
    RhythmPattern() = default;
    RhythmPattern(int subdiv) : subdivisions(subdiv) 
    {
        kickPattern.resize(subdiv, 0.0f);
        snarePattern.resize(subdiv, 0.0f);
        hiHatPattern.resize(subdiv, 0.0f);
        percussionPattern.resize(subdiv, 0.0f);
    }
};

//==============================================================================
/**
    Melodic characteristics for genre-specific generation
*/
struct MelodicCharacteristics
{
    std::vector<int> preferredIntervals;   // Common melodic intervals
    std::vector<int> avoidedIntervals;     // Intervals to avoid
    float stepwiseMotion = 0.7f;           // Preference for stepwise motion (0.0-1.0)
    float leapFrequency = 0.2f;            // Frequency of melodic leaps
    int maxLeapSize = 7;                   // Maximum leap in semitones
    float ornamentation = 0.1f;            // Amount of melodic ornamentation
    bool preferPentatonic = false;         // Prefer pentatonic scales
    float bluenessIntensity = 0.0f;        // Blue note usage (0.0-1.0)
};

//==============================================================================
/**
    Harmonic voicing preferences for different genres
*/
struct HarmonicVoicing
{
    std::vector<int> preferredExtensions;  // Common chord extensions (7, 9, 11, 13)
    std::vector<int> commonInversions;     // Preferred chord inversions
    float voiceSpacing = 1.0f;             // Voice spacing preference (tight=0.5, wide=1.5)
    float dissonanceLevel = 0.3f;          // Acceptable dissonance (0.0-1.0)
    bool preferOpenVoicings = false;       // Open vs closed voicings
    float bassMovement = 0.5f;             // Bass line activity level
};

//==============================================================================
/**
    Base class for all genre specialists
*/
class GenreSpecialist
{
public:
    GenreSpecialist(GenreType type);
    virtual ~GenreSpecialist() = default;
    
    // Core generation methods
    virtual MIDIPattern generateRhythmPattern(const GenerationParameters& params) = 0;
    virtual MIDIPattern generateMelodicPattern(const GenerationParameters& params) = 0;
    virtual MIDIPattern generateBassPattern(const GenerationParameters& params) = 0;
    virtual MIDIPattern generateChordPattern(const GenerationParameters& params) = 0;
    
    // Pattern enhancement and style application
    virtual void applyGenreStyle(MIDIPattern& pattern, const GenerationParameters& params) = 0;
    virtual void addGenreSpecificOrnamentation(MIDIPattern& pattern) = 0;
    virtual float calculateGenreAuthenticity(const MIDIPattern& pattern) = 0;
    
    // Genre characteristics access
    const RhythmPattern& getRhythmTemplate() const { return rhythmTemplate; }
    const MelodicCharacteristics& getMelodicCharacteristics() const { return melodicChars; }
    const HarmonicVoicing& getHarmonicVoicing() const { return harmonicVoicing; }
    
    GenreType getGenreType() const { return genreType; }
    const juce::String& getGenreName() const { return genreName; }
    
    // Tempo and timing characteristics
    virtual std::pair<float, float> getTempoRange() const = 0;
    virtual float getOptimalTempo() const = 0;
    virtual bool prefersQuantization() const { return true; }
    
protected:
    GenreType genreType;
    juce::String genreName;
    
    // Genre-specific characteristics
    RhythmPattern rhythmTemplate;
    MelodicCharacteristics melodicChars;
    HarmonicVoicing harmonicVoicing;
    
    // Helper methods for pattern generation
    void addSwingToPattern(MIDIPattern& pattern, float swingAmount);
    void humanizePattern(MIDIPattern& pattern, float humanization);
    void applyVelocityDynamics(MIDIPattern& pattern, const std::vector<float>& velocityCurve);
    
    // Music theory integration
    std::unique_ptr<MusicTheoryEngine> theoryEngine;
    
    // Common chord progressions for the genre
    virtual std::vector<std::vector<int>> getCommonProgressions() const = 0;
    virtual std::vector<MusicTheoryEngine::ScaleType> getCommonScales() const = 0;
};

//==============================================================================
/**
    Hip Hop specialist - Trap, Boom-bap, modern Hip Hop characteristics
*/
class HipHopSpecialist : public GenreSpecialist
{
public:
    HipHopSpecialist();
    
    // Core generation methods
    MIDIPattern generateRhythmPattern(const GenerationParameters& params) override;
    MIDIPattern generateMelodicPattern(const GenerationParameters& params) override;
    MIDIPattern generateBassPattern(const GenerationParameters& params) override;
    MIDIPattern generateChordPattern(const GenerationParameters& params) override;
    
    // Style application
    void applyGenreStyle(MIDIPattern& pattern, const GenerationParameters& params) override;
    void addGenreSpecificOrnamentation(MIDIPattern& pattern) override;
    float calculateGenreAuthenticity(const MIDIPattern& pattern) override;
    
    // Hip Hop specific characteristics
    std::pair<float, float> getTempoRange() const override { return {70.0f, 160.0f}; }
    float getOptimalTempo() const override { return 140.0f; }
    
    // Hip Hop specific methods
    void addTrapHiHats(MIDIPattern& pattern);
    void add808Pattern(MIDIPattern& pattern);
    void addBoomBapCharacteristics(MIDIPattern& pattern);
    
protected:
    std::vector<std::vector<int>> getCommonProgressions() const override;
    std::vector<MusicTheoryEngine::ScaleType> getCommonScales() const override;
    
private:
    void initializeHipHopCharacteristics();
};

//==============================================================================
/**
    Pop specialist - Commercial pop, hooks, accessible harmonies
*/
class PopSpecialist : public GenreSpecialist
{
public:
    PopSpecialist();
    
    // Core generation methods
    MIDIPattern generateRhythmPattern(const GenerationParameters& params) override;
    MIDIPattern generateMelodicPattern(const GenerationParameters& params) override;
    MIDIPattern generateBassPattern(const GenerationParameters& params) override;
    MIDIPattern generateChordPattern(const GenerationParameters& params) override;
    
    // Style application
    void applyGenreStyle(MIDIPattern& pattern, const GenerationParameters& params) override;
    void addGenreSpecificOrnamentation(MIDIPattern& pattern) override;
    float calculateGenreAuthenticity(const MIDIPattern& pattern) override;
    
    // Pop specific characteristics
    std::pair<float, float> getTempoRange() const override { return {80.0f, 140.0f}; }
    float getOptimalTempo() const override { return 120.0f; }
    
    // Pop specific methods
    void addCatchyHook(MIDIPattern& pattern);
    void addPopRhythmGuitar(MIDIPattern& pattern);
    void addCommercialDrumPattern(MIDIPattern& pattern);
    
protected:
    std::vector<std::vector<int>> getCommonProgressions() const override;
    std::vector<MusicTheoryEngine::ScaleType> getCommonScales() const override;
    
private:
    void initializePopCharacteristics();
};

//==============================================================================
/**
    EDM/Dance specialist - Electronic dance music characteristics
*/
class EDMSpecialist : public GenreSpecialist
{
public:
    EDMSpecialist();
    
    // Core generation methods
    MIDIPattern generateRhythmPattern(const GenerationParameters& params) override;
    MIDIPattern generateMelodicPattern(const GenerationParameters& params) override;
    MIDIPattern generateBassPattern(const GenerationParameters& params) override;
    MIDIPattern generateChordPattern(const GenerationParameters& params) override;
    
    // Style application
    void applyGenreStyle(MIDIPattern& pattern, const GenerationParameters& params) override;
    void addGenreSpecificOrnamentation(MIDIPattern& pattern) override;
    float calculateGenreAuthenticity(const MIDIPattern& pattern) override;
    
    // EDM specific characteristics
    std::pair<float, float> getTempoRange() const override { return {120.0f, 180.0f}; }
    float getOptimalTempo() const override { return 128.0f; }
    bool prefersQuantization() const override { return true; } // EDM is heavily quantized
    
    // EDM specific methods
    void addFourOnTheFloor(MIDIPattern& pattern);
    void addBuildUpElements(MIDIPattern& pattern);
    void addDropCharacteristics(MIDIPattern& pattern);
    void addArpeggiatedSynths(MIDIPattern& pattern);
    
protected:
    std::vector<std::vector<int>> getCommonProgressions() const override;
    std::vector<MusicTheoryEngine::ScaleType> getCommonScales() const override;
    
private:
    void initializeEDMCharacteristics();
};

//==============================================================================
/**
    R&B specialist - Contemporary and classic R&B characteristics
*/
class RnBSpecialist : public GenreSpecialist
{
public:
    RnBSpecialist();
    
    // Core generation methods
    MIDIPattern generateRhythmPattern(const GenerationParameters& params) override;
    MIDIPattern generateMelodicPattern(const GenerationParameters& params) override;
    MIDIPattern generateBassPattern(const GenerationParameters& params) override;
    MIDIPattern generateChordPattern(const GenerationParameters& params) override;
    
    // Style application
    void applyGenreStyle(MIDIPattern& pattern, const GenerationParameters& params) override;
    void addGenreSpecificOrnamentation(MIDIPattern& pattern) override;
    float calculateGenreAuthenticity(const MIDIPattern& pattern) override;
    
    // R&B specific characteristics
    std::pair<float, float> getTempoRange() const override { return {60.0f, 120.0f}; }
    float getOptimalTempo() const override { return 85.0f; }
    bool prefersQuantization() const override { return false; } // R&B has natural feel
    
    // R&B specific methods
    void addSoulfulVoicings(MIDIPattern& pattern);
    void addMelodicOrnamentation(MIDIPattern& pattern);
    void addGospelInfluences(MIDIPattern& pattern);
    void addContemporaryRnBGroove(MIDIPattern& pattern);
    
protected:
    std::vector<std::vector<int>> getCommonProgressions() const override;
    std::vector<MusicTheoryEngine::ScaleType> getCommonScales() const override;
    
private:
    void initializeRnBCharacteristics();
};

//==============================================================================
/**
    Genre detection and management system
*/
class GenreManager
{
public:
    GenreManager();
    ~GenreManager() = default;
    
    // Genre specialist creation and management
    std::unique_ptr<GenreSpecialist> createSpecialist(GenreType genre);
    GenreType detectGenre(const MIDIPattern& pattern);
    GenreType detectGenre(const GenerationParameters& params);
    
    // Genre analysis
    std::vector<std::pair<GenreType, float>> analyzeGenreCompatibility(const MIDIPattern& pattern);
    float calculateGenreDistance(GenreType genre1, GenreType genre2);
    
    // Genre recommendations
    std::vector<GenreType> getRecommendedGenres(const GenerationParameters& params);
    std::vector<GenreType> getCompatibleGenres(GenreType baseGenre);
    
    // Genre information
    static juce::String getGenreName(GenreType genre);
    static juce::String getGenreDescription(GenreType genre);
    static std::vector<GenreType> getAllGenres();
    
private:
    std::map<GenreType, std::unique_ptr<GenreSpecialist>> specialists;
    
    // Genre detection helpers
    float analyzeRhythmicComplexity(const MIDIPattern& pattern);
    float analyzeHarmonicComplexity(const MIDIPattern& pattern);
    float analyzeMelodicCharacter(const MIDIPattern& pattern);
    float analyzeTempoCharacteristics(const GenerationParameters& params);
    
    void initializeGenreRelationships();
    std::map<std::pair<GenreType, GenreType>, float> genreDistances;
};
