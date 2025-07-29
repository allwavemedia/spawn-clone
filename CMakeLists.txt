cmake_minimum_required(VERSION 3.15)
project(SpawnClone VERSION 1.0.0)

# Set C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add JUCE as a subdirectory
add_subdirectory(Libs/JUCE)

# Add the plugin target
juce_add_plugin(SpawnClone
    COMPANY_NAME "SpawnClone Audio"
    BUNDLE_ID "com.spawnclone.spawnclone"
    PLUGIN_VERSION ${PROJECT_VERSION}
    PLUGIN_MANUFACTURER_CODE "SPWN"
    PLUGIN_CODE "SCLN"
    FORMATS VST3 AU Standalone
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE
    IS_MIDI_EFFECT TRUE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_EMAIL "support@spawnclone.com"
    PLUGIN_MANUFACTURER_WEBSITE "https://spawnclone.com"
    PLUGIN_DESCRIPTION "AI-Powered MIDI Pattern Generator")

target_sources(SpawnClone PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
    Source/ParameterManager.cpp
    Source/ParameterManager.h
    Source/GenerationParameters.h
    Source/MIDIPattern.h
    Source/Note.h
    Source/PatternSerializer.cpp
    Source/PatternSerializer.h
    Source/PatternManager.cpp
    Source/PatternManager.h
    Source/ai/AIGenerationEngine.cpp
    Source/ai/AIGenerationEngine.h
    Source/ai/ThreadManager.cpp
    Source/ai/ThreadManager.h)

target_compile_definitions(SpawnClone PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

# Link JUCE modules
target_link_libraries(SpawnClone PUBLIC
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_plugin_client
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_core
    juce::juce_data_structures
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
)

# Target Include Directories
target_include_directories(SpawnClone PUBLIC "Source")

