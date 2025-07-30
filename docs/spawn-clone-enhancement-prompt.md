✅ AI Coding Agent Prompt: Enhance SpawnClone with Advanced Spawn-Inspired Features & Hybrid AI Research
Note: This prompt now includes a research-driven extension to explore cloud and open-source AI models, which may lead to a revised architecture.

1. High-Level Goal
Enhance the SpawnClone plugin with advanced usability, workflow, and architectural features inspired by Sauceware’s Spawn — specifically: Instrument Mode, Pattern History Persistence, and User-Controlled Preview Behavior — while exploring the feasibility of integrating cloud-based or open-source AI models to expand creative capabilities.

The goal is to elevate SpawnClone from a functional MVP to a professional-grade tool that matches the polished user experience of Spawn, with optional pathways for AI model augmentation — while preserving core principles of performance, stability, and user control.

2. Step-by-Step Instructions
Understand the Current Architecture
Read and analyze:
architecture.md – Understand the layered design, thread model, and component interfaces.
backlog.md – Review current epics and user stories.
prd.md – Confirm functional and non-functional requirements.
Identify where new features fit into the existing epic structure (e.g., Epic 3: UI & Pattern Management).
Implement “Instrument Mode”
Add a new toggle state in PluginState called isInstrumentMode.
When enabled, route incoming DAW MIDI through the AudioPreviewEngine without triggering AI generation.
Modify processBlock to accept and process MidiBuffer input.
Add a visual LED indicator in the UI.
Rationale: Prevents double-triggering when dragging MIDI back to the DAW.
Enhance Pattern History System
Expand PatternManager to support history navigation (undoLastGeneration, getRecentPatterns).
Serialize up to 50 patterns using ValueTree.
Add Previous/Next buttons in the UI.
Rationale: Addresses user feedback about losing ideas.
Add User-Controlled Preview Behavior
Add autoPlayOnGenerate (default: true) to PluginState.
Add a UI checkbox in settings.
Modify generation workflow to respect this setting.
Rationale: Prevents disruption to ongoing DAW playback.
Update Serialization and State Management
Ensure all new fields (isInstrumentMode, autoPlayOnGenerate, extended history) are properly serialized.
Extend Unit Testing
Add GoogleTest cases for:
undoLastGeneration() and bounds checking.
MIDI passthrough in Instrument Mode.
autoPlayOnGenerate toggle logic.
Update Documentation
Update backlog.md with new user stories under Epic 3.
Update SpawnClone_User_Guide.md to document new features.
NEW: Research AI Cloud & Open-Source Model Integration
Objective: Investigate how AI cloud APIs and open-source models could augment (not replace) the current rule-based engine.
Tasks:
Research Cloud AI APIs:
Investigate providers like Google Cloud Vertex AI, AWS SageMaker, Replicate, and RunPod.
Focus on models suitable for MIDI generation, music style transfer, or melodic completion.
Evaluate:
Latency and real-time viability
Cost per inference
Data privacy implications
Integration complexity (REST/gRPC)
Licensing and EULA restrictions
Research Open-Source Models:
Search Hugging Face and GitHub for models in the music-generation, MIDI, Transformer, or Diffusion domains.
Prioritize models with:
Permissive licenses (MIT, Apache 2.0, CC-BY)
Pre-trained weights available
Inference support in C++ or via ONNX/TFLite
Community activity and documentation
Examples to investigate:
MusicGen (Meta)
Jukebox (OpenAI)
MuseNet-style models
Transformer-based MIDI generators
Architectural Analysis:
Propose three integration pathways:
On-Device Only (Current) – Rule-based, no external dependencies.
Optional Cloud Mode – User enables cloud AI, with opt-in data policy.
Downloadable Models – User downloads and manages .tflite or .onnx models locally.
For each, analyze:
Impact on AIGenerationEngine interface
New dependencies and build requirements
UI/UX implications (e.g., network status, download progress)
Security and privacy safeguards
Update Constraints Section:
Re-evaluate current constraints:
❌ “DO NOT introduce TensorFlow, PyTorch, or network dependencies” → May be relaxed for optional modes.
❌ “DO NOT alter core AI engine logic” → May be extended, not replaced.
Propose updated constraints that allow opt-in, modular AI expansion without compromising the core offline experience.
Deliverables:
A short research report (200–300 words) summarizing findings.
A comparison table of top 3 models/APIs.
Updated architecture diagram (Mermaid) showing optional AI pathways.
New user story for “Cloud/AI Model Preferences” under a new Epic 6: Extended AI Capabilities.
3. Code Examples, Data Structures & Constraints
Relevant Files to Review:

architecture.md – For current architecture
backlog.md – For user story alignment
SpawnClone_UI_UX_Design_Review.md – For UI/UX guidance
team-fullstack.txt – For process and agent suitability
Data Models to Extend:

cpp
Copy Code
// In PluginState.h
struct PluginState {
    std::vector<MIDIPattern> patternHistory;
    int currentHistoryIndex;
    bool isInstrumentMode;
    bool autoPlayOnGenerate;
    bool useCloudAI;               // NEW
    String selectedLocalModel;     // NEW
    // ... other fields
};
Constraints (Updated):

✅ DO preserve rule-based engine as default and fallback.
✅ DO make cloud/model features opt-in with clear user consent.
✅ DO isolate external dependencies behind interfaces (e.g., IAIProvider).
✅ DO implement timeout and error fallbacks for network calls.
❌ DO NOT require internet for core functionality.
❌ DO NOT send MIDI data to cloud without explicit user approval.
4. Define a Strict Scope
Files You Can Modify:

PluginProcessor.cpp/h
PluginEditor.cpp/h
PatternManager.cpp/h
PluginState.h
SpawnClone_User_Guide.md
backlog.md
Files to Leave Unchanged:

AIGenerationEngine.cpp/h – Do not rewrite; extend via interface.
AudioPreviewEngine.cpp/h – Only extend for MIDI input.
Any third-party libraries unless part of a new opt-in feature.
Target Completion:

One pull request for feature enhancements.
One research document (docs/research/ai-model-integration.md) for review.
New user story added to backlog.