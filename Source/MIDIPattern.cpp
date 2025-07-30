/*
  ==============================================================================

    MIDIPattern.cpp
    Created: 29 Jul 2025
    Author:  Developer Agent

    Implementation of MIDIPattern serialization methods.

  ==============================================================================
*/

#include "MIDIPattern.h"

//==============================================================================
juce::ValueTree MIDIPattern::toValueTree() const
{
    juce::ValueTree tree("MIDIPattern");
    
    // Basic properties
    tree.setProperty("lengthInBeats", lengthInBeats, nullptr);
    tree.setProperty("id", id.toString(), nullptr);
    
    // Metadata (generation parameters)
    tree.setProperty("key", metadata.key, nullptr);
    tree.setProperty("scale", static_cast<int>(metadata.scale), nullptr);
    tree.setProperty("tempo", metadata.tempo, nullptr);
    tree.setProperty("complexity", metadata.rhythmicComplexity, nullptr);
    tree.setProperty("generationType", static_cast<int>(metadata.generationType), nullptr);
    tree.setProperty("patternLengthBeats", metadata.patternLengthBeats, nullptr);
    
    // Notes
    juce::ValueTree notesTree("Notes");
    for (const auto& note : notes)
    {
        juce::ValueTree noteTree("Note");
        noteTree.setProperty("pitch", note.pitch, nullptr);
        noteTree.setProperty("velocity", note.velocity, nullptr);
        noteTree.setProperty("startTime", note.startTime, nullptr);
        noteTree.setProperty("duration", note.duration, nullptr);
        
        notesTree.appendChild(noteTree, nullptr);
    }
    tree.appendChild(notesTree, nullptr);
    
    return tree;
}

//==============================================================================
void MIDIPattern::fromValueTree(const juce::ValueTree& tree)
{
    if (!tree.hasType("MIDIPattern"))
        return;
    
    // Clear existing data
    clear();
    
    // Basic properties
    lengthInBeats = tree.getProperty("lengthInBeats", 16.0);
    id = juce::Uuid(tree.getProperty("id", "").toString());
    
    // Metadata (generation parameters)
    metadata.key = tree.getProperty("key", 0);
    metadata.scale = static_cast<GenerationParameters::ScaleType>(
        static_cast<int>(tree.getProperty("scale", 0))
    );
    metadata.tempo = tree.getProperty("tempo", 120.0f);
    metadata.rhythmicComplexity = tree.getProperty("complexity", 0.5f);
    metadata.generationType = static_cast<GenerationParameters::GenerationType>(
        static_cast<int>(tree.getProperty("generationType", 0))
    );
    metadata.patternLengthBeats = tree.getProperty("patternLengthBeats", 16.0f);
    
    // Notes
    auto notesTree = tree.getChildWithName("Notes");
    if (notesTree.isValid())
    {
        for (int i = 0; i < notesTree.getNumChildren(); ++i)
        {
            auto noteTree = notesTree.getChild(i);
            if (noteTree.hasType("Note"))
            {
                Note note;
                note.pitch = noteTree.getProperty("pitch", 60);
                note.velocity = noteTree.getProperty("velocity", 127);
                note.startTime = noteTree.getProperty("startTime", 0.0);
                note.duration = noteTree.getProperty("duration", 1.0);
                
                notes.push_back(note);
            }
        }
    }
}
