#include "AIGenerationEngine.h"
#include "../PatternManager.h"
#include "ThreadManager.h"
#include <random>
#include <algorithm>
#include <chrono>
#include <thread>

AIGenerationEngine::AIGenerationEngine(ThreadManager& tm, PatternManager& pm) 
    : threadManager(tm), patternManager(pm)
{
}

void AIGenerationEngine::generatePattern(const GenerationParameters& params)
{
    threadManager.submitTask([this, params]
    {
        // Simulate AI processing time
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        MIDIPattern pattern;
        pattern.id = juce::Uuid();
        pattern.lengthInBeats = params.patternLengthBeats;
        pattern.metadata = params;

        // Generate pattern based on parameters
        generateIntelligentPattern(pattern, params);

        // Use MessageManager to add the pattern on the main thread
        juce::MessageManager::callAsync([this, pattern]
        {
            patternManager.addPattern(pattern);
        });
    });
}

//==============================================================================
void AIGenerationEngine::generateIntelligentPattern(MIDIPattern& pattern, const GenerationParameters& params)
{
    switch (params.generationType)
    {
        case GenerationParameters::GenerationType::Melody:
            generateMelody(pattern, params);
            break;
        case GenerationParameters::GenerationType::Chords:
            generateChords(pattern, params);
            break;
        case GenerationParameters::GenerationType::Bassline:
            generateBass(pattern, params);
            break;
        case GenerationParameters::GenerationType::Drums:
            generateDrum(pattern, params);
            break;
    }
}

void AIGenerationEngine::generateMelody(MIDIPattern& pattern, const GenerationParameters& params)
{
    auto scaleNotes = getScaleNotes(params.key + 60, params.scale); // C4 + key offset
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> floatDist(0.0f, 1.0f);
    
    double currentTime = 0.0;
    int numNotes = static_cast<int>(params.patternLengthBeats * (1.0 + params.rhythmicComplexity) * 2);
    double avgNoteDuration = params.patternLengthBeats / static_cast<double>(numNotes);
    
    int lastPitch = scaleNotes[0];
    
    for (int i = 0; i < numNotes && currentTime < params.patternLengthBeats; ++i)
    {
        Note note;
        
        // AI-like pitch selection: prefer stepwise motion with occasional leaps
        int pitchIndex;
        if (floatDist(gen) < 0.7f) // 70% stepwise motion
        {
            int lastIndex = 0;
            for (size_t j = 0; j < scaleNotes.size(); ++j)
            {
                if (scaleNotes[j] == lastPitch) { lastIndex = static_cast<int>(j); break; }
            }
            
            int direction = floatDist(gen) > 0.5f ? 1 : -1;
            pitchIndex = std::max(0, std::min(lastIndex + direction, static_cast<int>(scaleNotes.size()) - 1));
        }
        else // 30% leap
        {
            std::uniform_int_distribution<int> scaleDist(0, static_cast<int>(scaleNotes.size()) - 1);
            pitchIndex = scaleDist(gen);
        }
        
        note.pitch = scaleNotes[pitchIndex];
        lastPitch = note.pitch;
        
        // Adaptive rhythm based on complexity
        double duration = avgNoteDuration * (0.5 + floatDist(gen) * 1.5);
        if (params.rhythmicComplexity > 0.5)
        {
            // More complex rhythms
            duration *= floatDist(gen) > 0.5f ? 0.5 : 1.5;
        }
        
        note.startTime = getRandomTiming(params.rhythmicComplexity, currentTime);
        note.duration = std::min(duration, params.patternLengthBeats - note.startTime);
        note.velocity = getRandomVelocity(params.rhythmicComplexity);
        
        pattern.notes.push_back(note);
        currentTime += duration * 0.8; // Slight overlap potential
    }
}

void AIGenerationEngine::generateChords(MIDIPattern& pattern, const GenerationParameters& params)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> floatDist(0.0f, 1.0f);
    
    // Common chord progressions
    std::vector<std::vector<int>> progressions = {
        {1, 5, 6, 4}, // I-V-vi-IV
        {1, 6, 4, 5}, // I-vi-IV-V
        {1, 4, 5, 1}, // I-IV-V-I
        {6, 4, 1, 5}, // vi-IV-I-V
    };
    
    std::uniform_int_distribution<int> progDist(0, static_cast<int>(progressions.size()) - 1);
    auto progression = progressions[progDist(gen)];
    double chordDuration = params.patternLengthBeats / progression.size();
    
    for (size_t i = 0; i < progression.size(); ++i)
    {
        auto chordNotes = getChordNotes(params.key + 48, params.scale, progression[i]); // C3 + key
        double startTime = i * chordDuration;
        
        for (int noteIndex : chordNotes)
        {
            Note note;
            note.pitch = noteIndex;
            note.startTime = getRandomTiming(params.rhythmicComplexity * 0.3f, startTime);
            note.duration = chordDuration * (0.8 + floatDist(gen) * 0.4);
            note.velocity = static_cast<int>(getRandomVelocity(params.rhythmicComplexity) * 0.8); // Softer for chords
            
            pattern.notes.push_back(note);
        }
    }
}

void AIGenerationEngine::generateBass(MIDIPattern& pattern, const GenerationParameters& params)
{
    auto scaleNotes = getScaleNotes(params.key + 36, params.scale); // C2 + key offset
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> floatDist(0.0f, 1.0f);
    
    double beatDuration = 1.0; // Quarter note pulse
    int numBeats = static_cast<int>(params.patternLengthBeats);
    
    for (int beat = 0; beat < numBeats; ++beat)
    {
        Note note;
        
        // Bass often emphasizes root and fifth
        if (beat == 0 || floatDist(gen) < 0.6f)
        {
            note.pitch = scaleNotes[0]; // Root
        }
        else
        {
            // Sometimes fifth, sometimes other scale tones
            std::uniform_int_distribution<int> noteDist(0, static_cast<int>(scaleNotes.size()) - 1);
            // Calculate proper fifth degree index (4th index in 7-note scales, adjust for shorter scales)
            int fifthIndex = std::min(4, static_cast<int>(scaleNotes.size()) - 1);
            int noteIndex = floatDist(gen) < 0.4f ? fifthIndex : noteDist(gen);
            note.pitch = scaleNotes[noteIndex];
        }
        
        note.startTime = beat * beatDuration;
        note.duration = beatDuration * (0.7 + params.rhythmicComplexity * 0.3);
        note.velocity = static_cast<int>(getRandomVelocity(params.rhythmicComplexity) * 1.1); // Punchy bass
        
        pattern.notes.push_back(note);
        
        // Add syncopation based on complexity
        if (params.rhythmicComplexity > 0.6 && floatDist(gen) < 0.3f)
        {
            Note syncopated = note;
            syncopated.startTime += beatDuration * 0.75;
            syncopated.duration *= 0.5;
            syncopated.pitch += 12; // Octave up
            syncopated.velocity = static_cast<int>(syncopated.velocity * 0.8);
            
            if (syncopated.startTime + syncopated.duration <= params.patternLengthBeats)
            {
                pattern.notes.push_back(syncopated);
            }
        }
    }
}

void AIGenerationEngine::generateDrum(MIDIPattern& pattern, const GenerationParameters& params)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> floatDist(0.0f, 1.0f);
    
    // Standard drum mapping (General MIDI)
    const int kick = 36;
    const int snare = 38;
    const int hihat = 42;
    const int openHat = 46;
    
    double sixteenthNote = 0.25;
    int numSixteenths = static_cast<int>(params.patternLengthBeats / sixteenthNote);
    
    for (int i = 0; i < numSixteenths; ++i)
    {
        double time = i * sixteenthNote;
        bool isDownbeat = (i % 4) == 0;
        bool isBackbeat = (i % 4) == 2;
        
        // Kick drum pattern
        if (isDownbeat || (params.rhythmicComplexity > 0.6 && floatDist(gen) < 0.3f))
        {
            Note kick_note;
            kick_note.pitch = kick;
            kick_note.startTime = time;
            kick_note.duration = sixteenthNote;
            kick_note.velocity = static_cast<int>(getRandomVelocity(params.rhythmicComplexity) * 1.2);
            pattern.notes.push_back(kick_note);
        }
        
        // Snare on backbeat
        if (isBackbeat)
        {
            Note snare_note;
            snare_note.pitch = snare;
            snare_note.startTime = time;
            snare_note.duration = sixteenthNote;
            snare_note.velocity = getRandomVelocity(params.rhythmicComplexity);
            pattern.notes.push_back(snare_note);
        }
        
        // Hi-hat pattern
        if (params.rhythmicComplexity > 0.3)
        {
            bool addHihat = (i % 2) == 0; // Eighth notes
            if (params.rhythmicComplexity > 0.7)
            {
                addHihat = true; // Sixteenth notes for high complexity
            }
            
            if (addHihat)
            {
                Note hat_note;
                hat_note.pitch = floatDist(gen) < 0.8f ? hihat : openHat;
                hat_note.startTime = time;
                hat_note.duration = sixteenthNote * 0.5;
                hat_note.velocity = static_cast<int>(getRandomVelocity(params.rhythmicComplexity) * 0.6);
                pattern.notes.push_back(hat_note);
            }
        }
    }
}

//==============================================================================
std::vector<int> AIGenerationEngine::getScaleNotes(int rootNote, GenerationParameters::ScaleType scale)
{
    std::vector<int> intervals;
    
    switch (scale)
    {
        case GenerationParameters::ScaleType::Major:
            intervals = {0, 2, 4, 5, 7, 9, 11};
            break;
        case GenerationParameters::ScaleType::Minor:
            intervals = {0, 2, 3, 5, 7, 8, 10};
            break;
        case GenerationParameters::ScaleType::Pentatonic:
            intervals = {0, 2, 4, 7, 9};
            break;
        case GenerationParameters::ScaleType::Blues:
            intervals = {0, 3, 5, 6, 7, 10};
            break;
        case GenerationParameters::ScaleType::Dorian:
            intervals = {0, 2, 3, 5, 7, 9, 10};
            break;
        case GenerationParameters::ScaleType::Mixolydian:
            intervals = {0, 2, 4, 5, 7, 9, 10};
            break;
    }
    
    std::vector<int> notes;
    for (int interval : intervals)
    {
        notes.push_back(rootNote + interval);
    }
    
    return notes;
}

std::vector<int> AIGenerationEngine::getChordNotes(int rootNote, GenerationParameters::ScaleType scale, int chordDegree)
{
    auto scaleNotes = getScaleNotes(rootNote, scale);
    std::vector<int> chord;
    
    if (chordDegree >= 1 && chordDegree <= static_cast<int>(scaleNotes.size()))
    {
        int baseIndex = chordDegree - 1;
        
        // Triad: root, third, fifth (using proper interval calculations)
        chord.push_back(scaleNotes[baseIndex]);
        
        // Calculate third (skip one scale degree)
        int thirdIndex = (baseIndex + 2) % scaleNotes.size();
        chord.push_back(scaleNotes[thirdIndex]);
        
        // Calculate fifth (skip two scale degrees from root)
        int fifthIndex = (baseIndex + 4) % scaleNotes.size();
        // For scales shorter than 7 notes, adjust the fifth calculation
        if (scaleNotes.size() < 7)
        {
            fifthIndex = (baseIndex + std::min(4, static_cast<int>(scaleNotes.size()) - 1)) % scaleNotes.size();
        }
        chord.push_back(scaleNotes[fifthIndex]);
    }
    
    return chord;
}

int AIGenerationEngine::getRandomVelocity(float complexity)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    
    int baseVelocity = 80;
    int variation = static_cast<int>(complexity * 40);
    
    std::uniform_int_distribution<int> varDist(-variation, variation);
    return std::max(30, std::min(baseVelocity + varDist(gen), 127));
}

double AIGenerationEngine::getRandomTiming(float complexity, double baseTime)
{
    if (complexity < 0.3f) return baseTime; // Strict timing
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dist(-0.5, 0.5);
    
    double maxDeviation = complexity * 0.1; // Up to 10% timing deviation
    
    return baseTime + dist(gen) * maxDeviation;
}
