/*
  ==============================================================================

    MusicTheoryEngine.cpp
    Created: 29 Jul 2025
    Author:  Epic 4 Story 4.1 Implementation

    Advanced music theory engine implementation with comprehensive harmonic
    analysis, scale theory, and intelligent pattern generation capabilities.

  ==============================================================================
*/

#include "MusicTheoryEngine.h"
#include <algorithm>
#include <cmath>
#include <random>

//==============================================================================
MusicTheoryEngine::MusicTheoryEngine()
{
    initializeScales();
    initializeChords();
    initializeProgressions();
    
    // Initialize circle of fifths
    circleOfFifths = {0, 7, 2, 9, 4, 11, 6, 1, 8, 3, 10, 5};
    
    // Initialize harmonic functions
    harmonicFunctions[0] = "I";    // Tonic
    harmonicFunctions[1] = "bII";  // Neapolitan
    harmonicFunctions[2] = "II";   // Supertonic
    harmonicFunctions[3] = "bIII"; // Mediant (minor)
    harmonicFunctions[4] = "III";  // Mediant
    harmonicFunctions[5] = "IV";   // Subdominant
    harmonicFunctions[6] = "bV";   // Tritone substitute
    harmonicFunctions[7] = "V";    // Dominant
    harmonicFunctions[8] = "bVI";  // Submediant (minor)
    harmonicFunctions[9] = "VI";   // Submediant
    harmonicFunctions[10] = "bVII"; // Subtonic
    harmonicFunctions[11] = "VII";  // Leading tone
}

MusicTheoryEngine::~MusicTheoryEngine() = default;

//==============================================================================
// Task 4.1.1: Scale and Mode Definitions

void MusicTheoryEngine::initializeScales()
{
    // Major scales and modes
    scaleIntervals[ScaleType::Major] = {0, 2, 4, 5, 7, 9, 11};
    scaleIntervals[ScaleType::NaturalMinor] = {0, 2, 3, 5, 7, 8, 10};
    scaleIntervals[ScaleType::HarmonicMinor] = {0, 2, 3, 5, 7, 8, 11};
    scaleIntervals[ScaleType::MelodicMinor] = {0, 2, 3, 5, 7, 9, 11};
    
    // Church modes
    scaleIntervals[ScaleType::Dorian] = {0, 2, 3, 5, 7, 9, 10};
    scaleIntervals[ScaleType::Phrygian] = {0, 1, 3, 5, 7, 8, 10};
    scaleIntervals[ScaleType::Lydian] = {0, 2, 4, 6, 7, 9, 11};
    scaleIntervals[ScaleType::Mixolydian] = {0, 2, 4, 5, 7, 9, 10};
    scaleIntervals[ScaleType::Aeolian] = {0, 2, 3, 5, 7, 8, 10}; // Same as natural minor
    scaleIntervals[ScaleType::Locrian] = {0, 1, 3, 5, 6, 8, 10};
    
    // Special scales
    scaleIntervals[ScaleType::Pentatonic] = {0, 2, 4, 7, 9};
    scaleIntervals[ScaleType::BluesScale] = {0, 3, 5, 6, 7, 10};
    scaleIntervals[ScaleType::WholeTone] = {0, 2, 4, 6, 8, 10};
    scaleIntervals[ScaleType::Chromatic] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
}

std::vector<int> MusicTheoryEngine::getScaleIntervals(ScaleType scaleType) const
{
    auto it = scaleIntervals.find(scaleType);
    return (it != scaleIntervals.end()) ? it->second : std::vector<int>{};
}

std::vector<int> MusicTheoryEngine::getScaleNotes(int rootNote, ScaleType scaleType) const
{
    auto intervals = getScaleIntervals(scaleType);
    std::vector<int> notes;
    
    for (int interval : intervals)
    {
        notes.push_back(normalizeNote(rootNote + interval));
    }
    
    return notes;
}

bool MusicTheoryEngine::isNoteInScale(int note, int rootNote, ScaleType scaleType) const
{
    auto scaleNotes = getScaleNotes(rootNote, scaleType);
    int normalizedNote = normalizeNote(note);
    
    return std::find(scaleNotes.begin(), scaleNotes.end(), normalizedNote) != scaleNotes.end();
}

int MusicTheoryEngine::getScaleDegree(int note, int rootNote, ScaleType scaleType) const
{
    auto scaleNotes = getScaleNotes(rootNote, scaleType);
    int normalizedNote = normalizeNote(note);
    
    for (size_t i = 0; i < scaleNotes.size(); ++i)
    {
        if (scaleNotes[i] == normalizedNote)
            return static_cast<int>(i + 1);
    }
    
    return -1; // Not in scale
}

//==============================================================================
// Task 4.1.2: Chord Progression Analysis and Generation

void MusicTheoryEngine::initializeChords()
{
    // Triads
    chordIntervals[ChordType::Major] = {0, 4, 7};
    chordIntervals[ChordType::Minor] = {0, 3, 7};
    chordIntervals[ChordType::Diminished] = {0, 3, 6};
    chordIntervals[ChordType::Augmented] = {0, 4, 8};
    
    // Seventh chords
    chordIntervals[ChordType::Major7] = {0, 4, 7, 11};
    chordIntervals[ChordType::Minor7] = {0, 3, 7, 10};
    chordIntervals[ChordType::Dominant7] = {0, 4, 7, 10};
    chordIntervals[ChordType::Diminished7] = {0, 3, 6, 9};
    chordIntervals[ChordType::HalfDiminished7] = {0, 3, 6, 10};
    
    // Extended chords
    chordIntervals[ChordType::Major9] = {0, 4, 7, 11, 14};
    chordIntervals[ChordType::Minor9] = {0, 3, 7, 10, 14};
    chordIntervals[ChordType::Dominant9] = {0, 4, 7, 10, 14};
    
    // Suspended chords
    chordIntervals[ChordType::Sus2] = {0, 2, 7};
    chordIntervals[ChordType::Sus4] = {0, 5, 7};
    chordIntervals[ChordType::Add9] = {0, 4, 7, 14};
}

MusicTheoryEngine::Chord MusicTheoryEngine::generateChord(int root, ChordType type, int inversion) const
{
    Chord chord;
    chord.root = normalizeNote(root);
    chord.type = type;
    chord.inversion = inversion;
    
    auto intervals = chordIntervals.at(type);
    for (int interval : intervals)
    {
        chord.notes.push_back(normalizeNote(root + interval));
    }
    
    // Apply inversion
    for (int i = 0; i < inversion && !chord.notes.empty(); ++i)
    {
        int lowestNote = chord.notes[0];
        chord.notes.erase(chord.notes.begin());
        chord.notes.push_back(lowestNote + 12); // Move to next octave
    }
    
    return chord;
}

juce::String MusicTheoryEngine::Chord::getName() const
{
    const char* noteNames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    juce::String name = noteNames[root];
    
    switch (type)
    {
        case ChordType::Major: break; // No suffix for major
        case ChordType::Minor: name += "m"; break;
        case ChordType::Diminished: name += "dim"; break;
        case ChordType::Augmented: name += "aug"; break;
        case ChordType::Major7: name += "maj7"; break;
        case ChordType::Minor7: name += "m7"; break;
        case ChordType::Dominant7: name += "7"; break;
        case ChordType::Diminished7: name += "dim7"; break;
        case ChordType::HalfDiminished7: name += "m7b5"; break;
        case ChordType::Major9: name += "maj9"; break;
        case ChordType::Minor9: name += "m9"; break;
        case ChordType::Dominant9: name += "9"; break;
        case ChordType::Sus2: name += "sus2"; break;
        case ChordType::Sus4: name += "sus4"; break;
        case ChordType::Add9: name += "add9"; break;
    }
    
    if (inversion > 0)
    {
        name += "/" + juce::String(noteNames[notes[0] % 12]);
    }
    
    return name;
}

std::vector<MusicTheoryEngine::Chord> MusicTheoryEngine::generateDiatonicProgression(
    int key, ScaleType scale, const std::vector<int>& romanNumerals) const
{
    auto scaleNotes = getScaleNotes(key, scale);
    std::vector<Chord> progression;
    
    for (int degree : romanNumerals)
    {
        if (degree >= 1 && degree <= static_cast<int>(scaleNotes.size()))
        {
            int chordRoot = scaleNotes[degree - 1];
            ChordType chordType = ChordType::Major; // Default
            
            // Determine chord quality based on scale degree and scale type
            if (scale == ScaleType::Major)
            {
                if (degree == 1 || degree == 4 || degree == 5)
                    chordType = ChordType::Major;
                else if (degree == 2 || degree == 3 || degree == 6)
                    chordType = ChordType::Minor;
                else if (degree == 7)
                    chordType = ChordType::Diminished;
            }
            else if (scale == ScaleType::NaturalMinor)
            {
                if (degree == 1 || degree == 4 || degree == 5)
                    chordType = ChordType::Minor;
                else if (degree == 3 || degree == 6 || degree == 7)
                    chordType = ChordType::Major;
                else if (degree == 2)
                    chordType = ChordType::Diminished;
            }
            
            progression.push_back(generateChord(chordRoot, chordType));
        }
    }
    
    return progression;
}

void MusicTheoryEngine::initializeProgressions()
{
    // Common progressions by genre
    genreProgressions["pop"] = {
        {1, 5, 6, 4},    // I-V-vi-IV (very common)
        {6, 4, 1, 5},    // vi-IV-I-V 
        {1, 6, 4, 5},    // I-vi-IV-V
        {4, 1, 5, 6}     // IV-I-V-vi
    };
    
    genreProgressions["rock"] = {
        {1, 7, 4, 1},    // I-bVII-IV-I
        {1, 4, 5, 4},    // I-IV-V-IV
        {6, 4, 1, 5},    // vi-IV-I-V
        {1, 6, 4, 5}     // I-vi-IV-V
    };
    
    genreProgressions["jazz"] = {
        {2, 5, 1},       // ii-V-I
        {6, 2, 5, 1},    // vi-ii-V-I
        {1, 6, 2, 5},    // I-vi-ii-V
        {3, 6, 2, 5}     // iii-vi-ii-V
    };
    
    genreProgressions["blues"] = {
        {1, 1, 1, 1},    // I-I-I-I
        {4, 4, 1, 1},    // IV-IV-I-I
        {5, 4, 1, 5}     // V-IV-I-V
    };
    
    genreProgressions["electronic"] = {
        {1, 3, 6, 4},    // I-iii-vi-IV
        {6, 1, 4, 5},    // vi-I-IV-V
        {1, 5, 6, 4},    // I-V-vi-IV
        {4, 5, 6, 1}     // IV-V-vi-I
    };
}

std::vector<std::vector<int>> MusicTheoryEngine::getCommonProgressions(const juce::String& genre) const
{
    auto it = genreProgressions.find(genre.toLowerCase().toStdString());
    return (it != genreProgressions.end()) ? it->second : genreProgressions.at("pop");
}

//==============================================================================
// Task 4.1.3: Voice Leading and Harmonic Movement

float MusicTheoryEngine::calculateVoiceLeadingQuality(const Chord& from, const Chord& to) const
{
    if (from.notes.empty() || to.notes.empty())
        return 0.0f;
    
    int totalDistance = getVoiceLeadingDistance(from, to);
    int maxNotes = std::max(from.notes.size(), to.notes.size());
    
    // Normalize: shorter distances = higher quality
    float normalizedDistance = 1.0f - (static_cast<float>(totalDistance) / (maxNotes * 12.0f));
    
    // Bonus for common tones
    int commonTones = 0;
    for (int note1 : from.notes)
    {
        for (int note2 : to.notes)
        {
            if (normalizeNote(note1) == normalizeNote(note2))
            {
                commonTones++;
                break;
            }
        }
    }
    
    float commonToneBonus = static_cast<float>(commonTones) / maxNotes * 0.3f;
    
    // Penalty for parallel fifths/octaves
    float parallelPenalty = hasParallelFifthsOctaves(from, to) ? -0.2f : 0.0f;
    
    return juce::jlimit(0.0f, 1.0f, normalizedDistance + commonToneBonus + parallelPenalty);
}

int MusicTheoryEngine::getVoiceLeadingDistance(const Chord& from, const Chord& to) const
{
    if (from.notes.empty() || to.notes.empty())
        return 0;
    
    // Calculate minimum total voice leading distance
    std::vector<int> fromNotes = from.notes;
    std::vector<int> toNotes = to.notes;
    
    // Ensure both vectors have the same size for comparison
    size_t maxSize = std::max(fromNotes.size(), toNotes.size());
    while (fromNotes.size() < maxSize) fromNotes.push_back(fromNotes.back());
    while (toNotes.size() < maxSize) toNotes.push_back(toNotes.back());
    
    int totalDistance = 0;
    for (size_t i = 0; i < maxSize; ++i)
    {
        int distance = std::abs(fromNotes[i] - toNotes[i]);
        // Use the shorter path (considering octave equivalence)
        distance = std::min(distance, 12 - distance);
        totalDistance += distance;
    }
    
    return totalDistance;
}

bool MusicTheoryEngine::hasParallelFifthsOctaves(const Chord& chord1, const Chord& chord2) const
{
    if (chord1.notes.size() < 2 || chord2.notes.size() < 2)
        return false;
    
    for (size_t i = 0; i < chord1.notes.size() - 1; ++i)
    {
        for (size_t j = i + 1; j < chord1.notes.size(); ++j)
        {
            int interval1 = std::abs(chord1.notes[j] - chord1.notes[i]) % 12;
            
            if (i < chord2.notes.size() && j < chord2.notes.size())
            {
                int interval2 = std::abs(chord2.notes[j] - chord2.notes[i]) % 12;
                
                // Check for parallel fifths (7 semitones) or octaves (0 semitones)
                if ((interval1 == 7 && interval2 == 7) || (interval1 == 0 && interval2 == 0))
                {
                    return true;
                }
            }
        }
    }
    
    return false;
}

MusicTheoryEngine::Chord MusicTheoryEngine::optimizeVoiceLeading(
    const Chord& previousChord, const Chord& targetChord) const
{
    Chord optimized = targetChord;
    
    // Try different inversions to find the best voice leading
    float bestQuality = calculateVoiceLeadingQuality(previousChord, optimized);
    int bestInversion = optimized.inversion;
    
    for (int inversion = 0; inversion < static_cast<int>(targetChord.notes.size()); ++inversion)
    {
        Chord testChord = generateChord(targetChord.root, targetChord.type, inversion);
        float quality = calculateVoiceLeadingQuality(previousChord, testChord);
        
        if (quality > bestQuality)
        {
            bestQuality = quality;
            bestInversion = inversion;
            optimized = testChord;
        }
    }
    
    return optimized;
}

//==============================================================================
// Task 4.1.4: Interval and Tension Resolution

MusicTheoryEngine::IntervalType MusicTheoryEngine::getInterval(int note1, int note2) const
{
    int semitones = std::abs(note2 - note1) % 12;
    
    static const IntervalType intervals[] = {
        IntervalType::Unison, IntervalType::MinorSecond, IntervalType::MajorSecond,
        IntervalType::MinorThird, IntervalType::MajorThird, IntervalType::PerfectFourth,
        IntervalType::Tritone, IntervalType::PerfectFifth, IntervalType::MinorSixth,
        IntervalType::MajorSixth, IntervalType::MinorSeventh, IntervalType::MajorSeventh
    };
    
    return intervals[semitones];
}

float MusicTheoryEngine::getIntervalTension(IntervalType interval) const
{
    // Tension values from 0.0 (consonant) to 1.0 (highly dissonant)
    switch (interval)
    {
        case IntervalType::Unison:        return 0.0f;
        case IntervalType::Octave:        return 0.0f;
        case IntervalType::PerfectFifth:  return 0.1f;
        case IntervalType::PerfectFourth: return 0.2f;
        case IntervalType::MajorThird:    return 0.3f;
        case IntervalType::MinorThird:    return 0.3f;
        case IntervalType::MajorSixth:    return 0.4f;
        case IntervalType::MinorSixth:    return 0.4f;
        case IntervalType::MajorSecond:   return 0.6f;
        case IntervalType::MinorSeventh:  return 0.7f;
        case IntervalType::MajorSeventh:  return 0.8f;
        case IntervalType::MinorSecond:   return 0.9f;
        case IntervalType::Tritone:       return 1.0f;
        default:                          return 0.5f;
    }
}

float MusicTheoryEngine::calculateHarmonicTension(const Chord& chord) const
{
    if (chord.notes.size() < 2)
        return 0.0f;
    
    float totalTension = 0.0f;
    int intervalCount = 0;
    
    // Calculate tension for all intervals in the chord
    for (size_t i = 0; i < chord.notes.size(); ++i)
    {
        for (size_t j = i + 1; j < chord.notes.size(); ++j)
        {
            IntervalType interval = getInterval(chord.notes[i], chord.notes[j]);
            totalTension += getIntervalTension(interval);
            intervalCount++;
        }
    }
    
    return intervalCount > 0 ? totalTension / intervalCount : 0.0f;
}

std::vector<MusicTheoryEngine::Chord> MusicTheoryEngine::getTensionResolutions(
    const Chord& tensionChord, int key, ScaleType scale) const
{
    std::vector<Chord> resolutions;
    auto scaleNotes = getScaleNotes(key, scale);
    
    // Generate potential resolution chords
    for (int note : scaleNotes)
    {
        // Try different chord types for resolution
        std::vector<ChordType> resolutionTypes = {
            ChordType::Major, ChordType::Minor, ChordType::Dominant7
        };
        
        for (ChordType type : resolutionTypes)
        {
            Chord resolution = generateChord(note, type);
            
            // Check if this provides good tension resolution
            float tensionBefore = calculateHarmonicTension(tensionChord);
            float tensionAfter = calculateHarmonicTension(resolution);
            float voiceLeadingQuality = calculateVoiceLeadingQuality(tensionChord, resolution);
            
            // Good resolution: lower tension and smooth voice leading
            if (tensionAfter < tensionBefore && voiceLeadingQuality > 0.6f)
            {
                resolutions.push_back(resolution);
            }
        }
    }
    
    // Sort by quality (voice leading + tension reduction)
    std::sort(resolutions.begin(), resolutions.end(), 
        [this, &tensionChord](const Chord& a, const Chord& b) {
            float qualityA = calculateVoiceLeadingQuality(tensionChord, a);
            float qualityB = calculateVoiceLeadingQuality(tensionChord, b);
            return qualityA > qualityB;
        });
    
    // Return top 3 resolutions
    if (resolutions.size() > 3)
        resolutions.resize(3);
    
    return resolutions;
}

//==============================================================================
// Task 4.1.5: AI Generation Pipeline Integration

void MusicTheoryEngine::applyTheoryConstraints(MIDIPattern& pattern, const GenerationParameters& params) const
{
    // Extract key and scale from parameters
    int key = static_cast<int>(params.key);
    ScaleType scale = ScaleType::Major; // Default, could be parameter-driven
    
    // Map GenerationParameters::ScaleType to MusicTheoryEngine::ScaleType
    switch (params.scale)
    {
        case GenerationParameters::ScaleType::Major:
            scale = ScaleType::Major;
            break;
        case GenerationParameters::ScaleType::Minor:
            scale = ScaleType::NaturalMinor;
            break;
        case GenerationParameters::ScaleType::Pentatonic:
            scale = ScaleType::Pentatonic;
            break;
        case GenerationParameters::ScaleType::Blues:
            scale = ScaleType::BluesScale;
            break;
        case GenerationParameters::ScaleType::Dorian:
            scale = ScaleType::Dorian;
            break;
        case GenerationParameters::ScaleType::Mixolydian:
            scale = ScaleType::Mixolydian;
            break;
        default:
            scale = ScaleType::Major;
            break;
    }
    
    auto scaleNotes = getScaleNotes(key, scale);
    
    // Apply scale constraints to all notes
    for (auto& note : pattern.notes)
    {
        if (!isNoteInScale(note.pitch, key, scale))
        {
            // Find nearest scale note
            int bestNote = scaleNotes[0];
            int minDistance = 12;
            
            for (int scaleNote : scaleNotes)
            {
                int distance = std::abs(note.pitch - scaleNote);
                if (distance < minDistance)
                {
                    minDistance = distance;
                    bestNote = scaleNote;
                }
            }
            
            note.pitch = bestNote;
        }
    }
    
    // Ensure harmonic coherence if pattern has chords
    // This would be expanded based on chord detection in the pattern
}

std::vector<Note> MusicTheoryEngine::generateBassLine(
    const std::vector<Chord>& progression, double patternLength, double tempo) const
{
    std::vector<Note> bassLine;
    
    if (progression.empty())
        return bassLine;
    
    double chordDuration = patternLength / progression.size();
    double currentTime = 0.0;
    
    for (const auto& chord : progression)
    {
        // Use root note of chord for bass (could be enhanced with walking bass)
        Note bassNote;
        bassNote.pitch = chord.root + 24; // Place in bass register (2 octaves down)
        bassNote.velocity = 80; // Strong bass note
        bassNote.startTime = currentTime;
        bassNote.duration = chordDuration * 0.8; // Slightly shorter for articulation
        
        bassLine.push_back(bassNote);
        
        // Add optional passing tones for more sophisticated bass lines
        if (chordDuration > 0.5) // Only if chord is long enough
        {
            Note passingNote;
            passingNote.pitch = chord.notes[1] + 24; // Third of chord
            passingNote.velocity = 60; // Softer passing tone
            passingNote.startTime = currentTime + chordDuration * 0.6;
            passingNote.duration = chordDuration * 0.3;
            
            bassLine.push_back(passingNote);
        }
        
        currentTime += chordDuration;
    }
    
    return bassLine;
}

std::vector<Note> MusicTheoryEngine::generateMelody(
    const std::vector<Chord>& progression, int key, ScaleType scale,
    double patternLength, double tempo) const
{
    std::vector<Note> melody;
    auto scaleNotes = getScaleNotes(key, scale);
    
    if (progression.empty() || scaleNotes.empty())
        return melody;
    
    double chordDuration = patternLength / progression.size();
    double noteLength = chordDuration / 4.0; // Subdivide each chord
    double currentTime = 0.0;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> noteChoice(0, static_cast<int>(scaleNotes.size()) - 1);
    std::uniform_real_distribution<> velocityRange(0.6, 0.9);
    
    int lastPitch = scaleNotes[0] + 48; // Start in mid register
    
    for (const auto& chord : progression)
    {
        // Generate 4 notes per chord
        for (int i = 0; i < 4; ++i)
        {
            Note note;
            
            // Prefer chord tones (60% chance), scale notes otherwise
            std::uniform_real_distribution<> chordToneChance(0.0, 1.0);
            
            if (chordToneChance(gen) < 0.6 && !chord.notes.empty())
            {
                // Use chord tone
                std::uniform_int_distribution<> chordChoice(0, static_cast<int>(chord.notes.size()) - 1);
                note.pitch = chord.notes[chordChoice(gen)] + 48; // Place in melody register
            }
            else
            {
                // Use scale note
                note.pitch = scaleNotes[noteChoice(gen)] + 48;
            }
            
            // Smooth melodic motion (prefer steps over leaps)
            int interval = std::abs(note.pitch - lastPitch);
            if (interval > 7) // Large leap
            {
                // Try to find a closer note
                for (int scaleNote : scaleNotes)
                {
                    int testPitch = scaleNote + 48;
                    if (std::abs(testPitch - lastPitch) < interval)
                    {
                        note.pitch = testPitch;
                        break;
                    }
                }
            }
            
            note.velocity = static_cast<int>(velocityRange(gen) * 127); // Convert to MIDI range
            note.startTime = currentTime + (i * noteLength);
            note.duration = noteLength * 0.8; // Slight articulation
            
            melody.push_back(note);
            lastPitch = note.pitch;
        }
        
        currentTime += chordDuration;
    }
    
    return melody;
}

std::vector<MusicTheoryEngine::Chord> MusicTheoryEngine::harmonizeMelody(
    const std::vector<Note>& melody, int key, ScaleType scale) const
{
    std::vector<Chord> harmonization;
    auto scaleNotes = getScaleNotes(key, scale);
    
    if (melody.empty())
        return harmonization;
    
    // Group melody notes by time segments for chord analysis
    std::map<double, std::vector<int>> timeSegments;
    double segmentLength = 1.0; // 1 beat per chord
    
    for (const auto& note : melody)
    {
        int segment = static_cast<int>(note.startTime / segmentLength);
        timeSegments[segment].push_back(note.pitch);
    }
    
    // Generate chords for each segment
    for (const auto& segment : timeSegments)
    {
        if (segment.second.empty())
            continue;
        
        // Find the most prominent note in this segment
        std::map<int, int> noteFrequency;
        for (int pitch : segment.second)
        {
            int normalizedPitch = normalizeNote(pitch);
            noteFrequency[normalizedPitch]++;
        }
        
        int prominentNote = 0;
        int maxCount = 0;
        for (const auto& freq : noteFrequency)
        {
            if (freq.second > maxCount)
            {
                maxCount = freq.second;
                prominentNote = freq.first;
            }
        }
        
        // Find suitable chord containing this note
        for (int scaleNote : scaleNotes)
        {
            // Try different chord types
            std::vector<ChordType> chordTypes = {
                ChordType::Major, ChordType::Minor, ChordType::Dominant7
            };
            
            for (ChordType type : chordTypes)
            {
                Chord testChord = generateChord(scaleNote, type);
                
                // Check if chord contains the prominent melody note
                bool containsNote = false;
                for (int chordNote : testChord.notes)
                {
                    if (normalizeNote(chordNote) == prominentNote)
                    {
                        containsNote = true;
                        break;
                    }
                }
                
                if (containsNote)
                {
                    harmonization.push_back(testChord);
                    goto nextSegment; // Found suitable chord, move to next segment
                }
            }
        }
        
        // Fallback: use tonic chord
        harmonization.push_back(generateChord(key, ChordType::Major));
        
        nextSegment:;
    }
    
    return harmonization;
}

MusicTheoryEngine::TheoryAnalysis MusicTheoryEngine::analyzePattern(
    const MIDIPattern& pattern, const GenerationParameters& params) const
{
    TheoryAnalysis analysis;
    
    if (pattern.notes.empty())
    {
        analysis.suggestions.push_back("Pattern is empty");
        return analysis;
    }
    
    int key = static_cast<int>(params.key);
    ScaleType scale = ScaleType::Major; // Default
    
    // 1. Scale Compliance Analysis
    auto scaleNotes = getScaleNotes(key, scale);
    int scaleCompliantNotes = 0;
    
    for (const auto& note : pattern.notes)
    {
        if (isNoteInScale(note.pitch, key, scale))
            scaleCompliantNotes++;
    }
    
    analysis.scaleCompliance = static_cast<float>(scaleCompliantNotes) / pattern.notes.size();
    
    // 2. Harmonic Coherence Analysis
    // This would analyze chord progressions if present
    analysis.harmonicCoherence = 0.8f; // Placeholder - would need chord detection
    
    // 3. Voice Leading Quality
    // This would analyze voice leading between simultaneous notes
    analysis.voiceLeadingQuality = 0.7f; // Placeholder - needs chord progression analysis
    
    // 4. Generate Suggestions
    if (analysis.scaleCompliance < 0.8f)
    {
        analysis.suggestions.push_back("Consider using more notes from the " + 
                                     juce::String(key) + " scale");
    }
    
    if (analysis.harmonicCoherence < 0.6f)
    {
        analysis.suggestions.push_back("Improve harmonic progression coherence");
    }
    
    if (analysis.voiceLeadingQuality < 0.6f)
    {
        analysis.suggestions.push_back("Smoother voice leading would improve flow");
    }
    
    // 5. Overall Assessment
    float averageScore = (analysis.scaleCompliance + analysis.harmonicCoherence + 
                         analysis.voiceLeadingQuality) / 3.0f;
    analysis.isTheoricallySound = averageScore > 0.7f;
    
    if (analysis.suggestions.empty())
    {
        analysis.suggestions.push_back("Pattern shows good music theory compliance!");
    }
    
    return analysis;
}

//==============================================================================
// Helper Methods

int MusicTheoryEngine::calculateInterval(int note1, int note2) const
{
    return std::abs(note2 - note1);
}

std::vector<int> MusicTheoryEngine::getChordVoicing(const Chord& chord, int octave) const
{
    std::vector<int> voicing;
    int baseNote = octave * 12;
    
    for (int note : chord.notes)
    {
        voicing.push_back(baseNote + normalizeNote(note));
    }
    
    return voicing;
}

int MusicTheoryEngine::findBestInversion(const Chord& previousChord, const Chord& targetChord) const
{
    int bestInversion = 0;
    float bestQuality = 0.0f;
    
    for (int inversion = 0; inversion < static_cast<int>(targetChord.notes.size()); ++inversion)
    {
        Chord testChord = generateChord(targetChord.root, targetChord.type, inversion);
        float quality = calculateVoiceLeadingQuality(previousChord, testChord);
        
        if (quality > bestQuality)
        {
            bestQuality = quality;
            bestInversion = inversion;
        }
    }
    
    return bestInversion;
}

std::vector<int> MusicTheoryEngine::generateMelodyContour(int length, const std::vector<int>& scaleNotes) const
{
    std::vector<int> contour;
    
    if (scaleNotes.empty() || length <= 0)
        return contour;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> noteChoice(0, static_cast<int>(scaleNotes.size()) - 1);
    std::uniform_int_distribution<> direction(-1, 1); // -1=down, 0=same, 1=up
    
    int currentIndex = noteChoice(gen);
    contour.push_back(scaleNotes[currentIndex]);
    
    for (int i = 1; i < length; ++i)
    {
        int dir = direction(gen);
        currentIndex = juce::jlimit(0, static_cast<int>(scaleNotes.size()) - 1, 
                                   currentIndex + dir);
        contour.push_back(scaleNotes[currentIndex]);
    }
    
    return contour;
}

MusicTheoryEngine::Chord MusicTheoryEngine::analyzeChord(const std::vector<int>& notes) const
{
    Chord chord;
    
    if (notes.empty())
        return chord;
    
    // Normalize all notes to single octave
    std::vector<int> normalizedNotes;
    for (int note : notes)
    {
        normalizedNotes.push_back(normalizeNote(note));
    }
    
    // Remove duplicates and sort
    std::sort(normalizedNotes.begin(), normalizedNotes.end());
    normalizedNotes.erase(std::unique(normalizedNotes.begin(), normalizedNotes.end()), 
                         normalizedNotes.end());
    
    if (normalizedNotes.size() < 2)
        return chord; // Need at least 2 notes for a chord
    
    // Try to identify the chord by testing different roots
    for (int root : normalizedNotes)
    {
        // Calculate intervals from this potential root
        std::vector<int> intervals;
        for (int note : normalizedNotes)
        {
            intervals.push_back(normalizeNote(note - root));
        }
        std::sort(intervals.begin(), intervals.end());
        
        // Check against known chord patterns
        for (const auto& chordPattern : chordIntervals)
        {
            if (intervals == chordPattern.second)
            {
                chord.root = root;
                chord.type = chordPattern.first;
                chord.notes = normalizedNotes;
                return chord;
            }
        }
    }
    
    // If no exact match found, default to the lowest note as root
    chord.root = normalizedNotes[0];
    chord.type = ChordType::Major; // Default assumption
    chord.notes = normalizedNotes;
    
    return chord;
}
