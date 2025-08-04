/*
  ==============================================================================

    AudioPreviewEngine.cpp
    Created: 29 Jul 2025
    Author:  Epic 2 Story 2.2 Implementation

    Implementation of lightweight audio preview engine for immediate MIDI pattern playback.

  ==============================================================================
*/

#include "AudioPreviewEngine.h"
#include "InstrumentLibraryManager.h"
#include <cmath>

//==============================================================================
// AudioPreviewEngine Implementation

AudioPreviewEngine::AudioPreviewEngine()
{
    // Task 2.2.1: Initialize synthesizer
    initializeSynthesiser();
    
    // Add this as a listener to the keyboard state
    keyboardState.addListener(this);
}

AudioPreviewEngine::~AudioPreviewEngine()
{
    keyboardState.removeListener(this);
}

//==============================================================================
// Task 2.2.4: PluginProcessor Integration

void AudioPreviewEngine::prepareToPlay(double newSampleRate, int samplesPerBlock)
{
    sampleRate = newSampleRate;
    synthesiser.setCurrentPlaybackSampleRate(sampleRate);
    
    // Epic 8 Story 8.2: Prepare LayerEffectsProcessor
    layerEffects.prepareToPlay(sampleRate, samplesPerBlock, 2); // Stereo
    
    // Reset playback state
    totalSamplesProcessed = 0;
    playbackStartSample.store(0);
    currentPlaybackPosition.store(0.0);
}

void AudioPreviewEngine::releaseResources()
{
    synthesiser.allNotesOff(0, false);
    layerEffects.releaseResources();
}

void AudioPreviewEngine::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    // Clear the buffer
    buffer.clear();
    
    // Process pattern playback if playing
    if (isCurrentlyPlaying.load())
    {
        processPatternPlayback(midiMessages, buffer.getNumSamples());
    }
    
    // Let the synthesizer process the MIDI
    synthesiser.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
    
    // Epic 8 Story 8.2: Apply layer effects processing
    // For now, process the combined output through the melody layer effects
    // In a full implementation, we would separate layers by MIDI channel or note range
    if (buffer.getNumSamples() > 0)
    {
        layerEffects.processLayer(LayerEffectsProcessor::LayerType::Melody, buffer);
    }
    
    // Apply master volume
    const float volume = masterVolume.load();
    if (volume != 1.0f)
    {
        buffer.applyGain(volume);
    }
    
    // Update sample counter
    totalSamplesProcessed += buffer.getNumSamples();
}

//==============================================================================
// Task 2.2.5: Pattern Playback Control

void AudioPreviewEngine::loadPattern(const MIDIPattern& pattern)
{
    currentPattern = pattern;
    
    // Calculate pattern length in samples
    double effectiveTempo = currentPattern.metadata.tempo;
    if (dawTransportSync.load() && dawTempo.load() > 0)
    {
        effectiveTempo = dawTempo.load(); // Use DAW tempo when synced
    }
    
    if (effectiveTempo > 0)
    {
        patternLengthInSamples = (currentPattern.lengthInBeats * getSamplesPerBeat(effectiveTempo));
    }
    else
    {
        patternLengthInSamples = (currentPattern.lengthInBeats * getSamplesPerBeat(120.0)); // Default tempo
    }
}

void AudioPreviewEngine::startPlayback()
{
    if (!currentPattern.notes.empty())
    {
        // Stop any currently playing notes
        synthesiser.allNotesOff(0, false);
        
        // Reset playback position
        playbackStartSample.store(totalSamplesProcessed);
        currentPlaybackPosition.store(0.0);
        
        // Start playback
        isCurrentlyPlaying.store(true);
    }
}

void AudioPreviewEngine::stopPlayback()
{
    isCurrentlyPlaying.store(false);
    synthesiser.allNotesOff(0, false);
    currentPlaybackPosition.store(0.0);
}

double AudioPreviewEngine::getPlaybackPosition() const
{
    if (!isCurrentlyPlaying.load() || patternLengthInSamples <= 0)
        return 0.0;
    
    auto startSample = playbackStartSample.load();
    auto currentSample = totalSamplesProcessed;
    auto elapsedSamples = currentSample - startSample;
    
    double position = static_cast<double>(elapsedSamples) / patternLengthInSamples;
    
    if (looping.load())
    {
        position = std::fmod(position, 1.0);
    }
    else if (position >= 1.0)
    {
        // Pattern finished, stop playback
        const_cast<AudioPreviewEngine*>(this)->stopPlayback();
        return 1.0;
    }
    
    return position;
}

//==============================================================================
// Task 2.2.3: Sound Selection

void AudioPreviewEngine::setSoundType(SoundType type)
{
    if (currentSoundType != type)
    {
        currentSoundType = type;
        updateSynthesiserSounds();
    }
}

//==============================================================================
// Epic 4 Story 4.1: Enhanced tempo control

void AudioPreviewEngine::setTempo(double bpm)
{
    // Clamp tempo to reasonable range
    bpm = juce::jlimit(60.0, 200.0, bpm);
    currentTempo.store(bpm);
    
    // Update pattern timing calculations if a pattern is loaded
    if (!currentPattern.isEmpty())
    {
        // Recalculate timing based on new tempo
        patternLengthInSamples = (currentPattern.lengthInBeats * 60.0 / bpm) * sampleRate;
    }
}

//==============================================================================
// Task 2.2.1: Synthesizer Management

void AudioPreviewEngine::initializeSynthesiser()
{
    // Add voices for each sound type
    const int numVoices = 16; // Polyphony of 16 voices
    
    for (int i = 0; i < numVoices; ++i)
    {
        synthesiser.addVoice(new PreviewSynthVoice(SoundType::Piano));
    }
    
    // Initialize with piano sound
    updateSynthesiserSounds();
}

void AudioPreviewEngine::updateSynthesiserSounds()
{
    synthesiser.clearSounds();
    
    switch (currentSoundType)
    {
        case SoundType::Piano:
            createPianoSound();
            break;
        case SoundType::Synth:
            createSynthSound();
            break;
        case SoundType::Bass:
            createBassSound();
            break;
        case SoundType::Bell:
        case SoundType::Brass:
        case SoundType::Guitar:
        case SoundType::Keys:
        case SoundType::Mallet:
        case SoundType::Organ:
        case SoundType::Pluck:
        case SoundType::Strings:
            // For now, use Piano sound as fallback for new instrument types
            // Will be expanded with specific sound implementations in Task 9.1.4
            createPianoSound();
            break;
    }
}

void AudioPreviewEngine::createPianoSound()
{
    synthesiser.addSound(new PreviewSynthSound(SoundType::Piano));
}

void AudioPreviewEngine::createSynthSound()
{
    synthesiser.addSound(new PreviewSynthSound(SoundType::Synth));
}

void AudioPreviewEngine::createBassSound()
{
    synthesiser.addSound(new PreviewSynthSound(SoundType::Bass));
}

//==============================================================================
// Pattern Playback Processing

void AudioPreviewEngine::processPatternPlayback(juce::MidiBuffer& midiBuffer, int numSamples)
{
    if (currentPattern.notes.empty() || patternLengthInSamples <= 0)
        return;
    
    auto startSample = playbackStartSample.load();
    auto currentSample = totalSamplesProcessed;
    auto elapsedSamples = currentSample - startSample;
    
    // Calculate time positions in beats
    double effectiveTempo = currentPattern.metadata.tempo;
    if (dawTransportSync.load() && dawTempo.load() > 0)
    {
        effectiveTempo = dawTempo.load(); // Use DAW tempo when synced
    }
    else if (effectiveTempo <= 0)
    {
        effectiveTempo = 120.0; // Default tempo
    }
    
    double samplesPerBeat = getSamplesPerBeat(effectiveTempo);
    double startTimeBeats = static_cast<double>(elapsedSamples) / samplesPerBeat;
    double endTimeBeats = static_cast<double>(elapsedSamples + numSamples) / samplesPerBeat;
    
    // Handle looping
    if (looping.load())
    {
        double patternLength = currentPattern.lengthInBeats;
        startTimeBeats = std::fmod(startTimeBeats, patternLength);
        endTimeBeats = std::fmod(endTimeBeats, patternLength);
        
        if (endTimeBeats < startTimeBeats) // Wrapped around
        {
            // Process from startTime to end of pattern
            generateMidiEventsForPosition(midiBuffer, startTimeBeats, patternLength, numSamples);
            // Process from start of pattern to endTime
            generateMidiEventsForPosition(midiBuffer, 0.0, endTimeBeats, numSamples);
        }
        else
        {
            generateMidiEventsForPosition(midiBuffer, startTimeBeats, endTimeBeats, numSamples);
        }
    }
    else
    {
        // Non-looping playback
        if (startTimeBeats < currentPattern.lengthInBeats)
        {
            generateMidiEventsForPosition(midiBuffer, startTimeBeats, endTimeBeats, numSamples);
        }
        else
        {
            // Pattern finished
            isCurrentlyPlaying.store(false);
            synthesiser.allNotesOff(0, false);
        }
    }
    
    // Update position
    double position = startTimeBeats / currentPattern.lengthInBeats;
    if (looping.load())
    {
        position = std::fmod(position, 1.0);
    }
    currentPlaybackPosition.store(position);
}

void AudioPreviewEngine::generateMidiEventsForPosition(juce::MidiBuffer& midiBuffer, double startTime, double endTime, int bufferSize)
{
    // Get effective tempo
    double effectiveTempo = currentPattern.metadata.tempo;
    if (dawTransportSync.load() && dawTempo.load() > 0)
    {
        effectiveTempo = dawTempo.load(); // Use DAW tempo when synced
    }
    else if (effectiveTempo <= 0)
    {
        effectiveTempo = 120.0; // Default tempo
    }
    
    double samplesPerBeat = getSamplesPerBeat(effectiveTempo);
    for (const auto& note : currentPattern.notes)
    {
        // Check if note starts within this time window
        if (note.startTime >= startTime && note.startTime < endTime)
        {
            // Calculate sample position within buffer
            double relativeTime = note.startTime - startTime;
            int sampleOffset = static_cast<int>(relativeTime * samplesPerBeat);
            sampleOffset = juce::jlimit(0, bufferSize - 1, sampleOffset);
            
            // Add note on event
            auto noteOnMsg = juce::MidiMessage::noteOn(1, note.pitch, static_cast<float>(note.velocity) / 127.0f);
            midiBuffer.addEvent(noteOnMsg, sampleOffset);
        }
        
        // Check if note ends within this time window
        double noteEndTime = note.startTime + note.duration;
        if (noteEndTime >= startTime && noteEndTime < endTime)
        {
            // Calculate sample position within buffer
            double relativeTime = noteEndTime - startTime;
            int sampleOffset = static_cast<int>(relativeTime * samplesPerBeat);
            sampleOffset = juce::jlimit(0, bufferSize - 1, sampleOffset);
            
            // Add note off event
            auto noteOffMsg = juce::MidiMessage::noteOff(1, note.pitch, 0.0f);
            midiBuffer.addEvent(noteOffMsg, sampleOffset);
        }
    }
}

double AudioPreviewEngine::getSamplesPerBeat(double tempo) const
{
    // 60 seconds per minute / tempo = seconds per beat
    // seconds per beat * sample rate = samples per beat
    return (60.0 / tempo) * sampleRate;
}

//==============================================================================
// Task 2.2.3: DAW Transport Integration

void AudioPreviewEngine::updateWithDAWTransport(double currentTempo, bool isPlaying, double timeInBeats)
{
    dawTempo.store(currentTempo);
    dawIsPlaying.store(isPlaying);
    dawTimeInBeats = timeInBeats;
    
    // If DAW transport sync is enabled, sync playback state
    if (dawTransportSync.load())
    {
        if (isPlaying && !isCurrentlyPlaying.load())
        {
            // DAW started playing, start our playback
            startPlayback();
        }
        else if (!isPlaying && isCurrentlyPlaying.load())
        {
            // DAW stopped playing, stop our playback
            stopPlayback();
        }
    }
}

//==============================================================================
// MidiKeyboardStateListener implementation

void AudioPreviewEngine::handleNoteOn(juce::MidiKeyboardState* source, int midiChannel, int midiNoteNumber, float velocity)
{
    auto msg = juce::MidiMessage::noteOn(midiChannel, midiNoteNumber, velocity);
    juce::MidiBuffer buffer;
    buffer.addEvent(msg, 0);
    
    // Create temporary audio buffer for synthesizer
    juce::AudioBuffer<float> tempBuffer(2, 1);
    tempBuffer.clear();
    synthesiser.renderNextBlock(tempBuffer, buffer, 0, 1);
}

void AudioPreviewEngine::handleNoteOff(juce::MidiKeyboardState* source, int midiChannel, int midiNoteNumber, float velocity)
{
    auto msg = juce::MidiMessage::noteOff(midiChannel, midiNoteNumber, velocity);
    juce::MidiBuffer buffer;
    buffer.addEvent(msg, 0);
    
    // Create temporary audio buffer for synthesizer
    juce::AudioBuffer<float> tempBuffer(2, 1);
    tempBuffer.clear();
    synthesiser.renderNextBlock(tempBuffer, buffer, 0, 1);
}

//==============================================================================
// PreviewSynthVoice Implementation

PreviewSynthVoice::PreviewSynthVoice(AudioPreviewEngine::SoundType type) : soundType(type)
{
}

bool PreviewSynthVoice::canPlaySound(juce::SynthesiserSound* sound)
{
    return dynamic_cast<const PreviewSynthSound*>(sound) != nullptr;
}

void PreviewSynthVoice::startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound* sound, int currentPitchWheelPosition)
{
    auto* previewSound = dynamic_cast<const PreviewSynthSound*>(sound);
    if (previewSound == nullptr) return;
    
    // Calculate frequency
    auto frequency = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
    auto cyclesPerSample = frequency / getSampleRate();
    angleDelta = cyclesPerSample * 2.0 * juce::MathConstants<double>::pi;
    
    // Set level based on velocity
    level = velocity * 0.15; // Scale down for reasonable volume
    
    // Adjust sound characteristics based on type
    switch (previewSound->getSoundType())
    {
        case AudioPreviewEngine::SoundType::Piano:
        case AudioPreviewEngine::SoundType::Keys:
            level *= 0.8; // Slightly softer for piano/keys
            break;
        case AudioPreviewEngine::SoundType::Synth:
            level *= 1.0; // Normal level for synth
            break;
        case AudioPreviewEngine::SoundType::Bass:
            level *= 1.2; // Slightly louder for bass
            if (midiNoteNumber < 60) // Below middle C, enhance for bass
            {
                level *= 1.3;
            }
            break;
        case AudioPreviewEngine::SoundType::Bell:
        case AudioPreviewEngine::SoundType::Mallet:
            level *= 0.9; // Slightly softer for bell/mallet sounds
            break;
        case AudioPreviewEngine::SoundType::Brass:
            level *= 1.1; // Slightly louder for brass
            break;
        case AudioPreviewEngine::SoundType::Guitar:
        case AudioPreviewEngine::SoundType::Pluck:
            level *= 0.85; // Moderate level for plucked instruments
            break;
        case AudioPreviewEngine::SoundType::Organ:
            level *= 0.95; // Slightly softer for organ
            break;
        case AudioPreviewEngine::SoundType::Strings:
            level *= 0.75; // Softer for strings
            break;
    }
    
    tailOff = 0.0;
}

void PreviewSynthVoice::stopNote(float velocity, bool allowTailOff)
{
    if (allowTailOff)
    {
        if (tailOff == 0.0)
            tailOff = 1.0;
    }
    else
    {
        clearCurrentNote();
        angleDelta = 0.0;
    }
}

void PreviewSynthVoice::pitchWheelMoved(int newValue)
{
    // Not implemented for this simple synth
}

void PreviewSynthVoice::controllerMoved(int controllerNumber, int newValue)
{
    // Not implemented for this simple synth
}

void PreviewSynthVoice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
    if (angleDelta != 0.0)
    {
        if (tailOff > 0.0)
        {
            while (--numSamples >= 0)
            {
                auto currentSample = generateSample() * level * tailOff;
                
                for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
                    outputBuffer.addSample(i, startSample, currentSample);
                
                currentAngle += angleDelta;
                ++startSample;
                
                tailOff *= 0.99; // Exponential decay
                
                if (tailOff <= 0.005)
                {
                    clearCurrentNote();
                    angleDelta = 0.0;
                    break;
                }
            }
        }
        else
        {
            while (--numSamples >= 0)
            {
                auto currentSample = generateSample() * level;
                
                for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
                    outputBuffer.addSample(i, startSample, currentSample);
                
                currentAngle += angleDelta;
                ++startSample;
            }
        }
    }
}

float PreviewSynthVoice::generateSample()
{
    // Generate different waveforms based on sound type
    switch (soundType)
    {
        case AudioPreviewEngine::SoundType::Piano:
            // Piano-like sound: sine wave with some harmonics
            return static_cast<float>(std::sin(currentAngle) * 0.8 + std::sin(currentAngle * 2.0) * 0.1 + std::sin(currentAngle * 3.0) * 0.05);
            
        case AudioPreviewEngine::SoundType::Synth:
            // Synth sound: sawtooth wave
            return static_cast<float>(2.0 * (currentAngle / (2.0 * juce::MathConstants<double>::pi) - std::floor(currentAngle / (2.0 * juce::MathConstants<double>::pi) + 0.5)));
            
        case AudioPreviewEngine::SoundType::Bass:
            // Bass sound: sine wave with sub-harmonic
            return static_cast<float>(std::sin(currentAngle) * 0.7 + std::sin(currentAngle * 0.5) * 0.3);
            
        default:
            return static_cast<float>(std::sin(currentAngle));
    }
}

//==============================================================================
// Epic 9 Story 9.1: Instrument Library Integration

void AudioPreviewEngine::setInstrumentLibraryManager(InstrumentLibraryManager* manager)
{
    instrumentLibrary = manager;
    
    // Auto-load a default preset if available
    if (instrumentLibrary != nullptr)
    {
        auto presets = instrumentLibrary->getPresetsForCategory("Piano");
        if (!presets.isEmpty())
        {
            loadInstrumentPreset(presets.getFirst());
        }
    }
}

void AudioPreviewEngine::loadInstrumentPreset(const InstrumentLibraryManager::PresetData& preset)
{
    currentPreset = preset;
    currentPresetId = preset.presetId;
    
    // Update sound type based on preset category
    if (preset.category == "Piano") setSoundType(SoundType::Piano);
    else if (preset.category == "Synth") setSoundType(SoundType::Synth);
    else if (preset.category == "Bass") setSoundType(SoundType::Bass);
    else if (preset.category == "Bell") setSoundType(SoundType::Bell);
    else if (preset.category == "Brass") setSoundType(SoundType::Brass);
    else if (preset.category == "Guitar") setSoundType(SoundType::Guitar);
    else if (preset.category == "Keys") setSoundType(SoundType::Keys);
    else if (preset.category == "Mallet") setSoundType(SoundType::Mallet);
    else if (preset.category == "Organ") setSoundType(SoundType::Organ);
    else if (preset.category == "Pluck") setSoundType(SoundType::Pluck);
    else if (preset.category == "Strings") setSoundType(SoundType::Strings);
    
    // Apply synthesis parameters from preset (basic implementation for now)
    // Master volume is handled via existing mechanism, other params will be expanded
    
    DBG("Loaded instrument preset: " + preset.name + " (" + preset.category + ")");
}

void AudioPreviewEngine::autoSelectPreset(GenerationParameters::GenerationType type, 
                                         const juce::StringArray& styleTags)
{
    if (instrumentLibrary == nullptr)
        return;
        
    // Map generation type to instrument category
    juce::String category;
    switch (type)
    {
        case GenerationParameters::GenerationType::Melody:
            category = "Piano";
            break;
        case GenerationParameters::GenerationType::Chords:
            category = "Keys";
            break;
        case GenerationParameters::GenerationType::Bassline:
            category = "Bass";
            break;
        case GenerationParameters::GenerationType::Drums:
            category = "Synth"; // Fallback for drums
            break;
        default:
            category = "Piano";
            break;
    }
    
    // Get presets for category
    auto presets = instrumentLibrary->getPresetsForCategory(category);
    if (presets.isEmpty())
        return;
        
    // Try to find preset matching style tags
    InstrumentLibraryManager::PresetData selectedPreset = presets.getFirst();
    
    if (!styleTags.isEmpty())
    {
        for (const auto& preset : presets)
        {
            for (const auto& tag : styleTags)
            {
                if (preset.tags.contains(tag))
                {
                    selectedPreset = preset;
                    break;
                }
            }
        }
    }
    
    // Load the selected preset
    loadInstrumentPreset(selectedPreset);
}
