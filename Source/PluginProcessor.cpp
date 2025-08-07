/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ui/WaveformDisplayComponent.h"  // Epic 9.2 Story 9.1: For real-time audio visualization
#include "ai/Epic7CompatibilityLayer.h"  // Epic 7 Integration: Safe operations

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
    
    // PHASE 1A: Enable Advanced Synthesis Engine for professional audio quality
    audioPreviewEngine->setAdvancedSynthesisEnabled(true);
    
    // Epic 8 Story 8.3: Initialize multi-output manager
    multiOutputManager = std::make_unique<MultiOutputManager>();
    
    // Epic 9 Story 9.1: Initialize instrument library manager
    instrumentLibraryManager = std::make_unique<InstrumentLibraryManager>();
    
    // Initialize Live Performance System Integration
    livePerformanceIntegration = std::make_unique<spawnclone::processor::LivePerformanceIntegration>();
    
    // Initialize live performance with required components
    // Note: The ONNX client will be initialized when live performance is first enabled
    if (livePerformanceIntegration)
    {
        // Initialize with audio preview engine
        // ONNX client will be set up on-demand when live performance features are used
        livePerformanceIntegration->initialize(audioPreviewEngine.get(), nullptr);
        DBG("Live Performance Integration initialized (ONNX client will be set on-demand)");
    }
}

SpawnCloneAudioProcessor::~SpawnCloneAudioProcessor()
{
    // Epic 7 integration safe destruction
    try
    {
        // Any cleanup code should go here
    }
    catch (const std::exception& e)
    {
        juce::Logger::writeToLog("FATAL: Exception in ~SpawnCloneAudioProcessor(): " + juce::String(e.what()));
    }
    catch (...)
    {
        juce::Logger::writeToLog("FATAL: Unknown exception in ~SpawnCloneAudioProcessor()");
    }
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
    // Epic 2 Story 2.2: Prepare audio preview engine
    if (audioPreviewEngine)
    {
        audioPreviewEngine->prepareToPlay(sampleRate, samplesPerBlock);
    }
    
    // Epic 8 Story 8.3: Prepare multi-output manager
    if (multiOutputManager)
    {
        multiOutputManager->prepareToPlay(sampleRate, samplesPerBlock);
    }
    
    // Prepare Live Performance Integration
    if (livePerformanceIntegration)
    {
        livePerformanceIntegration->prepareToPlay(sampleRate, samplesPerBlock);
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
    
    // Release Live Performance Integration resources
    if (livePerformanceIntegration)
    {
        livePerformanceIntegration->releaseResources();
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
    // Epic 7 integration comprehensive safety wrapper
    try
    {
        // Validate buffer before any operations
        if (!Epic7CompatibilityLayer::validateAudioParametersReadOnly(getSampleRate(), buffer.getNumSamples(), buffer.getNumChannels()))
        {
            buffer.clear();
            return;
        }
        
        juce::ScopedNoDenormals noDenormals;
        auto totalNumInputChannels  = getTotalNumInputChannels();
        auto totalNumOutputChannels = getTotalNumOutputChannels();
        
        // Epic 2 Story 2.1 Task 2.1.1: Update host transport info
        try
        {
            updateHostInfo();
        }
        catch (...)
        {
            // Host info update failed, continue processing
        }
        
        // Live Performance Integration - Process MIDI triggers and update performance state
        try
        {
            if (livePerformanceIntegration && livePerformanceIntegration->isLivePerformanceEnabled())
            {
                auto transportInfo = getHostTransportInfo();
                livePerformanceIntegration->processMIDI(midiMessages, transportInfo.ppqPosition, transportInfo.isPlaying);
                livePerformanceIntegration->updatePerformanceState(transportInfo.ppqPosition, transportInfo.isPlaying);
            }
        }
        catch (...)
        {
            // Live performance processing failed, continue
        }

        // Clear any output channels that don't contain input data
        for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        {
            if (i < buffer.getNumChannels())
                buffer.clear (i, 0, buffer.getNumSamples());
        }

        // NEW FEATURE: Handle Instrument Mode
        try
        {
            if (parameterManager && parameterManager->getInstrumentMode())
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
        }
        catch (...)
        {
            // Audio processing failed - clear buffer for safety
            buffer.clear();
        }

        // Basic audio processing placeholder - now handled by AudioPreviewEngine
        for (int channel = 0; channel < totalNumInputChannels; ++channel)
        {
            auto* channelData = buffer.getWritePointer (channel);
            // Audio processing is now handled by AudioPreviewEngine
            (void)channelData; // Suppress unused variable warning
        }
        
        // Epic 9.2 Story 9.1: Send audio data to waveform display for real-time visualization
        try
        {
            if (waveformDisplay != nullptr)
            {
                waveformDisplay->pushAudioData(buffer);
            }
        }
        catch (...)
        {
            // Waveform display update failed, continue
        }
    }
    catch (const std::exception& e)
    {
        // Standard exception - clear buffer
        buffer.clear();
    }
    catch (...)
    {
        // Unknown exception - clear buffer
        buffer.clear();
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
    try 
    {
        juce::ValueTree state = Epic7CompatibilityLayer::safeCreateValueTree("state");
        
        // Safe ValueTree operations with Epic 7 compatibility
        Epic7CompatibilityLayer::safeValueTreeOperation("getParameterState", [&]() {
            if (parameterManager)
            {
                auto paramState = parameterManager->getState().copyState();
                if (paramState.isValid())
                    state.addChild(paramState, -1, nullptr);
            }
        });
        
        Epic7CompatibilityLayer::safeValueTreeOperation("getPatternManagerState", [&]() {
            if (patternManager)
            {
                auto patternState = patternManager->toValueTree();
                if (patternState.isValid())
                    state.addChild(patternState, -1, nullptr);
            }
        });
        
        Epic7CompatibilityLayer::safeValueTreeOperation("getPluginState", [&]() {
            auto pluginStateTree = pluginState.toValueTree();
            if (pluginStateTree.isValid())
                state.addChild(pluginStateTree, -1, nullptr);
        });

        if (state.isValid())
        {
            juce::MemoryOutputStream stream(destData, false);
            state.writeToStream(stream);
        }
    }
    catch (...)
    {
        juce::Logger::writeToLog("SpawnCloneAudioProcessor: Failed to save state information");
    }
}

void SpawnCloneAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    try
    {
        if (data == nullptr || sizeInBytes <= 0)
        {
            juce::Logger::writeToLog("SpawnCloneAudioProcessor: Invalid state data");
            return;
        }
        
        auto tree = juce::ValueTree::readFromData(data, sizeInBytes);
        if (tree.isValid())
        {
            // The "parameters" child is the one managed by AudioProcessorValueTreeState
            Epic7CompatibilityLayer::safeValueTreeOperation("setParameterState", [&]() {
                if (parameterManager)
                {
                    auto paramTree = tree.getChildWithName("parameters");
                    if (paramTree.isValid())
                        parameterManager->getState().replaceState(paramTree);
                }
            });

            // Find our custom pattern manager data
            Epic7CompatibilityLayer::safeValueTreeOperation("setPatternManagerState", [&]() {
                if (patternManager)
                {
                    auto patternManagerTree = tree.getChildWithName("PatternManager");
                    if (patternManagerTree.isValid())
                        patternManager->fromValueTree(patternManagerTree);
                }
            });
            
            // NEW: Restore PluginState
            Epic7CompatibilityLayer::safeValueTreeOperation("setPluginState", [&]() {
                auto pluginStateTree = tree.getChildWithName("PluginState");
                if (pluginStateTree.isValid())
                    pluginState.fromValueTree(pluginStateTree);
            });
        }
        else
        {
            juce::Logger::writeToLog("SpawnCloneAudioProcessor: Invalid ValueTree from state data");
        }
    }
    catch (...)
    {
        juce::Logger::writeToLog("SpawnCloneAudioProcessor: Failed to restore state information");
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

//==============================================================================
// Synthesis Parameter Control (Epic 4)

void SpawnCloneAudioProcessor::updateSynthesisParameter(const juce::String& paramName, float value, bool interpolate)
{
    if (audioPreviewEngine)
    {
        audioPreviewEngine->updateSynthesisParameter(paramName, value, interpolate);
    }
}

void SpawnCloneAudioProcessor::batchUpdateSynthesisParameters(const juce::StringPairArray& newParams)
{
    if (audioPreviewEngine)
    {
        audioPreviewEngine->batchUpdateSynthesisParameters(newParams);
    }
}

void SpawnCloneAudioProcessor::applyAIGenerationParameters(const GenerationParameters& aiParams)
{
    if (audioPreviewEngine)
    {
        audioPreviewEngine->applyAIGenerationParameters(aiParams);
    }
}

float SpawnCloneAudioProcessor::getSynthesisParameterValue(const juce::String& paramName) const
{
    if (audioPreviewEngine)
    {
        return audioPreviewEngine->getSynthesisParameterValue(paramName);
    }
    return 0.0f;
}

//==============================================================================
// Live Performance System Integration

void SpawnCloneAudioProcessor::setLivePerformanceEnabled(bool enabled)
{
    if (livePerformanceIntegration)
    {
        if (enabled && aiEngine)
        {
            // Set up ONNX client for live performance when enabling
            auto* onnxManager = aiEngine->getONNXModelManager();
            if (onnxManager)
            {
                auto daemonClient = onnxManager->getDaemonClient();
                livePerformanceIntegration->initialize(audioPreviewEngine.get(), daemonClient);
            }
        }
        
        livePerformanceIntegration->setLivePerformanceEnabled(enabled);
        DBG("Live Performance " + juce::String(enabled ? "ENABLED" : "DISABLED"));
    }
}

bool SpawnCloneAudioProcessor::isLivePerformanceEnabled() const
{
    return livePerformanceIntegration && livePerformanceIntegration->isLivePerformanceEnabled();
}

void SpawnCloneAudioProcessor::setLivePerformanceMode(spawnclone::ai::LivePerformanceEngine::PerformanceMode mode)
{
    if (livePerformanceIntegration)
    {
        livePerformanceIntegration->setPerformanceMode(mode);
    }
}

void SpawnCloneAudioProcessor::configureLiveAIModulation(bool enabled, float intensity)
{
    if (livePerformanceIntegration)
    {
        livePerformanceIntegration->configureAIModulation(enabled, intensity);
    }
}

void SpawnCloneAudioProcessor::triggerLivePatternGeneration()
{
    if (livePerformanceIntegration)
    {
        livePerformanceIntegration->triggerPatternGeneration();
    }
}

