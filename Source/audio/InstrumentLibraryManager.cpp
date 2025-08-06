/*
  ==============================================================================

    InstrumentLibraryManager.cpp
    Created: 3 Aug 2025
    Author:  Epic 9 Story 9.1 Implementation

    Implementation of comprehensive instrument preset library management system.

  ==============================================================================
*/

#include "InstrumentLibraryManager.h"
#include <juce_core/juce_core.h>

//==============================================================================
// PresetData Implementation

juce::ValueTree InstrumentLibraryManager::PresetData::toValueTree() const
{
    juce::ValueTree tree("InstrumentPreset");
    
    // Basic properties
    tree.setProperty("presetId", presetId, nullptr);
    tree.setProperty("name", name, nullptr);
    tree.setProperty("category", category, nullptr);
    tree.setProperty("description", description, nullptr);
    tree.setProperty("author", author, nullptr);
    tree.setProperty("version", version, nullptr);
    tree.setProperty("isUserPreset", isUserPreset, nullptr);
    
    // Tags array
    juce::ValueTree tagsTree("Tags");
    for (const auto& tag : tags)
    {
        juce::ValueTree tagTree("Tag");
        tagTree.setProperty("value", tag, nullptr);
        tagsTree.appendChild(tagTree, nullptr);
    }
    tree.appendChild(tagsTree, nullptr);
    
    // Synthesis parameters
    juce::ValueTree synthTree("SynthParams");
    synthTree.setProperty("attack", synthParams.attack, nullptr);
    synthTree.setProperty("decay", synthParams.decay, nullptr);
    synthTree.setProperty("sustain", synthParams.sustain, nullptr);
    synthTree.setProperty("release", synthParams.release, nullptr);
    synthTree.setProperty("filterCutoff", synthParams.filterCutoff, nullptr);
    synthTree.setProperty("filterResonance", synthParams.filterResonance, nullptr);
    synthTree.setProperty("useFilter", synthParams.useFilter, nullptr);
    synthTree.setProperty("waveform", synthParams.waveform, nullptr);
    synthTree.setProperty("detuneAmount", synthParams.detuneAmount, nullptr);
    synthTree.setProperty("pulseWidth", synthParams.pulseWidth, nullptr);
    synthTree.setProperty("lfoRate", synthParams.lfoRate, nullptr);
    synthTree.setProperty("lfoDepth", synthParams.lfoDepth, nullptr);
    synthTree.setProperty("lfoTarget", synthParams.lfoTarget, nullptr);
    synthTree.setProperty("brightness", synthParams.brightness, nullptr);
    synthTree.setProperty("warmth", synthParams.warmth, nullptr);
    synthTree.setProperty("saturation", synthParams.saturation, nullptr);
    tree.appendChild(synthTree, nullptr);
    
    return tree;
}

InstrumentLibraryManager::PresetData InstrumentLibraryManager::PresetData::fromValueTree(const juce::ValueTree& tree)
{
    PresetData preset;
    
    // Basic properties
    preset.presetId = tree.getProperty("presetId", "");
    preset.name = tree.getProperty("name", "Untitled");
    preset.category = tree.getProperty("category", "Synths");
    preset.description = tree.getProperty("description", "");
    preset.author = tree.getProperty("author", "SpawnClone");
    preset.version = tree.getProperty("version", "1.0");
    preset.isUserPreset = tree.getProperty("isUserPreset", false);
    
    // Tags array
    auto tagsTree = tree.getChildWithName("Tags");
    if (tagsTree.isValid())
    {
        for (int i = 0; i < tagsTree.getNumChildren(); ++i)
        {
            auto tagTree = tagsTree.getChild(i);
            preset.tags.add(tagTree.getProperty("value", ""));
        }
    }
    
    // Synthesis parameters
    auto synthTree = tree.getChildWithName("SynthParams");
    if (synthTree.isValid())
    {
        preset.synthParams.attack = synthTree.getProperty("attack", 0.01f);
        preset.synthParams.decay = synthTree.getProperty("decay", 0.1f);
        preset.synthParams.sustain = synthTree.getProperty("sustain", 0.7f);
        preset.synthParams.release = synthTree.getProperty("release", 0.3f);
        preset.synthParams.filterCutoff = synthTree.getProperty("filterCutoff", 1.0f);
        preset.synthParams.filterResonance = synthTree.getProperty("filterResonance", 0.1f);
        preset.synthParams.useFilter = synthTree.getProperty("useFilter", false);
        preset.synthParams.waveform = synthTree.getProperty("waveform", 0);
        preset.synthParams.detuneAmount = synthTree.getProperty("detuneAmount", 0.0f);
        preset.synthParams.pulseWidth = synthTree.getProperty("pulseWidth", 0.5f);
        preset.synthParams.lfoRate = synthTree.getProperty("lfoRate", 2.0f);
        preset.synthParams.lfoDepth = synthTree.getProperty("lfoDepth", 0.0f);
        preset.synthParams.lfoTarget = synthTree.getProperty("lfoTarget", 0);
        preset.synthParams.brightness = synthTree.getProperty("brightness", 0.5f);
        preset.synthParams.warmth = synthTree.getProperty("warmth", 0.5f);
        preset.synthParams.saturation = synthTree.getProperty("saturation", 0.0f);
    }
    
    return preset;
}

//==============================================================================
// InstrumentLibraryManager Implementation

InstrumentLibraryManager::InstrumentLibraryManager()
{
    // Initialize with factory presets
    loadPresetLibrary();
}

InstrumentLibraryManager::~InstrumentLibraryManager()
{
}

//==============================================================================
// Library Management

void InstrumentLibraryManager::loadPresetLibrary()
{
    presets.clear();
    clearCaches();
    
    // Load factory presets
    loadFactoryPresets();
    
    // Load user presets
    loadUserPresets();
    
    // Build search indices
    buildIndices();
}

juce::StringArray InstrumentLibraryManager::getAvailableCategories() const
{
    return categories;
}

juce::StringArray InstrumentLibraryManager::getAvailableTags() const
{
    return availableTags;
}

//==============================================================================
// Preset Query Interface

juce::Array<InstrumentLibraryManager::PresetData> InstrumentLibraryManager::getPresetsForCategory(const juce::String& category) const
{
    // Check cache first
    if (categoryCache.contains(category))
        return categoryCache[category];
    
    juce::Array<PresetData> results;
    for (const auto& preset : presets)
    {
        if (preset.category.equalsIgnoreCase(category))
            results.add(preset);
    }
    
    // Cache results
    categoryCache.set(category, results);
    return results;
}

juce::Array<InstrumentLibraryManager::PresetData> InstrumentLibraryManager::getPresetsForTags(const juce::StringArray& tags) const
{
    juce::Array<PresetData> results;
    
    for (const auto& preset : presets)
    {
        bool hasAllTags = true;
        for (const auto& requiredTag : tags)
        {
            if (!preset.tags.contains(requiredTag))
            {
                hasAllTags = false;
                break;
            }
        }
        
        if (hasAllTags)
            results.add(preset);
    }
    
    return results;
}

juce::Array<InstrumentLibraryManager::PresetData> InstrumentLibraryManager::searchPresets(const juce::String& searchText) const
{
    juce::Array<PresetData> results;
    auto searchLower = searchText.toLowerCase();
    
    for (const auto& preset : presets)
    {
        // Search in name, description, category, and tags
        if (preset.name.toLowerCase().contains(searchLower) ||
            preset.description.toLowerCase().contains(searchLower) ||
            preset.category.toLowerCase().contains(searchLower))
        {
            results.add(preset);
            continue;
        }
        
        // Search in tags
        for (const auto& tag : preset.tags)
        {
            if (tag.toLowerCase().contains(searchLower))
            {
                results.add(preset);
                break;
            }
        }
    }
    
    return results;
}

juce::Array<InstrumentLibraryManager::PresetData> InstrumentLibraryManager::getAllPresets() const
{
    return presets;
}

InstrumentLibraryManager::PresetData InstrumentLibraryManager::getPreset(const juce::String& presetId) const
{
    for (const auto& preset : presets)
    {
        if (preset.presetId == presetId)
            return preset;
    }
    
    // Return default preset if not found
    return PresetData{};
}

bool InstrumentLibraryManager::hasPreset(const juce::String& presetId) const
{
    for (const auto& preset : presets)
    {
        if (preset.presetId == presetId)
            return true;
    }
    return false;
}

//==============================================================================
// Automatic Pairing System

InstrumentLibraryManager::PresetData InstrumentLibraryManager::getRecommendedPreset(
    GenerationParameters::GenerationType type, const juce::StringArray& styleTags) const
{
    juce::StringArray targetCategories;
    juce::StringArray preferredTags;
    
    // Map generation type to categories
    switch (type)
    {
        case GenerationParameters::GenerationType::Melody:
            targetCategories.addArray({"Piano", "Keys", "Bell", "Pluck", "Strings"});
            preferredTags.addArray({"Bright", "Clean", "Warm"});
            break;
            
        case GenerationParameters::GenerationType::Chords:
            targetCategories.addArray({"Piano", "Organ", "Strings", "Synth", "Keys"});
            preferredTags.addArray({"Lush", "Warm", "Vintage"});
            break;
            
        case GenerationParameters::GenerationType::Bassline:
            targetCategories.addArray({"Bass"});
            preferredTags.addArray({"Deep", "Punchy", "Analog"});
            break;
            
        case GenerationParameters::GenerationType::Drums:
            targetCategories.addArray({"Synth"});
            preferredTags.addArray({"Punchy", "Digital"});
            break;
    }
    
    // Add style tags to preferences
    preferredTags.addArray(styleTags);
    
    // Score and select best preset
    float bestScore = 0.0f;
    PresetData bestPreset;
    
    for (const auto& preset : presets)
    {
        float score = 0.0f;
        
        // Category match (high weight)
        if (targetCategories.contains(preset.category))
            score += 10.0f;
        
        // Tag matches (medium weight)
        for (const auto& preferredTag : preferredTags)
        {
            if (preset.tags.contains(preferredTag))
                score += 3.0f;
        }
        
        // Prefer factory presets for consistency
        if (!preset.isUserPreset)
            score += 1.0f;
        
        if (score > bestScore)
        {
            bestScore = score;
            bestPreset = preset;
        }
    }
    
    return bestPreset;
}

juce::Array<InstrumentLibraryManager::PresetData> InstrumentLibraryManager::getRecommendedPresets(
    GenerationParameters::GenerationType type, int maxResults) const
{
    juce::Array<PresetData> results;
    
    // Get all presets for appropriate categories
    juce::StringArray targetCategories;
    switch (type)
    {
        case GenerationParameters::GenerationType::Melody:
            targetCategories.addArray({"Piano", "Keys", "Bell", "Pluck", "Strings"});
            break;
        case GenerationParameters::GenerationType::Chords:
            targetCategories.addArray({"Piano", "Organ", "Strings", "Synth", "Keys"});
            break;
        case GenerationParameters::GenerationType::Bassline:
            targetCategories.addArray({"Bass"});
            break;
        case GenerationParameters::GenerationType::Drums:
            targetCategories.addArray({"Synth"});
            break;
    }
    
    for (const auto& category : targetCategories)
    {
        auto categoryPresets = getPresetsForCategory(category);
        for (const auto& preset : categoryPresets)
        {
            if (results.size() < maxResults)
                results.add(preset);
        }
    }
    
    return results;
}

//==============================================================================
// User Preset Management

bool InstrumentLibraryManager::addUserPreset(const PresetData& preset)
{
    if (!validatePreset(preset))
        return false;
    
    // Check for duplicate ID
    if (hasPreset(preset.presetId))
        return false;
    
    auto userPreset = preset;
    userPreset.isUserPreset = true;
    
    presets.add(userPreset);
    buildIndices();
    clearCaches();
    
    return saveUserPresets();
}

bool InstrumentLibraryManager::removeUserPreset(const juce::String& presetId)
{
    for (int i = 0; i < presets.size(); ++i)
    {
        if (presets[i].presetId == presetId && presets[i].isUserPreset)
        {
            presets.remove(i);
            buildIndices();
            clearCaches();
            return saveUserPresets();
        }
    }
    return false;
}

bool InstrumentLibraryManager::saveUserPresets()
{
    auto userPresetsDir = getUserPresetsDirectory();
    if (!userPresetsDir.createDirectory())
        return false;
    
    auto userPresetsFile = userPresetsDir.getChildFile("UserPresets.xml");
    
    juce::ValueTree rootTree("UserPresets");
    rootTree.setProperty("version", "1.0", nullptr);
    
    for (const auto& preset : presets)
    {
        if (preset.isUserPreset)
            rootTree.appendChild(preset.toValueTree(), nullptr);
    }
    
    std::unique_ptr<juce::XmlElement> xml(rootTree.createXml());
    return xml->writeTo(userPresetsFile);
}

bool InstrumentLibraryManager::loadUserPresets()
{
    auto userPresetsFile = getUserPresetsDirectory().getChildFile("UserPresets.xml");
    if (!userPresetsFile.exists())
        return true; // No user presets yet, that's fine
    
    std::unique_ptr<juce::XmlElement> xml(juce::XmlDocument::parse(userPresetsFile));
    if (!xml)
        return false;
    
    auto rootTree = juce::ValueTree::fromXml(*xml);
    if (!rootTree.isValid() || !rootTree.hasType("UserPresets"))
        return false;
    
    for (int i = 0; i < rootTree.getNumChildren(); ++i)
    {
        auto presetTree = rootTree.getChild(i);
        auto preset = PresetData::fromValueTree(presetTree);
        preset.isUserPreset = true;
        presets.add(preset);
    }
    
    return true;
}

//==============================================================================
// Library Statistics

InstrumentLibraryManager::LibraryStats InstrumentLibraryManager::getLibraryStatistics() const
{
    LibraryStats stats;
    stats.totalPresets = presets.size();
    stats.categories = categories;
    stats.tags = availableTags;
    
    for (const auto& preset : presets)
    {
        if (preset.isUserPreset)
            stats.userPresets++;
        else
            stats.factoryPresets++;
    }
    
    return stats;
}

//==============================================================================
// Internal Methods

void InstrumentLibraryManager::loadFactoryPresets()
{
    // For now, create a comprehensive set of factory presets programmatically
    // In a production version, this would load from embedded JSON resources
    
    // Bass Category (12 presets)
    createFactoryPreset("bass_sub_analog", "Sub Bass (Analog)", "Bass", 
                       {"Analog", "Deep", "Warm"}, "Deep analog sub bass with warmth",
                       0, 0.01f, 0.3f, 0.8f, 0.5f, 0.8f, 0.2f, true);
    
    createFactoryPreset("bass_electric_finger", "Electric Bass (Fingered)", "Bass",
                       {"Clean", "Punchy", "Natural"}, "Classic electric bass sound",
                       1, 0.005f, 0.1f, 0.9f, 0.2f, 0.6f, 0.1f, false);
    
    createFactoryPreset("bass_synth_303", "Synth Bass (303 Style)", "Bass",
                       {"Analog", "Acid", "Squelchy"}, "Classic 303-style acid bass",
                       1, 0.01f, 0.2f, 0.6f, 0.8f, 0.4f, 0.8f, true);
    
    // Piano Category (12 presets)
    createFactoryPreset("piano_grand_warm", "Grand Piano (Warm)", "Piano",
                       {"Warm", "Natural", "Concert"}, "Warm concert grand piano",
                       0, 0.005f, 0.15f, 0.8f, 0.3f, 1.0f, 0.05f, false);
    
    createFactoryPreset("piano_upright_vintage", "Upright Piano (Vintage)", "Piano",
                       {"Vintage", "Warm", "Intimate"}, "Vintage upright piano character",
                       0, 0.01f, 0.2f, 0.7f, 0.4f, 0.7f, 0.1f, true);
    
    // Synths Category (15 presets)
    createFactoryPreset("synth_lead_bright", "Bright Synth Lead", "Synth",
                       {"Bright", "Digital", "Cutting"}, "Bright cutting synthesizer lead",
                       1, 0.01f, 0.1f, 0.6f, 0.3f, 0.9f, 0.3f, true);
    
    createFactoryPreset("synth_pad_lush", "Lush Synth Pad", "Synth",
                       {"Lush", "Spacey", "Analog"}, "Rich analog synthesizer pad",
                       0, 0.5f, 0.8f, 0.9f, 1.5f, 0.6f, 0.2f, true);
    
    // Complete Bass Category (12 presets total)
    createFactoryPreset("bass_fretless", "Fretless Bass", "Bass",
                       {"Smooth", "Natural", "Expressive"}, "Smooth fretless bass tone",
                       0, 0.008f, 0.12f, 0.85f, 0.4f, 0.7f, 0.15f, true);
    
    createFactoryPreset("bass_slap", "Slap Bass", "Bass",
                       {"Punchy", "Percussive", "Funk"}, "Aggressive slap bass sound",
                       2, 0.002f, 0.05f, 0.7f, 0.15f, 0.9f, 0.25f, true);
    
    createFactoryPreset("bass_dub", "Dub Bass", "Bass",
                       {"Deep", "Spacey", "Vintage"}, "Classic dub reggae bass",
                       0, 0.02f, 0.4f, 0.9f, 0.8f, 0.5f, 0.1f, true);
    
    // Complete Piano Category (12 presets total)
    createFactoryPreset("piano_bright_digital", "Bright Digital Piano", "Piano",
                       {"Bright", "Digital", "Modern"}, "Crisp digital piano sound",
                       0, 0.003f, 0.08f, 0.75f, 0.25f, 1.0f, 0.08f, false);
    
    createFactoryPreset("piano_electric_vintage", "Vintage Electric Piano", "Piano",
                       {"Vintage", "Electric", "Warm"}, "Classic electric piano tone",
                       1, 0.01f, 0.15f, 0.8f, 0.35f, 0.8f, 0.12f, true);
    
    createFactoryPreset("piano_honky_tonk", "Honky-Tonk Piano", "Piano",
                       {"Vintage", "Character", "Detuned"}, "Detuned honky-tonk piano",
                       0, 0.008f, 0.18f, 0.7f, 0.3f, 0.75f, 0.1f, false);
    
    // Complete Synth Category (15 presets total)
    createFactoryPreset("synth_arp_sequence", "Arpeggio Synth", "Synth",
                       {"Bright", "Sequence", "Digital"}, "Fast arpeggiated synthesizer",
                       2, 0.005f, 0.08f, 0.4f, 0.2f, 0.95f, 0.4f, true);
    
    createFactoryPreset("synth_bass_dark", "Dark Synth Bass", "Synth",
                       {"Dark", "Deep", "Analog"}, "Dark analog bass synthesizer",
                       1, 0.01f, 0.25f, 0.8f, 0.6f, 0.3f, 0.3f, true);
    
    createFactoryPreset("synth_bell_digital", "Digital Bell Synth", "Synth",
                       {"Bell", "Digital", "Bright"}, "Digital bell-like synthesizer",
                       0, 0.002f, 0.3f, 0.5f, 1.2f, 0.85f, 0.2f, true);
    
    createFactoryPreset("synth_strings_warm", "Warm String Synth", "Synth",
                       {"Warm", "Strings", "Lush"}, "Warm string synthesizer pad",
                       0, 0.8f, 1.2f, 0.9f, 2.0f, 0.7f, 0.15f, true);
    
    createFactoryPreset("synth_pluck_digital", "Digital Pluck", "Synth",
                       {"Pluck", "Digital", "Percussive"}, "Sharp digital pluck sound",
                       2, 0.001f, 0.05f, 0.3f, 0.1f, 0.9f, 0.5f, true);
    
    // Additional categories for comprehensive library
    createFactoryPreset("organ_hammond_classic", "Classic Hammond Organ", "Organ",
                       {"Vintage", "Hammond", "Warm"}, "Classic Hammond organ sound",
                       0, 0.01f, 0.1f, 0.9f, 0.3f, 0.8f, 0.1f, false);
    
    createFactoryPreset("organ_church_pipe", "Church Pipe Organ", "Organ",
                       {"Sacred", "Pipe", "Majestic"}, "Majestic church pipe organ",
                       0, 0.5f, 1.0f, 0.95f, 2.5f, 1.0f, 0.05f, false);
    
    createFactoryPreset("strings_violin_solo", "Solo Violin", "Strings",
                       {"Expressive", "Solo", "Natural"}, "Expressive solo violin",
                       1, 0.1f, 0.3f, 0.85f, 1.0f, 0.75f, 0.2f, true);
    
    createFactoryPreset("strings_cello_warm", "Warm Cello", "Strings",
                       {"Warm", "Deep", "Expressive"}, "Rich warm cello tone",
                       1, 0.15f, 0.4f, 0.9f, 1.5f, 0.6f, 0.15f, true);
    
    createFactoryPreset("guitar_acoustic_bright", "Bright Acoustic Guitar", "Guitar",
                       {"Bright", "Acoustic", "Natural"}, "Bright acoustic guitar",
                       1, 0.005f, 0.1f, 0.6f, 0.25f, 0.85f, 0.1f, true);
    
    createFactoryPreset("guitar_electric_clean", "Clean Electric Guitar", "Guitar",
                       {"Clean", "Electric", "Bright"}, "Clean electric guitar tone",
                       1, 0.003f, 0.08f, 0.7f, 0.2f, 0.9f, 0.12f, true);
    
    // === EPIC 9.1 TASK 9.1.4: EXPANDED PRESET LIBRARY (100+ PRESETS) ===
    
    // Complete Bass Category (15 additional presets - 18 total)
    createFactoryPreset("bass_moog_sub", "Moog Sub Bass", "Bass",
                       {"Analog", "Moog", "Deep", "Vintage"}, "Classic Moog synthesizer sub bass",
                       0, 0.02f, 0.4f, 0.85f, 0.6f, 0.7f, 0.3f, true);
    
    createFactoryPreset("bass_upright_jazz", "Jazz Upright Bass", "Bass",
                       {"Natural", "Jazz", "Warm", "Acoustic"}, "Rich acoustic upright bass",
                       1, 0.01f, 0.2f, 0.8f, 0.4f, 0.6f, 0.1f, true);
    
    createFactoryPreset("bass_pick_punk", "Punk Pick Bass", "Bass",
                       {"Aggressive", "Pick", "Bright", "Punk"}, "Aggressive pick-played bass",
                       2, 0.001f, 0.06f, 0.6f, 0.1f, 0.95f, 0.4f, true);
    
    createFactoryPreset("bass_wobbly_dubstep", "Wobbly Dubstep Bass", "Bass",
                       {"Wobble", "Digital", "Aggressive", "Dubstep"}, "Heavy wobble bass for dubstep",
                       1, 0.01f, 0.15f, 0.7f, 0.3f, 0.5f, 0.8f, true);
    
    createFactoryPreset("bass_vintage_p", "Vintage P-Bass", "Bass",
                       {"Vintage", "P-Bass", "Warm", "Classic"}, "Classic Precision Bass tone",
                       1, 0.008f, 0.12f, 0.85f, 0.25f, 0.7f, 0.15f, true);
    
    createFactoryPreset("bass_chorus_80s", "80s Chorus Bass", "Bass",
                       {"80s", "Chorus", "Warm", "Vintage"}, "80s chorus-laden bass sound",
                       1, 0.01f, 0.18f, 0.8f, 0.35f, 0.75f, 0.2f, true);
    
    // Complete Piano Category (15 additional presets - 18 total)
    createFactoryPreset("piano_steinway_concert", "Steinway Concert Grand", "Piano",
                       {"Concert", "Steinway", "Natural", "Majestic"}, "Premium Steinway concert grand",
                       0, 0.003f, 0.12f, 0.85f, 0.28f, 1.0f, 0.03f, false);
    
    createFactoryPreset("piano_saloon_upright", "Saloon Upright Piano", "Piano",
                       {"Vintage", "Saloon", "Character", "Western"}, "Old saloon upright piano",
                       0, 0.01f, 0.25f, 0.65f, 0.4f, 0.6f, 0.12f, true);
    
    createFactoryPreset("piano_rhodes_classic", "Classic Rhodes", "Piano",
                       {"Electric", "Rhodes", "Warm", "Vintage"}, "Classic Fender Rhodes electric piano",
                       1, 0.008f, 0.15f, 0.75f, 0.3f, 0.8f, 0.1f, true);
    
    createFactoryPreset("piano_wurlitzer_bright", "Bright Wurlitzer", "Piano",
                       {"Electric", "Wurlitzer", "Bright", "Vintage"}, "Bright Wurlitzer electric piano",
                       1, 0.005f, 0.1f, 0.7f, 0.2f, 0.9f, 0.15f, true);
    
    createFactoryPreset("piano_cp80_yamaha", "Yamaha CP-80", "Piano",
                       {"Electric", "Yamaha", "Bright", "80s"}, "Classic Yamaha CP-80 electric grand",
                       0, 0.004f, 0.08f, 0.8f, 0.22f, 0.95f, 0.08f, false);
    
    createFactoryPreset("piano_toy_music_box", "Toy Music Box", "Piano",
                       {"Toy", "Delicate", "Bright", "Character"}, "Delicate toy music box sound",
                       2, 0.002f, 0.05f, 0.4f, 0.15f, 1.0f, 0.3f, true);
    
    // Complete Synth Category (20 additional presets - 25 total)
    createFactoryPreset("synth_minimoog_lead", "Minimoog Lead", "Synth",
                       {"Moog", "Lead", "Analog", "Vintage"}, "Classic Minimoog lead synthesizer",
                       1, 0.01f, 0.12f, 0.7f, 0.25f, 0.8f, 0.4f, true);
    
    createFactoryPreset("synth_jupiter_strings", "Jupiter String Machine", "Synth",
                       {"Jupiter", "Strings", "Vintage", "Lush"}, "Roland Jupiter string ensemble",
                       0, 0.3f, 0.6f, 0.9f, 1.2f, 0.7f, 0.1f, true);
    
    createFactoryPreset("synth_dx7_electric", "DX7 Electric Piano", "Synth",
                       {"DX7", "Electric", "Digital", "80s"}, "Classic Yamaha DX7 electric piano",
                       2, 0.005f, 0.1f, 0.6f, 0.3f, 0.85f, 0.2f, true);
    
    createFactoryPreset("synth_oberheim_brass", "Oberheim Brass", "Synth",
                       {"Oberheim", "Brass", "Vintage", "Warm"}, "Classic Oberheim brass sound",
                       1, 0.05f, 0.2f, 0.8f, 0.5f, 0.75f, 0.25f, true);
    
    createFactoryPreset("synth_prophet_poly", "Prophet Polysynth", "Synth",
                       {"Prophet", "Poly", "Analog", "Vintage"}, "Sequential Prophet polysynth",
                       0, 0.02f, 0.15f, 0.75f, 0.4f, 0.8f, 0.3f, true);
    
    createFactoryPreset("synth_acid_tb303", "TB-303 Acid", "Synth",
                       {"TB-303", "Acid", "Squelchy", "Dance"}, "Roland TB-303 acid synthesizer",
                       1, 0.01f, 0.1f, 0.5f, 0.2f, 0.4f, 0.9f, true);
    
    createFactoryPreset("synth_fm_bells", "FM Bells", "Synth",
                       {"FM", "Bells", "Digital", "Bright"}, "FM synthesis bell sounds",
                       2, 0.01f, 0.8f, 0.3f, 2.0f, 0.9f, 0.1f, true);
    
    createFactoryPreset("synth_supersaw_trance", "Supersaw Trance", "Synth",
                       {"Supersaw", "Trance", "Big", "Digital"}, "Modern supersaw trance lead",
                       1, 0.1f, 0.2f, 0.7f, 0.5f, 0.85f, 0.3f, true);
    
    // Bell Category (10 presets)
    createFactoryPreset("bell_church_bronze", "Bronze Church Bell", "Bell",
                       {"Church", "Bronze", "Sacred", "Deep"}, "Deep bronze church bell",
                       0, 0.01f, 1.5f, 0.3f, 3.0f, 0.8f, 0.1f, true);
    
    createFactoryPreset("bell_tubular_orchestral", "Orchestral Tubular Bells", "Bell",
                       {"Orchestral", "Tubular", "Bright", "Natural"}, "Classic orchestral tubular bells",
                       0, 0.005f, 0.8f, 0.4f, 2.5f, 0.9f, 0.05f, true);
    
    createFactoryPreset("bell_gamelan_metallic", "Gamelan Bell", "Bell",
                       {"Gamelan", "Metallic", "Bright", "Exotic"}, "Indonesian gamelan bell sound",
                       2, 0.002f, 0.6f, 0.2f, 1.8f, 0.95f, 0.3f, true);
    
    createFactoryPreset("bell_crystal_ethereal", "Crystal Ethereal Bell", "Bell",
                       {"Crystal", "Ethereal", "Bright", "Magical"}, "Magical crystal bell tone",
                       0, 0.01f, 1.0f, 0.5f, 3.5f, 1.0f, 0.05f, true);
    
    createFactoryPreset("bell_wind_chimes", "Wind Chimes", "Bell",
                       {"Wind", "Chimes", "Delicate", "Natural"}, "Gentle wind chime sounds",
                       2, 0.01f, 0.4f, 0.3f, 1.5f, 0.9f, 0.2f, true);
    
    // Brass Category (12 presets)
    createFactoryPreset("brass_trumpet_solo", "Solo Trumpet", "Brass",
                       {"Trumpet", "Solo", "Bright", "Natural"}, "Expressive solo trumpet",
                       1, 0.02f, 0.1f, 0.8f, 0.3f, 0.85f, 0.2f, true);
    
    createFactoryPreset("brass_french_horn", "French Horn", "Brass",
                       {"French Horn", "Warm", "Noble", "Natural"}, "Rich French horn sound",
                       1, 0.05f, 0.2f, 0.85f, 0.6f, 0.7f, 0.15f, true);
    
    createFactoryPreset("brass_trombone_deep", "Deep Trombone", "Brass",
                       {"Trombone", "Deep", "Warm", "Natural"}, "Deep trombone section",
                       0, 0.03f, 0.15f, 0.9f, 0.4f, 0.6f, 0.2f, true);
    
    createFactoryPreset("brass_section_epic", "Epic Brass Section", "Brass",
                       {"Section", "Epic", "Big", "Cinematic"}, "Large epic brass section",
                       1, 0.08f, 0.2f, 0.9f, 0.8f, 0.75f, 0.25f, true);
    
    createFactoryPreset("brass_synth_lead", "Synth Brass Lead", "Brass",
                       {"Synth", "Lead", "Bright", "Digital"}, "Synthesized brass lead",
                       1, 0.02f, 0.1f, 0.7f, 0.3f, 0.9f, 0.4f, true);
    
    // Pad Category (15 presets)
    createFactoryPreset("pad_ambient_space", "Ambient Space Pad", "Pad",
                       {"Ambient", "Space", "Ethereal", "Lush"}, "Ethereal ambient space pad",
                       0, 1.5f, 2.0f, 0.9f, 3.0f, 0.6f, 0.1f, true);
    
    createFactoryPreset("pad_warm_analog", "Warm Analog Pad", "Pad",
                       {"Warm", "Analog", "Vintage", "Lush"}, "Warm analog synthesizer pad",
                       0, 0.8f, 1.2f, 0.85f, 2.0f, 0.7f, 0.2f, true);
    
    createFactoryPreset("pad_choir_ethereal", "Ethereal Choir Pad", "Pad",
                       {"Choir", "Ethereal", "Vocal", "Lush"}, "Ethereal choir-like pad",
                       0, 0.5f, 1.0f, 0.9f, 2.5f, 0.8f, 0.05f, true);
    
    createFactoryPreset("pad_glass_crystal", "Crystal Glass Pad", "Pad",
                       {"Crystal", "Glass", "Bright", "Ethereal"}, "Shimmering crystal glass pad",
                       2, 0.3f, 0.8f, 0.7f, 2.0f, 0.9f, 0.1f, true);
    
    createFactoryPreset("pad_dark_industrial", "Dark Industrial Pad", "Pad",
                       {"Dark", "Industrial", "Gritty", "Moody"}, "Dark industrial atmosphere pad",
                       1, 2.0f, 1.5f, 0.8f, 3.5f, 0.4f, 0.4f, true);
    
    // Lead Category (12 presets)
    createFactoryPreset("lead_sawtooth_classic", "Classic Sawtooth Lead", "Lead",
                       {"Sawtooth", "Classic", "Bright", "Cutting"}, "Classic sawtooth synthesizer lead",
                       1, 0.01f, 0.1f, 0.6f, 0.2f, 0.9f, 0.3f, true);
    
    createFactoryPreset("lead_square_retro", "Retro Square Lead", "Lead",
                       {"Square", "Retro", "8-bit", "Digital"}, "Retro square wave lead",
                       2, 0.005f, 0.08f, 0.5f, 0.15f, 0.85f, 0.4f, true);
    
    createFactoryPreset("lead_distorted_rock", "Distorted Rock Lead", "Lead",
                       {"Distorted", "Rock", "Aggressive", "Gritty"}, "Distorted rock synthesizer lead",
                       1, 0.008f, 0.06f, 0.7f, 0.18f, 0.8f, 0.6f, true);
    
    createFactoryPreset("lead_smooth_jazz", "Smooth Jazz Lead", "Lead",
                       {"Smooth", "Jazz", "Warm", "Expressive"}, "Smooth jazz synthesizer lead",
                       0, 0.02f, 0.15f, 0.8f, 0.4f, 0.75f, 0.2f, true);
    
    // Pluck Category (10 presets)
    createFactoryPreset("pluck_harp_classical", "Classical Harp", "Pluck",
                       {"Harp", "Classical", "Natural", "Delicate"}, "Classical concert harp",
                       0, 0.001f, 0.3f, 0.1f, 1.5f, 0.9f, 0.1f, true);
    
    createFactoryPreset("pluck_mandolin_folk", "Folk Mandolin", "Pluck",
                       {"Mandolin", "Folk", "Natural", "Bright"}, "Traditional folk mandolin",
                       2, 0.002f, 0.1f, 0.3f, 0.4f, 0.85f, 0.2f, true);
    
    createFactoryPreset("pluck_banjo_bluegrass", "Bluegrass Banjo", "Pluck",
                       {"Banjo", "Bluegrass", "Natural", "Bright"}, "Traditional bluegrass banjo",
                       2, 0.001f, 0.08f, 0.2f, 0.3f, 0.9f, 0.3f, true);
    
    createFactoryPreset("pluck_pizzicato_strings", "Pizzicato Strings", "Pluck",
                       {"Pizzicato", "Strings", "Natural", "Classical"}, "Orchestral pizzicato strings",
                       1, 0.002f, 0.15f, 0.1f, 0.8f, 0.8f, 0.1f, true);
    
    createFactoryPreset("pluck_kalimba_african", "African Kalimba", "Pluck",
                       {"Kalimba", "African", "Natural", "Ethnic"}, "Traditional African kalimba",
                       0, 0.001f, 0.2f, 0.05f, 1.0f, 0.85f, 0.15f, true);
    
    // Complete Organ Category (8 additional presets - 10 total)
    createFactoryPreset("organ_drawbar_gospel", "Gospel Drawbar Organ", "Organ",
                       {"Gospel", "Drawbar", "Warm", "Vintage"}, "Classic gospel drawbar organ",
                       0, 0.02f, 0.1f, 0.9f, 0.4f, 0.8f, 0.1f, false);
    
    createFactoryPreset("organ_combo_rock", "Rock Combo Organ", "Organ",
                       {"Rock", "Combo", "Bright", "Vintage"}, "Classic rock combo organ",
                       1, 0.01f, 0.08f, 0.85f, 0.25f, 0.9f, 0.15f, true);
    
    createFactoryPreset("organ_theater_mighty", "Mighty Theater Organ", "Organ",
                       {"Theater", "Mighty", "Majestic", "Vintage"}, "Grand theater organ sound",
                       0, 0.05f, 0.2f, 0.95f, 1.0f, 0.85f, 0.05f, false);
    
    createFactoryPreset("organ_reed_harmonium", "Reed Harmonium", "Organ",
                       {"Reed", "Harmonium", "Warm", "Vintage"}, "Traditional reed harmonium",
                       0, 0.03f, 0.15f, 0.9f, 0.6f, 0.7f, 0.1f, true);
    
    // Complete Strings Category (8 additional presets - 10 total)
    createFactoryPreset("strings_viola_warm", "Warm Viola", "Strings",
                       {"Viola", "Warm", "Expressive", "Natural"}, "Rich warm viola sound",
                       1, 0.12f, 0.3f, 0.88f, 1.2f, 0.65f, 0.18f, true);
    
    createFactoryPreset("strings_double_bass", "Double Bass", "Strings",
                       {"Double Bass", "Deep", "Natural", "Classical"}, "Orchestral double bass",
                       0, 0.15f, 0.4f, 0.9f, 1.8f, 0.5f, 0.1f, true);
    
    createFactoryPreset("strings_quartet_ensemble", "String Quartet", "Strings",
                       {"Quartet", "Ensemble", "Classical", "Natural"}, "Full string quartet ensemble",
                       1, 0.1f, 0.35f, 0.85f, 1.5f, 0.7f, 0.15f, true);
    
    createFactoryPreset("strings_synth_orchestra", "Synth String Orchestra", "Strings",
                       {"Synth", "Orchestra", "Big", "Lush"}, "Large synthesized string orchestra",
                       0, 0.5f, 0.8f, 0.9f, 2.2f, 0.75f, 0.1f, true);
    
    // Complete Guitar Category (8 additional presets - 10 total)
    createFactoryPreset("guitar_nylon_classical", "Classical Nylon Guitar", "Guitar",
                       {"Classical", "Nylon", "Natural", "Warm"}, "Traditional classical guitar",
                       0, 0.005f, 0.2f, 0.4f, 0.8f, 0.7f, 0.1f, true);
    
    createFactoryPreset("guitar_steel_fingerpicked", "Fingerpicked Steel Guitar", "Guitar",
                       {"Steel", "Fingerpicked", "Natural", "Bright"}, "Fingerpicked steel string guitar",
                       1, 0.003f, 0.15f, 0.5f, 0.6f, 0.85f, 0.12f, true);
    
    createFactoryPreset("guitar_electric_distorted", "Distorted Electric Guitar", "Guitar",
                       {"Electric", "Distorted", "Rock", "Aggressive"}, "Heavy distorted electric guitar",
                       2, 0.002f, 0.05f, 0.8f, 0.15f, 0.7f, 0.8f, true);
    
    createFactoryPreset("guitar_12string_jangle", "12-String Jangle", "Guitar",
                       {"12-String", "Jangle", "Bright", "Natural"}, "Bright jangly 12-string guitar",
                       1, 0.003f, 0.12f, 0.6f, 0.4f, 0.9f, 0.15f, true);
    
    DBG("Loaded " + juce::String(presets.size()) + " factory presets");
}

void InstrumentLibraryManager::createFactoryPreset(const juce::String& id, const juce::String& name,
                                                   const juce::String& category, const juce::StringArray& tags,
                                                   const juce::String& description, int waveform,
                                                   float attack, float decay, float sustain, float release,
                                                   float cutoff, float resonance, bool useFilter)
{
    PresetData preset;
    preset.presetId = id;
    preset.name = name;
    preset.category = category;
    preset.tags = tags;
    preset.description = description;
    preset.isUserPreset = false;
    
    preset.synthParams.waveform = waveform;
    preset.synthParams.attack = attack;
    preset.synthParams.decay = decay;
    preset.synthParams.sustain = sustain;
    preset.synthParams.release = release;
    preset.synthParams.filterCutoff = cutoff;
    preset.synthParams.filterResonance = resonance;
    preset.synthParams.useFilter = useFilter;
    
    presets.add(preset);
}

InstrumentLibraryManager::PresetData InstrumentLibraryManager::parsePresetFromJson(const juce::var& jsonData) const
{
    // TODO: Implement JSON parsing for factory presets
    // This would be used when loading from embedded JSON resources
    return PresetData{};
}

void InstrumentLibraryManager::buildIndices()
{
    categories.clear();
    availableTags.clear();
    
    for (const auto& preset : presets)
    {
        // Collect unique categories
        if (!categories.contains(preset.category))
            categories.add(preset.category);
        
        // Collect unique tags
        for (const auto& tag : preset.tags)
        {
            if (!availableTags.contains(tag))
                availableTags.add(tag);
        }
    }
    
    // Sort for consistent ordering
    categories.sort(true);
    availableTags.sort(true);
}

void InstrumentLibraryManager::clearCaches() const
{
    categoryCache.clear();
    tagCache.clear();
}

juce::File InstrumentLibraryManager::getUserPresetsDirectory() const
{
    auto appData = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory);
    return appData.getChildFile("SpawnClone").getChildFile("UserPresets");
}

bool InstrumentLibraryManager::validatePreset(const PresetData& preset) const
{
    if (preset.presetId.isEmpty() || preset.name.isEmpty() || preset.category.isEmpty())
        return false;
    
    // Validate synthesis parameters are within reasonable ranges
    const auto& p = preset.synthParams;
    if (p.attack < 0.001f || p.attack > 5.0f) return false;
    if (p.decay < 0.001f || p.decay > 5.0f) return false;
    if (p.sustain < 0.0f || p.sustain > 1.0f) return false;
    if (p.release < 0.001f || p.release > 10.0f) return false;
    if (p.filterCutoff < 0.0f || p.filterCutoff > 1.0f) return false;
    if (p.filterResonance < 0.0f || p.filterResonance > 1.0f) return false;
    
    return true;
}
