/*
  ==============================================================================

    UserStyleProfile.h
    Created: 2 Aug 2025
    Author:  Epic 6 Story 6.4 Implementation

    User preference learning and style adaptation system for personalized
    AI-driven MIDI pattern generation.

  ==============================================================================
*/

#pragma once

#include <juce_core/juce_core.h>
#include "GenerationParameters.h"
#include "ai/MusicTheoryEngine.h"
#include <unordered_map>
#include <vector>

//==============================================================================
/**
 * Epic 6 Story 6.4: User Style Profiles
 * 
 * Comprehensive user preference learning system that adapts AI generation
 * to individual musical styles and preferences over time.
 */

/** User feedback rating for generated patterns */
enum class PatternRating
{
    Discard = 1,     // User deleted/ignored - strong negative feedback
    Neutral = 3,     // User kept but no strong preference  
    Like = 4,        // User actively used - positive feedback
    Love = 5         // User favorited/saved - strong positive feedback
};

/** Feedback data for a specific generated pattern */
struct PatternFeedback
{
    juce::String patternId;              // Unique identifier for the pattern
    PatternRating rating;                // User's rating of the pattern
    GenerationParameters usedParams;     // Parameters used to generate this pattern
    double timestamp;                    // When feedback was given
    
    // Pattern characteristics (for learning)
    juce::String genreContext;           // Musical genre context when generated
    int patternLength;                   // Number of notes in pattern
    float complexityLevel;               // Calculated complexity of the pattern
};

/** Comprehensive user musical preference profile */
struct UserStyleProfile
{
    //==============================================================================
    // Harmonic Preferences
    
    /** Preference weights for different chord types (0.0-1.0) */
    std::unordered_map<MusicTheoryEngine::ExtendedChordType, float> chordTypePreferences;
    
    /** Preference weights for progression styles */
    std::unordered_map<GenerationParameters::HarmonicSettings::ProgressionStyle, float> progressionStylePreferences;
    
    /** Preferred harmonic complexity level (0.0-1.0) */
    float harmonicComplexityPreference = 0.5f;
    
    /** Preference for modal interchange usage (0.0-1.0) */
    float modalInterchangePreference = 0.3f;
    
    /** Preference for chromatic harmony (0.0-1.0) */
    float chromaticHarmonyPreference = 0.2f;
    
    //==============================================================================
    // Rhythmic Preferences
    
    /** Preferred rhythmic complexity level (0.0-1.0) */
    float rhythmicComplexityPreference = 0.5f;
    
    /** Preference for syncopated rhythms (0.0-1.0) */
    float syncopationPreference = 0.5f;
    
    /** Preference for swing feel (0.0-1.0) */
    float swingPreference = 0.0f;
    
    /** Preferred note density (0.0-1.0) */
    float noteDensityPreference = 0.6f;
    
    //==============================================================================
    // Melodic Preferences
    
    /** Preference weights for melodic contour shapes */
    std::unordered_map<GenerationParameters::MelodicContour, float> contourPreferences;
    
    /** Preference for interval variety in melodies (0.0-1.0) */
    float intervalVarietyPreference = 0.5f;
    
    /** Preference for chromaticism in melodies (0.0-1.0) */
    float melodicChromaticismPreference = 0.3f;
    
    /** Preferred melodic range in octaves */
    float melodicRangePreference = 1.5f;
    
    //==============================================================================
    // Genre and Context Preferences
    
    /** Affinity scores for different musical genres */
    std::unordered_map<std::string, float> genreAffinities;
    
    /** Preferred musical keys (C, C#, D, etc.) with usage frequency */
    std::unordered_map<int, float> keyPreferences;
    
    /** Preferred scales with usage frequency */
    std::unordered_map<GenerationParameters::ScaleType, float> scalePreferences;
    
    //==============================================================================
    // Learning Metadata
    
    /** Total number of patterns generated for this profile */
    int totalGenerations = 0;
    
    /** Total number of ratings provided by user */
    int totalRatings = 0;
    
    /** Average rating across all feedback (1.0-5.0) */
    float averageRating = 3.0f;
    
    /** Confidence level in learned preferences (0.0-1.0) */
    float learningConfidence = 0.0f;
    
    /** Timestamp of last profile update */
    double lastUpdated = 0.0;
    
    /** Unique identifier for this style profile */
    juce::String profileId;
    
    /** Human-readable profile name */
    juce::String profileName = "Default Style";
    
    /** Profile creation timestamp */
    double createdAt = 0.0;
    
    //==============================================================================
    // Serialization Methods
    
    /** Convert profile to ValueTree for persistence */
    juce::ValueTree toValueTree() const;
    
    /** Load profile from ValueTree */
    void fromValueTree(const juce::ValueTree& tree);
    
    /** Reset all preferences to default values */
    void resetToDefaults();
    
    /** Get confidence level for a specific preference category */
    float getCategoryConfidence(const juce::String& category) const;
    
    /** Get summary of most prominent preferences */
    juce::StringArray getPreferenceSummary() const;
};

//==============================================================================
/**
 * Manages user feedback collection and pattern rating
 */
class UserFeedbackCollector
{
public:
    UserFeedbackCollector();
    
    /** Record user feedback for a generated pattern */
    void recordFeedback(const juce::String& patternId,
                       PatternRating rating,
                       const GenerationParameters& params,
                       const juce::String& genreContext = "");
    
    /** Get all feedback records for analysis */
    const std::vector<PatternFeedback>& getAllFeedback() const { return feedbackHistory; }
    
    /** Get recent feedback (last N entries) */
    std::vector<PatternFeedback> getRecentFeedback(int numEntries = 20) const;
    
    /** Get feedback for specific genre context */
    std::vector<PatternFeedback> getFeedbackForGenre(const juce::String& genre) const;
    
    /** Calculate average rating for recent patterns */
    float getRecentAverageRating(int numEntries = 10) const;
    
    /** Export feedback data for analysis */
    juce::ValueTree exportFeedbackData() const;
    
    /** Import feedback data from ValueTree */
    void importFeedbackData(const juce::ValueTree& data);
    
    /** Clear all feedback history */
    void clearHistory();
    
private:
    std::vector<PatternFeedback> feedbackHistory;
    static constexpr int MAX_FEEDBACK_HISTORY = 1000;
    
    void maintainHistorySize();
    juce::String generateFeedbackId();
};
