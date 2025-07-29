# SpawnClone Project Backlog

## Introduction

This document serves as the official project backlog for SpawnClone. It is managed by the Scrum Master and translates the requirements from the **Product Requirements Document (PRD)** and the technical specifications from the **Architecture Document** into a structured, actionable plan for the development team.

Each epic from the PRD is broken down into its original user stories, which are then further decomposed into specific, granular technical tasks. This provides clarity for sprint planning and ensures all architectural and functional requirements are met.

---

## **Epic 1: Foundation & Core Infrastructure**

**Epic Goal:** Establish the foundational project infrastructure including build system, plugin architecture, basic DAW loading capability, and a minimal AI engine that can generate simple MIDI patterns. This epic delivers the essential technical foundation while providing initial pattern generation functionality to validate core concepts and enable early testing.

### **Story 1.1: Project Setup and Build System**

**As a developer, I want a properly configured JUCE project with a CMake build system, so that I can build the plugin across all target platforms (Windows, macOS, Linux) with a consistent development environment.**

#### Technical Tasks:

- **Task 1.1.1:** Initialize a new JUCE project using the Projucer or `juce-cmake`.
- **Task 1.1.2:** Configure `CMakeLists.txt` to define the project, link JUCE, and set C++17 as the minimum standard.
- **Task 1.1.3:** Add build targets for VST3, AudioUnit (AU), and Standalone application formats.
- **Task 1.1.4:** Create the initial directory structure as defined in the architecture: `/Source`, `/Libs`, `/docs`, etc.
- **Task 1.1.5:** Set up a basic GitHub Actions workflow (`.github/workflows/release.yml`) that triggers on push and performs a checkout and build on `macos-latest`, `windows-latest`, and `ubuntu-latest`.
- **Task 1.1.6:** Create a `README.md` with initial build instructions for all three platforms.
- **Task 1.1.7:** Add a `.gitignore` file tailored for C++/JUCE/CMake development.

### **Story 1.2: Basic Plugin Architecture and DAW Loading**

**As a music producer, I want the plugin to load successfully in my DAW with a basic UI, so that I can verify the plugin installation and see the initial interface.**

#### Technical Tasks:

- **Task 1.2.1:** Implement the main `PluginProcessor` (`SpawnCloneProcessor`) and `PluginEditor` (`SpawnCloneEditor`) classes.
- **Task 1.2.2:** Ensure the `PluginProcessor` constructor initializes correctly and the plugin metadata (name, version) is set.
- **Task 1.2.3:** Create a placeholder UI in `SpawnCloneEditor` with a simple background color and a label to confirm it's running.
- **Task 1.2.4:** Implement the basic `getStateInformation` and `setStateInformation` methods for saving/loading state, even if it's empty initially.
- **Task 1.2.5:** Manually test loading the compiled VST3/AU plugin in at least one major DAW on macOS (e.g., Logic Pro) and Windows (e.g., Reaper).

### **Story 1.3: Core MIDI Pattern Data Structure**

**As a developer, I want a robust MIDI pattern data structure with serialization, so that generated patterns can be stored, manipulated, and exported reliably.**

#### Technical Tasks:

- **Task 1.3.1:** Define the `MIDIPattern` struct/class in `/src/core` as specified in the architecture, including a container for notes and metadata fields.
- **Task 1.3.2:** Define the `Note` struct with `pitch`, `velocity`, `startTime`, and `duration`.
- **Task 1.3.3:** Implement serialization/deserialization logic for `MIDIPattern` to/from a JUCE `ValueTree` for state management.
- **Task 1.3.4:** Create a utility function in a `MIDIProcessor` class to export a `MIDIPattern` object to a standard MIDI file (`juce::MidiFile`).
- **Task 1.3.5:** Write unit tests (GoogleTest) for the `MIDIPattern` data structure, covering creation, modification, and serialization.

### **Story 1.4: Basic AI Engine Infrastructure**

**As a developer, I want a foundational AI engine that can generate simple MIDI sequences, so that the core pattern generation workflow is established for future enhancement.**

#### Technical Tasks:

- **Task 1.4.1:** Create the `AIGenerationEngine` class in `/src/ai` which runs on a `juce::Thread`.
- **Task 1.4.2:** Implement the `ThreadManager` class to manage the lifecycle of the AI worker thread.
- **Task 1.4.3:** Use a `juce::AbstractFifo` to pass generation requests from the `PluginProcessor` to the `AIGenerationEngine`.
- **Task 1.4.4:** Use a second `juce::AbstractFifo` to pass the resulting `MIDIPattern` back to the `PluginProcessor`.
- **Task 1.4.5:** Implement the rule-based generation algorithm within `AIGenerationEngine` that creates a musically coherent C Major scale pattern.
- **Task 1.4.6:** Add a "Generate" button to the UI that triggers the generation workflow via the `PluginProcessor`.

### **Story 1.5: Parameter Management System**

**As a music producer, I want to adjust generation parameters through plugin controls, so that I can influence the style and characteristics of generated MIDI patterns.**

#### Technical Tasks:

- **Task 1.5.1:** Implement the `ParameterManager` class using `juce::AudioProcessorValueTreeState`.
- **Task 1.5.2:** Define the `GenerationParameters` struct as specified in the architecture.
- **Task 1.5.3:** Create and register all user-facing parameters (Key, Scale, Tempo, Complexity, Type) with the host DAW via the `AudioProcessorValueTreeState`.
- **Task 1.5.4:** Add basic UI controls (sliders, combo boxes) to the `PluginEditor` and connect them to the `AudioProcessorValueTreeState`.
- **Task 1.5.5:** Ensure the `AIGenerationEngine` can safely access the latest parameters from the `ParameterManager` for its generation task.

---

## **Epic 2: DAW Integration & Audio Preview**

**Epic Goal:** Implement comprehensive DAW host communication, real-time synchronization capabilities, and a lightweight audio preview engine. This epic establishes the plugin as a fully functional DAW instrument with audio feedback, enabling users to hear generated patterns immediately and experience seamless integration with their production workflow.

### **Story 2.1: Host DAW Communication and Synchronization**

**As a music producer, I want the plugin to automatically synchronize with my DAW's tempo and key, so that generated patterns align perfectly with my project settings without manual configuration.**

#### Technical Tasks:

- **Task 2.1.1:** In `PluginProcessor`, access the `AudioPlayHead` to get the current tempo, time signature, and transport state.
- **Task 2.1.2:** Update the UI to reflect the host's tempo in real-time.
- **Task 2.1.3:** Implement logic to use the host's key/scale information if available, otherwise fall back to manual settings.
- **Task 2.1.4:** Ensure parameter changes from the host (automation) are correctly handled by the `AudioProcessorValueTreeState`.

### **Story 2.2: Lightweight Audio Preview Engine**

**As a music producer, I want to immediately hear generated MIDI patterns through built-in sounds, so that I can quickly evaluate musical ideas without setting up external instruments.**

#### Technical Tasks:

- **Task 2.2.1:** Implement the `AudioPreviewEngine` class in `/src/audio`.
- **Task 2.2.2:** Use a simple `juce::Synthesiser` with a basic wavetable voice.
- **Task 2.2.3:** Create three simple sounds (piano, synth, bass) and implement logic to switch between them based on the `generationType` parameter.
- **Task 2.2.4:** The `PluginProcessor` will own the `AudioPreviewEngine` and call its rendering method within the `processBlock` callback.
- **Task 2.2.5:** Implement a "Preview" button in the UI that sends the selected `MIDIPattern` to the `AudioPreviewEngine` for playback via a lock-free queue.

### **Story 2.3: MIDI Export and Drag-Drop Functionality**

**As a music producer, I want to drag generated patterns directly into my DAW timeline, so that I can quickly integrate AI-generated ideas into my production workflow.**

#### Technical Tasks:

- **Task 2.3.1:** Implement the drag-and-drop source functionality in the `PluginEditor`.
- **Task 2.3.2:** When a drag operation starts, use the `MIDIProcessor` to convert the selected `MIDIPattern` into a standard MIDI file format in memory.
- **Task 2.3.3:** Use the `juce::DragAndDropContainer` to initiate the native drag operation with the MIDI data.
- **Task 2.3.4:** Implement "Copy to Clipboard" and "Export to File" buttons that use the same `MIDIProcessor` utility functions.

### **Story 2.4: Real-Time Audio Thread Safety**

**As a developer, I want all audio processing to maintain real-time safety standards, so that the plugin never causes audio dropouts or interrupts the user's creative flow.**

#### Technical Tasks:

- **Task 2.4.1:** Review all code that runs on the audio thread (primarily `PluginProcessor::processBlock`).
- **Task 2.4.2:** Ensure no locks, memory allocations, or other blocking operations occur in the audio callback.
- **Task 2.4.3:** All communication between the UI/AI threads and the audio thread must use lock-free data structures like `juce::AbstractFifo`.
- **Task 2.4.4:** Use atomic variables within the `ParameterManager` for all parameters that need to be read by the audio thread.

### **Story 2.5: Pattern Playback and Transport Control**

**As a music producer, I want to control pattern playback with start/stop buttons and looping, so that I can evaluate generated patterns in context with my project.**

#### Technical Tasks:

- **Task 2.5.1:** Add play/stop and loop toggle buttons to the UI for the preview engine.
- **Task 2.5.2:** Implement the playback logic within the `AudioPreviewEngine` to handle starting, stopping, and looping MIDI playback.
- **Task 2.5.3:** Add a UI element to visualize the playback position within the pattern.

---

## **Epic 3: User Interface & Pattern Management**

**Epic Goal:** Develop a comprehensive user interface with all parameter controls, pattern history management, and drag-and-drop export functionality.

### **Story 3.1: Comprehensive UI Implementation**

**As a music producer, I want a clean, intuitive interface to control all generation parameters, so that I can easily shape the musical output.**

#### Technical Tasks:

- **Task 3.1.1:** Design and implement the final UI layout using JUCE components.
- **Task 3.1.2:** Replace all placeholder UI elements with final, styled components.
- **Task 3.1.3:** Ensure the UI is resizable and scales correctly on high-DPI displays.
- **Task 3.1.4:** Implement the master volume control for the audio preview.

### **Story 3.2: Pattern History Panel**

**As a music producer, I want to see a history of my generated patterns, so that I can compare, revisit, and manage my ideas.**

#### Technical Tasks:

- **Task 3.2.1:** Implement the `PatternManager` class to store a list of `MIDIPattern` objects.
- **Task 3.2.2:** Create a UI panel (e.g., a `juce::ListBox`) to display the pattern history.
- **Task 3.2.3:** Each item in the list should display a visual representation of the MIDI and have buttons for preview, favorite, and delete.
- **Task 3.2.4:** Connect the UI to the `PatternManager` so that it updates in real-time as new patterns are generated.
- **Task 3.2.5:** Ensure the `patternHistory` is saved and restored as part of the `PluginState`.

---

## **Epic 4: AI Model Integration & Musical Intelligence**

**Epic Goal:** Integrate the custom rule-based AI engine with genre-specific logic for Hip Hop, Pop, Dance, EDM, and R&B pattern generation with musical coherence validation.

### **Story 4.1: Rule-Based AI Engine Integration**

**As a developer, I want to integrate the custom rule-based C++ logic into the project, so that the AI engine can generate musically intelligent patterns.**

#### Technical Tasks:

- **Task 4.1.1:** Add the `AIGenerationEngine.cpp` and its dependencies to the CMake build system.
- **Task 4.1.2:** Ensure the generation parameters are correctly passed to the engine.
- **Task 4.1.3:** Implement the core generation loop in the `AIGenerationEngine`: prepare parameters, run generation, and post-process the results.
- **Task 4.1.4:** Write a pre-processing function to convert `GenerationParameters` into the format expected by the engine.
- **Task 4.1.5:** Write a post-processing function to convert the engine's output back into a `MIDIPattern` object.

### **Story 4.2: Genre-Specific Pattern Generation**

**As a music producer, I want the AI to generate patterns that are stylistically appropriate for modern genres, so that the output is immediately useful in my projects.**

#### Technical Tasks:

- **Task 4.2.1:** Replace any placeholder random generation algorithm with the real rule-based generation logic.
- **Task 4.2.2:** Ensure the `generationType` parameter (Melody, Chords, Bassline) correctly influences the engine's output.
- **Task 4.2.3:** Test the generated output for musical coherence and stylistic appropriateness.

---

## **Epic 5: Performance Optimization & Testing**

**Epic Goal:** Conduct comprehensive testing across target DAWs, optimize performance for real-time audio requirements, and prepare for a beta release.

### **Story 5.1: Comprehensive Testing**

**As a developer, I want a full suite of tests, so that I can ensure the plugin is stable and reliable.**

#### Technical Tasks:

- **Task 5.1.1:** Write unit tests for all critical components, aiming for >80% coverage.
- **Task 5.1.2:** Create integration tests for the full generation and preview workflows.
- **Task 5.1.3:** Use `auvaltool` and the VST3 validator in the CI pipeline to catch format-specific issues.
- **Task 5.1.4:** Perform manual QA testing in Ableton Live, Logic Pro, and Reaper on both Windows and macOS.

### **Story 5.2: Performance Profiling and Optimization**

**As a music producer, I want the plugin to be lightweight and efficient, so that it doesn't slow down my creative process.**

#### Technical Tasks:

- **Task 5.2.1:** Profile the plugin's CPU and memory usage during generation and preview.
- **Task 5.2.2:** Optimize the audio preview engine to meet the <5% CPU and <32MB memory targets.
- **Task 5.2.3:** Optimize the AI inference code to ensure generation completes within the 2-5 second target.

### **Story 5.3: Build Automation and Release**

**As a developer, I want an automated build and release process, so that I can easily create signed installers for users.**

#### Technical Tasks:

- **Task 5.3.1:** Enhance the GitHub Actions workflow to handle code signing for both macOS and Windows (using secrets).
- **Task 5.3.2:** Add a step to the macOS build to notarize the plugin with Apple.
- **Task 5.3.3:** Add steps to create a `.pkg` installer for macOS and a `.exe` installer for Windows.
- **Task 5.3.4:** Configure the workflow to automatically create a GitHub Release and upload the installers when a new version tag is pushed.
