/*
  ==============================================================================

    PatternVisualizationComponent.cpp
    Created: 29 Jul 2025  
    Author:  Epic 8 Implementation

    Implementation of modern pattern visualization component for SpawnClone.

  ==============================================================================
*/

#include "PatternVisualizationComponent.h"

//==============================================================================
PatternVisualizationComponent::PatternVisualizationComponent()
{
    setOpaque(true);
}

PatternVisualizationComponent::~PatternVisualizationComponent()
{
}

//==============================================================================
void PatternVisualizationComponent::paint(juce::Graphics& g)
{
    drawBackground(g);
    drawGrid(g);
    drawPitchAxis(g);
    drawTimeAxis(g);
    drawNotes(g);
    
    // Draw drag-over overlay if files are being dragged (Epic 8 Story 8.3)
    if (isDragOver)
        drawDragOverlay(g);
}

void PatternVisualizationComponent::resized()
{
    // Component resized - recalculate layout if needed
    repaint();
}

//==============================================================================
void PatternVisualizationComponent::setPattern(const MIDIPattern& pattern)
{
    currentPattern = pattern;
    repaint();
}

void PatternVisualizationComponent::clearPattern()
{
    currentPattern = MIDIPattern();
    repaint();
}

void PatternVisualizationComponent::setZoomLevel(float newZoom)
{
    zoomLevel = juce::jmax(0.1f, juce::jmin(5.0f, newZoom));
    repaint();
}

void PatternVisualizationComponent::setShowGrid(bool shouldShowGrid)
{
    showGrid = shouldShowGrid;
    repaint();
}

void PatternVisualizationComponent::setShowVelocity(bool shouldShow)
{
    showVelocity = shouldShow;
    repaint();
}

//==============================================================================
void PatternVisualizationComponent::drawBackground(juce::Graphics& g)
{
    g.fillAll(backgroundColour);
}

void PatternVisualizationComponent::drawGrid(juce::Graphics& g)
{
    if (!showGrid)
        return;
        
    g.setColour(gridColour);
    
    auto bounds = getLocalBounds();
    // Create display area by manually calculating margins
    auto displayArea = juce::Rectangle<int>(bounds.getX() + leftMargin, 
                                           bounds.getY() + topMargin,
                                           bounds.getWidth() - leftMargin - rightMargin,
                                           bounds.getHeight() - topMargin - bottomMargin);
    
    // Vertical grid lines (time)
    auto scaledSpacing = static_cast<int>(gridSpacing * zoomLevel);
    for (int x = displayArea.getX(); x < displayArea.getRight(); x += scaledSpacing)
    {
        g.drawVerticalLine(x, static_cast<float>(displayArea.getY()), static_cast<float>(displayArea.getBottom()));
    }
    
    // Horizontal grid lines (pitch)
    for (int y = displayArea.getY(); y < displayArea.getBottom(); y += noteHeight)
    {
        g.drawHorizontalLine(y, static_cast<float>(displayArea.getX()), static_cast<float>(displayArea.getRight()));
    }
}

void PatternVisualizationComponent::drawNotes(juce::Graphics& g)
{
    for (const auto& note : currentPattern.notes)
    {
        auto noteRect = getNoteRectangle(note);
        auto colour = getNoteColour(note);
        
        // Draw note rectangle with rounded corners
        g.setColour(colour);
        g.fillRoundedRectangle(noteRect.toFloat(), 2.0f);
        
        // Draw note border
        g.setColour(colour.brighter(0.3f));
        g.drawRoundedRectangle(noteRect.toFloat(), 2.0f, 1.0f);
    }
}

void PatternVisualizationComponent::drawTimeAxis(juce::Graphics& g)
{
    g.setColour(juce::Colours::lightgrey);
    g.setFont(juce::Font(juce::FontOptions(10.0f)));
    
    auto bounds = getLocalBounds();
    auto displayArea = juce::Rectangle<int>(bounds.getX() + leftMargin, 
                                           bounds.getY() + topMargin,
                                           bounds.getWidth() - leftMargin - rightMargin,
                                           bounds.getHeight() - topMargin - bottomMargin);
    
    // Draw time markers based on pattern length
    auto patternLength = currentPattern.lengthInBeats;
    if (patternLength > 0.0)
    {
        auto scaledSpacing = static_cast<int>(gridSpacing * zoomLevel);
        auto beatsPerGrid = 1.0; // 1 beat per grid line
        
        for (int x = displayArea.getX(); x < displayArea.getRight(); x += scaledSpacing)
        {
            auto beat = static_cast<int>((x - displayArea.getX()) / scaledSpacing * beatsPerGrid);
            auto beatText = juce::String(beat + 1); // 1-based beat numbers
            
            g.drawText(beatText, x - 10, bounds.getY(), 20, topMargin,
                      juce::Justification::centred, true);
        }
    }
}

void PatternVisualizationComponent::drawPitchAxis(juce::Graphics& g)
{
    g.setColour(juce::Colours::lightgrey);
    g.setFont(juce::Font(juce::FontOptions(9.0f)));
    
    auto bounds = getLocalBounds();
    auto displayArea = juce::Rectangle<int>(bounds.getX() + leftMargin, 
                                           bounds.getY() + topMargin,
                                           bounds.getWidth() - leftMargin - rightMargin,
                                           bounds.getHeight() - topMargin - bottomMargin);
    
    // Draw pitch labels (MIDI note numbers to note names)
    static const char* noteNames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    
    int startPitch = 60; // Middle C
    int numVisibleNotes = displayArea.getHeight() / noteHeight;
    
    for (int i = 0; i < numVisibleNotes; ++i)
    {
        int pitch = startPitch + numVisibleNotes - i - 1;
        int octave = pitch / 12 - 1;
        auto noteName = juce::String(noteNames[pitch % 12]) + juce::String(octave);
        
        int y = displayArea.getY() + i * noteHeight;
        g.drawText(noteName, 5, y, leftMargin - 10, noteHeight,
                  juce::Justification::centredRight, true);
    }
}

//==============================================================================
juce::Rectangle<int> PatternVisualizationComponent::getNoteRectangle(const Note& note)
{
    auto bounds = getLocalBounds();
    auto displayArea = juce::Rectangle<int>(bounds.getX() + leftMargin, 
                                           bounds.getY() + topMargin,
                                           bounds.getWidth() - leftMargin - rightMargin,
                                           bounds.getHeight() - topMargin - bottomMargin);
    
    // Calculate position based on time and pitch
    auto timeScale = displayArea.getWidth() / juce::jmax(1.0, currentPattern.lengthInBeats);
    auto x = displayArea.getX() + static_cast<int>(note.startTime * timeScale * zoomLevel);
    auto width = juce::jmax(2, static_cast<int>(note.duration * timeScale * zoomLevel));
    
    // Calculate Y position (higher pitch = lower Y)
    int startPitch = 60; // Middle C
    int numVisibleNotes = displayArea.getHeight() / noteHeight;
    int maxPitch = startPitch + numVisibleNotes - 1;
    
    auto pitchFromTop = maxPitch - note.pitch;
    auto y = displayArea.getY() + pitchFromTop * noteHeight;
    
    return juce::Rectangle<int>(x, y, width, noteHeight - 1);
}

juce::Colour PatternVisualizationComponent::getNoteColour(const Note& note)
{
    auto baseColour = noteColour;
    
    if (showVelocity)
    {
        // Adjust brightness based on velocity (0-127)
        auto brightness = note.velocity / 127.0f;
        return baseColour.withBrightness(0.3f + brightness * 0.7f);
    }
    
    return baseColour;
}

//==============================================================================
// Drag and Drop Support (Epic 8 Story 8.3)

bool PatternVisualizationComponent::isInterestedInFileDrag(const juce::StringArray& files)
{
    // Accept MIDI files
    for (const auto& filename : files)
    {
        if (juce::File(filename).hasFileExtension(".mid") || 
            juce::File(filename).hasFileExtension(".midi"))
        {
            return true;
        }
    }
    return false;
}

void PatternVisualizationComponent::fileDragEnter(const juce::StringArray& files, int x, int y)
{
    juce::ignoreUnused(files, x, y);
    isDragOver = true;
    repaint();
}

void PatternVisualizationComponent::fileDragMove(const juce::StringArray& files, int x, int y)
{
    juce::ignoreUnused(files, x, y);
    // Could update drag position indicator here if desired
}

void PatternVisualizationComponent::fileDragExit(const juce::StringArray& files)
{
    juce::ignoreUnused(files);
    isDragOver = false;
    repaint();
}

void PatternVisualizationComponent::filesDropped(const juce::StringArray& files, int x, int y)
{
    juce::ignoreUnused(x, y);
    isDragOver = false;
    
    // Load the first MIDI file found
    for (const auto& filename : files)
    {
        juce::File file(filename);
        if (file.hasFileExtension(".mid") || file.hasFileExtension(".midi"))
        {
            if (loadMIDIFile(file))
            {
                repaint();
                break; // Only load the first valid MIDI file
            }
        }
    }
}

//==============================================================================
// MIDI Import/Export Functionality

bool PatternVisualizationComponent::loadMIDIFile(const juce::File& file)
{
    juce::FileInputStream fileStream(file);
    if (!fileStream.openedOk())
        return false;
    
    juce::MidiFile midiFile;
    if (!midiFile.readFrom(fileStream))
        return false;
    
    // Convert MIDI file to our pattern format
    currentPattern = convertMidiFileToPattern(midiFile);
    return true;
}

MIDIPattern PatternVisualizationComponent::convertMidiFileToPattern(const juce::MidiFile& midiFile)
{
    MIDIPattern pattern;
    
    // Get the first track with note events
    for (int trackIndex = 0; trackIndex < midiFile.getNumTracks(); ++trackIndex)
    {
        const auto* track = midiFile.getTrack(trackIndex);
        if (track == nullptr) continue;
        
        double timeFormat = midiFile.getTimeFormat();
        if (timeFormat <= 0) timeFormat = 480; // Default PPQN
        
        std::map<int, Note> activeNotes; // pitch -> Note (for note-off matching)
        
        for (int eventIndex = 0; eventIndex < track->getNumEvents(); ++eventIndex)
        {
            const auto& event = track->getEventPointer(eventIndex);
            const auto& midiMessage = event->message;
            double timeInBeats = event->message.getTimeStamp() / timeFormat;
            
            if (midiMessage.isNoteOn())
            {
                Note note;
                note.pitch = midiMessage.getNoteNumber();
                note.velocity = midiMessage.getVelocity();
                note.startTime = timeInBeats;
                note.duration = 0.25; // Default duration, will be updated by note-off
                
                activeNotes[note.pitch] = note;
            }
            else if (midiMessage.isNoteOff())
            {
                int pitch = midiMessage.getNoteNumber();
                auto it = activeNotes.find(pitch);
                if (it != activeNotes.end())
                {
                    it->second.duration = timeInBeats - it->second.startTime;
                    pattern.notes.push_back(it->second);
                    activeNotes.erase(it);
                }
            }
        }
        
        // Add any remaining active notes (no note-off found)
        for (const auto& pair : activeNotes)
        {
            pattern.notes.push_back(pair.second);
        }
        
        // If we found notes, use this track
        if (!pattern.notes.empty())
            break;
    }
    
    // Calculate pattern length based on the last note
    pattern.lengthInBeats = 16.0; // Default
    if (!pattern.notes.empty())
    {
        double lastNoteEnd = 0.0;
        for (const auto& note : pattern.notes)
        {
            double noteEnd = note.startTime + note.duration;
            if (noteEnd > lastNoteEnd)
                lastNoteEnd = noteEnd;
        }
        pattern.lengthInBeats = std::max(4.0, std::ceil(lastNoteEnd / 4.0) * 4.0); // Round up to nearest 4 beats
    }
    
    // Set basic metadata
    pattern.id = juce::Uuid();
    pattern.metadata.tempo = 120.0f;
    pattern.metadata.key = 0; // C
    pattern.metadata.scale = GenerationParameters::ScaleType::Major;
    
    return pattern;
}

void PatternVisualizationComponent::exportCurrentPatternToFile()
{
    juce::FileChooser chooser("Save MIDI Pattern", 
                             juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),
                             "*.mid");
    
    chooser.launchAsync(juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
                       [this](const juce::FileChooser& fc)
                       {
                           auto file = fc.getResult();
                           if (file != juce::File{})
                           {
                               exportPatternToMIDI(file, currentPattern);
                           }
                       });
}

bool PatternVisualizationComponent::exportPatternToMIDI(const juce::File& outputFile, const MIDIPattern& pattern)
{
    juce::MidiFile midiFile;
    midiFile.setTicksPerQuarterNote(480);
    
    juce::MidiMessageSequence track;
    
    // Convert pattern notes to MIDI messages
    for (const auto& patternNote : pattern.notes)
    {
        double startTicks = patternNote.startTime * 480.0; // Convert beats to ticks
        double endTicks = (patternNote.startTime + patternNote.duration) * 480.0;
        
        // Note on message
        juce::MidiMessage noteOnMsg = juce::MidiMessage::noteOn(1, patternNote.pitch, static_cast<juce::uint8>(patternNote.velocity));
        noteOnMsg.setTimeStamp(startTicks);
        track.addEvent(noteOnMsg);
        
        // Note off message
        juce::MidiMessage noteOffMsg = juce::MidiMessage::noteOff(1, patternNote.pitch, static_cast<juce::uint8>(0));
        noteOffMsg.setTimeStamp(endTicks);
        track.addEvent(noteOffMsg);
    }
    
    // Sort events by timestamp
    track.sort();
    
    // Add track to MIDI file
    midiFile.addTrack(track);
    
    // Write to file
    juce::FileOutputStream fileStream(outputFile);
    if (!fileStream.openedOk())
        return false;
    
    return midiFile.writeTo(fileStream);
}

//==============================================================================
// Drawing Methods

void PatternVisualizationComponent::drawDragOverlay(juce::Graphics& g)
{
    g.setColour(dragOverColour);
    g.fillAll();
    
    // Draw drag-and-drop hint text
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(juce::FontOptions(16.0f)));
    g.drawText("Drop MIDI file here to import pattern", 
               getLocalBounds(), juce::Justification::centred, true);
}
