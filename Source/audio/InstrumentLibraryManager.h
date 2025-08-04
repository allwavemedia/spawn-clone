/*
  ==============================================================================

    InstrumentLibraryManager.h
    Created: 3 Aug 2025
    Author:  Epic 9 Story 9.1 Implementation

    Comprehensive instrument preset library management system for SpawnClone.
    Provides categorized presets with tag-based filtering and automatic pairing.

  ==============================================================================
*/

#pragma once

#include <juce_core/juce_core.h>
#include <juce_data_structures/juce_data_structures.h>
#include "GenerationParameters.h"

//==============================================================================
/**
 * Epic 9 Story 9.1: Curated Instrument Library Manager
 * 
 * Manages a comprehensive library of professional instrument presets organized
 * by categories and tags, with automatic pairing based on generation parameters.
 */
class InstrumentLibraryManager
{
public:
    //==============================================================================
    struct PresetData
    {
        juce::String presetId;           // Unique identifier
        juce::String name;               // Display name
        juce::String category;           // Bass, Bell, Brass, Guitar, etc.
        juce::StringArray tags;          // Analog, Dirty, Lush, Spacey, etc.
        juce::String description;        // User-readable description
        
        // Synthesis Parameters
        struct SynthParams
        {
            // ADSR Envelope
            float attack = 0.01f;        // 0.001 to 5.0 seconds
            float decay = 0.1f;          // 0.001 to 5.0 seconds  
            float sustain = 0.7f;        // 0.0 to 1.0 level
            float release = 0.3f;        // 0.001 to 10.0 seconds
            
            // Filter Parameters
            float filterCutoff = 1.0f;   // 0.0 to 1.0 normalized
            float filterResonance = 0.1f; // 0.0 to 1.0
            bool useFilter = false;      // Enable/disable filter
            
            // Oscillator Parameters  
            int waveform = 0;            // 0=Sine, 1=Saw, 2=Square, 3=Triangle
            float detuneAmount = 0.0f;   // -12.0 to +12.0 semitones
            float pulseWidth = 0.5f;     // 0.1 to 0.9 (for square wave)
            
            // Modulation Parameters
            float lfoRate = 2.0f;        // 0.1 to 20.0 Hz
            float lfoDepth = 0.0f;       // 0.0 to 1.0
            int lfoTarget = 0;           // 0=None, 1=Pitch, 2=Filter, 3=Amplitude
            
            // Character Parameters
            float brightness = 0.5f;     // 0.0 to 1.0 (high frequency emphasis)
            float warmth = 0.5f;         // 0.0 to 1.0 (low frequency emphasis)
            float saturation = 0.0f;     // 0.0 to 1.0 (harmonic distortion)
        } synthParams;
        
        // Metadata
        juce::String author = "SpawnClone";
        juce::String version = "1.0";
        bool isUserPreset = false;       // vs. factory preset
        
        // Serialization support
        juce::ValueTree toValueTree() const;
        static PresetData fromValueTree(const juce::ValueTree& tree);
    };
    
    //==============================================================================
    InstrumentLibraryManager();
    ~InstrumentLibraryManager();
    
    //==============================================================================
    // Library Management
    
    /** Load preset library from embedded resources */
    void loadPresetLibrary();
    
    /** Get all available categories */
    juce::StringArray getAvailableCategories() const;
    
    /** Get all available tags */
    juce::StringArray getAvailableTags() const;
    
    /** Get total number of presets */
    int getTotalPresetCount() const { return presets.size(); }
    
    //==============================================================================
    // Preset Query Interface
    
    /** Get all presets for a specific category */
    juce::Array<PresetData> getPresetsForCategory(const juce::String& category) const;
    
    /** Get presets matching specific tags */
    juce::Array<PresetData> getPresetsForTags(const juce::StringArray& tags) const;
    
    /** Get presets matching search text */
    juce::Array<PresetData> searchPresets(const juce::String& searchText) const;
    
    /** Get specific preset by ID */
    PresetData getPreset(const juce::String& presetId) const;
    
    /** Check if preset exists */
    bool hasPreset(const juce::String& presetId) const;
    
    //==============================================================================
    // Automatic Pairing System
    
    /** Get recommended preset based on generation parameters */
    PresetData getRecommendedPreset(GenerationParameters::GenerationType type,
                                   const juce::StringArray& styleTags = {}) const;
    
    /** Get multiple recommended presets for variety */
    juce::Array<PresetData> getRecommendedPresets(GenerationParameters::GenerationType type,
                                                  int maxResults = 5) const;
    
    //==============================================================================
    // User Preset Management
    
    /** Add user-created preset */
    bool addUserPreset(const PresetData& preset);
    
    /** Remove user preset */
    bool removeUserPreset(const juce::String& presetId);
    
    /** Save user presets to disk */
    bool saveUserPresets();
    
    /** Load user presets from disk */
    bool loadUserPresets();
    
    //==============================================================================
    // Library Statistics
    
    struct LibraryStats
    {
        int totalPresets = 0;
        int factoryPresets = 0;
        int userPresets = 0;
        juce::StringArray categories;
        juce::StringArray tags;
    };
    
    LibraryStats getLibraryStatistics() const;

private:
    //==============================================================================
    // Internal Data
    
    juce::Array<PresetData> presets;
    juce::StringArray categories;
    juce::StringArray availableTags;
    
    // Performance optimization caches
    mutable juce::HashMap<juce::String, juce::Array<PresetData>> categoryCache;
    mutable juce::HashMap<juce::String, juce::Array<PresetData>> tagCache;
    
    //==============================================================================
    // Internal Methods
    
    /** Load factory presets from embedded JSON */
    void loadFactoryPresets();
    
    /** Create a factory preset programmatically */
    void createFactoryPreset(const juce::String& id, const juce::String& name,
                           const juce::String& category, const juce::StringArray& tags,
                           const juce::String& description, int waveform,
                           float attack, float decay, float sustain, float release,
                           float cutoff, float resonance, bool useFilter);
    
    /** Parse preset data from JSON */
    PresetData parsePresetFromJson(const juce::var& jsonData) const;
    
    /** Build category and tag indices */
    void buildIndices();
    
    /** Clear all caches */
    void clearCaches() const;
    
    /** Get user presets directory */
    juce::File getUserPresetsDirectory() const;
    
    /** Validate preset data */
    bool validatePreset(const PresetData& preset) const;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(InstrumentLibraryManager)
};
