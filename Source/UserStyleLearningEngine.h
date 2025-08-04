/*
  ==============================================================================

    UserStyleLearningEngine.h
    Created: 2 Aug 2025
    Author:  Epic 6 Story 6.4 Implementation

    Machine learning engine for analyzing user preferences and adapting
    AI generation parameters to match individual musical styles.

  ==============================================================================
*/

#pragma once

#include <juce_core/juce_core.h>
#include "UserStyleProfile.h"
#include "MIDIPattern.h"
#include "ai/MusicTheoryEngine.h"
#include <vector>
#include <random>

//==============================================================================
/**
 * Epic 6 Story 6.4: User Style Learning Engine
 * 
 * Analyzes user feedback and generated patterns to learn preferences,
 * then adapts generation parameters for personalized AI output.
 */
class UserStyleLearningEngine
{
public:
    UserStyleLearningEngine(MusicTheoryEngine* musicEngine);
    
    //==============================================================================
    // Pattern Analysis
    
    /** Comprehensive analysis of a generated pattern's characteristics */
    struct PatternAnalysis
    {
        // Harmonic characteristics
        std::vector<MusicTheoryEngine::ExtendedChordType> chordsUsed;
        GenerationParameters::HarmonicSettings::ProgressionStyle inferredProgressionStyle;
        float harmonicComplexity = 0.0f;        // 0.0-1.0 calculated complexity
        float harmonicTension = 0.0f;           // Average tension level
        
        // Rhythmic characteristics
        float rhythmicComplexity = 0.0f;        // 0.0-1.0 calculated complexity
        float syncopationLevel = 0.0f;          // Amount of syncopation detected
        float swingFeel = 0.0f;                 // Detected swing characteristics
        float noteDensity = 0.0f;               // Notes per beat
        
        // Melodic characteristics
        GenerationParameters::MelodicContour detectedContour;
        float intervalVariety = 0.0f;           // Variety of melodic intervals
        float chromaticUsage = 0.0f;            // Amount of chromatic movement
        float melodicRange = 0.0f;              // Range in semitones
        
        // Context information
        juce::String inferredGenre;             // Best-match genre classification
        int patternLength = 0;                  // Number of notes
        float overallComplexity = 0.0f;         // Combined complexity metric
        
        // Confidence scores for each analysis component
        float harmonicConfidence = 0.0f;
        float rhythmicConfidence = 0.0f;
        float melodicConfidence = 0.0f;
    };
    
    /** Analyze a MIDI pattern and extract musical characteristics */
    PatternAnalysis analyzePattern(const MIDIPattern& pattern, 
                                 const GenerationParameters& params);
    
    //==============================================================================
    // Preference Learning
    
    /** Update user style profile based on pattern feedback */
    void updateStyleProfile(const PatternFeedback& feedback, 
                          UserStyleProfile& profile);
    
    /** Batch update profile from multiple feedback entries */
    void batchUpdateProfile(const std::vector<PatternFeedback>& feedbackBatch,
                          UserStyleProfile& profile);
    
    /** Calculate preference weight for a specific musical element */
    float calculateElementWeight(const juce::String& elementType, 
                               const juce::var& elementValue,
                               const UserStyleProfile& profile);
    
    //==============================================================================
    // Parameter Adaptation
    
    /** Adapt generation parameters based on learned user style */
    GenerationParameters adaptParameters(const GenerationParameters& baseParams, 
                                       const UserStyleProfile& profile,
                                       float adaptationStrength = 1.0f);
    
    /** Get detailed influence report for last parameter adaptation */
    struct AdaptationInfluence
    {
        std::vector<std::string> adaptedParameters;      // Which params were changed
        std::vector<juce::String> influencingPreferences; // Which preferences caused changes
        std::vector<float> adaptationAmounts;             // How much each param changed
        float overallPersonalizationWeight = 0.0f;        // Total personalization applied
        juce::String confidenceLevel;                     // "Low", "Medium", "High"
        juce::String reasoning;                           // Human-readable explanation
    };
    
    AdaptationInfluence getLastAdaptationInfluence() const { return lastInfluence; }
    
    //==============================================================================
    // Genre and Context Analysis
    
    /** Classify pattern into most likely genre */
    juce::String classifyGenre(const PatternAnalysis& analysis);
    
    /** Update genre preferences based on feedback */
    void updateGenrePreferences(const PatternFeedback& feedback,
                              UserStyleProfile& profile);
    
    /** Get recommendations for parameters based on genre context */
    GenerationParameters getGenreRecommendations(const juce::String& genre,
                                                const UserStyleProfile& profile);
    
    //==============================================================================
    // Learning Configuration
    
    /** Set learning rate for preference updates (0.0-1.0) */
    void setLearningRate(float rate) { learningRate = juce::jlimit(0.0f, 1.0f, rate); }
    
    /** Get current learning rate */
    float getLearningRate() const { return learningRate; }
    
    /** Set minimum samples needed for confident adaptation */
    void setMinSamplesForConfidence(int samples) { minSamplesForConfidence = samples; }
    
    /** Enable/disable specific learning categories */
    void setLearningEnabled(const juce::String& category, bool enabled);
    
    /** Get learning statistics */
    struct LearningStats
    {
        int totalPatternsAnalyzed = 0;
        int totalFeedbackProcessed = 0;
        float currentLearningRate = 0.1f;
        float averageAnalysisConfidence = 0.0f;
        juce::StringArray activeCategories;
        double lastUpdateTime = 0.0;
    };
    
    LearningStats getLearningStats() const;
    
private:
    MusicTheoryEngine* musicEngine;
    AdaptationInfluence lastInfluence;
    std::mt19937 rng;
    
    // Learning configuration
    float learningRate = 0.1f;
    int minSamplesForConfidence = 10;
    std::unordered_map<juce::String, bool> categoryEnabled;
    
    // Analysis statistics
    int totalAnalyses = 0;
    int totalUpdates = 0;
    
    //==============================================================================
    // Private Analysis Methods
    
    /** Analyze harmonic content of pattern */
    void analyzeHarmonicContent(const MIDIPattern& pattern,
                              const GenerationParameters& params,
                              PatternAnalysis& analysis);
    
    /** Analyze rhythmic characteristics */
    void analyzeRhythmicContent(const MIDIPattern& pattern,
                              PatternAnalysis& analysis);
    
    /** Analyze melodic characteristics */
    void analyzeMelodicContent(const MIDIPattern& pattern,
                             PatternAnalysis& analysis);
    
    /** Calculate overall pattern complexity */
    float calculateOverallComplexity(const PatternAnalysis& analysis);
    
    /** Infer musical genre from analysis */
    juce::String inferGenreFromAnalysis(const PatternAnalysis& analysis);
    
    //==============================================================================
    // Private Learning Methods
    
    /** Update harmonic preferences based on feedback */
    void updateHarmonicPreferences(const PatternFeedback& feedback,
                                 const PatternAnalysis& analysis,
                                 UserStyleProfile& profile);
    
    /** Update rhythmic preferences based on feedback */
    void updateRhythmicPreferences(const PatternFeedback& feedback,
                                 const PatternAnalysis& analysis,
                                 UserStyleProfile& profile);
    
    /** Update melodic preferences based on feedback */
    void updateMelodicPreferences(const PatternFeedback& feedback,
                                const PatternAnalysis& analysis,
                                UserStyleProfile& profile);
    
    /** Apply preference weight smoothing */
    float smoothPreferenceUpdate(float currentWeight, float newEvidence, 
                               float confidence, PatternRating rating);
    
    /** Calculate adaptation confidence level */
    float calculateAdaptationConfidence(const UserStyleProfile& profile,
                                      const juce::String& category);
    
    /** Generate human-readable reasoning for adaptations */
    juce::String generateAdaptationReasoning(const AdaptationInfluence& influence,
                                           const UserStyleProfile& profile);
    
    //==============================================================================
    // Constants
    
    static constexpr float STRONG_POSITIVE_MULTIPLIER = 1.5f;
    static constexpr float POSITIVE_MULTIPLIER = 1.0f;
    static constexpr float NEUTRAL_MULTIPLIER = 0.0f;
    static constexpr float NEGATIVE_MULTIPLIER = -0.5f;
    static constexpr float STRONG_NEGATIVE_MULTIPLIER = -1.0f;
    
    static constexpr float MIN_CONFIDENCE_FOR_ADAPTATION = 0.3f;
    static constexpr float HIGH_CONFIDENCE_THRESHOLD = 0.7f;
    static constexpr float MAX_ADAPTATION_AMOUNT = 0.3f; // Max change per adaptation
};
