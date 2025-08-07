# E2E Test Data Management for SpawnClone

This directory contains test data and reference files for end-to-end testing of the SpawnClone audio pipeline.

## Directory Structure

### `/audio/`
- **reference/** - Reference audio files for output comparison
- **test_signals/** - Generated test signals for audio analysis
- **corrupted/** - Corrupted files for error handling tests

### `/midi/`
- **patterns/** - Test MIDI patterns categorized by complexity
- **sequences/** - Complete MIDI sequences for testing
- **malformed/** - Invalid MIDI files for error testing

### `/onnx/`
- **test_models/** - Lightweight ONNX models for testing
- **mock_models/** - Mock models for specific test scenarios
- **corrupted_models/** - Invalid models for error handling

### `/presets/`
- **test_presets/** - Instrument presets for testing
- **validation_presets/** - Presets for output validation

## Usage

Test data files are automatically loaded by the `TestDataManager` class. Files are organized by category and can be accessed using descriptive names:

```cpp
auto testData = TestDataManager::getInstance();
auto hipHopPattern = testData.getTestPattern("simple", "hip_hop");
auto referenceAudio = testData.getAudioReference("hip_hop", "basic_drum_pattern");
```

## File Naming Convention

- **Audio files**: `{genre}_{pattern_type}_{variation}.wav`
- **MIDI files**: `{complexity}_{genre}_{pattern_id}.mid`
- **ONNX models**: `{model_type}_{complexity}.onnx`
- **Presets**: `{instrument}_{style}_preset.json`

## Test Data Generation

Test data can be generated using the SpawnClone application in test mode:

1. Generate patterns with known parameters
2. Export audio with consistent settings
3. Save reference files with descriptive names
4. Document expected characteristics

This ensures consistent and reproducible test results across different development environments.
