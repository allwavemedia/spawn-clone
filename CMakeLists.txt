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
    Source/MIDIPattern.cpp
    Source/Note.h
    Source/PatternSerializer.cpp
    Source/PatternSerializer.h
    Source/PatternManager.cpp
    Source/PatternManager.h
    Source/PatternVisualizationComponent.cpp
    Source/PatternVisualizationComponent.h
    Source/ai/AIGenerationEngine.cpp
    Source/ai/AIGenerationEngine.h
    Source/ai/ThreadManager.cpp
    Source/ai/ThreadManager.h
    Source/ai/ONNXModelManager.cpp
    Source/ai/ONNXModelManager.h
    Source/ai/CloudAPIManager.cpp
    Source/ai/CloudAPIManager.h
    Source/ai/ModelCacheManager.cpp
    Source/ai/ModelCacheManager.h
    Source/ai/SecureCredentialManager.cpp
    Source/ai/SecureCredentialManager.h
    Source/ai/MusicTheoryEngine.cpp
    Source/ai/MusicTheoryEngine.h
    Source/ai/GenreSpecialist_Fixed.cpp
    Source/ai/GenreSpecialist.h
    Source/audio/AudioPreviewEngine.cpp
    Source/audio/AudioPreviewEngine.h
    Source/audio/MIDIExporter.cpp
    Source/audio/MIDIExporter.h)

target_compile_definitions(SpawnClone PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    CLOUD_API_SIMULATION=1)

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

# Optional ONNX Runtime Integration
option(USE_ONNX_RUNTIME "Enable ONNX Runtime for Quality Mode" OFF)

if(USE_ONNX_RUNTIME)
    find_package(onnxruntime QUIET)
    
    if(onnxruntime_FOUND)
        target_link_libraries(SpawnClone PRIVATE onnxruntime::onnxruntime)
        target_compile_definitions(SpawnClone PRIVATE ONNX_RUNTIME_AVAILABLE=1)
        message(STATUS "ONNX Runtime found - Quality Mode enabled")
    else()
        message(STATUS "ONNX Runtime not found - Quality Mode will use fallback")
    endif()
else()
    message(STATUS "ONNX Runtime disabled - Quality Mode will use fallback")
endif()

# Epic 4 Story 4.2 Test Executable
add_executable(Epic4_Story4_2_CoreTest
    Source/Epic4_Story4_2_CoreTest.cpp
    Source/ai/GenreSpecialist_Fixed.cpp
    Source/ai/MusicTheoryEngine.cpp
)

target_include_directories(Epic4_Story4_2_CoreTest PRIVATE Source Source/ai)
target_link_libraries(Epic4_Story4_2_CoreTest PRIVATE 
    juce::juce_core
    juce::juce_audio_basics
)
target_compile_features(Epic4_Story4_2_CoreTest PRIVATE cxx_std_17)

