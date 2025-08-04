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

//==============================================================================
// Epic 6 Story 6.1: Advanced Generation Functions

std::vector<int> MusicTheoryEngine::generateMelodicContour(const std::vector<int>& scale,
                                                           int numNotes,
                                                           GenerationParameters::MelodicContour contourShape,
                                                           float intervalVariety)
{
    std::vector<int> melody;
    if (scale.empty() || numNotes <= 0) return melody;

    std::random_device rd;
    std::mt19937 gen(rd());
    
    int currentNoteIndex = scale.size() / 2; // Start in middle of scale
    melody.push_back(scale[currentNoteIndex]);

    for (int i = 1; i < numNotes; ++i)
    {
        int step = calculateNextStep(i, numNotes, contourShape, intervalVariety, gen);
        currentNoteIndex = std::clamp(currentNoteIndex + step, 0, (int)scale.size() - 1);
        melody.push_back(scale[currentNoteIndex]);
    }

    return melody;
}

std::vector<double> MusicTheoryEngine::generateRhythmicPattern(int numNotes,
                                                               double totalBeats,
                                                               float complexity,
                                                               float density,
                                                               float swing)
{
    std::vector<double> rhythm;
    if (numNotes <= 0) return rhythm;

    // Calculate the actual time span based on density
    double actualTimeSpan = totalBeats / std::max(0.1f, density);
    double beatIncrement = actualTimeSpan / numNotes;
    
    for (int i = 0; i < numNotes; ++i)
    {
        double position = i * beatIncrement;
        
        // Apply swing to off-beats
        if (swing > 0.0 && (i % 2 != 0))
        {
            position += beatIncrement * swing * 0.5;
        }
        
        rhythm.push_back(position);
    }
    
    return rhythm;
}

int MusicTheoryEngine::calculateNextStep(int noteIndex, int totalNotes, 
                                         GenerationParameters::MelodicContour contourShape, 
                                         float intervalVariety, std::mt19937& gen)
{
    std::uniform_int_distribution<> intervalDist(1, 1 + (int)(intervalVariety * 3));
    int step = intervalDist(gen);

    switch (contourShape)
    {
        case GenerationParameters::MelodicContour::Rising:
            return step;
        case GenerationParameters::MelodicContour::Falling:
            return -step;
        case GenerationParameters::MelodicContour::Wave:
        {
            // Create a wave pattern that goes up for the first quarter, down for the second quarter, etc.
            int quarterLength = std::max(1, totalNotes / 4);
            int currentQuarter = noteIndex / quarterLength;
            return (currentQuarter % 2 == 0) ? step : -step;
        }
        case GenerationParameters::MelodicContour::Arch:
            return (noteIndex < totalNotes / 2) ? step : -step;
        case GenerationParameters::MelodicContour::Random:
        default:
            return (gen() % 2 == 0) ? step : -step;
    }
}

//==============================================================================
// Epic 6 Story 6.2: Dynamic Pattern Variation Implementation

MusicTheoryEngine::PatternGenome MusicTheoryEngine::evolvePattern(const PatternGenome& parent1, 
                                                                  const PatternGenome& parent2,
                                                                  const GenerationParameters& params)
{
    PatternGenome offspring;
    
    // Apply crossover if rate allows
    std::mt19937 gen{std::random_device{}()};
    if (std::uniform_real_distribution<float>(0.0f, 1.0f)(gen) < params.variation.crossoverRate)
    {
        offspring = crossoverPatterns(parent1, parent2, params);
    }
    else
    {
        // Select fitter parent
        offspring = (parent1.fitness > parent2.fitness) ? parent1 : parent2;
    }
    
    // Apply mutation
    offspring = mutatePattern(offspring, params);
    
    // Update generation info
    offspring.generation = std::max(parent1.generation, parent2.generation) + 1;
    offspring.id = juce::Uuid().toString();
    offspring.timestamp = juce::Time::getCurrentTime().toMilliseconds() / 1000.0;
    
    // Calculate new fitness
    offspring.fitness = calculatePatternFitness(offspring, params);
    
    return offspring;
}

MusicTheoryEngine::PatternGenome MusicTheoryEngine::mutatePattern(const PatternGenome& pattern,
                                                                  const GenerationParameters& params)
{
    PatternGenome mutated = pattern;
    std::mt19937 gen{std::random_device{}()};
    std::uniform_real_distribution<float> mutationChance(0.0f, 1.0f);
    
    for (auto& note : mutated.notes)
    {
        if (mutationChance(gen) < params.variation.mutationRate)
        {
            switch (params.variation.scope)
            {
                case GenerationParameters::VariationSettings::VariationScope::Rhythm:
                    // Mutate timing
                    note.startTime += std::normal_distribution<double>(0.0, 0.1)(gen);
                    note.duration *= std::uniform_real_distribution<double>(0.8, 1.2)(gen);
                    break;
                    
                case GenerationParameters::VariationSettings::VariationScope::Melody:
                    // Mutate pitch
                    note.pitch += std::uniform_int_distribution<int>(-2, 2)(gen);
                    note.pitch = std::clamp(note.pitch, 0, 127);
                    break;
                    
                case GenerationParameters::VariationSettings::VariationScope::Harmony:
                    // Mutate velocity (affects harmonic perception)
                    note.velocity = std::clamp(note.velocity + std::uniform_int_distribution<int>(-20, 20)(gen), 1, 127);
                    break;
                    
                case GenerationParameters::VariationSettings::VariationScope::All:
                default:
                    // Mutate any aspect
                    if (mutationChance(gen) < 0.33f)
                        note.startTime += std::normal_distribution<double>(0.0, 0.1)(gen);
                    if (mutationChance(gen) < 0.33f)
                        note.pitch += std::uniform_int_distribution<int>(-2, 2)(gen);
                    if (mutationChance(gen) < 0.33f)
                        note.velocity = std::clamp(note.velocity + std::uniform_int_distribution<int>(-10, 10)(gen), 1, 127);
                    break;
            }
        }
    }
    
    return mutated;
}

MusicTheoryEngine::PatternGenome MusicTheoryEngine::crossoverPatterns(const PatternGenome& parent1,
                                                                      const PatternGenome& parent2,
                                                                      const GenerationParameters& params)
{
    PatternGenome offspring;
    std::mt19937 gen{std::random_device{}()};
    
    // Determine crossover point
    size_t minSize = std::min(parent1.notes.size(), parent2.notes.size());
    size_t maxSize = std::max(parent1.notes.size(), parent2.notes.size());
    
    if (minSize == 0)
    {
        return parent1.notes.empty() ? parent2 : parent1;
    }
    
    size_t crossoverPoint = std::uniform_int_distribution<size_t>(1, minSize - 1)(gen);
    
    // Combine genetic material
    offspring.notes.reserve(maxSize);
    
    // First part from parent1
    for (size_t i = 0; i < crossoverPoint && i < parent1.notes.size(); ++i)
    {
        offspring.notes.push_back(parent1.notes[i]);
    }
    
    // Second part from parent2
    for (size_t i = crossoverPoint; i < parent2.notes.size(); ++i)
    {
        offspring.notes.push_back(parent2.notes[i]);
    }
    
    return offspring;
}

float MusicTheoryEngine::calculatePatternFitness(const PatternGenome& pattern,
                                                 const GenerationParameters& params)
{
    if (pattern.notes.empty())
        return 0.0f;
        
    float fitness = 0.0f;
    
    // Musical coherence factors
    float rhythmicConsistency = 0.0f;
    float melodicFlow = 0.0f;
    float harmoniousness = 0.0f;
    
    // Calculate rhythmic consistency
    std::vector<double> intervals;
    for (size_t i = 1; i < pattern.notes.size(); ++i)
    {
        intervals.push_back(pattern.notes[i].startTime - pattern.notes[i-1].startTime);
    }
    
    if (!intervals.empty())
    {
        double meanInterval = std::accumulate(intervals.begin(), intervals.end(), 0.0) / intervals.size();
        double variance = 0.0;
        for (double interval : intervals)
        {
            variance += (interval - meanInterval) * (interval - meanInterval);
        }
        variance /= intervals.size();
        rhythmicConsistency = 1.0f / (1.0f + static_cast<float>(variance));
    }
    
    // Calculate melodic flow (prefer smaller pitch intervals)
    for (size_t i = 1; i < pattern.notes.size(); ++i)
    {
        int interval = std::abs(pattern.notes[i].pitch - pattern.notes[i-1].pitch);
        melodicFlow += 1.0f / (1.0f + interval * 0.1f);  // Penalize large jumps
    }
    melodicFlow /= std::max(1.0f, static_cast<float>(pattern.notes.size() - 1));
    
    // Calculate basic harmoniousness (prefer notes in key)
    // This is a simplified version - could be enhanced with scale awareness
    for (const auto& note : pattern.notes)
    {
        int pitchClass = note.pitch % 12;
        // Basic major scale preference (C major for simplicity)
        std::vector<int> majorScale = {0, 2, 4, 5, 7, 9, 11};
        bool inScale = std::find(majorScale.begin(), majorScale.end(), pitchClass) != majorScale.end();
        harmoniousness += inScale ? 1.0f : 0.3f;
    }
    harmoniousness /= std::max(1.0f, static_cast<float>(pattern.notes.size()));
    
    // Weight and combine factors
    fitness = (rhythmicConsistency * 0.3f) + (melodicFlow * 0.4f) + (harmoniousness * 0.3f);
    
    return std::clamp(fitness, 0.0f, 1.0f);
}

MusicTheoryEngine::PatternGenome MusicTheoryEngine::morphPatterns(const PatternGenome& source,
                                                                  const PatternGenome& target,
                                                                  float morphAmount)
{
    PatternGenome morphed;
    morphAmount = std::clamp(morphAmount, 0.0f, 1.0f);
    
    size_t minSize = std::min(source.notes.size(), target.notes.size());
    size_t maxSize = std::max(source.notes.size(), target.notes.size());
    
    morphed.notes.reserve(maxSize);
    
    // Morph existing notes
    for (size_t i = 0; i < minSize; ++i)
    {
        Note morphedNote;
        morphedNote.pitch = static_cast<int>(
            source.notes[i].pitch * (1.0f - morphAmount) + 
            target.notes[i].pitch * morphAmount
        );
        morphedNote.startTime = 
            source.notes[i].startTime * (1.0 - morphAmount) + 
            target.notes[i].startTime * morphAmount;
        morphedNote.duration = 
            source.notes[i].duration * (1.0 - morphAmount) + 
            target.notes[i].duration * morphAmount;
        morphedNote.velocity = static_cast<int>(
            source.notes[i].velocity * (1.0f - morphAmount) + 
            target.notes[i].velocity * morphAmount
        );
        
        morphed.notes.push_back(morphedNote);
    }
    
    // Add remaining notes if target is longer
    if (target.notes.size() > source.notes.size() && morphAmount > 0.5f)
    {
        for (size_t i = minSize; i < target.notes.size(); ++i)
        {
            morphed.notes.push_back(target.notes[i]);
        }
    }
    // Add remaining notes if source is longer
    else if (source.notes.size() > target.notes.size() && morphAmount < 0.5f)
    {
        for (size_t i = minSize; i < source.notes.size(); ++i)
        {
            morphed.notes.push_back(source.notes[i]);
        }
    }
    
    morphed.generation = std::max(source.generation, target.generation);
    morphed.id = juce::Uuid().toString();
    morphed.timestamp = juce::Time::getCurrentTime().toMilliseconds() / 1000.0;
    
    return morphed;
}

std::vector<MusicTheoryEngine::PatternGenome> MusicTheoryEngine::generateVariations(
    const PatternGenome& basePattern,
    int numVariations,
    const GenerationParameters& params)
{
    std::vector<PatternGenome> variations;
    variations.reserve(numVariations);
    
    for (int i = 0; i < numVariations; ++i)
    {
        // Create variation by applying different levels of mutation
        GenerationParameters varParams = params;
        varParams.variation.mutationRate = 0.1f + (i * 0.1f);  // Increasing mutation rate
        
        PatternGenome variation = mutatePattern(basePattern, varParams);
        variation.fitness = calculatePatternFitness(variation, params);
        variations.push_back(variation);
    }
    
    return variations;
}

std::vector<MusicTheoryEngine::PatternGenome> MusicTheoryEngine::selectElitePatterns(
    const std::vector<PatternGenome>& population,
    int numElite)
{
    std::vector<PatternGenome> sorted = population;
    std::sort(sorted.begin(), sorted.end(), 
              [](const PatternGenome& a, const PatternGenome& b) {
                  return a.fitness > b.fitness;
              });
    
    if (sorted.size() <= static_cast<size_t>(numElite))
        return sorted;
        
    return std::vector<PatternGenome>(sorted.begin(), sorted.begin() + numElite);
}

MusicTheoryEngine::PatternGenome MusicTheoryEngine::convertToGenome(const MIDIPattern& pattern)
{
    PatternGenome genome;
    genome.notes = pattern.notes;
    genome.id = juce::Uuid().toString();
    genome.timestamp = juce::Time::getCurrentTime().toMilliseconds() / 1000.0;
    genome.generation = 0;
    return genome;
}

MIDIPattern MusicTheoryEngine::convertFromGenome(const PatternGenome& genome)
{
    MIDIPattern pattern;
    pattern.notes = genome.notes;
    return pattern;
}

//==============================================================================
// PatternEvolutionEngine Implementation

MusicTheoryEngine::PatternEvolutionEngine::PatternEvolutionEngine(MusicTheoryEngine* engine)
    : engine(engine), evolutionRng(std::random_device{}())
{
}

void MusicTheoryEngine::PatternEvolutionEngine::initializePopulation(
    const std::vector<MIDIPattern>& seedPatterns,
    const GenerationParameters& params)
{
    currentPopulation.clear();
    generationHistory.clear();
    currentGeneration = 0;
    
    // Convert seed patterns to genomes
    for (const auto& pattern : seedPatterns)
    {
        PatternGenome genome = engine->convertToGenome(pattern);
        genome.fitness = engine->calculatePatternFitness(genome, params);
        currentPopulation.push_back(genome);
    }
    
    // Save initial generation
    generationHistory.push_back(currentPopulation);
}

void MusicTheoryEngine::PatternEvolutionEngine::evolveGeneration(const GenerationParameters& params)
{
    if (currentPopulation.empty())
        return;
        
    std::vector<PatternGenome> newGeneration;
    size_t populationSize = currentPopulation.size();
    
    // Sort current population by fitness
    std::sort(currentPopulation.begin(), currentPopulation.end(),
              [](const PatternGenome& a, const PatternGenome& b) {
                  return a.fitness > b.fitness;
              });
    
    // Keep elite (top 20%)
    size_t eliteCount = std::max(1UL, populationSize / 5);
    for (size_t i = 0; i < eliteCount; ++i)
    {
        newGeneration.push_back(currentPopulation[i]);
    }
    
    // Generate offspring to fill rest of population
    std::uniform_int_distribution<size_t> parentSelector(0, std::min(populationSize, eliteCount * 2) - 1);
    
    while (newGeneration.size() < populationSize)
    {
        size_t parent1Idx = parentSelector(evolutionRng);
        size_t parent2Idx = parentSelector(evolutionRng);
        
        PatternGenome offspring = engine->evolvePattern(
            currentPopulation[parent1Idx],
            currentPopulation[parent2Idx],
            params
        );
        
        newGeneration.push_back(offspring);
    }
    
    currentPopulation = newGeneration;
    currentGeneration++;
    
    // Maintain diversity
    maintainDiversity();
    
    // Save generation history (limit to prevent memory growth)
    generationHistory.push_back(currentPopulation);
    if (generationHistory.size() > static_cast<size_t>(params.variation.generationHistory))
    {
        generationHistory.erase(generationHistory.begin());
    }
}

std::vector<MusicTheoryEngine::PatternGenome> MusicTheoryEngine::PatternEvolutionEngine::getCurrentElite(int count)
{
    return engine->selectElitePatterns(currentPopulation, count);
}

MusicTheoryEngine::PatternGenome MusicTheoryEngine::PatternEvolutionEngine::getPatternFromHistory(int generation, int index)
{
    if (generation < 0 || generation >= static_cast<int>(generationHistory.size()))
        return {};
        
    const auto& genPatterns = generationHistory[generation];
    if (index < 0 || index >= static_cast<int>(genPatterns.size()))
        return {};
        
    return genPatterns[index];
}

bool MusicTheoryEngine::PatternEvolutionEngine::revertToGeneration(int targetGeneration)
{
    if (targetGeneration < 0 || targetGeneration >= static_cast<int>(generationHistory.size()))
        return false;
        
    currentPopulation = generationHistory[targetGeneration];
    currentGeneration = targetGeneration;
    return true;
}

MusicTheoryEngine::PatternEvolutionEngine::EvolutionStats MusicTheoryEngine::PatternEvolutionEngine::getStats() const
{
    EvolutionStats stats;
    stats.currentGeneration = currentGeneration;
    stats.populationSize = static_cast<int>(currentPopulation.size());
    
    if (!currentPopulation.empty())
    {
        float totalFitness = 0.0f;
        float maxFitness = 0.0f;
        
        for (const auto& genome : currentPopulation)
        {
            totalFitness += genome.fitness;
            maxFitness = std::max(maxFitness, genome.fitness);
        }
        
        stats.averageFitness = totalFitness / currentPopulation.size();
        stats.bestFitness = maxFitness;
    }
    
    // Build fitness history from generation history
    for (const auto& generation : generationHistory)
    {
        if (!generation.empty())
        {
            float genBestFitness = 0.0f;
            for (const auto& genome : generation)
            {
                genBestFitness = std::max(genBestFitness, genome.fitness);
            }
            stats.fitnessHistory.push_back(genBestFitness);
        }
    }
    
    return stats;
}

void MusicTheoryEngine::PatternEvolutionEngine::maintainDiversity()
{
    // Remove patterns that are too similar (simplified diversity check)
    const float similarityThreshold = 0.95f;
    
    for (auto it1 = currentPopulation.begin(); it1 != currentPopulation.end(); ++it1)
    {
        for (auto it2 = it1 + 1; it2 != currentPopulation.end();)
        {
            // Simple similarity check based on note count and average pitch
            bool similar = false;
            
            if (it1->notes.size() == it2->notes.size() && !it1->notes.empty())
            {
                float avgPitch1 = 0.0f, avgPitch2 = 0.0f;
                for (const auto& note : it1->notes) avgPitch1 += note.pitch;
                for (const auto& note : it2->notes) avgPitch2 += note.pitch;
                
                avgPitch1 /= it1->notes.size();
                avgPitch2 /= it2->notes.size();
                
                if (std::abs(avgPitch1 - avgPitch2) < 2.0f)  // Very similar patterns
                {
                    similar = true;
                }
            }
            
            if (similar && it1->fitness > it2->fitness)
            {
                it2 = currentPopulation.erase(it2);
            }
            else
            {
                ++it2;
            }
        }
    }
}

float MusicTheoryEngine::PatternEvolutionEngine::calculateDiversity() const
{
    if (currentPopulation.size() < 2)
        return 1.0f;
        
    float totalDifference = 0.0f;
    int comparisons = 0;
    
    for (size_t i = 0; i < currentPopulation.size(); ++i)
    {
        for (size_t j = i + 1; j < currentPopulation.size(); ++j)
        {
            // Simple diversity metric based on fitness difference
            totalDifference += std::abs(currentPopulation[i].fitness - currentPopulation[j].fitness);
            comparisons++;
        }
    }
    
    return comparisons > 0 ? totalDifference / comparisons : 0.0f;
}

//==============================================================================
// Epic 6 Story 6.3: Advanced Chord Progressions Implementation

juce::String MusicTheoryEngine::AdvancedChord::getName() const
{
    juce::String name;
    
    // Root note names
    const char* noteNames[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    name += noteNames[root % 12];
    
    // Chord quality
    switch (type)
    {
        case ExtendedChordType::Major: name += ""; break;
        case ExtendedChordType::Minor: name += "m"; break;
        case ExtendedChordType::Diminished: name += "dim"; break;
        case ExtendedChordType::Augmented: name += "aug"; break;
        case ExtendedChordType::Major7: name += "maj7"; break;
        case ExtendedChordType::Minor7: name += "m7"; break;
        case ExtendedChordType::Dominant7: name += "7"; break;
        case ExtendedChordType::Diminished7: name += "dim7"; break;
        case ExtendedChordType::HalfDiminished7: name += "m7b5"; break;
        case ExtendedChordType::Major9: name += "maj9"; break;
        case ExtendedChordType::Minor9: name += "m9"; break;
        case ExtendedChordType::Dominant9: name += "9"; break;
        case ExtendedChordType::Major11: name += "maj11"; break;
        case ExtendedChordType::Minor11: name += "m11"; break;
        case ExtendedChordType::Dominant11: name += "11"; break;
        case ExtendedChordType::Major13: name += "maj13"; break;
        case ExtendedChordType::Minor13: name += "m13"; break;
        case ExtendedChordType::Dominant13: name += "13"; break;
        case ExtendedChordType::Sus2: name += "sus2"; break;
        case ExtendedChordType::Sus4: name += "sus4"; break;
        case ExtendedChordType::Add9: name += "add9"; break;
        default: name += ""; break;
    }
    
    // Alterations
    if (!alterations.empty())
    {
        name += "(";
        for (size_t i = 0; i < alterations.size(); ++i)
        {
            if (i > 0) name += ",";
            int alt = alterations[i];
            if (alt > 0) name += "#" + juce::String(alt);
            else name += "b" + juce::String(-alt);
        }
        name += ")";
    }
    
    // Bass note (slash chord)
    if (bass >= 0 && bass != root)
    {
        name += "/" + juce::String(noteNames[bass % 12]);
    }
    
    return name;
}

juce::String MusicTheoryEngine::AdvancedChord::getRomanNumeral(int key, ScaleType scale) const
{
    // Calculate scale degree
    int scaleDegree = ((root - key) % 12 + 12) % 12;
    
    // Roman numerals (uppercase for major, lowercase for minor)
    const char* majorNumerals[] = { "I", "bII", "II", "bIII", "III", "IV", "bV", "V", "bVI", "VI", "bVII", "VII" };
    const char* minorNumerals[] = { "i", "bii", "ii", "biii", "iii", "iv", "bv", "v", "bvi", "vi", "bvii", "vii" };
    
    juce::String numeral;
    
    switch (type)
    {
        case ExtendedChordType::Major:
        case ExtendedChordType::Major7:
        case ExtendedChordType::Major9:
        case ExtendedChordType::Major11:
        case ExtendedChordType::Major13:
            numeral = majorNumerals[scaleDegree];
            break;
            
        case ExtendedChordType::Minor:
        case ExtendedChordType::Minor7:
        case ExtendedChordType::Minor9:
        case ExtendedChordType::Minor11:
        case ExtendedChordType::Minor13:
            numeral = minorNumerals[scaleDegree];
            break;
            
        case ExtendedChordType::Dominant7:
        case ExtendedChordType::Dominant9:
        case ExtendedChordType::Dominant11:
        case ExtendedChordType::Dominant13:
            numeral = juce::String(majorNumerals[scaleDegree]) + "7";
            break;
            
        case ExtendedChordType::Diminished:
        case ExtendedChordType::Diminished7:
            numeral = juce::String(minorNumerals[scaleDegree]) + "°";
            break;
            
        default:
            numeral = majorNumerals[scaleDegree];
            break;
    }
    
    return numeral;
}

MusicTheoryEngine::AdvancedChord MusicTheoryEngine::generateAdvancedChord(
    int root, ExtendedChordType type, const GenerationParameters& params) const
{
    AdvancedChord chord;
    chord.root = normalizeNote(root);
    chord.type = type;
    
    // Generate base chord tones
    switch (type)
    {
        case ExtendedChordType::Major:
            chord.notes = {0, 4, 7};
            break;
        case ExtendedChordType::Minor:
            chord.notes = {0, 3, 7};
            break;
        case ExtendedChordType::Diminished:
            chord.notes = {0, 3, 6};
            break;
        case ExtendedChordType::Augmented:
            chord.notes = {0, 4, 8};
            break;
        case ExtendedChordType::Major7:
            chord.notes = {0, 4, 7, 11};
            break;
        case ExtendedChordType::Minor7:
            chord.notes = {0, 3, 7, 10};
            break;
        case ExtendedChordType::Dominant7:
            chord.notes = {0, 4, 7, 10};
            break;
        case ExtendedChordType::Diminished7:
            chord.notes = {0, 3, 6, 9};
            break;
        case ExtendedChordType::HalfDiminished7:
            chord.notes = {0, 3, 6, 10};
            break;
        case ExtendedChordType::Major9:
            chord.notes = {0, 4, 7, 11, 14};
            break;
        case ExtendedChordType::Minor9:
            chord.notes = {0, 3, 7, 10, 14};
            break;
        case ExtendedChordType::Dominant9:
            chord.notes = {0, 4, 7, 10, 14};
            break;
        case ExtendedChordType::Sus2:
            chord.notes = {0, 2, 7};
            break;
        case ExtendedChordType::Sus4:
            chord.notes = {0, 5, 7};
            break;
        case ExtendedChordType::Add9:
            chord.notes = {0, 4, 7, 14};
            break;
        default:
            chord.notes = {0, 4, 7}; // Default to major triad
            break;
    }
    
    // Convert to absolute pitches
    for (auto& note : chord.notes)
    {
        note = normalizeNote(chord.root + note);
    }
    
    // Calculate tension level
    chord.tension = calculateChordTension(chord, params.key, static_cast<MusicTheoryEngine::ScaleType>(params.scale));
    
    return chord;
}

std::vector<MusicTheoryEngine::AdvancedChord> MusicTheoryEngine::generateAdvancedProgression(
    int key, ScaleType scale, const GenerationParameters& params, int numChords) const
{
    std::vector<AdvancedChord> progression;
    std::mt19937 gen{std::random_device{}()};
    
    // Get scale notes for key
    auto scaleNotes = getScaleNotes(key, scale);
    
    // Generate progression based on style
    switch (params.harmony.style)
    {
        case GenerationParameters::HarmonicSettings::ProgressionStyle::Classical:
            progression = generateClassicalProgression(key, scale, params, numChords);
            break;
            
        case GenerationParameters::HarmonicSettings::ProgressionStyle::Jazz:
            progression = generateJazzProgression(key, scale, params, numChords);
            break;
            
        case GenerationParameters::HarmonicSettings::ProgressionStyle::Pop:
            progression = generatePopProgression(key, scale, params, numChords);
            break;
            
        case GenerationParameters::HarmonicSettings::ProgressionStyle::Electronic:
            progression = generateElectronicProgression(key, scale, params, numChords);
            break;
            
        default:
            progression = generatePopProgression(key, scale, params, numChords);
            break;
    }
    
    // Apply modal interchange if requested
    if (params.harmony.modalInterchange > 0.0f)
    {
        progression = applyModalInterchange(progression, key, scale, params.harmony.modalInterchange);
    }
    
    // Add secondary dominants if requested
    if (params.harmony.secondaryDominants > 0.0f)
    {
        progression = addSecondaryDominants(progression, key, scale, params.harmony.secondaryDominants);
    }
    
    // Optimize voice leading
    progression = optimizeAdvancedVoiceLeading(progression, params);
    
    return progression;
}

float MusicTheoryEngine::calculateChordTension(const AdvancedChord& chord, int key, MusicTheoryEngine::ScaleType scale) const
{
    float tension = 0.0f;
    
    // Base tension from chord type
    switch (chord.type)
    {
        case ExtendedChordType::Major:
        case ExtendedChordType::Minor:
            tension = 0.1f;
            break;
        case ExtendedChordType::Dominant7:
            tension = 0.6f;
            break;
        case ExtendedChordType::Diminished7:
        case ExtendedChordType::HalfDiminished7:
            tension = 0.8f;
            break;
        case ExtendedChordType::Dominant9:
        case ExtendedChordType::Dominant11:
        case ExtendedChordType::Dominant13:
            tension = 0.7f;
            break;
        default:
            tension = 0.3f;
            break;
    }
    
    // Add tension for non-diatonic notes
    auto scaleNotes = getScaleNotes(key, scale);
    for (int note : chord.notes)
    {
        bool inScale = std::find(scaleNotes.begin(), scaleNotes.end(), note % 12) != scaleNotes.end();
        if (!inScale)
            tension += 0.2f;
    }
    
    // Add tension for alterations
    tension += chord.alterations.size() * 0.1f;
    
    return std::clamp(tension, 0.0f, 1.0f);
}

// Helper functions for different progression styles
std::vector<MusicTheoryEngine::AdvancedChord> MusicTheoryEngine::generateClassicalProgression(
    int key, ScaleType scale, const GenerationParameters& params, int numChords) const
{
    std::vector<AdvancedChord> progression;
    std::mt19937 gen{std::random_device{}()};
    
    // Classical I-IV-V-I with embellishments
    std::vector<int> classicalPattern = {0, 3, 4, 0}; // I-IV-V-I in scale degrees
    
    for (int i = 0; i < numChords; ++i)
    {
        int scaleDegree = classicalPattern[i % classicalPattern.size()];
        auto scaleNotes = getScaleNotes(key, scale);
        int chordRoot = scaleNotes[scaleDegree % scaleNotes.size()];
        
        ExtendedChordType chordType = ExtendedChordType::Major;
        if (scale == ScaleType::NaturalMinor && (scaleDegree == 0 || scaleDegree == 3))
            chordType = ExtendedChordType::Minor;
        
        // Add 7ths occasionally for complexity
        if (params.harmony.complexity >= GenerationParameters::HarmonicSettings::HarmonicComplexity::Intermediate &&
            std::uniform_real_distribution<float>(0.0f, 1.0f)(gen) < 0.3f)
        {
            chordType = (chordType == ExtendedChordType::Major) ? ExtendedChordType::Major7 : ExtendedChordType::Minor7;
        }
        
        progression.push_back(generateAdvancedChord(chordRoot, chordType, params));
    }
    
    return progression;
}

std::vector<MusicTheoryEngine::AdvancedChord> MusicTheoryEngine::generateJazzProgression(
    int key, ScaleType scale, const GenerationParameters& params, int numChords) const
{
    std::vector<AdvancedChord> progression;
    std::mt19937 gen{std::random_device{}()};
    
    // Jazz ii-V-I progression with extensions
    std::vector<std::pair<int, ExtendedChordType>> jazzPattern = {
        {1, ExtendedChordType::Minor7},     // ii7
        {4, ExtendedChordType::Dominant7},  // V7
        {0, ExtendedChordType::Major7},     // Imaj7
        {5, ExtendedChordType::Minor7}      // vi7
    };
    
    auto scaleNotes = getScaleNotes(key, scale);
    
    for (int i = 0; i < numChords; ++i)
    {
        auto& pattern = jazzPattern[i % jazzPattern.size()];
        int chordRoot = scaleNotes[pattern.first % scaleNotes.size()];
        ExtendedChordType chordType = pattern.second;
        
        // Upgrade to extensions based on complexity
        if (params.harmony.complexity >= GenerationParameters::HarmonicSettings::HarmonicComplexity::Advanced)
        {
            if (chordType == ExtendedChordType::Minor7 && std::uniform_real_distribution<float>(0.0f, 1.0f)(gen) < 0.4f)
                chordType = ExtendedChordType::Minor9;
            else if (chordType == ExtendedChordType::Dominant7 && std::uniform_real_distribution<float>(0.0f, 1.0f)(gen) < 0.5f)
                chordType = ExtendedChordType::Dominant9;
        }
        
        progression.push_back(generateAdvancedChord(chordRoot, chordType, params));
    }
    
    return progression;
}

std::vector<MusicTheoryEngine::AdvancedChord> MusicTheoryEngine::generatePopProgression(
    int key, ScaleType scale, const GenerationParameters& params, int numChords) const
{
    std::vector<AdvancedChord> progression;
    std::mt19937 gen{std::random_device{}()};
    
    // Popular vi-IV-I-V progression
    std::vector<std::pair<int, ExtendedChordType>> popPattern = {
        {5, ExtendedChordType::Minor},      // vi
        {3, ExtendedChordType::Major},      // IV
        {0, ExtendedChordType::Major},      // I
        {4, ExtendedChordType::Major}       // V
    };
    
    auto scaleNotes = getScaleNotes(key, scale);
    
    for (int i = 0; i < numChords; ++i)
    {
        auto& pattern = popPattern[i % popPattern.size()];
        int chordRoot = scaleNotes[pattern.first % scaleNotes.size()];
        ExtendedChordType chordType = pattern.second;
        
        // Add occasional 7ths or sus chords for modern pop sound
        if (params.harmony.complexity >= GenerationParameters::HarmonicSettings::HarmonicComplexity::Intermediate)
        {
            float rand = std::uniform_real_distribution<float>(0.0f, 1.0f)(gen);
            if (rand < 0.2f && chordType == ExtendedChordType::Major)
                chordType = ExtendedChordType::Major7;
            else if (rand < 0.15f)
                chordType = ExtendedChordType::Sus4;
        }
        
        progression.push_back(generateAdvancedChord(chordRoot, chordType, params));
    }
    
    return progression;
}

std::vector<MusicTheoryEngine::AdvancedChord> MusicTheoryEngine::generateElectronicProgression(
    int key, ScaleType scale, const GenerationParameters& params, int numChords) const
{
    std::vector<AdvancedChord> progression;
    std::mt19937 gen{std::random_device{}()};
    
    // Electronic music often uses simpler progressions with emphasis on rhythm
    std::vector<std::pair<int, ExtendedChordType>> electronicPattern = {
        {0, ExtendedChordType::Minor},      // i
        {6, ExtendedChordType::Major},      // bVII
        {3, ExtendedChordType::Major},      // IV
        {0, ExtendedChordType::Minor}       // i
    };
    
    auto scaleNotes = getScaleNotes(key, scale);
    
    for (int i = 0; i < numChords; ++i)
    {
        auto& pattern = electronicPattern[i % electronicPattern.size()];
        int chordRoot = scaleNotes[pattern.first % scaleNotes.size()];
        ExtendedChordType chordType = pattern.second;
        
        // Add sus chords and add9 for electronic flavor
        if (params.harmony.complexity >= GenerationParameters::HarmonicSettings::HarmonicComplexity::Intermediate)
        {
            float rand = std::uniform_real_distribution<float>(0.0f, 1.0f)(gen);
            if (rand < 0.3f)
                chordType = ExtendedChordType::Sus2;
            else if (rand < 0.2f)
                chordType = ExtendedChordType::Add9;
        }
        
        progression.push_back(generateAdvancedChord(chordRoot, chordType, params));
    }
    
    return progression;
}

//==============================================================================
// AdvancedHarmonyEngine Implementation

MusicTheoryEngine::AdvancedHarmonyEngine::AdvancedHarmonyEngine(MusicTheoryEngine* engine)
    : engine(engine)
{
    initializeProgressionTemplates();
}

void MusicTheoryEngine::AdvancedHarmonyEngine::initializeProgressionTemplates()
{
    // Initialize common progression templates for each style
    // These are scale degree patterns that can be transposed to any key
    
    progressionTemplates[GenerationParameters::HarmonicSettings::ProgressionStyle::Classical] = {
        {0, 3, 4, 0},           // I-IV-V-I
        {0, 5, 3, 4, 0},        // I-vi-IV-V-I
        {0, 1, 4, 0}            // I-ii-V-I
    };
    
    progressionTemplates[GenerationParameters::HarmonicSettings::ProgressionStyle::Jazz] = {
        {1, 4, 0},              // ii-V-I
        {0, 5, 1, 4, 0},        // I-vi-ii-V-I
        {2, 4, 0, 5}            // iii-V-I-vi
    };
    
    progressionTemplates[GenerationParameters::HarmonicSettings::ProgressionStyle::Pop] = {
        {5, 3, 0, 4},           // vi-IV-I-V
        {0, 4, 5, 3},           // I-V-vi-IV
        {5, 3, 4, 4}            // vi-IV-V-V
    };
    
    progressionTemplates[GenerationParameters::HarmonicSettings::ProgressionStyle::Electronic] = {
        {0, 6, 3, 4},           // i-bVII-IV-V
        {0, 5, 6, 4},           // i-vi-bVII-V
        {0, 2, 6, 0}            // i-III-bVII-i
    };
}

std::vector<MusicTheoryEngine::AdvancedChord> MusicTheoryEngine::AdvancedHarmonyEngine::generateContextualProgression(
    int key, ScaleType scale, const GenerationParameters& params)
{
    // Use the engine's generateAdvancedProgression method
    return engine->generateAdvancedProgression(key, scale, params, 8);
}

std::vector<MusicTheoryEngine::AdvancedChord> MusicTheoryEngine::AdvancedHarmonyEngine::evolveHarmony(
    const std::vector<AdvancedChord>& seedProgression,
    const GenerationParameters& params,
    float evolutionAmount)
{
    std::vector<AdvancedChord> evolved = seedProgression;
    std::mt19937 gen{std::random_device{}()};
    
    for (auto& chord : evolved)
    {
        // Evolve chord complexity based on evolution amount
        if (std::uniform_real_distribution<float>(0.0f, 1.0f)(gen) < evolutionAmount)
        {
            // Upgrade chord to more complex version
            switch (chord.type)
            {
                case ExtendedChordType::Major:
                    if (params.harmony.complexity >= GenerationParameters::HarmonicSettings::HarmonicComplexity::Intermediate)
                        chord = engine->generateAdvancedChord(chord.root, ExtendedChordType::Major7, params);
                    break;
                case ExtendedChordType::Minor:
                    if (params.harmony.complexity >= GenerationParameters::HarmonicSettings::HarmonicComplexity::Intermediate)
                        chord = engine->generateAdvancedChord(chord.root, ExtendedChordType::Minor7, params);
                    break;
                case ExtendedChordType::Major7:
                    if (params.harmony.complexity >= GenerationParameters::HarmonicSettings::HarmonicComplexity::Advanced)
                        chord = engine->generateAdvancedChord(chord.root, ExtendedChordType::Major9, params);
                    break;
                default:
                    break;
            }
        }
    }
    
    return evolved;
}

MusicTheoryEngine::AdvancedHarmonyEngine::HarmonicAnalysis MusicTheoryEngine::AdvancedHarmonyEngine::analyzeProgression(
    const std::vector<AdvancedChord>& progression, int key, ScaleType scale) const
{
    HarmonicAnalysis analysis;
    
    // Analyze each chord's function and tension
    for (const auto& chord : progression)
    {
        analysis.functions.push_back(analyzeHarmonicFunction(chord, key, scale));
        analysis.tensions.push_back(engine->calculateChordTension(chord, key, scale));
    }
    
    // Calculate overall coherence
    float coherence = 0.0f;
    if (!progression.empty())
    {
        // Simple coherence based on diatonic content
        auto scaleNotes = engine->getScaleNotes(key, scale);
        int diatonicChords = 0;
        
        for (const auto& chord : progression)
        {
            bool isDiatonic = std::find(scaleNotes.begin(), scaleNotes.end(), chord.root) != scaleNotes.end();
            if (isDiatonic) diatonicChords++;
        }
        
        coherence = static_cast<float>(diatonicChords) / progression.size();
    }
    
    analysis.overallCoherence = coherence;
    
    // Check for strong cadence (V-I or vii-I)
    if (progression.size() >= 2)
    {
        const auto& penultimate = progression[progression.size() - 2];
        const auto& final = progression.back();
        
        int penultimateScaleDegree = ((penultimate.root - key) % 12 + 12) % 12;
        int finalScaleDegree = ((final.root - key) % 12 + 12) % 12;
        
        // V-I cadence (scale degree 7 to 0) or vii-I (scale degree 11 to 0)
        if ((penultimateScaleDegree == 7 || penultimateScaleDegree == 11) && finalScaleDegree == 0)
        {
            analysis.hasStrongCadence = true;
        }
    }
    
    // Calculate harmonic rhythm score (based on tension changes)
    if (analysis.tensions.size() > 1)
    {
        float tensionVariation = 0.0f;
        for (size_t i = 1; i < analysis.tensions.size(); ++i)
        {
            tensionVariation += std::abs(analysis.tensions[i] - analysis.tensions[i-1]);
        }
        analysis.harmonicRhythmScore = tensionVariation / (analysis.tensions.size() - 1);
    }
    
    return analysis;
}

std::vector<std::vector<MusicTheoryEngine::AdvancedChord>> MusicTheoryEngine::AdvancedHarmonyEngine::generateProgressionVariations(
    const std::vector<AdvancedChord>& baseProgression,
    const GenerationParameters& params,
    int numVariations)
{
    std::vector<std::vector<AdvancedChord>> variations;
    std::mt19937 gen{std::random_device{}()};
    
    for (int v = 0; v < numVariations; ++v)
    {
        std::vector<AdvancedChord> variation = baseProgression;
        
        // Apply different types of variations
        for (auto& chord : variation)
        {
            float rand = std::uniform_real_distribution<float>(0.0f, 1.0f)(gen);
            
            if (rand < 0.3f) // 30% chance to substitute chord
            {
                // Generate substitution based on harmonic function
                auto substitutions = engine->generateSubstitutions(chord, params);
                if (!substitutions.empty())
                {
                    int substIndex = std::uniform_int_distribution<int>(0, substitutions.size() - 1)(gen);
                    chord = substitutions[substIndex];
                }
            }
            else if (rand < 0.2f) // 20% chance to change complexity
            {
                // Upgrade or downgrade chord complexity
                if (chord.type == ExtendedChordType::Major && params.harmony.complexity >= GenerationParameters::HarmonicSettings::HarmonicComplexity::Intermediate)
                {
                    chord = engine->generateAdvancedChord(chord.root, ExtendedChordType::Major7, params);
                }
                else if (chord.type == ExtendedChordType::Major7 && params.harmony.complexity >= GenerationParameters::HarmonicSettings::HarmonicComplexity::Advanced)
                {
                    chord = engine->generateAdvancedChord(chord.root, ExtendedChordType::Major9, params);
                }
            }
        }
        
        variations.push_back(variation);
    }
    
    return variations;
}

MusicTheoryEngine::AdvancedChord::Function MusicTheoryEngine::AdvancedHarmonyEngine::analyzeHarmonicFunction(
    const AdvancedChord& chord, int key, ScaleType scale) const
{
    int scaleDegree = ((chord.root - key) % 12 + 12) % 12;
    
    // Basic functional analysis
    switch (scaleDegree)
    {
        case 0: // I
        case 5: // vi (relative minor, still tonic function)
            return AdvancedChord::Function::Tonic;
            
        case 3: // IV
        case 1: // ii (common subdominant substitute)
            return AdvancedChord::Function::Subdominant;
            
        case 7: // V
        case 11: // vii (leading tone, dominant function)
            return AdvancedChord::Function::Dominant;
            
        default:
            return AdvancedChord::Function::Other;
    }
}

// Stub implementations for missing methods
std::vector<MusicTheoryEngine::AdvancedChord> MusicTheoryEngine::applyModalInterchange(
    const std::vector<AdvancedChord>& progression, int key, ScaleType scale, float interchangeAmount) const
{
    // Simple implementation - return original progression for now
    return progression;
}

std::vector<MusicTheoryEngine::AdvancedChord> MusicTheoryEngine::addSecondaryDominants(
    const std::vector<AdvancedChord>& progression, int key, ScaleType scale, float amount) const
{
    // Simple implementation - return original progression for now
    return progression;
}

std::vector<float> MusicTheoryEngine::calculateTensionCurve(
    const std::vector<AdvancedChord>& progression, const GenerationParameters& params) const
{
    std::vector<float> tensions;
    for (const auto& chord : progression)
    {
        tensions.push_back(calculateChordTension(chord, params.key, static_cast<MusicTheoryEngine::ScaleType>(params.scale)));
    }
    return tensions;
}

std::vector<MusicTheoryEngine::AdvancedChord> MusicTheoryEngine::optimizeAdvancedVoiceLeading(
    const std::vector<AdvancedChord>& progression, const GenerationParameters& params) const
{
    // Simple implementation - return original progression for now
    return progression;
}

std::vector<MusicTheoryEngine::AdvancedChord> MusicTheoryEngine::generateSubstitutions(
    const AdvancedChord& originalChord, const GenerationParameters& params) const
{
    std::vector<AdvancedChord> substitutions;
    
    // Add a few simple substitutions
    if (originalChord.type == ExtendedChordType::Major)
    {
        // Add relative minor
        int relativeMinor = (originalChord.root + 9) % 12;
        substitutions.push_back(generateAdvancedChord(relativeMinor, ExtendedChordType::Minor, params));
    }
    
    return substitutions;
}
