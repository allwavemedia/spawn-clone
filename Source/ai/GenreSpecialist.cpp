/*
  ==============================================================================

    GenreSpecialist.cpp
    Created: 29 Jul 2025
    Author: BMad Master
    
    Epic 4 Story 4.2: Genre-Specific Intelligence Systems
    
    Implementation of genre-specific pattern generation with authentic
    musical characteristics for Hip Hop, Pop, EDM, R&B.

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
            note.endTime += swingOffset;
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
        note.startTime = std::max(0.0f, note.startTime + timingOffset);
        note.endTime = std::max(note.startTime + 0.01f, note.endTime + timingOffset);
        
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
    pattern.length = params.patternLength;
    pattern.bpm = params.tempo;
    
    // Generate basic hip hop drum pattern
    float beatLength = 60.0f / params.tempo; // Length of one beat in seconds
    int steps = static_cast<int>(params.patternLength * 4 * 4); // 16th note steps
    
    for (int step = 0; step < steps; ++step)
    {
        float time = step * beatLength / 4.0f; // 16th note timing
        int patternIndex = step % 32;
        
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
        
        // Add snare
        if (rhythmTemplate.snarePattern[patternIndex] > 0.0f)
        {
            Note snare;
            snare.noteNumber = 38; // D2 - Snare
            snare.startTime = time;
            snare.endTime = time + 0.1f;
            snare.velocity = static_cast<int>(127 * rhythmTemplate.snarePattern[patternIndex]);
            pattern.notes.push_back(snare);
        }
        
        // Add hi-hat
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
    
    float beatLength = 60.0f / pattern.bpm;
    int steps = static_cast<int>(pattern.length * 4 * 8); // 32nd note steps
    
    for (int step = 0; step < steps; ++step)
    {
        // Add rapid hi-hat rolls (trap characteristic)
        if (step % 16 >= 12 && step % 16 <= 15) // Last quarter of each beat
        {
            if (gen() % 3 == 0) // 33% chance for rapid hits
            {
                Note hihat;
                hihat.noteNumber = 44; // F#2 - Open Hi-hat for rolls
                hihat.startTime = (step * beatLength / 8.0f) * timingDist(gen);
                hihat.endTime = hihat.startTime + 0.03f;
                hihat.velocity = static_cast<int>(127 * velocityDist(gen));
                pattern.notes.push_back(hihat);
            }
        }
    }
}

void HipHopSpecialist::add808Pattern(MIDIPattern& pattern)
{
    // Add 808-style bass drum with pitch modulation
    float beatLength = 60.0f / pattern.bpm;
    
    // Add 808 on beat 1 and sometimes on 3.5
    Note bass808;
    bass808.noteNumber = 24; // C1 - Sub bass
    bass808.startTime = 0.0f;
    bass808.endTime = beatLength * 0.8f; // Long decay
    bass808.velocity = 110;
    pattern.notes.push_back(bass808);
    
    // Optional 808 on beat 3.5 (trap style)
    if (pattern.length >= 1.0f)
    {
        Note bass808_2;
        bass808_2.noteNumber = 24;
        bass808_2.startTime = beatLength * 3.5f;
        bass808_2.endTime = bass808_2.startTime + beatLength * 0.5f;
        bass808_2.velocity = 95;
        pattern.notes.push_back(bass808_2);
    }
}

MIDIPattern HipHopSpecialist::generateMelodicPattern(const GenerationParameters& params)
{
    MIDIPattern pattern;
    pattern.length = params.patternLength;
    pattern.bpm = params.tempo;
    
    // Generate pentatonic-based melody
    auto scales = getCommonScales();
    auto scale = scales[0]; // Minor pentatonic
    auto scaleNotes = theoryEngine->getScaleNotes(60, scale); // C minor pentatonic
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> noteChoiceDist(0, static_cast<int>(scaleNotes.size() - 1));
    std::uniform_real_distribution<float> rhythmDist(0.25f, 1.0f);
    
    float currentTime = 0.0f;
    float beatLength = 60.0f / params.tempo;
    
    while (currentTime < pattern.length)
    {
        Note note;
        note.noteNumber = scaleNotes[noteChoiceDist(gen)] + 12; // Octave up for melody
        note.startTime = currentTime;
        
        float noteDuration = rhythmDist(gen) * beatLength;
        note.endTime = note.startTime + noteDuration;
        note.velocity = 80 + (gen() % 30); // Velocity 80-110
        
        pattern.notes.push_back(note);
        currentTime += noteDuration + (beatLength * 0.25f); // Add some space
    }
    
    return pattern;
}

MIDIPattern HipHopSpecialist::generateBassPattern(const GenerationParameters& params)
{
    MIDIPattern pattern;
    pattern.length = params.patternLength;
    pattern.bpm = params.tempo;
    
    // Generate bass line using theory engine
    auto progressions = getCommonProgressions();
    auto progression = progressions[0]; // i - VII - VI - VII
    
    float beatLength = 60.0f / params.tempo;
    int currentChord = 0;
    
    for (float time = 0.0f; time < pattern.length; time += beatLength)
    {
        int chordIndex = currentChord % progression.size();
        int bassNote = 48 + progression[chordIndex]; // C3 + chord root
        
        Note bass;
        bass.noteNumber = bassNote;
        bass.startTime = time;
        bass.endTime = time + beatLength * 0.8f;
        bass.velocity = 100;
        pattern.notes.push_back(bass);
        
        currentChord++;
    }
    
    return pattern;
}

MIDIPattern HipHopSpecialist::generateChordPattern(const GenerationParameters& params)
{
    MIDIPattern pattern;
    pattern.length = params.patternLength;
    pattern.bpm = params.tempo;
    
    // Hip hop often uses simple triads or power chords
    auto progressions = getCommonProgressions();
    auto progression = progressions[0];
    
    float beatLength = 60.0f / params.tempo;
    
    for (int i = 0; i < progression.size() && i * beatLength < pattern.length; ++i)
    {
        float time = i * beatLength;
        int root = 60 + progression[i]; // C4 + chord root
        
        // Simple triad
        std::vector<int> chord = {root, root + 3, root + 7}; // Minor triad
        
        for (int noteNum : chord)
        {
            Note note;
            note.noteNumber = noteNum;
            note.startTime = time;
            note.endTime = time + beatLength * 0.9f;
            note.velocity = 70;
            pattern.notes.push_back(note);
        }
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
            if (note.noteNumber % 12 == 7) // If it's a fifth
            {
                note.noteNumber -= 1; // Make it a tritone (blue note)
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
        if (note.noteNumber >= 35 && note.noteNumber <= 51)
            drumNotes++;
        
        // Count bass notes
        if (note.noteNumber >= 24 && note.noteNumber <= 48)
            bassNotes++;
        
        // Check for pentatonic scale usage
        int pitchClass = note.noteNumber % 12;
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
        MusicTheoryEngine::ScaleType::Minor,
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
    initializePopCharacteristics();
}

//==============================================================================
// PopSpecialist Implementation
//==============================================================================

PopSpecialist::PopSpecialist() : GenreSpecialist(GenreType::Pop)
{
    initializePopCharacteristics();
}

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
