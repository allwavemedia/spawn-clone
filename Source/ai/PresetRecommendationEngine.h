#pragma once

#include <juce_core/juce_core.h>
#include <juce_data_structures/juce_data_structures.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <map>
#include <vector>
#include <functional>
#include "../audio/InstrumentLibraryManager.h"
#include "MusicTheoryEngine.h"

namespace SpawnClone {

/**
 * Epic 9.3 Story 9.3.1: Intelligent Preset Recommendation Engine
 * 
 * AI-powered system that analyzes musical context, user preferences, and harmonic
 * compatibility to suggest complementary instrument presets for enhanced workflows.
 */
class PresetRecommendationEngine
{
public:
    /**
     * Musical Context Analysis
     */
    struct MusicalContext
    {
        // Harmonic Analysis
        MusicTheoryEngine::Key currentKey = MusicTheoryEngine::Key::C_MAJOR;
        int tempo = 120;                    // BPM
        float timeSignature = 4.0f;         // 4/4, 3/4, etc.
        
        // Genre & Style
        std::string primaryGenre = "Electronic";
        std::string mood = "Energetic";     // Energetic, Calm, Dark, Bright, etc.
        float intensity = 0.5f;             // 0.0 = Ambient, 1.0 = Aggressive
        
        // Current Pattern Context
        int patternComplexity = 50;         // 0-100 scale
        bool hasLeadInstrument = false;
        bool hasBasInstrument = false;
        bool hasPercussion = false;
        bool hasPads = false;
    };

    /**
     * User Preference Learning System
     */
    struct UserPreference
    {
        // Tag Preferences (learned over time)
        std::unordered_map<std::string, float> tagWeights;  // Tag -> preference score
        
        // Instrument Category Preferences
        std::unordered_map<std::string, float> categoryPreferences;
        
        // Harmonic Preferences
        std::vector<MusicTheoryEngine::Key> preferredKeys;
        std::vector<std::string> preferredGenres;
        
        // Usage Statistics
        std::unordered_map<std::string, int> presetUsageCount;
        std::unordered_map<std::string, float> presetRatings;  // User ratings 0.0-1.0
        
        // Contextual Preferences
        std::unordered_map<std::string, std::vector<std::string>> contextualPairings; // Genre -> preferred preset names
        
        // Learning Settings
        bool enableLearning = true;        // Enable/disable preference learning
    };

    /**
     * Recommendation Result
     */
    struct Recommendation
    {
        std::string presetName;
        std::string category;
        float compatibilityScore = 0.0f;   // 0.0-1.0
        float userPreferenceScore = 0.0f;  // 0.0-1.0
        float finalScore = 0.0f;           // Weighted combination
        
        // Explanation for user
        std::string reasonCode;            // "harmonic_match", "genre_compatible", "user_favorite", etc.
        std::string explanation;           // Human-readable explanation
        
        // Preset data
        InstrumentLibraryManager::PresetData presetData;
    };

    /**
     * Recommendation Configuration
     */
    struct RecommendationSettings
    {
        int maxRecommendations = 8;
        float harmonicWeight = 0.4f;       // Weight for harmonic compatibility
        float genreWeight = 0.3f;          // Weight for genre matching
        float userPrefWeight = 0.3f;       // Weight for user preferences
        
        bool enableLearning = true;        // Enable user preference learning
        bool diversifyResults = true;      // Ensure category diversity
        float noveltyFactor = 0.1f;        // Balance between familiar and new suggestions
        
        // Filtering
        bool excludeCurrentPresets = true; // Don't recommend already active presets
        std::vector<std::string> excludeCategories;
    };

public:
    PresetRecommendationEngine();
    ~PresetRecommendationEngine() = default;

    // Core Recommendation Functions
    std::vector<Recommendation> getRecommendations(
        const MusicalContext& context,
        const RecommendationSettings& settings
    );
    
    // Convenience overload with default settings
    std::vector<Recommendation> getRecommendations(
        const MusicalContext& context
    );

    std::vector<Recommendation> getComplementaryInstruments(
        const std::vector<std::string>& currentPresets,
        const MusicalContext& context,
        const RecommendationSettings& settings
    );
    
    // Convenience overload with default settings
    std::vector<Recommendation> getComplementaryInstruments(
        const std::vector<std::string>& currentPresets,
        const MusicalContext& context
    );

    // Context Analysis
    MusicalContext analyzeCurrentContext(
        const std::vector<std::string>& activePresets,
        int currentTempo = 120,
        const std::string& detectedKey = "C"
    );

    // User Learning & Preferences
    void recordPresetUsage(const std::string& presetName, const MusicalContext& context);
    void recordPresetRating(const std::string& presetName, float rating); // 0.0-1.0
    void recordPresetPairing(const std::string& preset1, const std::string& preset2, bool successful);
    
    UserPreference getUserPreferences() const { return userPreferences; }
    void loadUserPreferences(const UserPreference& prefs) { userPreferences = prefs; }

    // Harmonic Compatibility Analysis
    float calculateHarmonicCompatibility(
        const InstrumentLibraryManager::PresetData& preset,
        const MusicalContext& context
    );

    // Genre & Style Matching
    float calculateGenreCompatibility(
        const InstrumentLibraryManager::PresetData& preset,
        const MusicalContext& context
    );

    // Integration with Existing Systems
    void setInstrumentLibrary(std::shared_ptr<InstrumentLibraryManager> library) {
        instrumentLibrary = library;
    }
    
    void setMusicTheoryEngine(std::shared_ptr<MusicTheoryEngine> engine) {
        musicTheory = engine;
    }

    // Serialization for User Preference Persistence
    juce::XmlElement* saveUserPreferences();
    void loadUserPreferences(const juce::XmlElement& xml);

private:
    // Core Systems
    std::shared_ptr<InstrumentLibraryManager> instrumentLibrary;
    std::shared_ptr<MusicTheoryEngine> musicTheory;
    
    // User Learning System
    UserPreference userPreferences;
    
    // Internal Analysis Functions
    std::vector<std::string> getCompatibleTags(const MusicalContext& context);
    std::vector<std::string> getNeededCategories(
        const std::vector<std::string>& currentPresets
    );
    
    float calculateUserPreferenceScore(const InstrumentLibraryManager::PresetData& preset);
    float calculateNoveltyScore(const std::string& presetName);
    
    void updateUserPreferences(const std::string& presetName, const MusicalContext& context);
    
    // Recommendation Algorithms
    std::vector<Recommendation> generateHarmonicRecommendations(const MusicalContext& context, int count);
    std::vector<Recommendation> generateGenreBasedRecommendations(const MusicalContext& context, int count);
    std::vector<Recommendation> generateUserBasedRecommendations(int count);
    std::vector<Recommendation> generateComplementaryRecommendations(
        const std::vector<std::string>& currentPresets, 
        const MusicalContext& context, 
        int count
    );
    
    // Result Processing
    void rankAndFilterRecommendations(
        std::vector<Recommendation>& recommendations,
        const RecommendationSettings& settings
    );
    
    void ensureCategoryDiversity(
        std::vector<Recommendation>& recommendations,
        int maxPerCategory = 2
    );
    
    std::string generateExplanation(const Recommendation& rec, const MusicalContext& context);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetRecommendationEngine)
};

} // namespace SpawnClone
