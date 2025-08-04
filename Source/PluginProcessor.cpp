/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SpawnCloneAudioProcessor::SpawnCloneAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       // Epic 8 Story 8.3: 6-channel output for multi-layer routing
                       .withOutput ("Output", juce::AudioChannelSet::create6point0(), true)
                     #endif
                       )
#endif
{
    parameterManager = std::make_unique<ParameterManager>(*this);
    patternManager = std::make_unique<PatternManager>();
    aiEngine = std::make_unique<AIGenerationEngine>(threadManager, *patternManager);
    
    // Epic 2 Story 2.2: Initialize audio preview engine
    audioPreviewEngine = std::make_unique<AudioPreviewEngine>();
    
    // Epic 8 Story 8.3: Initialize multi-output manager
    multiOutputManager = std::make_unique<MultiOutputManager>();
}

SpawnCloneAudioProcessor::~SpawnCloneAudioProcessor()
{
}

//==============================================================================
const juce::String SpawnCloneAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SpawnCloneAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SpawnCloneAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SpawnCloneAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SpawnCloneAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SpawnCloneAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SpawnCloneAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SpawnCloneAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SpawnCloneAudioProcessor::getProgramName (int index)
{
    return {};
}

void SpawnCloneAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SpawnCloneAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Epic 2 Story 2.2 Task 2.2.4: Prepare audio preview engine
    if (audioPreviewEngine)
    {
        audioPreviewEngine->prepareToPlay(sampleRate, samplesPerBlock);
    }
    
    // Epic 8 Story 8.3: Prepare multi-output manager
    if (multiOutputManager)
    {
        multiOutputManager->prepareToPlay(sampleRate, samplesPerBlock);
    }
}

void SpawnCloneAudioProcessor::releaseResources()
{
    // Epic 2 Story 2.2: Release audio preview engine resources
    if (audioPreviewEngine)
    {
        audioPreviewEngine->releaseResources();
    }
    
    // Epic 8 Story 8.3: Release multi-output manager resources
    if (multiOutputManager)
    {
        multiOutputManager->releaseResources();
    }
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SpawnCloneAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    // Epic 8 Story 8.3: Support 6-channel output for multi-layer routing
    auto outputChannels = layouts.getMainOutputChannelSet().size();
    
    // Support stereo (2 channels) for compatibility and 6 channels for multi-output
    if (outputChannels == 2 || outputChannels == 6)
        return true;
    
    return false;
  #else
    // For non-MIDI effects, support standard layouts plus 6-channel
    auto outputChannels = layouts.getMainOutputChannelSet().size();
    
    if (outputChannels != 2 && outputChannels != 6)
        return false;

    // This checks if the input layout matches the output layout for non-synths
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void SpawnCloneAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    // Epic 2 Story 2.1 Task 2.1.1: Update host transport info
    updateHostInfo();

    // Clear any output channels that don't contain input data
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // NEW FEATURE: Handle Instrument Mode
    if (parameterManager->getInstrumentMode())
    {
        // In Instrument Mode, route incoming MIDI through audio preview engine
        // without triggering AI generation (prevents double-triggering)
        
        // Epic 2 Story 2.2 Task 2.2.4: Process audio through preview engine
        if (audioPreviewEngine)
        {
            audioPreviewEngine->processBlock(buffer, midiMessages);
        }
    }
    else
    {
        // Normal mode - clear incoming MIDI as we generate our own patterns
        midiMessages.clear();
        
        // Epic 2 Story 2.2: Process audio preview playback
        if (audioPreviewEngine)
        {
            audioPreviewEngine->processBlock(buffer, midiMessages);
        }
    }

    // Basic audio processing placeholder - now handled by AudioPreviewEngine
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);
        // Audio processing is now handled by AudioPreviewEngine
        (void)channelData; // Suppress unused variable warning
    }
}

//==============================================================================
bool SpawnCloneAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SpawnCloneAudioProcessor::createEditor()
{
    return new SpawnCloneAudioProcessorEditor (*this);
}

//==============================================================================
void SpawnCloneAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    juce::ValueTree state("state");
    state.addChild(parameterManager->getState().copyState(), -1, nullptr);
    state.addChild(patternManager->toValueTree(), -1, nullptr);
    
    // NEW: Add PluginState serialization
    state.addChild(pluginState.toValueTree(), -1, nullptr);

    juce::MemoryOutputStream stream(destData, false);
    state.writeToStream(stream);
}

void SpawnCloneAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    auto tree = juce::ValueTree::readFromData(data, sizeInBytes);
    if (tree.isValid())
    {
        // The "parameters" child is the one managed by AudioProcessorValueTreeState
        parameterManager->getState().replaceState(tree.getChildWithName("parameters"));

        // Find our custom pattern manager data
        auto patternManagerTree = tree.getChildWithName("PatternManager");
        if (patternManagerTree.isValid())
        {
            patternManager->fromValueTree(patternManagerTree);
        }
        
        // NEW: Restore PluginState
        auto pluginStateTree = tree.getChildWithName("PluginState");
        if (pluginStateTree.isValid())
        {
            pluginState.fromValueTree(pluginStateTree);
        }
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SpawnCloneAudioProcessor();
}

//==============================================================================
// NEW FEATURE IMPLEMENTATIONS

bool SpawnCloneAudioProcessor::navigateToPreviousPattern()
{
    bool success = pluginState.navigateToPreviousPattern();
    if (success)
    {
        // Notify UI of pattern change
        sendChangeMessage();
        
        // If auto-play is enabled, start playing the pattern
        if (parameterManager->getAutoPlayOnGenerate())
        {
            // TODO: Start playback when AudioPreviewEngine is implemented
        }
    }
    return success;
}

bool SpawnCloneAudioProcessor::navigateToNextPattern()
{
    bool success = pluginState.navigateToNextPattern();
    if (success)
    {
        // Notify UI of pattern change
        sendChangeMessage();
        
        // If auto-play is enabled, start playing the pattern
        if (parameterManager->getAutoPlayOnGenerate())
        {
            // TODO: Start playback when AudioPreviewEngine is implemented
        }
    }
    return success;
}

void SpawnCloneAudioProcessor::generateNewPattern()
{
    // Get current parameters
    auto params = parameterManager->getCurrentParameters();
    
    // Epic 2 Story 2.1 Task 2.1.3: Use host tempo if available
    if (isHostTempoAvailable())
    {
        params.tempo = getHostTempo();
    }
    
    // Request AI generation using the correct method name
    aiEngine->generatePattern(params);
    
    // Note: The AI engine will callback when generation is complete
    // and the new pattern will be added to pluginState.patternHistory
    // via the existing PatternManager integration
}

//==============================================================================
// Epic 2 Story 2.1: Host DAW Communication Implementation

SpawnCloneAudioProcessor::HostTransportInfo SpawnCloneAudioProcessor::getHostTransportInfo() const
{
    return lastHostInfo;
}

void SpawnCloneAudioProcessor::updateHostInfo()
{
    auto* playHead = getPlayHead();
    
    if (playHead != nullptr)
    {
        auto posInfo = playHead->getPosition();
        
        if (posInfo.hasValue())
        {
            // Update tempo information (Task 2.1.1)
            if (posInfo->getBpm().hasValue())
            {
                lastHostInfo.tempo = *posInfo->getBpm();
                lastHostInfo.hostTempoAvailable = true;
            }
            else
            {
                lastHostInfo.hostTempoAvailable = false;
            }
            
            // Update time signature (Task 2.1.1)
            if (posInfo->getTimeSignature().hasValue())
            {
                auto timeSig = *posInfo->getTimeSignature();
                lastHostInfo.timeSigNumerator = timeSig.numerator;
                lastHostInfo.timeSigDenominator = timeSig.denominator;
                lastHostInfo.hostTimeSigAvailable = true;
            }
            else
            {
                lastHostInfo.hostTimeSigAvailable = false;
            }
            
            // Update transport state (Task 2.1.1)
            lastHostInfo.isPlaying = posInfo->getIsPlaying();
            lastHostInfo.isRecording = posInfo->getIsRecording();
            if (posInfo->getPpqPosition().hasValue())
            {
                lastHostInfo.ppqPosition = *posInfo->getPpqPosition();
            }
            
            // Task 2.2.3: Update AudioPreviewEngine with DAW transport
            if (audioPreviewEngine)
            {
                double currentTempo = lastHostInfo.hostTempoAvailable ? lastHostInfo.tempo : 120.0;
                double timeInBeats = lastHostInfo.ppqPosition;
                audioPreviewEngine->updateWithDAWTransport(currentTempo, lastHostInfo.isPlaying, timeInBeats);
            }
        }
        else
        {
            // Host doesn't provide position info
            lastHostInfo.hostTempoAvailable = false;
            lastHostInfo.hostTimeSigAvailable = false;
        }
    }
    else
    {
        // No play head available (standalone mode)
        lastHostInfo.hostTempoAvailable = false;
        lastHostInfo.hostTimeSigAvailable = false;
        lastHostInfo.isPlaying = false;
        lastHostInfo.isRecording = false;
    }
}

//==============================================================================
// Epic 2 Story 2.2: Audio Preview Engine Methods

void SpawnCloneAudioProcessor::previewCurrentPattern()
{
    if (audioPreviewEngine && patternManager)
    {
        auto currentPattern = patternManager->getCurrentPattern();
        if (currentPattern.has_value())
        {
            // Set sound type based on generation type
            auto params = parameterManager->getCurrentParameters();
            AudioPreviewEngine::SoundType soundType = AudioPreviewEngine::SoundType::Piano;
            
            switch (params.generationType)
            {
                case GenerationParameters::GenerationType::Melody:
                    soundType = AudioPreviewEngine::SoundType::Piano;
                    break;
                case GenerationParameters::GenerationType::Chords:
                    soundType = AudioPreviewEngine::SoundType::Synth;
                    break;
                case GenerationParameters::GenerationType::Bassline:
                    soundType = AudioPreviewEngine::SoundType::Bass;
                    break;
                case GenerationParameters::GenerationType::Drums:
                    soundType = AudioPreviewEngine::SoundType::Synth; // Use synth for drums
                    break;
            }
            
            audioPreviewEngine->setSoundType(soundType);
            audioPreviewEngine->loadPattern(*currentPattern);
            audioPreviewEngine->startPlayback();
        }
    }
}

void SpawnCloneAudioProcessor::stopAudioPreview()
{
    if (audioPreviewEngine)
    {
        audioPreviewEngine->stopPlayback();
    }
}
