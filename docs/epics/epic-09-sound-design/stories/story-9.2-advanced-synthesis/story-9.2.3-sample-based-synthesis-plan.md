# Epic 9.2 Story 9.2.3 – **Sample-Based Synthesis** Technical Plan  
**Author:** BMad Dev Agent ‑ Full-Stack Developer  
**Date:** 2025-08-05  

---

## 1 🎯 Objectives  
1. Implement a **multi-sample engine** with velocity-layer support.  
2. Provide **real-time pitch-shifting & time-stretching** (±24 semitones min).  
3. Deliver **seamless loop playback** (Forward / Ping-Pong / One-Shot) with cross-fade.  
4. Add **memory-efficient streaming & SSD/RAM caching**.  
5. Integrate with existing **AdvancedSynthesisEngine**, voice allocator, UI & preset system.  
6. Achieve **professional audio quality** while keeping CPU < 4 % per voice.  

---

## 2 📋 Requirements Recap  
| Area | Requirement | Source Doc | Acceptance Criteria |  
|------|-------------|------------|---------------------|  
| Velocity Layers | Up-to 8 layers, 0-127 velocity mapping, cross-fade width | Workflow Plan §75-83 | Natural velocity transitions, no audible jumps |  
| Pitch/Time | Pitch shift ±24 st, playbackSpeed 0.5-2.0× | Workflow Plan §76-79 | < 1 ms param latency, < 0.5 cent error |  
| Looping | Start/End, Forward/Ping-Pong, Crossfade ms | Story 9.2.2 Report §206-210 | No clicks at loop points |  
| Caching | RAM pool + async preload on voice start | Epic Plan §78-80 | ≤ 30 ms max sample fetch |  

---

## 3 🏗️ Architectural Overview  
```
AdvancedSynthesisEngine
 └─ SynthVoice
     └─ SampleEngine
         ├─ PitchShifter
         ├─ LoopManager
         ├─ SampleCache (NEW – shared)
         └─ AsyncSampleLoader (NEW)
```

### 3.1 Key Modules  
| Class | File | Role |  
|-------|------|------|  
| `SampleCache` | `Source/audio/SampleCache.h/.cpp` | Global LRU cache, mmap’d streaming, ref counted buffers |  
| `AsyncSampleLoader` | `Source/audio/AsyncSampleLoader.h/.cpp` | Background thread pool, pre-loads missing samples into `SampleCache` |  
| `SampleParams` (extend) | `.h` | Add `preloadHint`, `streamingMode {Auto,RAM,Disk}` |  
| `SampleEngine` (extend) | `.cpp` | Hook into cache, cross-fade, oversample interpolation |  

---

## 4 🗂️ New Files  
1. `Source/audio/SampleCache.h/.cpp`  
2. `Source/audio/AsyncSampleLoader.h/.cpp`  
3. *Unit tests*:  
   * `tests/unit/test_SampleCache.cpp`  
   * `tests/unit/test_SampleEngine.cpp` (velocity-layer & looping)  
4. *Integration tests*:  
   * `Source/test_SampleBasedSynthesis.cpp` (extend placeholder)  

---

## 5 🔥 Sample Caching Strategy  
* **Chunk size:** 256 kB blocks (≈ 5 762 samples @44.1 kHz float mono).  
* **LRU capacity:** Configurable (64 MB default).  
* **Pre-load:** `preloadHint` (%) loads initial seconds into RAM on preset switch.  
* **Disk stream:** Memory-map large .wav files via JUCE FileInputStream.  
* **Threading:** `AsyncSampleLoader` uses JUCE ThreadPool (2 workers).  

---

## 6 🎛️ Pitch / Time Processing  
* **Alg:** Simple ratio resample (high-quality sinc 8-tap) for MVP.  
* **Future:** Replace with OTA-LPP or Elastique once ONNX runtime proven.  
* **Param smoothing:** 5 ms slew limiter.  

---

## 7 🔁 Loop Manager Enhancements  
* Cross-fade window converted from ms → samples per rate.  
* Ping-Pong keeps internal `playDirection`, no `while` loops in audio thread.  
* Detect zero-crossing fallback if user loop invalid.  

---

## 8 🔌 Integration Steps  
1. **Preset schema** – extend `InstrumentLibraryManager::PresetData::SampleLayer`.  
2. **UI** – expose sample-specific knobs in `PresetBrowserComponent`.  
3. **AudioPreviewEngine** – allow `SynthesisType::Sample` preview.  
4. **CMake** – add new files & test targets.  
5. **Serialization** – update JSON save/load paths.  

---

## 9 🧪 Test Plan  
| Test | Type | Description | Pass |  
|------|------|-------------|------|  
| Velocity Layer Selection | Unit | 0-127 velocity triggers correct buffer & cross-fade | 0/1 |  
| Pitch Ratio Accuracy | Unit | ±24 st within 0.5 cent error | 0/1 |  
| Loop Seam | Unit | RMS(diff) < -70 dB between loop points | 0/1 |  
| Cache LRU Evict | Unit | Old blocks evicted after cap | 0/1 |  
| Streaming Latency | Integration | First read < 30 ms from cold | 0/1 |  
| CPU Load 8 voices | Perf | < 4 % on M-series baseline | 0/1 |  

---

## 10 ⚡ Performance Considerations  
* **Lock-free** read path inside audio thread; atomic ref counts.  
* Re-use `juce::dsp::LagrangeInterpolator` where possible.  
* **SIMD** cross-fade (`juce::FloatVectorOperations::multiplyAdd`).  
* Avoid heap alloc in `noteOn` once cache hit.  

---

## 11 📆 Timeline / Milestones  
| Day | Task | Output |  
|-----|------|--------|  
| 1 | Implement `SampleCache`, unit test | Cache compiles & tests green |  
| 2 | Extend `SampleEngine` → cache integration | Plays first sample layer |  
| 3 | Velocity layers + cross-fade | Layer tests pass |  
| 4 | LoopManager upgrades | Seamless loops verified |  
| 5 | Async loader + streaming perf | Streaming test pass |  

---

## 12 🚧 Risks & Mitigations  
* **Cache thrash** on small RAM setups → expose max MB preference.  
* **Pitch-shift quality** → plan upgrade to HQ alg in Epic 10.  
* **Large sample I/O** on HDD → fallback to RAM-only mode.  

---

## 13 ✅ Definition of Done  
1. All unit/integration tests pass (section 9).  
2. CMake builds with no warnings (`-Wall ‑Wextra ‑Werror`).  
3. CPU & memory metrics meet gates.  
4. Documentation updated (this file + API comments).  

---

*Prepared for implementation.*  