/*
  ==============================================================================

    GenreSpecialist_Extended.cpp
    Created: 29 Jul 2025
    Author: BMad Master
    
    Epic 4 Story 4.2: Genre-Specific Intelligence Systems
    
    Extended implementations for Pop, EDM, R&B specialists and GenreManager.

  ==============================================================================
*/

#include "GenreSpecialist.h"
#include <juce_core/juce_core.h>
#include <random>
#include <algorithm>
#include <cmath>

//==============================================================================
// PopSpecialist Implementation
//==============================================================================

void PopSpecialist::initializePopCharacteristics()
{
    // Initialize rhythm template for Pop (16th note subdivisions)
    rhythmTemplate = RhythmPattern(16);
    
    // Typical Pop kick pattern (four-on-the-floor variation)
    rhythmTemplate.kickPattern[0] = 1.0f;    // Beat 1
    rhythmTemplate.kickPattern[8] = 0.7f;    // Beat 3
    
    // Snare on 2 and 4 (backbeat)
    rhythmTemplate.snarePattern[4] = 0.9f;   // Beat 2
    rhythmTemplate.snarePattern[12] = 0.9f;  // Beat 4
    
    // Hi-hat pattern with subtle variations
    for (int i = 0; i < 16; i += 2)
    {
        rhythmTemplate.hiHatPattern[i] = (i % 8 == 0) ? 0.6f : 0.4f;
    }
    
    // Add some 16th note hi-hats for movement
    rhythmTemplate.hiHatPattern[1] = 0.3f;
    rhythmTemplate.hiHatPattern[3] = 0.3f;
    rhythmTemplate.hiHatPattern[9] = 0.3f;
    rhythmTemplate.hiHatPattern[11] = 0.3f;
    
    // Melodic characteristics - accessible, catchy intervals
    melodicChars.preferredIntervals = {1, 2, 3, 4, 5}; // Major scale intervals
    melodicChars.avoidedIntervals = {11}; // Avoid major 7th for accessibility
    melodicChars.stepwiseMotion = 0.8f; // High preference for stepwise motion
    melodicChars.leapFrequency = 0.15f; // Lower leap frequency for catchiness
    melodicChars.maxLeapSize = 5; // Keep leaps small (perfect 4th max)
    melodicChars.ornamentation = 0.3f; // Moderate ornamentation
    melodicChars.preferPentatonic = false;
    melodicChars.bluenessIntensity = 0.1f; // Minimal blues influence
    
    // Harmonic voicing - clear, bright chords
    harmonicVoicing.preferredExtensions = {9}; // Add9 chords are popular
    harmonicVoicing.commonInversions = {0, 1}; // Root and first inversion
    harmonicVoicing.voiceSpacing = 0.8f; // Tighter spacing for clarity
    harmonicVoicing.dissonanceLevel = 0.2f; // Low dissonance for commercial appeal
    harmonicVoicing.preferOpenVoicings = false; // Closed voicings preferred
    harmonicVoicing.bassMovement = 0.4f; // Moderate bass movement
}

MIDIPattern PopSpecialist::generateRhythmPattern(const GenerationParameters& params)
{
    MIDIPattern pattern;
    pattern.length = params.patternLength;
    pattern.bpm = params.tempo;
    
    float beatLength = 60.0f / params.tempo;
    int steps = static_cast<int>(params.patternLength * 4 * 4); // 16th note steps
    
    for (int step = 0; step < steps; ++step)
    {
        float time = step * beatLength / 4.0f;
        int patternIndex = step % 16;
        
        // Add kick drum
        if (rhythmTemplate.kickPattern[patternIndex] > 0.0f)
        {
            Note kick;
            kick.noteNumber = 36; // C2 - Kick drum
            kick.startTime = time;
            kick.endTime = time + 0.1f;
            kick.velocity = static_cast<int>(127 * rhythmTemplate.kickPattern[patternIndex]);
            pattern.notes.push_back(kick);
        }
        
        // Add snare with slight variation
        if (rhythmTemplate.snarePattern[patternIndex] > 0.0f)
        {
            Note snare;
            snare.noteNumber = 38; // D2 - Snare
            snare.startTime = time;
            snare.endTime = time + 0.1f;
            snare.velocity = static_cast<int>(127 * rhythmTemplate.snarePattern[patternIndex]);
            pattern.notes.push_back(snare);
        }
        
        // Add hi-hat with commercial feel
        if (rhythmTemplate.hiHatPattern[patternIndex] > 0.0f)
        {
            Note hihat;
            hihat.noteNumber = 42; // F#2 - Closed Hi-hat
            hihat.startTime = time;
            hihat.endTime = time + 0.05f;
            hihat.velocity = static_cast<int>(127 * rhythmTemplate.hiHatPattern[patternIndex]);
            pattern.notes.push_back(hihat);
        }
    }
    
    // Add pop-specific elements
    addCommercialDrumPattern(pattern);
    
    return pattern;
}

MIDIPattern PopSpecialist::generateMelodicPattern(const GenerationParameters& params)
{
    MIDIPattern pattern;
    pattern.length = params.patternLength;
    pattern.bpm = params.tempo;
    
    // Generate major scale-based melody
    auto scales = getCommonScales();
    auto scale = scales[0]; // Major scale
    auto scaleNotes = theoryEngine->getScaleNotes(60, scale); // C major
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> noteChoiceDist(0, static_cast<int>(scaleNotes.size() - 1));
    
    float currentTime = 0.0f;
    float beatLength = 60.0f / params.tempo;
    
    // Create catchy, repetitive melody
    std::vector<int> melodicPhrase;
    for (int i = 0; i < 4; ++i) // 4-note phrase
    {
        melodicPhrase.push_back(scaleNotes[noteChoiceDist(gen)] + 12); // Octave up
    }
    
    int phraseIndex = 0;
    while (currentTime < pattern.length)
    {
        Note note;
        note.noteNumber = melodicPhrase[phraseIndex % melodicPhrase.size()];
        note.startTime = currentTime;
        note.endTime = currentTime + beatLength * 0.5f; // Eighth note duration
        note.velocity = 85;
        
        pattern.notes.push_back(note);
        currentTime += beatLength * 0.5f;
        phraseIndex++;
    }
    
    return pattern;
}

MIDIPattern PopSpecialist::generateBassPattern(const GenerationParameters& params)
{
    MIDIPattern pattern;
    pattern.length = params.patternLength;
    pattern.bpm = params.tempo;
    
    // Generate bass line using common pop progressions
    auto progressions = getCommonProgressions();
    auto progression = progressions[0]; // I - V - vi - IV
    
    float beatLength = 60.0f / params.tempo;
    
    for (int i = 0; i < progression.size() && i * beatLength < pattern.length; ++i)
    {
        float time = i * beatLength;
        int bassNote = 48 + progression[i]; // C3 + chord root
        
        Note bass;
        bass.noteNumber = bassNote;
        bass.startTime = time;
        bass.endTime = time + beatLength * 0.8f;
        bass.velocity = 85;
        pattern.notes.push_back(bass);
    }
    
    return pattern;
}

MIDIPattern PopSpecialist::generateChordPattern(const GenerationParameters& params)
{
    MIDIPattern pattern;
    pattern.length = params.patternLength;
    pattern.bpm = params.tempo;
    
    // Pop uses clear, bright triads and add9 chords
    auto progressions = getCommonProgressions();
    auto progression = progressions[0]; // I - V - vi - IV
    
    float beatLength = 60.0f / params.tempo;
    
    for (int i = 0; i < progression.size() && i * beatLength < pattern.length; ++i)
    {
        float time = i * beatLength;
        int root = 60 + progression[i]; // C4 + chord root
        
        // Create triads with optional add9
        std::vector<int> chord;
        if (i == 0 || i == 3) // Major chords (I and IV)
        {
            chord = {root, root + 4, root + 7, root + 14}; // Major add9
        }
        else if (i == 2) // Minor chord (vi)
        {
            chord = {root, root + 3, root + 7}; // Minor triad
        }
        else // Dominant chord (V)
        {
            chord = {root, root + 4, root + 7, root + 10}; // Dominant 7th
        }
        
        for (int noteNum : chord)
        {
            Note note;
            note.noteNumber = noteNum;
            note.startTime = time;
            note.endTime = time + beatLength * 0.9f;
            note.velocity = 65;
            pattern.notes.push_back(note);
        }
    }
    
    return pattern;
}

void PopSpecialist::applyGenreStyle(MIDIPattern& pattern, const GenerationParameters& params)
{
    // Pop music is usually straight, no swing
    // Add minimal humanization for polished sound
    humanizePattern(pattern, 0.1f);
    
    // Even velocity dynamics for commercial appeal
    std::vector<float> velocityCurve = {1.0f, 0.9f, 0.95f, 0.9f};
    applyVelocityDynamics(pattern, velocityCurve);
}

void PopSpecialist::addGenreSpecificOrnamentation(MIDIPattern& pattern)
{
    // Add subtle ornamentation typical of pop music
    std::random_device rd;
    std::mt19937 gen(rd());
    
    for (auto& note : pattern.notes)
    {
        // 15% chance to add passing tones
        if (gen() % 7 == 0)
        {
            // Add grace note before main note
            Note grace;
            grace.noteNumber = note.noteNumber - 1;
            grace.startTime = note.startTime - 0.05f;
            grace.endTime = note.startTime;
            grace.velocity = note.velocity * 0.7f;
            pattern.notes.push_back(grace);
        }
    }
}

float PopSpecialist::calculateGenreAuthenticity(const MIDIPattern& pattern)
{
    float score = 0.0f;
    int totalNotes = pattern.notes.size();
    
    if (totalNotes == 0) return 0.0f;
    
    int majorScaleNotes = 0;
    int clearRhythms = 0;
    
    for (const auto& note : pattern.notes)
    {
        // Check for major scale usage
        int pitchClass = note.noteNumber % 12;
        if (pitchClass == 0 || pitchClass == 2 || pitchClass == 4 || 
            pitchClass == 5 || pitchClass == 7 || pitchClass == 9 || pitchClass == 11)
            majorScaleNotes++;
        
        // Check for clear rhythmic placement (on beats or strong subdivisions)
        float beatTime = fmod(note.startTime, 60.0f / 120.0f); // Assume 120 BPM
        if (beatTime < 0.05f || fmod(beatTime, 0.25f) < 0.05f)
            clearRhythms++;
    }
    
    score += (float)majorScaleNotes / totalNotes * 0.5f;
    score += (float)clearRhythms / totalNotes * 0.5f;
    
    return juce::jlimit(0.0f, 1.0f, score);
}

std::vector<std::vector<int>> PopSpecialist::getCommonProgressions() const
{
    return {
        {0, 7, 9, 5},      // I - V - vi - IV (most common pop progression)
        {0, 5, 9, 7},      // I - IV - vi - V
        {9, 7, 0, 5},      // vi - V - I - IV
        {0, 9, 5, 7}       // I - vi - IV - V (50s progression)
    };
}

std::vector<MusicTheoryEngine::ScaleType> PopSpecialist::getCommonScales() const
{
    return {
        MusicTheoryEngine::ScaleType::Major,
        MusicTheoryEngine::ScaleType::Minor,
        MusicTheoryEngine::ScaleType::Mixolydian,
        MusicTheoryEngine::ScaleType::Dorian
    };
}

void PopSpecialist::addCommercialDrumPattern(MIDIPattern& pattern)
{
    // Add commercial pop drum fills and variations
    float beatLength = 60.0f / pattern.bpm;
    
    // Add crash on beat 1 of new sections
    Note crash;
    crash.noteNumber = 49; // C#3 - Crash cymbal
    crash.startTime = 0.0f;
    crash.endTime = beatLength * 2.0f; // Long decay
    crash.velocity = 90;
    pattern.notes.push_back(crash);
    
    // Add subtle percussion fills
    if (pattern.length >= 2.0f) // If pattern is 2 bars or longer
    {
        float fillTime = beatLength * 7.5f; // Beat 7.5 (end of second bar)
        
        // Tom fill
        Note tom1;
        tom1.noteNumber = 45; // A2 - Mid tom
        tom1.startTime = fillTime;
        tom1.endTime = fillTime + 0.1f;
        tom1.velocity = 80;
        pattern.notes.push_back(tom1);
        
        Note tom2;
        tom2.noteNumber = 43; // G2 - High tom
        tom2.startTime = fillTime + 0.125f;
        tom2.endTime = fillTime + 0.225f;
        tom2.velocity = 75;
        pattern.notes.push_back(tom2);
    }
}

void PopSpecialist::addCatchyHook(MIDIPattern& pattern)
{
    // Implementation for catchy melodic hooks
    // This would add memorable melodic phrases characteristic of pop music
}

void PopSpecialist::addPopRhythmGuitar(MIDIPattern& pattern)
{
    // Implementation for rhythm guitar patterns common in pop
    // This would add strumming patterns and chord voicings
}

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
    std::vector<std::pair<GenreType, float>> compatibility = analyzeGenreCompatibility(pattern);
    
    if (!compatibility.empty())
    {
        // Return the genre with highest compatibility score
        std::sort(compatibility.begin(), compatibility.end(), 
                  [](const auto& a, const auto& b) { return a.second > b.second; });
        return compatibility[0].first;
    }
    
    return GenreType::Generic;
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
    std::vector<std::pair<GenreType, float>> results;
    
    // Test each genre specialist
    for (auto genre : getAllGenres())
    {
        if (genre == GenreType::Generic) continue;
        
        auto specialist = createSpecialist(genre);
        float authenticity = specialist->calculateGenreAuthenticity(pattern);
        results.emplace_back(genre, authenticity);
    }
    
    return results;
}

float GenreManager::calculateGenreDistance(GenreType genre1, GenreType genre2)
{
    auto key = std::make_pair(genre1, genre2);
    auto it = genreDistances.find(key);
    if (it != genreDistances.end())
        return it->second;
    
    // Default distance if not found
    return 1.0f;
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
    // Initialize genre distance relationships (0.0 = identical, 1.0 = completely different)
    genreDistances[{GenreType::HipHop, GenreType::RnB}] = 0.3f;
    genreDistances[{GenreType::HipHop, GenreType::Pop}] = 0.4f;
    genreDistances[{GenreType::Pop, GenreType::Rock}] = 0.2f;
    genreDistances[{GenreType::Pop, GenreType::Dance}] = 0.3f;
    genreDistances[{GenreType::EDM, GenreType::Dance}] = 0.1f;
    genreDistances[{GenreType::RnB, GenreType::Jazz}] = 0.3f;
    // Add more relationships as needed
}
