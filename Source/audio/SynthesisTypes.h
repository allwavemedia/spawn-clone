#pragma once

namespace spawnclone::audio
{
    // Forward declaration for synthesis type
    enum class SynthesisType
    {
        Wavetable = 0,
        Subtractive = 1,
        Sample = 2,
        Hybrid = 3
    };
    
    // Forward declaration for synthesis parameters - complete definition in AdvancedSynthesisEngine.h
    struct SynthesisParameters;
    
    // Common synthesis parameter types that can be shared across components
    // without creating circular dependencies
    
    struct SampleParams
    {
        enum class LoopMode
        {
            OneShot,
            Forward,
            PingPong
        };
        
        // Basic sample parameters
        int sampleIndex = 0;
        float rootNote = 60.0f;
        float pitchShiftRange = 24.0f;
        float startOffset = 0.0f;
        float playbackSpeed = 1.0f;
        
        // Loop parameters
        bool enableLooping = false;
        LoopMode loopMode = LoopMode::Forward;
        int loopStart = 0;
        int loopEnd = -1;
        float loopCrossfade = 10.0f;
        
        // Playback parameters
        bool reversePlayback = false;
        
        // Velocity layers
        bool enableVelocityLayers = false;
        float velocityCrossfadeWidth = 10.0f;
        
        // Sample map entry structure for velocity layers
        struct SampleMapEntry
        {
            int sampleIndex = 0;
            int minVelocity = 0;
            int maxVelocity = 127;
        };
        
        std::vector<SampleMapEntry> sampleMap;
    };
}
