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
    FORMATS AU VST3 Standalone
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
    PLUGIN_NUM_OUTPUT_CHANNELS 6
    VST3_AUTO_MANIFEST TRUE
    VST3_CATEGORIES "Instrument|Generator"
)

target_link_libraries(SpawnClone PRIVATE
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_core
    juce::juce_data_structures
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_dsp
)

# VST3 Parameter Compatibility Configuration
target_compile_definitions(SpawnClone PRIVATE
    JUCE_VST3_CAN_REPLACE_VST2=1
    JUCE_FORCE_USE_LEGACY_PARAM_IDS=0
    JUCE_USE_STUDIO_ONE_COMPATIBLE_PARAMETERS=1
    JUCE_IGNORE_VST3_MISMATCHED_PARAMETER_ID_WARNING=1
)

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
    Source/ai/PresetRecommendationEngine.cpp
    Source/ai/PresetRecommendationEngine.h
    Source/audio/AudioPreviewEngine.cpp
    Source/audio/AudioPreviewEngine.h
    Source/audio/AdvancedSynthesisEngine.cpp
    Source/audio/AdvancedSynthesisEngine.h
    Source/audio/SynthesisParameterMapper.cpp
    Source/audio/SynthesisParameterMapper.h
    Source/audio/SampleCache.cpp
    Source/audio/SampleCache.h
    Source/audio/AsyncSampleLoader.cpp
    Source/audio/AsyncSampleLoader.h
    Source/audio/SynthVoice.h
    Source/audio/SynthVoice.cpp
    Source/audio/synthesis/SampleEngine.h
    Source/audio/synthesis/SampleEngine.cpp
    Source/audio/synthesis/PitchShifter.h
    Source/audio/synthesis/PitchShifter.cpp
    Source/audio/synthesis/LoopManager.h
    Source/audio/synthesis/LoopManager.cpp
    Source/audio/InstrumentLibraryManager.cpp
    Source/audio/InstrumentLibraryManager.h
    Source/audio/LayerEffectsProcessor.cpp
    Source/audio/LayerEffectsProcessor.h
    Source/audio/RealTimeProcessor.cpp
    Source/audio/RealTimeProcessor.h
    Source/audio/MultiOutputManager.cpp
    Source/audio/MultiOutputManager.h
    Source/audio/TransportControlsComponent.cpp
    Source/audio/TransportControlsComponent.h
    Source/audio/MIDIExporter.cpp
    Source/audio/MIDIExporter.h
    Source/audio/EffectProcessor.cpp
    Source/audio/EffectProcessor.h
    Source/audio/EffectsChain.cpp
    Source/audio/EffectsChain.h
    Source/audio/ReverbEngine.cpp
    Source/audio/ReverbEngine.h
    Source/audio/DelayEngine.cpp
    Source/audio/DelayEngine.h
    Source/audio/ModulationEngine.cpp
    Source/audio/ModulationEngine.h
    Source/audio/DelayXYController.cpp
    Source/audio/DelayXYController.h
    Source/audio/ModulationXYController.cpp
    Source/audio/ModulationXYController.h
    Source/ui/ReverbXYController.cpp
    Source/ui/ReverbXYController.h
    Source/ui/ExperimentPadComponent.cpp
    Source/ui/ExperimentPadComponent.h
    Source/ui/PresetBrowserComponent.cpp
    Source/ui/PresetBrowserComponent.h
    Source/ui/SynthesisControlPanel.cpp
    Source/ui/SynthesisControlPanel.h
)

target_include_directories(SpawnClone PRIVATE
    Source)

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
        Source/audio/AdvancedSynthesisEngine.cpp
        Source/audio/SynthesisParameterMapper.cpp
        Source/audio/LayerEffectsProcessor.cpp
        Source/audio/InstrumentLibraryManager.cpp
        Source/audio/RealTimeProcessor.cpp
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
        juce::juce_dsp
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
        Source/audio/AdvancedSynthesisEngine.cpp
        Source/audio/LayerEffectsProcessor.cpp
        Source/audio/InstrumentLibraryManager.cpp
        Source/audio/RealTimeProcessor.cpp
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
        juce::juce_dsp
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
        Source/audio/AdvancedSynthesisEngine.cpp
        Source/audio/LayerEffectsProcessor.cpp
        Source/audio/InstrumentLibraryManager.cpp
        Source/audio/RealTimeProcessor.cpp
        Source/ui/SynthesisControlPanel.cpp
    )

    target_include_directories(test_Performance PRIVATE 
        Source 
        Source/ai
        Source/audio
        Source/ui
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
        juce::juce_dsp
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
        Source/PatternSerializer.cpp
        Source/ai/GenreSpecialist_Extended.cpp
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
    # Epic 9 Story 9.2.3: SampleCache Unit Tests
    add_executable(test_SampleCache
        tests/unit/test_SampleCache.cpp
        Source/audio/SampleCache.cpp
    )

    target_include_directories(test_SampleCache PRIVATE 
        Source 
        Source/audio
    )

    target_link_libraries(test_SampleCache PRIVATE 
        gtest_main
        juce::juce_core
        juce::juce_audio_basics
    )

    target_compile_features(test_SampleCache PRIVATE cxx_std_17)
    gtest_discover_tests(test_SampleCache)

    # Epic 9 Story 9.2.3: Sample-Based Synthesis Integration Tests
    add_executable(test_SampleBasedSynthesis
        Source/test_SampleBasedSynthesis.cpp
        Source/audio/AdvancedSynthesisEngine.cpp
        Source/audio/AdvancedSynthesisEngine.h
        Source/audio/SynthVoice.cpp
        Source/audio/SynthVoice.h
        Source/audio/synthesis/SampleEngine.cpp
        Source/audio/synthesis/SampleEngine.h
        Source/audio/synthesis/PitchShifter.cpp
        Source/audio/synthesis/PitchShifter.h
        Source/audio/synthesis/LoopManager.cpp
        Source/audio/synthesis/LoopManager.h
        Source/audio/SampleCache.cpp
        Source/audio/SampleCache.h
        Source/audio/AsyncSampleLoader.cpp
        Source/audio/AsyncSampleLoader.h
        Source/audio/EffectsChain.cpp
        Source/audio/EffectProcessor.cpp
        Source/audio/ReverbEngine.cpp
        Source/audio/DelayEngine.cpp
        Source/audio/ModulationEngine.cpp
    )

    target_include_directories(test_SampleBasedSynthesis PRIVATE
        Source
    )

    target_link_libraries(test_SampleBasedSynthesis PRIVATE
        gtest_main
        juce::juce_core
        juce::juce_audio_basics
        juce::juce_audio_processors
        juce::juce_dsp
    )

    target_compile_features(test_SampleBasedSynthesis PRIVATE cxx_std_17)
    gtest_discover_tests(test_SampleBasedSynthesis)
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

# Epic 9 Story 9.1: Curated Instrument Library Unit Tests
add_executable(test_Epic9Story91
    Source/test_epic9_story9_1.cpp
    Source/audio/InstrumentLibraryManager.cpp
    Source/audio/AudioPreviewEngine.cpp
    Source/audio/LayerEffectsProcessor.cpp
    Source/audio/RealTimeProcessor.cpp
    Source/MIDIPattern.cpp
    Source/PatternSerializer.cpp
)

target_include_directories(test_Epic9Story91 PRIVATE 
    Source
    Source/audio
    Source/ui
    Source/ai
)

target_link_libraries(test_Epic9Story91 PRIVATE 
    juce::juce_core
    juce::juce_audio_basics
    juce::juce_audio_processors
    juce::juce_dsp
    juce::juce_gui_basics
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
)

target_compile_features(test_Epic9Story91 PRIVATE cxx_std_17)
gtest_discover_tests(test_Epic9Story91)

# Epic 8 Story 8.4 Test: Advanced Real-Time Processing
add_executable(Epic4_Story4_RealTimeTest
    tests/epic4/Epic4_Story4_RealTimeTest.cpp
    Source/audio/RealTimeProcessor.cpp
    Source/audio/LayerEffectsProcessor.cpp
)

target_include_directories(Epic4_Story4_RealTimeTest PRIVATE 
    Source 
    Source/audio
    Source/ui
    Source/ai
)

target_link_libraries(Epic4_Story4_RealTimeTest PRIVATE 
    juce::juce_core
    juce::juce_audio_basics
    juce::juce_audio_processors
    juce::juce_dsp
    juce::juce_gui_basics
)

target_compile_features(Epic4_Story4_RealTimeTest PRIVATE cxx_std_17)

# Epic 9.2: Advanced Synthesis Engine Unit Tests
add_executable(test_AdvancedSynthesisEngine
    tests/unit/test_AdvancedSynthesisEngine.cpp
    Source/audio/AdvancedSynthesisEngine.cpp
    Source/audio/SynthVoice.cpp
    Source/audio/SampleCache.cpp
    Source/audio/AsyncSampleLoader.cpp
    Source/audio/EffectsChain.cpp
    Source/audio/EffectProcessor.cpp
    Source/audio/ReverbEngine.cpp
    Source/audio/DelayEngine.cpp
    Source/audio/ModulationEngine.cpp
    Source/audio/synthesis/SampleEngine.cpp
    Source/audio/synthesis/PitchShifter.cpp
    Source/audio/synthesis/LoopManager.cpp
)

target_link_libraries(test_AdvancedSynthesisEngine PRIVATE
    gtest_main
    juce::juce_core
    juce::juce_audio_basics
    juce::juce_audio_processors
    juce::juce_dsp
)

target_compile_features(test_AdvancedSynthesisEngine PRIVATE cxx_std_17)
target_include_directories(test_AdvancedSynthesisEngine PRIVATE "Source")

include(GoogleTest)
gtest_discover_tests(test_AdvancedSynthesisEngine)

# Debug synthesis executable
add_executable(debug_synthesis
    debug_synthesis.cpp
    Source/audio/AdvancedSynthesisEngine.cpp
)

target_link_libraries(debug_synthesis PRIVATE
    juce::juce_core
    juce::juce_audio_basics
    juce::juce_audio_processors
    juce::juce_dsp
)

target_compile_features(debug_synthesis PRIVATE cxx_std_17)
target_include_directories(debug_synthesis PRIVATE "Source")

# Minimal debug executable
add_executable(minimal_debug
    minimal_debug.cpp
    Source/audio/AdvancedSynthesisEngine.cpp
    Source/ui/SynthesisControlPanel.cpp
)

target_link_libraries(minimal_debug PRIVATE
    juce::juce_core
    juce::juce_audio_basics
    juce::juce_audio_processors
    juce::juce_dsp
)

target_compile_features(minimal_debug PRIVATE cxx_std_17)
target_include_directories(minimal_debug PRIVATE "Source" "Source/ui")

# Epic 6 Phase 1: Professional Reverb Demo
add_executable(epic6_reverb_demo
    epic6_reverb_demo.cpp
    Source/audio/EffectProcessor.cpp
    Source/audio/EffectsChain.cpp
    Source/audio/ReverbEngine.cpp
    Source/audio/DelayEngine.cpp
)

target_link_libraries(epic6_reverb_demo PRIVATE
    juce::juce_core
    juce::juce_audio_basics
    juce::juce_audio_processors
    juce::juce_audio_formats
    juce::juce_dsp
)

target_compile_features(epic6_reverb_demo PRIVATE cxx_std_17)
target_include_directories(epic6_reverb_demo PRIVATE "Source")

# Epic 6 Phase 2: Professional Reverb XY Controller Demo
add_executable(epic6_xy_demo
    epic6_xy_demo.cpp
    Source/audio/EffectProcessor.cpp
    Source/audio/EffectsChain.cpp
    Source/audio/ReverbEngine.cpp
    Source/ui/ReverbXYController.cpp
)

target_link_libraries(epic6_xy_demo PRIVATE
    juce::juce_core
    juce::juce_audio_basics
    juce::juce_audio_processors
    juce::juce_audio_formats
    juce::juce_dsp
)

target_compile_features(epic6_xy_demo PRIVATE cxx_std_17)
target_include_directories(epic6_xy_demo PRIVATE "Source")

# Epic 6 Story 6.9: Professional DelayEngine Demo
add_executable(epic6_delay_demo
    epic6_delay_demo.cpp
    Source/audio/EffectProcessor.cpp
    Source/audio/EffectsChain.cpp
    Source/audio/ReverbEngine.cpp
    Source/audio/DelayEngine.cpp
)

target_link_libraries(epic6_delay_demo PRIVATE
    juce::juce_core
    juce::juce_audio_basics
    juce::juce_audio_processors
    juce::juce_audio_formats
    juce::juce_dsp
)

target_compile_features(epic6_delay_demo PRIVATE cxx_std_17)
target_include_directories(epic6_delay_demo PRIVATE "Source")

# Epic 6 Story 6.10: Professional ModulationEngine Demo
add_executable(epic6_modulation_demo
    epic6_modulation_demo.cpp
    Source/audio/EffectProcessor.cpp
    Source/audio/EffectsChain.cpp
    Source/audio/ReverbEngine.cpp
    Source/audio/DelayEngine.cpp
    Source/audio/ModulationEngine.cpp
)

target_link_libraries(epic6_modulation_demo PRIVATE
    juce::juce_core
    juce::juce_audio_basics
    juce::juce_audio_processors
    juce::juce_audio_formats
    juce::juce_dsp
)

target_compile_features(epic6_modulation_demo PRIVATE cxx_std_17)
target_include_directories(epic6_modulation_demo PRIVATE "Source")

# Epic 6 Story 6.11: XY Controllers Integration Demo
add_executable(epic6_xy_controllers_demo
    epic6_xy_controllers_demo.cpp
    Source/audio/EffectProcessor.cpp
    Source/audio/EffectsChain.cpp
    Source/audio/ReverbEngine.cpp
    Source/audio/DelayEngine.cpp
    Source/audio/ModulationEngine.cpp
    Source/audio/DelayXYController.cpp
    Source/audio/ModulationXYController.cpp
)

target_link_libraries(epic6_xy_controllers_demo PRIVATE
    juce::juce_core
    juce::juce_audio_basics
    juce::juce_audio_processors
    juce::juce_audio_formats
    juce::juce_dsp
)

target_compile_features(epic6_xy_controllers_demo PRIVATE cxx_std_17)
target_include_directories(epic6_xy_controllers_demo PRIVATE "Source")

# Epic 8 Story 8.4: Advanced Real-Time Processing Demo
add_executable(epic8_story8_4_realtime_demo
    epic8_story8_4_realtime_demo.cpp
    Source/audio/RealTimeProcessor.cpp
    Source/audio/LayerEffectsProcessor.cpp
    Source/audio/EffectProcessor.cpp
    Source/audio/EffectsChain.cpp
    Source/audio/ReverbEngine.cpp
    Source/audio/DelayEngine.cpp
    Source/audio/ModulationEngine.cpp
)

target_link_libraries(epic8_story8_4_realtime_demo PRIVATE
    juce::juce_core
    juce::juce_audio_basics
    juce::juce_audio_processors
    juce::juce_audio_formats
    juce::juce_dsp
)

target_compile_features(epic8_story8_4_realtime_demo PRIVATE cxx_std_17)
target_include_directories(epic8_story8_4_realtime_demo PRIVATE "Source")

# Epic 9.3: Tag-Based Sound Pairing System
# Implementation begins here with intelligent preset recommendation engine
