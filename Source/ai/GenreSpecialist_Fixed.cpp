/*
  ==============================================================================

    GenreSpecialist_Fixed.cpp
    Created: 29 Jul 2025
    Author: BMad Master
    
    Epic 4 Story 4.2: Genre-Specific Intelligence Systems
    
    Fixed implementation with correct JUCE Note and MIDIPattern structure.

  ==============================================================================
*/

#include "GenreSpecialist.h"
#include <random>
#include <algorithm>
#include <cmath>

//==============================================================================
// GenreSpecialist Base Class Implementation
//==============================================================================

GenreSpecialist::GenreSpecialist(GenreType type)
    : genreType(type)
    , theoryEngine(std::make_unique<MusicTheoryEngine>())
{
    switch (type)
    {
        case GenreType::HipHop:     genreName = "Hip Hop"; break;
        case GenreType::Pop:        genreName = "Pop"; break;
        case GenreType::Dance:      genreName = "Dance"; break;
        case GenreType::EDM:        genreName = "EDM"; break;
        case GenreType::RnB:        genreName = "R&B"; break;
        case GenreType::Rock:       genreName = "Rock"; break;
        case GenreType::Jazz:       genreName = "Jazz"; break;
        case GenreType::Latin:      genreName = "Latin"; break;
        case GenreType::Funk:       genreName = "Funk"; break;
        default:                    genreName = "Generic"; break;
    }
}

void GenreSpecialist::addSwingToPattern(MIDIPattern& pattern, float swingAmount)
{
    if (swingAmount <= 0.0f) return;
    
    for (auto& note : pattern.notes)
    {
        // Apply swing to 16th note subdivisions
        int subdivision = static_cast<int>(note.startTime * 16.0) % 2;
        if (subdivision == 1) // Off-beat 16th notes
        {
            float swingOffset = swingAmount * 0.1f; // Max 10% timing shift
            note.startTime += swingOffset;
        }
    }
}

void GenreSpecialist::humanizePattern(MIDIPattern& pattern, float humanization)
{
    if (humanization <= 0.0f) return;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> timingDist(-humanization * 0.02f, humanization * 0.02f);
    std::uniform_real_distribution<float> velocityDist(-humanization * 10.0f, humanization * 10.0f);
    
    for (auto& note : pattern.notes)
    {
        // Humanize timing (±2% max)
        float timingOffset = timingDist(gen);
        note.startTime = std::max(0.0, note.startTime + timingOffset);
        
        // Humanize velocity (±10 max)
        int velocityOffset = static_cast<int>(velocityDist(gen));
        note.velocity = juce::jlimit(1, 127, note.velocity + velocityOffset);
    }
}

void GenreSpecialist::applyVelocityDynamics(MIDIPattern& pattern, const std::vector<float>& velocityCurve)
{
    if (velocityCurve.empty()) return;
    
    for (auto& note : pattern.notes)
    {
        int curveIndex = static_cast<int>(note.startTime * velocityCurve.size()) % velocityCurve.size();
        float multiplier = velocityCurve[curveIndex];
        note.velocity = juce::jlimit(1, 127, static_cast<int>(note.velocity * multiplier));
    }
}

//==============================================================================
// HipHopSpecialist Implementation
//==============================================================================

HipHopSpecialist::HipHopSpecialist() : GenreSpecialist(GenreType::HipHop)
{
    initializeHipHopCharacteristics();
}

void HipHopSpecialist::initializeHipHopCharacteristics()
{
    // Initialize rhythm template for Hip Hop (32nd note subdivisions for trap)
    rhythmTemplate = RhythmPattern(32);
    
    // Typical Hip Hop kick pattern (on 1 and 3.5)
    rhythmTemplate.kickPattern[0] = 1.0f;    // Beat 1
    rhythmTemplate.kickPattern[14] = 0.8f;   // Beat 3.5 (trap kick)
    
    // Snare on 2 and 4
    rhythmTemplate.snarePattern[8] = 0.9f;   // Beat 2
    rhythmTemplate.snarePattern[24] = 0.9f;  // Beat 4
    
    // Trap hi-hat pattern with complex subdivisions
    for (int i = 0; i < 32; i += 2)
    {
        rhythmTemplate.hiHatPattern[i] = (i % 8 == 0) ? 0.7f : 0.4f; // Accent every 4th hi-hat
    }
    
    // Add trap-style triplet hi-hats
    rhythmTemplate.hiHatPattern[5] = 0.3f;   // Triplet
    rhythmTemplate.hiHatPattern[6] = 0.3f;   // Triplet
    rhythmTemplate.hiHatPattern[13] = 0.3f;  // Triplet
    rhythmTemplate.hiHatPattern[14] = 0.3f;  // Triplet
    
    // Melodic characteristics - prefer pentatonic and minor scales
    melodicChars.preferredIntervals = {1, 2, 3, 5, 7}; // Minor pentatonic intervals
    melodicChars.avoidedIntervals = {6, 11}; // Avoid major 6th and major 7th
    melodicChars.stepwiseMotion = 0.6f;
    melodicChars.leapFrequency = 0.3f;
    melodicChars.maxLeapSize = 12; // Allow octave leaps
    melodicChars.preferPentatonic = true;
    melodicChars.bluenessIntensity = 0.7f; // Strong blues influence
    
    // Harmonic voicing - simple, powerful chords
    harmonicVoicing.preferredExtensions = {7}; // Dominant 7ths common
    harmonicVoicing.commonInversions = {0}; // Root position preferred
    harmonicVoicing.voiceSpacing = 1.2f; // Wider spacing
    harmonicVoicing.dissonanceLevel = 0.4f; // Moderate dissonance
    harmonicVoicing.preferOpenVoicings = true;
    harmonicVoicing.bassMovement = 0.8f; // Active bass lines
}

MIDIPattern HipHopSpecialist::generateRhythmPattern(const GenerationParameters& params)
{
    MIDIPattern pattern;
    pattern.lengthInBeats = params.patternLengthBeats;
    pattern.metadata = params;
    
    float beatLength = 60.0f / params.tempo; // Length of one beat in seconds
    int steps = static_cast<int>(params.patternLengthBeats * 4); // 16th note steps
    
    for (int step = 0; step < steps; ++step)
    {
        double time = step * 0.25; // 16th note timing in beats
        int patternIndex = step % 32;
        
        // Add kick drum
        if (rhythmTemplate.kickPattern[patternIndex] > 0.0f)
        {
            Note kick;
            kick.pitch = 36; // C2 - Kick drum
            kick.startTime = time;
            kick.duration = 0.25; // Quarter note duration
            kick.velocity = static_cast<int>(127 * rhythmTemplate.kickPattern[patternIndex]);
            pattern.notes.push_back(kick);
        }
        
        // Add snare
        if (rhythmTemplate.snarePattern[patternIndex] > 0.0f)
        {
            Note snare;
            snare.pitch = 38; // D2 - Snare
            snare.startTime = time;
            snare.duration = 0.25;
            snare.velocity = static_cast<int>(127 * rhythmTemplate.snarePattern[patternIndex]);
            pattern.notes.push_back(snare);
        }
        
        // Add hi-hat
        if (rhythmTemplate.hiHatPattern[patternIndex] > 0.0f)
        {
            Note hihat;
            hihat.pitch = 42; // F#2 - Closed Hi-hat
            hihat.startTime = time;
            hihat.duration = 0.125; // Eighth note duration
            hihat.velocity = static_cast<int>(127 * rhythmTemplate.hiHatPattern[patternIndex]);
            pattern.notes.push_back(hihat);
        }
    }
    
    // Add trap-specific elements
    addTrapHiHats(pattern);
    add808Pattern(pattern);
    
    return pattern;
}

void HipHopSpecialist::addTrapHiHats(MIDIPattern& pattern)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> velocityDist(0.2f, 0.6f);
    std::uniform_real_distribution<float> timingDist(0.98f, 1.02f);
    
    int steps = static_cast<int>(pattern.lengthInBeats * 8); // 32nd note steps
    
    for (int step = 0; step < steps; ++step)
    {
        // Add rapid hi-hat rolls (trap characteristic)
        if (step % 16 >= 12 && step % 16 <= 15) // Last quarter of each beat
        {
            if (gen() % 3 == 0) // 33% chance for rapid hits
            {
                Note hihat;
                hihat.pitch = 44; // F#2 - Open Hi-hat for rolls
                hihat.startTime = (step * 0.125) * timingDist(gen); // 32nd note timing
                hihat.duration = 0.0625; // Very short duration
                hihat.velocity = static_cast<int>(127 * velocityDist(gen));
                pattern.notes.push_back(hihat);
            }
        }
    }
}

void HipHopSpecialist::add808Pattern(MIDIPattern& pattern)
{
    // Add 808-style bass drum with long decay
    // Add 808 on beat 1 and sometimes on 3.5
    Note bass808;
    bass808.pitch = 24; // C1 - Sub bass
    bass808.startTime = 0.0;
    bass808.duration = 2.0; // Long decay
    bass808.velocity = 110;
    pattern.notes.push_back(bass808);
    
    // Optional 808 on beat 3.5 (trap style)
    if (pattern.lengthInBeats >= 4.0)
    {
        Note bass808_2;
        bass808_2.pitch = 24;
        bass808_2.startTime = 3.5;
        bass808_2.duration = 1.0;
        bass808_2.velocity = 95;
        pattern.notes.push_back(bass808_2);
    }
}

MIDIPattern HipHopSpecialist::generateMelodicPattern(const GenerationParameters& params)
{
    MIDIPattern pattern;
    pattern.lengthInBeats = params.patternLengthBeats;
    pattern.metadata = params;
    
    // Generate pentatonic-based melody
    auto scales = getCommonScales();
    auto scale = scales[0]; // Minor pentatonic
    auto scaleNotes = theoryEngine->getScaleNotes(60, scale); // C minor pentatonic
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> noteChoiceDist(0, static_cast<int>(scaleNotes.size() - 1));
    std::uniform_real_distribution<float> rhythmDist(0.25f, 1.0f);
    
    double currentTime = 0.0;
    
    while (currentTime < pattern.lengthInBeats)
    {
        Note note;
        note.pitch = scaleNotes[noteChoiceDist(gen)] + 12; // Octave up for melody
        note.startTime = currentTime;
        
        float noteDuration = rhythmDist(gen);
        note.duration = noteDuration;
        note.velocity = 80 + (gen() % 30); // Velocity 80-110
        
        pattern.notes.push_back(note);
        currentTime += noteDuration + 0.25; // Add some space
    }
    
    return pattern;
}

MIDIPattern HipHopSpecialist::generateBassPattern(const GenerationParameters& params)
{
    MIDIPattern pattern;
    pattern.lengthInBeats = params.patternLengthBeats;
    pattern.metadata = params;
    
    // Generate bass line using theory engine
    auto progressions = getCommonProgressions();
    auto progression = progressions[0]; // i - VII - VI - VII
    
    double currentTime = 0.0;
    int currentChord = 0;
    
    while (currentTime < pattern.lengthInBeats)
    {
        int chordIndex = currentChord % progression.size();
        int bassNote = 48 + progression[chordIndex]; // C3 + chord root
        
        Note bass;
        bass.pitch = bassNote;
        bass.startTime = currentTime;
        bass.duration = 0.8; // Most of the beat
        bass.velocity = 100;
        pattern.notes.push_back(bass);
        
        currentTime += 1.0; // Move to next beat
        currentChord++;
    }
    
    return pattern;
}

MIDIPattern HipHopSpecialist::generateChordPattern(const GenerationParameters& params)
{
    MIDIPattern pattern;
    pattern.lengthInBeats = params.patternLengthBeats;
    pattern.metadata = params;
    
    // Hip hop often uses simple triads or power chords
    auto progressions = getCommonProgressions();
    auto progression = progressions[0];
    
    double currentTime = 0.0;
    int chordIndex = 0;
    
    while (currentTime < pattern.lengthInBeats && chordIndex < progression.size())
    {
        int root = 60 + progression[chordIndex]; // C4 + chord root
        
        // Simple triad
        std::vector<int> chord = {root, root + 3, root + 7}; // Minor triad
        
        for (int noteNum : chord)
        {
            Note note;
            note.pitch = noteNum;
            note.startTime = currentTime;
            note.duration = 0.9; // Most of the beat
            note.velocity = 70;
            pattern.notes.push_back(note);
        }
        
        currentTime += 1.0; // Move to next beat
        chordIndex++;
    }
    
    return pattern;
}

void HipHopSpecialist::applyGenreStyle(MIDIPattern& pattern, const GenerationParameters& params)
{
    // Apply minimal swing (hip hop is usually straight)
    addSwingToPattern(pattern, 0.1f);
    
    // Add moderate humanization
    humanizePattern(pattern, 0.3f);
    
    // Create velocity dynamics (strong beats emphasized)
    std::vector<float> velocityCurve = {1.0f, 0.7f, 0.9f, 0.7f}; // Accent on 1 and 3
    applyVelocityDynamics(pattern, velocityCurve);
}

void HipHopSpecialist::addGenreSpecificOrnamentation(MIDIPattern& pattern)
{
    // Add some blue notes and slides characteristic of hip hop
    std::random_device rd;
    std::mt19937 gen(rd());
    
    for (auto& note : pattern.notes)
    {
        // 20% chance to add blue note inflection
        if (gen() % 5 == 0)
        {
            if (note.pitch % 12 == 7) // If it's a fifth
            {
                note.pitch -= 1; // Make it a tritone (blue note)
            }
        }
    }
}

float HipHopSpecialist::calculateGenreAuthenticity(const MIDIPattern& pattern)
{
    float score = 0.0f;
    int totalNotes = pattern.notes.size();
    
    if (totalNotes == 0) return 0.0f;
    
    // Check for hip hop characteristics
    int drumNotes = 0;
    int bassNotes = 0;
    int pentatonicNotes = 0;
    
    for (const auto& note : pattern.notes)
    {
        // Count drum notes (typical GM mapping)
        if (note.pitch >= 35 && note.pitch <= 51)
            drumNotes++;
        
        // Count bass notes
        if (note.pitch >= 24 && note.pitch <= 48)
            bassNotes++;
        
        // Check for pentatonic scale usage
        int pitchClass = note.pitch % 12;
        if (pitchClass == 0 || pitchClass == 3 || pitchClass == 5 || 
            pitchClass == 7 || pitchClass == 10) // C minor pentatonic
            pentatonicNotes++;
    }
    
    // Score based on hip hop characteristics
    score += (float)drumNotes / totalNotes * 0.4f;        // 40% weight for drum presence
    score += (float)bassNotes / totalNotes * 0.3f;        // 30% weight for bass presence  
    score += (float)pentatonicNotes / totalNotes * 0.3f;  // 30% weight for pentatonic usage
    
    return juce::jlimit(0.0f, 1.0f, score);
}

std::vector<std::vector<int>> HipHopSpecialist::getCommonProgressions() const
{
    return {
        {0, 10, 8, 10},    // i - VII - VI - VII (very common in hip hop)
        {0, 3, 5, 10},     // i - III - v - VII
        {0, 5, 10, 8},     // i - v - VII - VI
        {0, 8, 10, 5}      // i - VI - VII - v
    };
}

std::vector<MusicTheoryEngine::ScaleType> HipHopSpecialist::getCommonScales() const
{
    return {
        MusicTheoryEngine::ScaleType::Pentatonic,
        MusicTheoryEngine::ScaleType::NaturalMinor,
        MusicTheoryEngine::ScaleType::BluesScale,
        MusicTheoryEngine::ScaleType::Dorian
    };
}

void HipHopSpecialist::addBoomBapCharacteristics(MIDIPattern& pattern)
{
    // Add boom-bap specific elements (classic hip hop from 90s)
    // This would include specific drum patterns, sampling characteristics, etc.
    // Implementation would go here for boom-bap style patterns
}

//==============================================================================
// PopSpecialist Implementation
//==============================================================================

PopSpecialist::PopSpecialist() : GenreSpecialist(GenreType::Pop)
{
    // Initialize Pop characteristics - simplified for build
}

MIDIPattern PopSpecialist::generateRhythmPattern(const GenerationParameters& params)
{
    MIDIPattern pattern;
    pattern.lengthInBeats = params.patternLengthBeats;
    pattern.metadata = params;
    return pattern;
}

MIDIPattern PopSpecialist::generateMelodicPattern(const GenerationParameters& params)
{
    MIDIPattern pattern;
    pattern.lengthInBeats = params.patternLengthBeats;
    pattern.metadata = params;
    return pattern;
}

MIDIPattern PopSpecialist::generateBassPattern(const GenerationParameters& params)
{
    MIDIPattern pattern;
    pattern.lengthInBeats = params.patternLengthBeats;
    pattern.metadata = params;
    return pattern;
}

MIDIPattern PopSpecialist::generateChordPattern(const GenerationParameters& params)
{
    MIDIPattern pattern;
    pattern.lengthInBeats = params.patternLengthBeats;
    pattern.metadata = params;
    return pattern;
}

void PopSpecialist::applyGenreStyle(MIDIPattern& pattern, const GenerationParameters& params) {}
void PopSpecialist::addGenreSpecificOrnamentation(MIDIPattern& pattern) {}
float PopSpecialist::calculateGenreAuthenticity(const MIDIPattern& pattern) { return 0.5f; }

std::vector<std::vector<int>> PopSpecialist::getCommonProgressions() const
{
    return {{0, 7, 9, 5}};
}

std::vector<MusicTheoryEngine::ScaleType> PopSpecialist::getCommonScales() const
{
    return {MusicTheoryEngine::ScaleType::Major};
}

void PopSpecialist::addCatchyHook(MIDIPattern& pattern) {}
void PopSpecialist::addPopRhythmGuitar(MIDIPattern& pattern) {}
void PopSpecialist::addCommercialDrumPattern(MIDIPattern& pattern) {}

//==============================================================================
// EDMSpecialist Implementation
//==============================================================================

EDMSpecialist::EDMSpecialist() : GenreSpecialist(GenreType::EDM) {}

MIDIPattern EDMSpecialist::generateRhythmPattern(const GenerationParameters& params)
{
    MIDIPattern pattern;
    pattern.lengthInBeats = params.patternLengthBeats;
    pattern.metadata = params;
    return pattern;
}

MIDIPattern EDMSpecialist::generateMelodicPattern(const GenerationParameters& params)
{
    MIDIPattern pattern;
    pattern.lengthInBeats = params.patternLengthBeats;
    pattern.metadata = params;
    return pattern;
}

MIDIPattern EDMSpecialist::generateBassPattern(const GenerationParameters& params)
{
    MIDIPattern pattern;
    pattern.lengthInBeats = params.patternLengthBeats;
    pattern.metadata = params;
    return pattern;
}

MIDIPattern EDMSpecialist::generateChordPattern(const GenerationParameters& params)
{
    MIDIPattern pattern;
    pattern.lengthInBeats = params.patternLengthBeats;
    pattern.metadata = params;
    return pattern;
}

void EDMSpecialist::applyGenreStyle(MIDIPattern& pattern, const GenerationParameters& params) {}
void EDMSpecialist::addGenreSpecificOrnamentation(MIDIPattern& pattern) {}
float EDMSpecialist::calculateGenreAuthenticity(const MIDIPattern& pattern) { return 0.5f; }

std::vector<std::vector<int>> EDMSpecialist::getCommonProgressions() const
{
    return {{0, 5, 7, 5}};
}

std::vector<MusicTheoryEngine::ScaleType> EDMSpecialist::getCommonScales() const
{
    return {MusicTheoryEngine::ScaleType::Major};
}

void EDMSpecialist::addFourOnTheFloor(MIDIPattern& pattern) {}
void EDMSpecialist::addBuildUpElements(MIDIPattern& pattern) {}
void EDMSpecialist::addDropCharacteristics(MIDIPattern& pattern) {}
void EDMSpecialist::addArpeggiatedSynths(MIDIPattern& pattern) {}

//==============================================================================
// RnBSpecialist Implementation
//==============================================================================

RnBSpecialist::RnBSpecialist() : GenreSpecialist(GenreType::RnB) {}

MIDIPattern RnBSpecialist::generateRhythmPattern(const GenerationParameters& params)
{
    MIDIPattern pattern;
    pattern.lengthInBeats = params.patternLengthBeats;
    pattern.metadata = params;
    return pattern;
}

MIDIPattern RnBSpecialist::generateMelodicPattern(const GenerationParameters& params)
{
    MIDIPattern pattern;
    pattern.lengthInBeats = params.patternLengthBeats;
    pattern.metadata = params;
    return pattern;
}

MIDIPattern RnBSpecialist::generateBassPattern(const GenerationParameters& params)
{
    MIDIPattern pattern;
    pattern.lengthInBeats = params.patternLengthBeats;
    pattern.metadata = params;
    return pattern;
}

MIDIPattern RnBSpecialist::generateChordPattern(const GenerationParameters& params)
{
    MIDIPattern pattern;
    pattern.lengthInBeats = params.patternLengthBeats;
    pattern.metadata = params;
    return pattern;
}

void RnBSpecialist::applyGenreStyle(MIDIPattern& pattern, const GenerationParameters& params) {}
void RnBSpecialist::addGenreSpecificOrnamentation(MIDIPattern& pattern) {}
float RnBSpecialist::calculateGenreAuthenticity(const MIDIPattern& pattern) { return 0.5f; }

std::vector<std::vector<int>> RnBSpecialist::getCommonProgressions() const
{
    return {{0, 2, 5, 7}};
}

std::vector<MusicTheoryEngine::ScaleType> RnBSpecialist::getCommonScales() const
{
    return {MusicTheoryEngine::ScaleType::NaturalMinor};
}

void RnBSpecialist::addSoulfulVoicings(MIDIPattern& pattern) {}
void RnBSpecialist::addMelodicOrnamentation(MIDIPattern& pattern) {}
void RnBSpecialist::addGospelInfluences(MIDIPattern& pattern) {}
void RnBSpecialist::addContemporaryRnBGroove(MIDIPattern& pattern) {}

//==============================================================================
// GenreManager Implementation  
//==============================================================================

GenreManager::GenreManager()
{
    initializeGenreRelationships();
}

std::unique_ptr<GenreSpecialist> GenreManager::createSpecialist(GenreType genre)
{
    switch (genre)
    {
        case GenreType::HipHop:
            return std::make_unique<HipHopSpecialist>();
        case GenreType::Pop:
            return std::make_unique<PopSpecialist>();
        case GenreType::EDM:
        case GenreType::Dance:
            return std::make_unique<EDMSpecialist>();
        case GenreType::RnB:
            return std::make_unique<RnBSpecialist>();
        default:
            return std::make_unique<HipHopSpecialist>(); // Default to Hip Hop
    }
}

GenreType GenreManager::detectGenre(const MIDIPattern& pattern)
{
    return GenreType::Generic; // Simplified for build
}

GenreType GenreManager::detectGenre(const GenerationParameters& params)
{
    float tempo = params.tempo;
    
    // Simple tempo-based genre detection
    if (tempo >= 120.0f && tempo <= 180.0f)
        return GenreType::EDM;
    else if (tempo >= 80.0f && tempo <= 140.0f)
        return GenreType::Pop;
    else if (tempo >= 70.0f && tempo <= 160.0f)
        return GenreType::HipHop;
    else if (tempo >= 60.0f && tempo <= 120.0f)
        return GenreType::RnB;
    
    return GenreType::Generic;
}

std::vector<std::pair<GenreType, float>> GenreManager::analyzeGenreCompatibility(const MIDIPattern& pattern)
{
    return {}; // Simplified for build
}

float GenreManager::calculateGenreDistance(GenreType genre1, GenreType genre2)
{
    return 1.0f; // Simplified for build
}

std::vector<GenreType> GenreManager::getRecommendedGenres(const GenerationParameters& params)
{
    GenreType detected = detectGenre(params);
    return getCompatibleGenres(detected);
}

std::vector<GenreType> GenreManager::getCompatibleGenres(GenreType baseGenre)
{
    std::vector<GenreType> compatible;
    
    switch (baseGenre)
    {
        case GenreType::HipHop:
            compatible = {GenreType::RnB, GenreType::Pop, GenreType::Funk};
            break;
        case GenreType::Pop:
            compatible = {GenreType::Rock, GenreType::Dance, GenreType::RnB};
            break;
        case GenreType::EDM:
        case GenreType::Dance:
            compatible = {GenreType::Pop, GenreType::HipHop};
            break;
        case GenreType::RnB:
            compatible = {GenreType::HipHop, GenreType::Pop, GenreType::Jazz};
            break;
        default:
            compatible = {GenreType::Pop, GenreType::HipHop};
            break;
    }
    
    return compatible;
}

juce::String GenreManager::getGenreName(GenreType genre)
{
    switch (genre)
    {
        case GenreType::HipHop:     return "Hip Hop";
        case GenreType::Pop:        return "Pop";
        case GenreType::Dance:      return "Dance";
        case GenreType::EDM:        return "EDM";
        case GenreType::RnB:        return "R&B";
        case GenreType::Rock:       return "Rock";
        case GenreType::Jazz:       return "Jazz";
        case GenreType::Latin:      return "Latin";
        case GenreType::Funk:       return "Funk";
        default:                    return "Generic";
    }
}

juce::String GenreManager::getGenreDescription(GenreType genre)
{
    switch (genre)
    {
        case GenreType::HipHop:     
            return "Urban rhythms with trap hi-hats, 808 bass, and pentatonic melodies";
        case GenreType::Pop:        
            return "Commercial melodies with accessible harmonies and clear rhythms";
        case GenreType::Dance:      
            return "Four-on-the-floor beats designed for danceability";
        case GenreType::EDM:        
            return "Electronic dance music with synthetic textures and energetic rhythms";
        case GenreType::RnB:        
            return "Soulful harmonies with complex chord voicings and melodic ornamentation";
        default:                    
            return "General purpose musical patterns";
    }
}

std::vector<GenreType> GenreManager::getAllGenres()
{
    return {
        GenreType::HipHop,
        GenreType::Pop,
        GenreType::Dance,
        GenreType::EDM,
        GenreType::RnB,
        GenreType::Rock,
        GenreType::Jazz,
        GenreType::Latin,
        GenreType::Funk,
        GenreType::Generic
    };
}

void GenreManager::initializeGenreRelationships()
{
    // Initialize genre distance relationships (simplified for build)
}
