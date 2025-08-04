/*
  ==============================================================================

    AudioPreviewEngine.h
    Created: 29 Jul 2025
    Author:  Epic 2 Story 2.2 Implementation

    Lightweight audio preview engine for immediate MIDI pattern playback.
    Provides built-in sounds for quick evaluation of generated patterns.

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_core/juce_core.h>
#include "MIDIPattern.h"
#include "GenerationParameters.h"
#include "LayerEffectsProcessor.h"

//==============================================================================
/**
 * Epic 2 Story 2.2: Lightweight Audio Preview Engine
 * 
 * Provides immediate audio feedback for generated MIDI patterns using
 * built-in synthesizer sounds (piano, synth, bass).
 */
class AudioPreviewEngine : public juce::MidiKeyboardStateListener
{
public:
    //==============================================================================
    AudioPreviewEngine();
    ~AudioPreviewEngine() override;
    
    //==============================================================================
    // Task 2.2.4: Integration with PluginProcessor
    
    /** Prepare for playback */
    void prepareToPlay(double sampleRate, int samplesPerBlock);
    
    /** Release audio resources */
    void releaseResources();
    
    /** Process audio block with MIDI pattern playback */
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages);
    
    //==============================================================================
    // Task 2.2.3: DAW Transport Integration
    
    /** Update with DAW transport information */
    void updateWithDAWTransport(double currentTempo, bool isPlaying, double timeInBeats);
    
    /** Sync playback with DAW transport */
    void setDAWTransportSync(bool shouldSync) { dawTransportSync.store(shouldSync); }
    bool isDAWTransportSyncEnabled() const { return dawTransportSync.load(); }
    
    //==============================================================================
    // Task 2.2.5: Pattern Playback Control
    
    /** Load a pattern for preview playback */
    void loadPattern(const MIDIPattern& pattern);
    
    /** Start pattern playback */
    void startPlayback();
    
    /** Stop pattern playback */
    void stopPlayback();
    
    /** Check if currently playing */
    bool isPlaying() const { return isCurrentlyPlaying.load(); }
    
    /** Set loop mode */
    void setLooping(bool shouldLoop) { looping.store(shouldLoop); }
    bool isLooping() const { return looping.load(); }
    
    /** Get current playback position (0.0 to 1.0) */
    double getPlaybackPosition() const;
    
    //==============================================================================
    // Epic 4 Story 4.1: Enhanced tempo control
    
    /** Set playback tempo in BPM */
    void setTempo(double bpm);
    
    /** Get current tempo in BPM */
    double getTempo() const { return currentTempo.load(); }

    //==============================================================================
    // Task 2.2.3: Sound Selection
    
    enum class SoundType
    {
        Piano = 0,
        Synth = 1,
        Bass = 2
    };
    
    /** Set the current sound type */
    void setSoundType(SoundType type);
    SoundType getCurrentSoundType() const { return currentSoundType; }
    
    /** Set master volume for preview playback */
    void setMasterVolume(float volume) { masterVolume.store(volume); }
    float getMasterVolume() const { return masterVolume.load(); }
    
    //==============================================================================
    // Epic 8 Story 8.2: ExperimentPad Interface
    
    /** Set global pitch shift from ExperimentPad Y-axis (-12 to +12 semitones) */
    void setGlobalPitchShift(float semitones) { layerEffects.setGlobalPitchShift(semitones); }
    
    /** Set filter cutoff for effects morphing from ExperimentPad X-axis */
    void setFilterCutoff(float cutoff) { 
        layerEffects.setFilterCutoff(LayerEffectsProcessor::LayerType::Melody, cutoff);
        layerEffects.setFilterCutoff(LayerEffectsProcessor::LayerType::Chords, cutoff * 0.7f);
        layerEffects.setFilterCutoff(LayerEffectsProcessor::LayerType::Bass, cutoff * 0.5f);
    }
    
    /** Set reverb mix for effects morphing */
    void setReverbMix(float mix) {
        layerEffects.setReverbMix(LayerEffectsProcessor::LayerType::Melody, mix);
        layerEffects.setReverbMix(LayerEffectsProcessor::LayerType::Chords, mix * 1.2f);
        layerEffects.setReverbMix(LayerEffectsProcessor::LayerType::Bass, mix * 0.3f);
    }
    
    /** Set delay feedback for effects morphing */
    void setDelayFeedback(float feedback) {
        layerEffects.setDelayFeedback(LayerEffectsProcessor::LayerType::Melody, feedback);
        layerEffects.setDelayFeedback(LayerEffectsProcessor::LayerType::Chords, feedback * 0.8f);
        layerEffects.setDelayFeedback(LayerEffectsProcessor::LayerType::Bass, feedback * 0.2f);
    }
    
    /** Get LayerEffectsProcessor for advanced control */
    LayerEffectsProcessor& getLayerEffects() { return layerEffects; }
    
    //==============================================================================
    // Task 2.2.1: Synthesizer Management
    
    /** Get the internal synthesizer for advanced control */
    juce::Synthesiser& getSynthesiser() { return synthesiser; }
    
    // MidiKeyboardStateListener implementation
    void handleNoteOn(juce::MidiKeyboardState* source, int midiChannel, int midiNoteNumber, float velocity) override;
    void handleNoteOff(juce::MidiKeyboardState* source, int midiChannel, int midiNoteNumber, float velocity) override;

private:
    //==============================================================================
    // Audio Components
    juce::Synthesiser synthesiser;
    juce::MidiKeyboardState keyboardState;
    
    // Playback State
    std::atomic<bool> isCurrentlyPlaying{false};
    std::atomic<bool> looping{true};
    std::atomic<double> currentPlaybackPosition{0.0};
    
    // DAW Transport Integration
    std::atomic<bool> dawTransportSync{false};
    std::atomic<double> dawTempo{120.0};
    std::atomic<bool> dawIsPlaying{false};
    double dawTimeInBeats = 0.0;
    
    // Epic 4 Story 4.1: Enhanced tempo control
    std::atomic<double> currentTempo{120.0};
    
    // Pattern Data
    MIDIPattern currentPattern;
    double sampleRate = 44100.0;
    double patternStartTime = 0.0;
    double patternLengthInSamples = 0.0;
    
    // Sound Management
    SoundType currentSoundType = SoundType::Piano;
    
    // Master Volume Control
    std::atomic<float> masterVolume{0.7f}; // Default to 70% volume
    
    // Epic 8 Story 8.2: Per-Layer Effects Processing
    LayerEffectsProcessor layerEffects;
    
    // Timing
    std::atomic<int64_t> playbackStartSample{0};
    int64_t totalSamplesProcessed = 0;
    
    //==============================================================================
    // Private Methods
    
    /** Initialize the synthesizer with sounds (Task 2.2.2) */
    void initializeSynthesiser();
    
    /** Create piano sound */
    void createPianoSound();
    
    /** Create synth sound */
    void createSynthSound();
    
    /** Create bass sound */
    void createBassSound();
    
    /** Update synthesizer with current sound type */
    void updateSynthesiserSounds();
    
    /** Process pattern playback for current block */
    void processPatternPlayback(juce::MidiBuffer& midiBuffer, int numSamples);
    
    /** Convert pattern notes to MIDI events for current playback position */
    void generateMidiEventsForPosition(juce::MidiBuffer& midiBuffer, double startTime, double endTime, int bufferSize);
    
    /** Calculate samples per beat based on tempo */
    double getSamplesPerBeat(double tempo) const;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPreviewEngine)
};

//==============================================================================
/**
 * Custom synthesizer voice for the audio preview engine
 */
class PreviewSynthVoice : public juce::SynthesiserVoice
{
public:
    PreviewSynthVoice(AudioPreviewEngine::SoundType type);
    
    bool canPlaySound(juce::SynthesiserSound* sound) override;
    void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound* sound, int currentPitchWheelPosition) override;
    void stopNote(float velocity, bool allowTailOff) override;
    void pitchWheelMoved(int newValue) override;
    void controllerMoved(int controllerNumber, int newValue) override;
    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override;

private:
    AudioPreviewEngine::SoundType soundType;
    double currentAngle = 0.0;
    double angleDelta = 0.0;
    double level = 0.0;
    double tailOff = 0.0;
    
    float generateSample(); // Add this missing declaration
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PreviewSynthVoice)
};

//==============================================================================
/**
 * Custom synthesizer sound for the audio preview engine
 */
class PreviewSynthSound : public juce::SynthesiserSound
{
public:
    PreviewSynthSound(AudioPreviewEngine::SoundType type) : soundType(type) {}
    
    bool appliesToNote(int midiNoteNumber) override { return true; }
    bool appliesToChannel(int midiChannel) override { return true; }
    
    AudioPreviewEngine::SoundType getSoundType() const { return soundType; }

private:
    AudioPreviewEngine::SoundType soundType;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PreviewSynthSound)
};
