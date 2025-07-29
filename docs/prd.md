# SpawnClone Product Requirements Document (PRD)

## Goals and Background Context

### Goals
- Create a free, open-source alternative to Sauceware's Spawn audio plugin
- Deliver AI-powered MIDI pattern generation for Hip Hop, Pop, Dance, EDM, and R&B genres
- Provide seamless DAW integration through VST3/AU plugin formats
- Enable drag-and-drop MIDI export functionality with lightweight audio preview
- Establish a simplified yet powerful workflow focused on core functionality

### Background Context

The music production landscape currently lacks accessible, open-source AI-powered MIDI generation tools. While commercial solutions like Sauceware's Spawn exist, they create barriers for independent producers and developers seeking royalty-free pattern generation capabilities. This project addresses the need for a democratized tool that provides core AI MIDI generation functionality within professional DAW environments.

The SpawnClone project focuses on delivering the essential features that make Spawn valuable - AI-driven pattern generation, real-time DAW integration, and intuitive export workflows - while maintaining a simplified approach suitable for V1.0 development. This foundation enables future expansion while providing immediate value to the music production community.

### Change Log

| Date | Version | Description | Author |
|------|---------|-------------|--------|
| July 28, 2025 | 1.0 | Initial PRD creation from technical specification | PM John |

## Requirements

### Functional

**FR1:** The plugin shall automatically detect and synchronize with the host DAW's current tempo (BPM) and musical key/scale information.

**FR2:** The plugin shall provide user controls for musical key selection (all 12 standard keys: C, C#, D, D#, E, F, F#, G, G#, A, A#, B).

**FR3:** The plugin shall provide user controls for musical scale selection (Major, Minor, Pentatonic, Blues, Dorian, Mixolydian).

**FR4:** The plugin shall provide a tempo override control that defaults to the host DAW's tempo but allows manual adjustment.

**FR5:** The plugin shall provide a rhythmic complexity control (slider/buttons) to define pattern density and syncopation (0.0-1.0 range).

**FR6:** The plugin shall provide generation type selection for "Melody," "Chords," or "Bassline" pattern types.

**FR7:** The plugin shall generate a single MIDI pattern per generation request (not multiple variations like original Spawn).

**FR8:** The plugin shall enable users to drag-and-drop generated MIDI patterns directly into the DAW timeline.

**FR9:** The plugin shall enable users to copy MIDI data to system clipboard for external use.

**FR10:** The plugin shall enable users to export MIDI patterns as standard .mid files.

**FR11:** The plugin shall maintain a session history of generated patterns with preview, favorite, delete, and drag capabilities.

**FR12:** The plugin shall provide lightweight audio preview using automatically selected instrument sounds based on generation type (piano for melody, synth for chords, bass for bassline).

**FR13:** The plugin shall provide a master volume control for the audio preview engine.

**FR14:** The AI engine shall generate musically coherent patterns optimized for Hip Hop, Pop, Dance, EDM, and R&B genres.

**FR15:** The plugin shall generate loopable patterns typically 4-8 bars in length suitable for immediate musical use.

### Non Functional

**NFR1:** The plugin shall support VST3, AudioUnit (AU), and Standalone formats for cross-platform compatibility.

**NFR2:** The plugin shall operate on Windows, macOS, and Linux platforms.

**NFR3:** The audio preview engine shall maintain CPU usage under 5% on modern systems during preview playback.

**NFR4:** The audio preview engine shall use less than 32MB of memory for sound library storage.

**NFR5:** The plugin shall complete MIDI pattern generation within 1-2 seconds on typical hardware.

**NFR6:** The plugin shall maintain real-time audio safety with no audio dropouts during generation or preview.

**NFR7:** The plugin shall be built using C++17 and the JUCE 7.x framework for industry standard compatibility.

**NFR8:** The AI engine shall use a custom, high-performance rule-based system for on-device generation without cloud or large model dependencies.

**NFR9:** The plugin shall load within 3 seconds in supported DAWs.

**NFR10:** The plugin shall handle concurrent operations (generation, preview, UI interaction) without blocking the audio thread.

## User Interface Design Goals

### Overall UX Vision

SpawnClone's interface should embody the philosophy of "sophisticated simplicity" - providing powerful AI-driven MIDI generation capabilities through an intuitive, uncluttered interface that doesn't overwhelm users with options. The design should feel modern and professional, suitable for integration into professional DAW environments, while remaining accessible to producers of all skill levels. The interface should emphasize immediate creative flow, enabling rapid iteration between generation parameters and musical output.

### Key Interaction Paradigms

- **One-Click Generation**: Primary workflow centers around a prominent "Generate" button with instant visual and audio feedback
- **Parameter Adjustment**: Real-time parameter changes with immediate preview capability, avoiding complex dialog boxes or multi-step processes
- **Drag-and-Drop Export**: Direct manipulation of generated patterns for seamless DAW integration
- **Session History Navigation**: Simple list-based browsing of generated patterns with inline preview controls
- **Context-Aware Automation**: Intelligent defaults based on DAW context (tempo, key detection) to minimize manual configuration

### Core Screens and Views

**Main Plugin Interface**: Single-window design containing all primary controls - parameter section (key, scale, tempo, complexity, generation type), central generation button, audio preview controls, and pattern history panel

**Pattern History Panel**: Expandable/collapsible sidebar or bottom panel displaying generated patterns as visual waveforms or MIDI representations with quick action buttons (preview, favorite, delete, drag)

**Settings/Configuration View**: Minimal overlay or expandable section for advanced preferences such as AI model settings, export format options, and plugin behavior customization

### Accessibility: WCAG AA

The plugin interface will meet WCAG AA standards to ensure accessibility for producers with disabilities, including proper keyboard navigation, screen reader compatibility, and sufficient color contrast ratios for all interface elements.

### Branding

Clean, modern aesthetic aligned with professional audio software standards. Dark theme optimized for studio environments with accent colors for key interactive elements. Visual design should complement existing DAW interfaces rather than compete for attention. Typography should prioritize clarity and readability in low-light studio conditions.

### Target Device and Platforms: Desktop Only

Optimized specifically for desktop environments within DAW hosts (Windows, macOS, Linux). Interface designed for mouse and keyboard interaction with consideration for high-DPI displays and various screen sizes common in studio setups.

## Technical Assumptions

### Repository Structure: Monorepo

Single repository containing the complete plugin codebase, documentation, build scripts, and AI model resources. This approach simplifies dependency management and enables atomic commits across all plugin components while maintaining clear separation between core engine, UI, and platform-specific builds.

### Service Architecture

**Modular Plugin Architecture**: Single executable plugin with clearly separated modules - AI Generation Engine, Audio Preview Engine, MIDI Processor, UI Components, and DAW Integration Layer. Each module has defined interfaces enabling independent development and testing while maintaining real-time audio thread safety through lock-free communication patterns.

### Testing Requirements

**Comprehensive Testing Strategy**: Unit testing for all core components (AI engine, MIDI processing, audio preview), integration testing for DAW compatibility across VST3/AU formats, and automated performance testing to validate real-time audio requirements. Manual testing procedures for complex DAW integration scenarios and AI model validation against musical quality metrics.

### Additional Technical Assumptions and Requests

- **Development Framework**: JUCE 7.x as the primary framework for cross-platform audio plugin development, providing proven DAW integration capabilities and audio processing infrastructure
- **AI Model Integration**: Custom rule-based AI engine for on-device generation.
- **Build System**: CMake-based build system supporting all target platforms (Windows, macOS, Linux) with automated CI/CD pipeline for plugin validation across major DAWs.
- **Programming Language**: C++17 minimum, ensuring modern language capabilities while maintaining broad compiler compatibility.
- **Audio Threading**: Strict real-time audio thread separation with lock-free communication mechanisms (lock-free FIFOs) for AI generation results and UI state changes.
- **Memory Management**: RAII principles throughout codebase with careful attention to audio thread memory allocation restrictions.
- **Plugin Formats**: VST3, AudioUnit (AU), and Standalone support for maximum compatibility.
- **AI Model Storage**: AI logic is embedded within the plugin binary, eliminating external dependencies for end users.
- **Performance Profiling**: Integration of performance monitoring tools to validate real-time audio constraints and optimize CPU/memory usage during development.

## Epic List

### Epic 1: Foundation & Core Infrastructure
Establish project foundation with basic plugin architecture, build system, and core AI engine infrastructure while delivering initial MIDI generation capability.

### Epic 2: DAW Integration & Audio Preview
Implement complete DAW integration with VST3/AU/Standalone formats, host communication, and lightweight audio preview engine for immediate pattern feedback.

### Epic 3: User Interface & Pattern Management
Develop comprehensive user interface with all parameter controls, pattern history management, and drag-and-drop export functionality.

### Epic 4: AI Model Integration & Musical Intelligence
Integrate the custom rule-based AI engine with genre-specific logic for Hip Hop, Pop, Dance, EDM, and R&B pattern generation with musical coherence validation.

### Epic 5: Performance Optimization & Testing
Comprehensive testing across target DAWs, performance optimization for real-time audio requirements, and preparation for beta release.

## Epic 1: Foundation & Core Infrastructure

**Epic Goal:** Establish the foundational project infrastructure including build system, plugin architecture, basic DAW loading capability, and a minimal AI engine that can generate simple MIDI patterns. This epic delivers the essential technical foundation while providing initial pattern generation functionality to validate core concepts and enable early testing.

### Story 1.1: Project Setup and Build System

**As a developer,**  
**I want a properly configured JUCE project with CMake build system,**  
**so that I can build the plugin across all target platforms (Windows, macOS, Linux) with consistent development environment.**

#### Acceptance Criteria

1. CMake configuration supports building VST3 and AU plugin formats
2. Project structure follows JUCE best practices with separated source folders for core, UI, and platform-specific code
3. Build system successfully compiles on Windows (Visual Studio), macOS (Xcode), and Linux (GCC/Clang)
4. Git repository is initialized with appropriate .gitignore for C++/JUCE development
5. Basic CI/CD pipeline is configured to validate builds on all target platforms
6. Documentation includes build instructions and development environment setup

### Story 1.2: Basic Plugin Architecture and DAW Loading

**As a music producer,**  
**I want the plugin to load successfully in my DAW with basic UI,**  
**so that I can verify the plugin installation and see the initial interface.**

#### Acceptance Criteria

1. Plugin loads without errors in at least 3 major DAWs (Ableton Live, Logic Pro, FL Studio)
2. Basic UI displays with placeholder controls for future functionality
3. Plugin responds to basic DAW operations (open/close, bypass, automation)
4. Plugin state can be saved and restored within DAW sessions
5. Plugin displays correct name, version, and manufacturer information in DAW plugin browser
6. No audio dropouts or crashes occur during basic plugin operations

### Story 1.3: Core MIDI Pattern Data Structure

**As a developer,**  
**I want a robust MIDI pattern data structure with serialization,**  
**so that generated patterns can be stored, manipulated, and exported reliably.**

#### Acceptance Criteria

1. MIDIPattern class stores note data with pitch, velocity, timing, and channel information
2. Pattern length and tempo metadata are properly maintained
3. Patterns can be serialized to/from JSON for session storage
4. Patterns can be exported as standard MIDI files (.mid format)
5. Thread-safe access patterns support real-time audio thread requirements
6. Unit tests validate all MIDI pattern operations and edge cases

### Story 1.4: Basic AI Engine Infrastructure

**As a developer,**  
**I want a foundation AI engine that can generate simple MIDI sequences,**  
**so that the core pattern generation workflow is established for future enhancement.**

#### Acceptance Criteria

1. AIGenerationEngine interface is defined with async generation methods
2. Basic pattern generator creates simple monophonic melodies using music theory rules
3. Generation parameters (key, scale, tempo) are properly processed
4. Generated patterns are musically valid (notes within scale, proper timing)
5. Generation operates on background thread without blocking UI or audio
6. At least 10 different patterns can be generated for the same parameters (randomization)

### Story 1.5: Parameter Management System

**As a music producer,**  
**I want to adjust generation parameters through plugin controls,**  
**so that I can influence the style and characteristics of generated MIDI patterns.**

#### Acceptance Criteria

1. Plugin parameters are properly registered with DAW automation system
2. Key selection supports all 12 chromatic keys with proper naming
3. Scale selection includes Major, Minor, Pentatonic, and Blues scales
4. Tempo parameter synchronizes with DAW tempo or allows manual override
5. Rhythmic complexity parameter (0.0-1.0) affects pattern generation
6. Parameter changes trigger automatic pattern regeneration with visual feedback

## Epic 2: DAW Integration & Audio Preview

**Epic Goal:** Implement comprehensive DAW host communication, real-time synchronization capabilities, and lightweight audio preview engine. This epic establishes the plugin as a fully functional DAW instrument with audio feedback, enabling users to hear generated patterns immediately and experience seamless integration with their production workflow.

### Story 2.1: Host DAW Communication and Synchronization

**As a music producer,**  
**I want the plugin to automatically synchronize with my DAW's tempo and key,**  
**so that generated patterns align perfectly with my project settings without manual configuration.**

#### Acceptance Criteria

1. Plugin automatically detects and displays host DAW tempo (BPM) in real-time
2. Tempo changes in DAW are reflected immediately in plugin interface
3. Plugin detects DAW musical key/scale information when available
4. Transport state (play/stop/recording) is monitored and affects plugin behavior
5. Plugin properly handles sample rate changes and buffer size adjustments
6. Host automation of plugin parameters works correctly across all target DAWs

### Story 2.2: Lightweight Audio Preview Engine

**As a music producer,**  
**I want to immediately hear generated MIDI patterns through built-in sounds,**  
**so that I can quickly evaluate musical ideas without setting up external instruments.**

#### Acceptance Criteria

1. Audio preview engine renders MIDI patterns using lightweight wavetable synthesis
2. Automatic instrument selection based on generation type (piano/lead/bass sounds)
3. Audio preview maintains <5% CPU usage during playback on modern systems
4. Preview engine uses <32MB memory for all instrument samples and wavetables
5. Real-time audio rendering without dropouts or artifacts
6. Master volume control for preview engine with proper gain staging

### Story 2.3: MIDI Export and Drag-Drop Functionality

**As a music producer,**  
**I want to drag generated patterns directly into my DAW timeline,**  
**so that I can quickly integrate AI-generated ideas into my production workflow.**

#### Acceptance Criteria

1. Drag-and-drop from plugin to DAW timeline works in all target DAWs
2. MIDI data maintains correct timing, velocity, and note information during transfer
3. Patterns can be copied to system clipboard in standard MIDI format
4. Export to .mid file functionality with user-selectable save location
5. Exported MIDI files open correctly in external applications and DAWs
6. Pattern metadata (tempo, key, scale) is preserved in exported files

### Story 2.4: Real-Time Audio Thread Safety

**As a developer,**  
**I want all audio processing to maintain real-time safety standards,**  
**so that the plugin never causes audio dropouts or interrupts the user's creative flow.**

#### Acceptance Criteria

1. All audio processing code is real-time safe with no blocking operations
2. Lock-free communication between UI thread and audio thread using FIFOs
3. Memory allocation on audio thread is eliminated or pre-allocated
4. Audio processing maintains consistent performance under CPU stress testing
5. Plugin passes real-time safety validation in audio testing frameworks
6. No mutex locks or blocking operations in audio callback paths

### Story 2.5: Pattern Playback and Transport Control

**As a music producer,**  
**I want to control pattern playback with start/stop buttons and looping,**  
**so that I can evaluate generated patterns in context with my project.**

#### Acceptance Criteria

1. Play/stop buttons control pattern preview playback independently of DAW transport
2. Pattern playback can loop continuously for extended evaluation
3. Playback position indicator shows current location within the pattern
4. Pattern playback synchronizes with DAW transport when enabled
5. Multiple patterns can be queued for sequential playback comparison
6. Playback responds immediately to user controls without latency

## Checklist Results Report

### PM Checklist Validation - Executive Summary

**Overall PRD Completeness:** 85% ✅  
**MVP Scope Appropriateness:** Just Right ✅  
**Readiness for Architecture Phase:** Ready ✅  

### Category Analysis Results

| Category                         | Status | Critical Issues |
| -------------------------------- | ------ | --------------- |
| 1. Problem Definition & Context  | PARTIAL | Missing quantifiable success metrics, limited user research documentation |
| 2. MVP Scope Definition          | PASS | Well-defined scope with clear boundaries, appropriate complexity |
| 3. User Experience Requirements  | PASS | Comprehensive UI goals, interaction paradigms clearly defined |
| 4. Functional Requirements       | PASS | Clear, testable requirements with consistent terminology |
| 5. Non-Functional Requirements   | PASS | Specific performance constraints, platform requirements well-defined |
| 6. Epic & Story Structure        | PASS | Sequential epics with clear value delivery, well-sized stories |
| 7. Technical Guidance            | PASS | Clear technical constraints and framework decisions |
| 8. Cross-Functional Requirements | PARTIAL | Data requirements could be more explicit for AI model integration |
| 9. Clarity & Communication       | PASS | Well-structured, professional documentation |

### Key Strengths
- Crystal clear technical requirements and constraints
- Well-sequenced epic structure enabling incremental delivery  
- Appropriate MVP scope balancing ambition with achievability
- Professional-grade functional and non-functional requirements
- Comprehensive UI/UX vision with "sophisticated simplicity" approach

### Recommendations for Future Enhancement
- Add quantifiable success metrics (downloads, user satisfaction scores)
- Document AI model training data requirements more explicitly
- Consider expanding competitive analysis section

### Final Assessment: **READY FOR ARCHITECT PHASE**

The PRD provides an excellent foundation for technical architecture design with all essential requirements clearly defined.

## Next Steps

### UX Expert Prompt

"Please review the SpawnClone PRD and create a comprehensive UI/UX design specification. Focus on the sophisticated simplicity vision, ensuring the interface enables rapid creative workflow while maintaining professional audio software standards. Pay special attention to the drag-and-drop export functionality, pattern history management, and real-time parameter adjustment workflows."

### Architect Prompt

"Please review the SpawnClone PRD and create a detailed technical architecture document. Focus on the modular plugin architecture with real-time audio thread safety, a custom rule-based AI engine, and cross-platform JUCE implementation. Ensure the architecture supports all functional and non-functional requirements while maintaining the specified performance constraints."
