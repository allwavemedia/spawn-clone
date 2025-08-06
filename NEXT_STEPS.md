# BMad Orchestrator: ONNX Runtime C++ Integration - Next Steps

## Status: Root Cause Identified ✅

**CRITICAL FINDING**: Segmentation faults occur even with simple models and C API calls, indicating a fundamental issue with the Homebrew ONNX Runtime installation on this macOS ARM64 system.

## Immediate Solutions (Choose One):

### Option 1: Fix Homebrew Installation (Recommended)
```bash
# Uninstall current ONNX Runtime
brew uninstall onnxruntime onnx

# Clean Homebrew cache
brew cleanup

# Reinstall with specific version
brew install onnxruntime

# Alternative: Try different version
# brew install onnxruntime@1.21.0
```

### Option 2: Manual Installation from Official Release
```bash
# Download official ARM64 release
curl -L "https://github.com/microsoft/onnxruntime/releases/download/v1.22.1/onnxruntime-osx-arm64-1.22.1.tgz" -o onnxruntime.tgz
tar -xzf onnxruntime.tgz

# Update CMakeLists.txt to point to manual installation
# set(ONNXRUNTIME_ROOT_PATH "/path/to/onnxruntime-osx-arm64-1.22.1")
```

### Option 3: Python Subprocess Workaround (Immediate)
- Keep using Python ONNX Runtime (which works perfectly)
- Create C++ wrapper that calls Python script via subprocess
- Temporary solution while fixing C++ integration

## Validation Tests Created:

1. **test_c_api.cpp** - Tests ONNX Runtime C API directly
2. **test_simple_model.cpp** - Tests with synthetic ONNX models
3. **test_midi_model_proper.cpp** - Improved MIDI model loading
4. **Python validation** - Confirms models work in Python

## Key Findings:

- ✅ Models are valid (821MB base, 115MB token)
- ✅ Python ONNX Runtime 1.22.1 works perfectly
- ❌ C/C++ ONNX Runtime causes segfaults immediately
- ❌ Issue exists with ANY model (simple synthetic → complex MIDI)
- ❌ Issue exists with both C and C++ APIs

## Next Iteration Steps:

1. **Choose solution approach** (Homebrew fix vs manual install vs Python subprocess)
2. **Test with working ONNX Runtime C++ installation**
3. **Update ONNXModelManager.cpp** with proven working patterns
4. **Implement proper MIDI tokenization** (from skytnt/midi-model research)
5. **Add inference pipeline** for music generation

## Files Ready for Next Phase:

- `test_c_api.cpp` - C API test framework
- `test_midi_model_proper.cpp` - Improved model loading
- `ONNXModelManager.cpp` - Needs update with working patterns
- Python validation scripts - Working reference implementation

## Recommendations:

**Immediate**: Try Option 1 (Homebrew reinstall) first, then Option 2 if needed.
**Short-term**: Option 3 (Python subprocess) for rapid prototyping.
**Long-term**: Proper C++ integration once runtime is stable.

---
*Status: Ready to continue iteration with runtime installation fix*
