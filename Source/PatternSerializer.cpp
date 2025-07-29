/*
  ==============================================================================

    PatternSerializer.cpp
    Created: 28 Jul 2025
    Author:  Developer Agent

    Implementation of MIDIPattern serialization.

  ==============================================================================
*/

#include "PatternSerializer.h"

//==============================================================================
// ValueTree Identifier definitions
const juce::Identifier PatternSerializer::PATTERN_ID("MIDIPattern");
const juce::Identifier PatternSerializer::ID_PROP("id");
const juce::Identifier PatternSerializer::LENGTH_PROP("lengthInBeats");

const juce::Identifier PatternSerializer::METADATA_ID("Metadata");
const juce::Identifier PatternSerializer::KEY_PROP("key");
const juce::Identifier PatternSerializer::SCALE_PROP("scale");
const juce::Identifier PatternSerializer::TEMPO_PROP("tempo");
const juce::Identifier PatternSerializer::COMPLEXITY_PROP("rhythmicComplexity");
const juce::Identifier PatternSerializer::GEN_TYPE_PROP("generationType");
const juce::Identifier PatternSerializer::PATTERN_LENGTH_PROP("patternLengthBeats");

const juce::Identifier PatternSerializer::NOTES_ID("Notes");
const juce::Identifier PatternSerializer::NOTE_ID("Note");
const juce::Identifier PatternSerializer::PITCH_PROP("pitch");
const juce::Identifier PatternSerializer::VELOCITY_PROP("velocity");
const juce::Identifier PatternSerializer::START_TIME_PROP("startTime");
const juce::Identifier PatternSerializer::DURATION_PROP("duration");

//==============================================================================
juce::ValueTree PatternSerializer::toValueTree(const MIDIPattern& pattern)
{
    juce::ValueTree tree(PATTERN_ID);
    tree.setProperty(ID_PROP, pattern.id.toString(), nullptr);
    tree.setProperty(LENGTH_PROP, pattern.lengthInBeats, nullptr);

    // --- Serialize Metadata ---
    juce::ValueTree metadataTree(METADATA_ID);
    metadataTree.setProperty(KEY_PROP, pattern.metadata.key, nullptr);
    metadataTree.setProperty(SCALE_PROP, static_cast<int>(pattern.metadata.scale), nullptr);
    metadataTree.setProperty(TEMPO_PROP, pattern.metadata.tempo, nullptr);
    metadataTree.setProperty(COMPLEXITY_PROP, pattern.metadata.rhythmicComplexity, nullptr);
    metadataTree.setProperty(GEN_TYPE_PROP, static_cast<int>(pattern.metadata.generationType), nullptr);
    metadataTree.setProperty(PATTERN_LENGTH_PROP, pattern.metadata.patternLengthBeats, nullptr);
    tree.addChild(metadataTree, -1, nullptr);

    // --- Serialize Notes ---
    juce::ValueTree notesTree(NOTES_ID);
    for (const auto& note : pattern.notes)
    {
        juce::ValueTree noteTree(NOTE_ID);
        noteTree.setProperty(PITCH_PROP, note.pitch, nullptr);
        noteTree.setProperty(VELOCITY_PROP, note.velocity, nullptr);
        noteTree.setProperty(START_TIME_PROP, note.startTime, nullptr);
        noteTree.setProperty(DURATION_PROP, note.duration, nullptr);
        notesTree.addChild(noteTree, -1, nullptr);
    }
    tree.addChild(notesTree, -1, nullptr);

    return tree;
}

//==============================================================================
MIDIPattern PatternSerializer::fromValueTree(const juce::ValueTree& tree)
{
    MIDIPattern pattern;
    if (!tree.hasType(PATTERN_ID))
        return pattern;

    pattern.id = juce::Uuid(tree.getProperty(ID_PROP).toString());
    pattern.lengthInBeats = tree.getProperty(LENGTH_PROP);

    // --- Deserialize Metadata ---
    juce::ValueTree metadataTree = tree.getChildWithName(METADATA_ID);
    if (metadataTree.isValid())
    {
        pattern.metadata.key = metadataTree.getProperty(KEY_PROP);
        pattern.metadata.scale = static_cast<GenerationParameters::ScaleType>((int)metadataTree.getProperty(SCALE_PROP));
        pattern.metadata.tempo = metadataTree.getProperty(TEMPO_PROP);
        pattern.metadata.rhythmicComplexity = metadataTree.getProperty(COMPLEXITY_PROP);
        pattern.metadata.generationType = static_cast<GenerationParameters::GenerationType>((int)metadataTree.getProperty(GEN_TYPE_PROP));
        pattern.metadata.patternLengthBeats = metadataTree.getProperty(PATTERN_LENGTH_PROP);
    }

    // --- Deserialize Notes ---
    juce::ValueTree notesTree = tree.getChildWithName(NOTES_ID);
    if (notesTree.isValid())
    {
        for (const auto& noteTree : notesTree)
        {
            if (noteTree.hasType(NOTE_ID))
            {
                Note note;
                note.pitch = noteTree.getProperty(PITCH_PROP);
                note.velocity = noteTree.getProperty(VELOCITY_PROP);
                note.startTime = noteTree.getProperty(START_TIME_PROP);
                note.duration = noteTree.getProperty(DURATION_PROP);
                pattern.notes.push_back(note);
            }
        }
    }

    return pattern;
}
