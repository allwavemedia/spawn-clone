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
    PLUGIN_DESCRIPTION "AI-Powered MIDI Pattern Generator"
    # Epic 8 Story 8.3: Multi-Output Configuration
    PLUGIN_NUM_OUTPUT_CHANNELS 6)

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
    Source/PatternHistoryListBox.cpp
    Source/PatternHistoryListBox.h
    Source/PatternSearchComponent.cpp
    Source/PatternSearchComponent.h
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
    Source/audio/LayerEffectsProcessor.cpp
    Source/audio/LayerEffectsProcessor.h
    Source/audio/MultiOutputManager.cpp
    Source/audio/MultiOutputManager.h
    Source/audio/TransportControlsComponent.cpp
    Source/audio/TransportControlsComponent.h
    Source/audio/MIDIExporter.cpp
    Source/audio/MIDIExporter.h
    Source/ui/ExperimentPadComponent.cpp
    Source/ui/ExperimentPadComponent.h)

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
    juce::juce_dsp
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

# Testing Configuration
option(BUILD_TESTS "Build unit tests" ON)

if(BUILD_TESTS)
    # Google Test
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    
    # Epic 2 Story 2.2: AudioPreviewEngine Unit Tests
    add_executable(test_AudioPreviewEngine
        Source/test_AudioPreviewEngine.cpp
        Source/audio/AudioPreviewEngine.cpp
        Source/ParameterManager.cpp
        Source/PatternManager.cpp
        Source/PatternHistoryListBox.cpp
        Source/PatternSearchComponent.cpp
        Source/PatternSerializer.cpp
        Source/ai/MusicTheoryEngine.cpp
        Source/ai/GenreSpecialist_Fixed.cpp
    )

    target_include_directories(test_AudioPreviewEngine PRIVATE 
        Source 
        Source/ai 
        Source/audio
    )
    
    target_link_libraries(test_AudioPreviewEngine PRIVATE 
        gtest_main
        juce::juce_core
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_processors
        juce::juce_audio_utils
    )
    
    target_compile_features(test_AudioPreviewEngine PRIVATE cxx_std_17)
    
    include(GoogleTest)
    gtest_discover_tests(test_AudioPreviewEngine)
    
    # Epic 5 Story 5.1: Transport Controls Unit Tests
    add_executable(test_TransportControls
        Source/test_TransportControls.cpp
        Source/audio/AudioPreviewEngine.cpp
        Source/audio/TransportControlsComponent.cpp
        Source/ParameterManager.cpp
        Source/PatternManager.cpp
        Source/PatternSerializer.cpp
        Source/ai/MusicTheoryEngine.cpp
        Source/ai/GenreSpecialist_Fixed.cpp
    )

    target_include_directories(test_TransportControls PRIVATE 
        Source 
        Source/ai 
        Source/audio
    )
    
    target_link_libraries(test_TransportControls PRIVATE 
        gtest_main
        juce::juce_core
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_gui_basics
    )
    
    target_compile_features(test_TransportControls PRIVATE cxx_std_17)
    gtest_discover_tests(test_TransportControls)
    
    # Epic 5 Story 5.1: Visual-Audio Synchronization Unit Tests
    add_executable(test_VisualAudioSync
        Source/test_VisualAudioSync.cpp
        Source/PatternVisualizationComponent.cpp
        Source/MIDIPattern.cpp
        Source/ParameterManager.cpp
        Source/PatternManager.cpp
        Source/PatternSerializer.cpp
        Source/ai/MusicTheoryEngine.cpp
        Source/ai/GenreSpecialist_Fixed.cpp
    )

    target_include_directories(test_VisualAudioSync PRIVATE 
        Source 
        Source/ai
    )
    
    target_link_libraries(test_VisualAudioSync PRIVATE 
        gtest_main
        juce::juce_core
        juce::juce_audio_basics
        juce::juce_audio_processors
        juce::juce_gui_basics
        juce::juce_data_structures
    )
    
    target_compile_features(test_VisualAudioSync PRIVATE cxx_std_17)
    gtest_discover_tests(test_VisualAudioSync)
    
    # Epic 5 Story 5.2: Performance Profiling Tests
    add_executable(test_Performance
        Source/test_Performance.cpp
        Source/audio/AudioPreviewEngine.cpp
        Source/audio/TransportControlsComponent.cpp
        Source/PatternVisualizationComponent.cpp
        Source/ai/AIGenerationEngine.cpp
        Source/ai/ThreadManager.cpp
        Source/ai/ONNXModelManager.cpp
        Source/ai/CloudAPIManager.cpp
        Source/ai/ModelCacheManager.cpp
        Source/ai/SecureCredentialManager.cpp
        Source/PatternManager.cpp
        Source/ParameterManager.cpp
        Source/PatternSerializer.cpp
        Source/ai/MusicTheoryEngine.cpp
        Source/ai/GenreSpecialist_Fixed.cpp
    )

    target_include_directories(test_Performance PRIVATE 
        Source 
        Source/ai
        Source/audio
    )
    
    target_link_libraries(test_Performance PRIVATE 
        gtest_main
        juce::juce_core
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_gui_basics
        juce::juce_data_structures
    )
    
    target_compile_features(test_Performance PRIVATE cxx_std_17)
    gtest_discover_tests(test_Performance)
    
    # Epic 6 Story 6.1: Advanced Generation Unit Tests
    add_executable(test_AdvancedGeneration
        Source/test_AdvancedGeneration.cpp
        Source/ai/MusicTheoryEngine.cpp
    )

    target_include_directories(test_AdvancedGeneration PRIVATE 
        Source 
        Source/ai
    )
    
    target_link_libraries(test_AdvancedGeneration PRIVATE 
        gtest_main
        juce::juce_core
    )
    
    target_compile_features(test_AdvancedGeneration PRIVATE cxx_std_17)
    gtest_discover_tests(test_AdvancedGeneration)
    
    # Epic 6 Story 6.2: Pattern Evolution Unit Tests
    add_executable(test_PatternEvolution
        Source/test_PatternEvolution.cpp
        Source/ai/MusicTheoryEngine.cpp
    )

    target_include_directories(test_PatternEvolution PRIVATE 
        Source 
        Source/ai
    )
    
    target_link_libraries(test_PatternEvolution PRIVATE 
        gtest_main
        juce::juce_core
    )
    
    target_compile_features(test_PatternEvolution PRIVATE cxx_std_17)
    gtest_discover_tests(test_PatternEvolution)
    
    # Epic 6 Story 6.3: Advanced Harmony Unit Tests
    add_executable(test_AdvancedHarmony
        Source/test_AdvancedHarmony.cpp
        Source/ai/MusicTheoryEngine.cpp
    )

    target_include_directories(test_AdvancedHarmony PRIVATE 
        Source 
        Source/ai
    )
    
    target_link_libraries(test_AdvancedHarmony PRIVATE 
        gtest_main
        juce::juce_core
    )
    
    target_compile_features(test_AdvancedHarmony PRIVATE cxx_std_17)
    gtest_discover_tests(test_AdvancedHarmony)
    
    # Epic 6 Story 6.4: User Style Profiles Unit Tests
    add_executable(test_UserStyleProfiles
        Source/test_UserStyleProfiles.cpp
        Source/UserStyleProfile.cpp
        Source/UserStyleLearningEngine.cpp
        Source/ai/MusicTheoryEngine.cpp
    )

    target_include_directories(test_UserStyleProfiles PRIVATE 
        Source 
        Source/ai
    )
    
    target_link_libraries(test_UserStyleProfiles PRIVATE 
        gtest_main
        juce::juce_core
        juce::juce_data_structures
        juce::juce_events
    )
    
    target_compile_features(test_UserStyleProfiles PRIVATE cxx_std_17)
    gtest_discover_tests(test_UserStyleProfiles)
    
    # Epic 7 Story 7.1 Task 7.1.6: Fast Mode Generation Tests
    add_executable(test_FastModeGeneration
        Source/test_FastModeGeneration.cpp
        Source/ai/AIGenerationEngine.cpp
        Source/ai/MusicTheoryEngine.cpp
        Source/ai/CloudAPIManager.cpp
        Source/ai/ONNXModelManager.cpp
        Source/ai/ModelCacheManager.cpp
        Source/ai/SecureCredentialManager.cpp
        Source/PatternManager.cpp
        Source/ai/ThreadManager.cpp
        Source/MIDIPattern.cpp
        Source/ParameterManager.cpp
    )
    
    target_include_directories(test_FastModeGeneration PRIVATE 
        Source 
        Source/ai
        ${CMAKE_CURRENT_SOURCE_DIR}/Libs/googletest/include
    )
    
    target_link_libraries(test_FastModeGeneration PRIVATE 
        gtest_main
        juce::juce_core
        juce::juce_events
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_processors
        juce::juce_gui_basics
    )
    
    target_compile_features(test_FastModeGeneration PRIVATE cxx_std_17)
    gtest_discover_tests(test_FastModeGeneration)
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

# Epic 8 Story 8.1 Test: Experiment Pad Component Tests
if(GTEST_FOUND)
    add_executable(test_ExperimentPadComponent
        Source/test_ExperimentPadComponent.cpp
        Source/ui/ExperimentPadComponent.cpp
    )
    
    target_include_directories(test_ExperimentPadComponent PRIVATE 
        Source 
        Source/ui
        Source/ai
        Source/audio
    )
    
    target_link_libraries(test_ExperimentPadComponent PRIVATE 
        GTest::gtest 
        GTest::gtest_main
        juce::juce_core
        juce::juce_audio_basics
        juce::juce_audio_processors
        juce::juce_gui_basics
    )
    
    target_compile_features(test_ExperimentPadComponent PRIVATE cxx_std_17)
    gtest_discover_tests(test_ExperimentPadComponent)
endif()

