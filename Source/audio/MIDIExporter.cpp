/*
  ==============================================================================

    MIDIExporter.cpp
    Created: 29 Jul 2025
    Author:  Epic 2 Story 2.3 Implementation

    Implementation of MIDI export and drag-drop functionality.

  ==============================================================================
*/

#include "MIDIExporter.h"
#include <ctime>
#include <iomanip>
#include <sstream>

//==============================================================================
// MIDIExporter Implementation

MIDIExporter::MIDIExporter()
{
}

MIDIExporter::~MIDIExporter()
{
}

//==============================================================================
// Task 2.3.2: MIDI File Conversion

juce::MidiFile MIDIExporter::patternToMidiFile(const MIDIPattern& pattern)
{
    juce::MidiFile midiFile;
    midiFile.setTicksPerQuarterNote(getTicksPerQuarterNote());
    
    // Create main track
    juce::MidiMessageSequence track;
    
    // Add tempo and time signature events
    addTempoEvents(track, pattern);
    
    // Add note events
    addNoteEvents(track, pattern);
    
    // Set track name
    auto trackName = generateTrackName(pattern);
    track.addEvent(juce::MidiMessage::textMetaEvent(3, trackName), 0.0);
    
    // Add end of track event
    auto endTime = pattern.lengthInBeats * getTicksPerQuarterNote();
    track.addEvent(juce::MidiMessage::endOfTrack(), endTime);
    
    // Update timing and add to file
    track.updateMatchedPairs();
    midiFile.addTrack(track);
    
    return midiFile;
}

juce::MemoryBlock MIDIExporter::patternToMidiData(const MIDIPattern& pattern)
{
    auto midiFile = patternToMidiFile(pattern);
    
    juce::MemoryBlock data;
    juce::MemoryOutputStream stream(data, false);
    
    if (midiFile.writeTo(stream))
    {
        return data;
    }
    
    return {};
}

//==============================================================================
// Task 2.3.4: Export Functions

bool MIDIExporter::exportPatternToFile(const MIDIPattern& pattern, const juce::File& destinationFile)
{
    if (!validatePatternForExport(pattern))
        return false;
    
    auto midiFile = patternToMidiFile(pattern);
    
    juce::FileOutputStream stream(destinationFile);
    if (stream.openedOk())
    {
        return midiFile.writeTo(stream);
    }
    
    return false;
}

bool MIDIExporter::copyPatternToClipboard(const MIDIPattern& pattern)
{
    if (!validatePatternForExport(pattern))
        return false;
    
    auto midiData = patternToMidiData(pattern);
    if (midiData.getSize() > 0)
    {
        // Convert to base64 for clipboard
        auto base64Data = juce::Base64::toBase64(midiData.getData(), midiData.getSize());
        juce::SystemClipboard::copyTextToClipboard("MIDI:" + base64Data);
        return true;
    }
    
    return false;
}

juce::String MIDIExporter::getPatternAsDragData(const MIDIPattern& pattern)
{
    auto midiData = patternToMidiData(pattern);
    if (midiData.getSize() > 0)
    {
        return juce::Base64::toBase64(midiData.getData(), midiData.getSize());
    }
    
    return {};
}

//==============================================================================
// Utility Functions

juce::String MIDIExporter::getSuggestedFileName(const MIDIPattern& pattern)
{
    juce::String fileName = "SpawnClone_Pattern_";
    
    // Add timestamp
    auto time = std::time(nullptr);
    auto tm = *std::localtime(&time);
    
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y%m%d_%H%M%S");
    fileName += juce::String(oss.str());
    
    // Add pattern info
    if (pattern.metadata.tempo > 0)
    {
        fileName += "_" + juce::String(static_cast<int>(pattern.metadata.tempo)) + "bpm";
    }
    
    fileName += ".mid";
    return fileName;
}

bool MIDIExporter::validatePatternForExport(const MIDIPattern& pattern)
{
    // Check if pattern has notes
    if (pattern.notes.empty())
        return false;
    
    // Check if pattern has valid length
    if (pattern.lengthInBeats <= 0)
        return false;
    
    // Check if notes are within pattern bounds
    for (const auto& note : pattern.notes)
    {
        if (note.startTime < 0 || note.startTime >= pattern.lengthInBeats)
            return false;
        if (note.duration <= 0)
            return false;
        if (note.pitch < 0 || note.pitch > 127)
            return false;
        if (note.velocity < 0 || note.velocity > 127)
            return false;
    }
    
    return true;
}

juce::String MIDIExporter::getMidiFileInfo(const MIDIPattern& pattern)
{
    juce::String info;
    info += "Pattern Length: " + juce::String(pattern.lengthInBeats, 1) + " beats\n";
    info += "Note Count: " + juce::String(pattern.notes.size()) + "\n";
    info += "Tempo: " + juce::String(static_cast<int>(pattern.metadata.tempo)) + " BPM\n";
    info += "Key: " + juce::String(pattern.metadata.key) + "\n";
    
    if (!pattern.notes.empty())
    {
        auto minPitch = std::min_element(pattern.notes.begin(), pattern.notes.end(),
                                       [](const Note& a, const Note& b) { return a.pitch < b.pitch; });
        auto maxPitch = std::max_element(pattern.notes.begin(), pattern.notes.end(),
                                       [](const Note& a, const Note& b) { return a.pitch < b.pitch; });
        
        info += "Pitch Range: " + juce::String(minPitch->pitch) + " - " + juce::String(maxPitch->pitch) + "\n";
    }
    
    return info;
}

//==============================================================================
// Private Methods

juce::String MIDIExporter::generateTrackName(const MIDIPattern& pattern)
{
    juce::String name = "SpawnClone Pattern";
    
    if (pattern.metadata.tempo > 0)
    {
        name += " (" + juce::String(static_cast<int>(pattern.metadata.tempo)) + " BPM)";
    }
    
    return name;
}

void MIDIExporter::addTempoEvents(juce::MidiMessageSequence& track, const MIDIPattern& pattern)
{
    if (pattern.metadata.tempo > 0)
    {
        // Add tempo event at the beginning
        auto tempoEvent = juce::MidiMessage::tempoMetaEvent(static_cast<int>(60000000 / pattern.metadata.tempo));
        track.addEvent(tempoEvent, 0.0);
    }
    
    // Add time signature (assume 4/4 for now)
    auto timeSigEvent = juce::MidiMessage::timeSignatureMetaEvent(4, 4);
    track.addEvent(timeSigEvent, 0.0);
}

void MIDIExporter::addNoteEvents(juce::MidiMessageSequence& track, const MIDIPattern& pattern)
{
    int ticksPerQuarter = getTicksPerQuarterNote();
    
    for (const auto& note : pattern.notes)
    {
        // Calculate timing in MIDI ticks
        double noteStartTicks = note.startTime * ticksPerQuarter;
        double noteEndTicks = (note.startTime + note.duration) * ticksPerQuarter;
        
        // Create note on event
        auto noteOnMsg = juce::MidiMessage::noteOn(1, note.pitch, static_cast<juce::uint8>(note.velocity));
        track.addEvent(noteOnMsg, noteStartTicks);
        
        // Create note off event
        auto noteOffMsg = juce::MidiMessage::noteOff(1, note.pitch);
        track.addEvent(noteOffMsg, noteEndTicks);
    }
}

int MIDIExporter::getTicksPerQuarterNote()
{
    return 480; // Standard MIDI resolution
}

//==============================================================================
// MIDIDragSource Implementation

MIDIDragSource::MIDIDragSource()
{
    setSize(120, 30);
}

MIDIDragSource::~MIDIDragSource()
{
}

//==============================================================================
// Task 2.3.1: Drag-and-Drop Implementation

void MIDIDragSource::setPattern(const MIDIPattern& pattern)
{
    currentPattern = pattern;
    patternLoaded = true;
    repaint();
}

//==============================================================================
// Component Overrides

void MIDIDragSource::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Choose color based on state
    juce::Colour bgColour = normalColour;
    if (isDragging)
        bgColour = dragColour;
    else if (isMouseOver())
        bgColour = hoverColour;
    
    // Draw background
    g.setColour(bgColour);
    g.fillRoundedRectangle(bounds, 5.0f);
    
    // Draw border
    g.setColour(juce::Colours::darkgrey);
    g.drawRoundedRectangle(bounds, 5.0f, 1.0f);
    
    // Draw text
    g.setColour(juce::Colours::black);
    g.setFont(12.0f);
    
    juce::String text = patternLoaded ? "Drag MIDI" : "No Pattern";
    g.drawText(text, bounds, juce::Justification::centred);
    
    // Draw MIDI icon
    if (patternLoaded)
    {
        auto iconBounds = bounds.removeFromLeft(20).reduced(2);
        g.setColour(juce::Colours::darkgreen);
        g.fillEllipse(iconBounds);
        g.setColour(juce::Colours::white);
        g.drawText("♪", iconBounds, juce::Justification::centred);
    }
}

void MIDIDragSource::mouseDown(const juce::MouseEvent& event)
{
    if (patternLoaded && dragEnabled)
    {
        isDragging = false;
        repaint();
    }
}

void MIDIDragSource::mouseDrag(const juce::MouseEvent& event)
{
    if (patternLoaded && dragEnabled && event.getDistanceFromDragStart() > 5)
    {
        if (!isDragging)
        {
            startDragOperation(event);
        }
    }
}

//==============================================================================
// Task 2.3.3: Drag Operation Implementation

void MIDIDragSource::startDragOperation(const juce::MouseEvent& event)
{
    isDragging = true;
    repaint();
    
    // Create drag description
    auto dragDescription = createDragDescription();
    
    // Get MIDI data
    auto midiData = MIDIExporter::getPatternAsDragData(currentPattern);
    
    // Start the drag operation
    startDragging(dragDescription, this, juce::ScaledImage(), true);
}

juce::var MIDIDragSource::createDragDescription() const
{
    juce::DynamicObject::Ptr dragData = new juce::DynamicObject();
    
    dragData->setProperty("type", "midi_pattern");
    dragData->setProperty("source", "SpawnClone");
    dragData->setProperty("pattern_id", currentPattern.id.toString());
    dragData->setProperty("length_beats", currentPattern.lengthInBeats);
    dragData->setProperty("note_count", static_cast<int>(currentPattern.notes.size()));
    dragData->setProperty("tempo", currentPattern.metadata.tempo);
    
    // Add MIDI data
    auto midiData = MIDIExporter::getPatternAsDragData(currentPattern);
    dragData->setProperty("midi_data", midiData);
    
    return juce::var(dragData.get());
}
