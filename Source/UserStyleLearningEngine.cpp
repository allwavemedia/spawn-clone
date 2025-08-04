/*
  ==============================================================================

    UserStyleLearningEngine.cpp
    Created: 2 Aug 2025
    Author:  Epic 6 Story 6.4 Implementation

    Implementation of machine learning engine for user preference analysis
    and adaptive AI generation parameter optimization.

  ==============================================================================
*/

#include "UserStyleLearningEngine.h"
#include <algorithm>
#include <cmath>
#include <numeric>

//==============================================================================
UserStyleLearningEngine::UserStyleLearningEngine(MusicTheoryEngine* musicEngine)
    : musicEngine(musicEngine)
    , rng(std::random_device{}())
{
    // Initialize category enablement
    categoryEnabled["harmonic"] = true;
    categoryEnabled["rhythmic"] = true;
    categoryEnabled["melodic"] = true;
    categoryEnabled["genre"] = true;
}

//==============================================================================
// Pattern Analysis Implementation

UserStyleLearningEngine::PatternAnalysis UserStyleLearningEngine::analyzePattern(
    const MIDIPattern& pattern, const GenerationParameters& params)
{
    PatternAnalysis analysis;
    
    // Analyze different aspects of the pattern
    analyzeHarmonicContent(pattern, params, analysis);
    analyzeRhythmicContent(pattern, analysis);
    analyzeMelodicContent(pattern, analysis);
    
    // Calculate overall metrics
    analysis.overallComplexity = calculateOverallComplexity(analysis);
    analysis.inferredGenre = inferGenreFromAnalysis(analysis);
    analysis.patternLength = static_cast<int>(pattern.notes.size());
    
    ++totalAnalyses;
    return analysis;
}

void UserStyleLearningEngine::analyzeHarmonicContent(const MIDIPattern& pattern,
                                                    const GenerationParameters& params,
                                                    PatternAnalysis& analysis)
{
    if (!categoryEnabled["harmonic"] || pattern.notes.empty())
    {
        analysis.harmonicConfidence = 0.0f;
        return;
    }
    
    // Extract chord information if harmony is enabled
    if (params.harmony.enableAdvancedHarmony)
    {
        // Simplified harmony analysis - in full implementation would analyze chord progressions
        analysis.harmonicComplexity = static_cast<float>(params.harmony.complexity) / 3.0f;
        analysis.inferredProgressionStyle = params.harmony.style;
        analysis.harmonicTension = params.harmony.chromaticism;
        analysis.harmonicConfidence = 0.8f;
    }
    else
    {
        // Basic harmonic analysis from note content
        std::set<int> uniquePitches;
        for (const auto& note : pattern.notes)
            uniquePitches.insert(note.pitch % 12);
        
        // More unique pitches suggests higher harmonic complexity
        analysis.harmonicComplexity = juce::jmin(1.0f, uniquePitches.size() / 7.0f);
        analysis.harmonicConfidence = 0.6f;
    }
}

void UserStyleLearningEngine::analyzeRhythmicContent(const MIDIPattern& pattern,
                                                    PatternAnalysis& analysis)
{
    if (!categoryEnabled["rhythmic"] || pattern.notes.empty())
    {
        analysis.rhythmicConfidence = 0.0f;
        return;
    }
    
    // Calculate note density
    if (pattern.notes.size() > 1)
    {
        double totalDuration = 0.0;
        for (const auto& note : pattern.notes)
            totalDuration = juce::jmax(totalDuration, note.startTime + note.duration);
        
        analysis.noteDensity = static_cast<float>(pattern.notes.size()) / static_cast<float>(totalDuration + 0.1);
    }
    
    // Analyze rhythmic complexity based on timing variations
    std::vector<double> startTimes;
    for (const auto& note : pattern.notes)
        startTimes.push_back(note.startTime);
    
    if (startTimes.size() > 2)
    {
        std::sort(startTimes.begin(), startTimes.end());
        
        // Calculate interval variations (simplified syncopation detection)
        std::vector<double> intervals;
        for (size_t i = 1; i < startTimes.size(); ++i)
            intervals.push_back(startTimes[i] - startTimes[i-1]);
        
        if (!intervals.empty())
        {
            double meanInterval = std::accumulate(intervals.begin(), intervals.end(), 0.0) / intervals.size();
            double variance = 0.0;
            for (double interval : intervals)
                variance += (interval - meanInterval) * (interval - meanInterval);
            variance /= intervals.size();
            
            // Higher variance suggests more syncopation
            analysis.syncopationLevel = juce::jmin(1.0f, static_cast<float>(std::sqrt(variance) / meanInterval));
            analysis.rhythmicComplexity = analysis.syncopationLevel * 0.7f + analysis.noteDensity * 0.3f;
        }
    }
    
    analysis.rhythmicConfidence = 0.7f;
}

void UserStyleLearningEngine::analyzeMelodicContent(const MIDIPattern& pattern,
                                                   PatternAnalysis& analysis)
{
    if (!categoryEnabled["melodic"] || pattern.notes.empty())
    {
        analysis.melodicConfidence = 0.0f;
        return;
    }
    
    // Sort notes by start time for melodic analysis
    std::vector<Note> sortedNotes = pattern.notes;
    std::sort(sortedNotes.begin(), sortedNotes.end(),
        [](const Note& a, const Note& b) { return a.startTime < b.startTime; });
    
    if (sortedNotes.size() < 2)
    {
        analysis.melodicConfidence = 0.3f;
        return;
    }
    
    // Calculate melodic range
    int minPitch = sortedNotes[0].pitch;
    int maxPitch = sortedNotes[0].pitch;
    for (const auto& note : sortedNotes)
    {
        minPitch = juce::jmin(minPitch, note.pitch);
        maxPitch = juce::jmax(maxPitch, note.pitch);
    }
    analysis.melodicRange = static_cast<float>(maxPitch - minPitch) / 12.0f; // In octaves
    
    // Analyze intervals for variety and chromaticism
    std::vector<int> intervals;
    for (size_t i = 1; i < sortedNotes.size(); ++i)
    {
        int interval = std::abs(sortedNotes[i].pitch - sortedNotes[i-1].pitch);
        intervals.push_back(interval);
    }
    
    if (!intervals.empty())
    {
        // Calculate interval variety (diversity of interval sizes)
        std::set<int> uniqueIntervals(intervals.begin(), intervals.end());
        analysis.intervalVariety = static_cast<float>(uniqueIntervals.size()) / 12.0f; // Normalize by max possible intervals
        
        // Count chromatic movement (half-step intervals)
        int chromaticCount = std::count(intervals.begin(), intervals.end(), 1);
        analysis.chromaticUsage = static_cast<float>(chromaticCount) / static_cast<float>(intervals.size());
        
        // Detect melodic contour (simplified)
        int upward = 0, downward = 0;
        for (size_t i = 1; i < sortedNotes.size(); ++i)
        {
            if (sortedNotes[i].pitch > sortedNotes[i-1].pitch) ++upward;
            else if (sortedNotes[i].pitch < sortedNotes[i-1].pitch) ++downward;
        }
        
        if (upward > downward * 1.5f)
            analysis.detectedContour = GenerationParameters::MelodicContour::Rising;
        else if (downward > upward * 1.5f)
            analysis.detectedContour = GenerationParameters::MelodicContour::Falling;
        else
            analysis.detectedContour = GenerationParameters::MelodicContour::Wave;
    }
    
    analysis.melodicConfidence = 0.8f;
}

float UserStyleLearningEngine::calculateOverallComplexity(const PatternAnalysis& analysis)
{
    float weightedSum = 0.0f;
    float totalWeight = 0.0f;
    
    if (analysis.harmonicConfidence > 0.0f)
    {
        weightedSum += analysis.harmonicComplexity * analysis.harmonicConfidence * 0.4f;
        totalWeight += analysis.harmonicConfidence * 0.4f;
    }
    
    if (analysis.rhythmicConfidence > 0.0f)
    {
        weightedSum += analysis.rhythmicComplexity * analysis.rhythmicConfidence * 0.4f;
        totalWeight += analysis.rhythmicConfidence * 0.4f;
    }
    
    if (analysis.melodicConfidence > 0.0f)
    {
        float melodicComplexity = (analysis.intervalVariety + analysis.chromaticUsage) * 0.5f;
        weightedSum += melodicComplexity * analysis.melodicConfidence * 0.2f;
        totalWeight += analysis.melodicConfidence * 0.2f;
    }
    
    return totalWeight > 0.0f ? weightedSum / totalWeight : 0.5f;
}

juce::String UserStyleLearningEngine::inferGenreFromAnalysis(const PatternAnalysis& analysis)
{
    // Simplified genre classification based on characteristics
    if (analysis.harmonicComplexity > 0.7f && analysis.chromaticUsage > 0.3f)
        return "Jazz";
    else if (analysis.syncopationLevel > 0.6f && analysis.rhythmicComplexity > 0.6f)
        return "Hip Hop";
    else if (analysis.noteDensity > 0.8f && analysis.rhythmicComplexity > 0.5f)
        return "Electronic";
    else if (analysis.harmonicComplexity < 0.4f && analysis.rhythmicComplexity < 0.4f)
        return "Pop";
    else
        return "Contemporary";
}

//==============================================================================
// Preference Learning Implementation

void UserStyleLearningEngine::updateStyleProfile(const PatternFeedback& feedback, 
                                                UserStyleProfile& profile)
{
    if (!musicEngine) return;
    
    // Analyze the pattern that received feedback
    MIDIPattern pattern; // Would need to reconstruct from feedback - simplified for now
    auto analysis = analyzePattern(pattern, feedback.usedParams);
    
    // Update different preference categories
    if (categoryEnabled["harmonic"])
        updateHarmonicPreferences(feedback, analysis, profile);
    
    if (categoryEnabled["rhythmic"])
        updateRhythmicPreferences(feedback, analysis, profile);
    
    if (categoryEnabled["melodic"])
        updateMelodicPreferences(feedback, analysis, profile);
    
    if (categoryEnabled["genre"])
        updateGenrePreferences(feedback, profile);
    
    // Update profile metadata
    profile.totalRatings++;
    profile.averageRating = (profile.averageRating * (profile.totalRatings - 1) + 
                           static_cast<float>(feedback.rating)) / profile.totalRatings;
    profile.learningConfidence = juce::jmin(1.0f, profile.totalRatings / 50.0f);
    profile.lastUpdated = juce::Time::getCurrentTime().toMilliseconds();
    
    ++totalUpdates;
}

void UserStyleLearningEngine::updateHarmonicPreferences(const PatternFeedback& feedback,
                                                       const PatternAnalysis& analysis,
                                                       UserStyleProfile& profile)
{
    float ratingMultiplier = 0.0f;
    switch (feedback.rating)
    {
        case PatternRating::Love: ratingMultiplier = STRONG_POSITIVE_MULTIPLIER; break;
        case PatternRating::Like: ratingMultiplier = POSITIVE_MULTIPLIER; break;
        case PatternRating::Neutral: ratingMultiplier = NEUTRAL_MULTIPLIER; break;
        case PatternRating::Discard: ratingMultiplier = NEGATIVE_MULTIPLIER; break;
    }
    
    // Update harmonic complexity preference
    float complexityEvidence = analysis.harmonicComplexity * ratingMultiplier;
    profile.harmonicComplexityPreference = smoothPreferenceUpdate(
        profile.harmonicComplexityPreference, complexityEvidence, 
        analysis.harmonicConfidence, feedback.rating);
    
    // Update progression style preference
    if (analysis.harmonicConfidence > 0.5f)
    {
        auto& styleWeight = profile.progressionStylePreferences[analysis.inferredProgressionStyle];
        styleWeight = smoothPreferenceUpdate(styleWeight, ratingMultiplier * 0.1f, 
                                           analysis.harmonicConfidence, feedback.rating);
    }
    
    // Update modal interchange preference
    if (feedback.usedParams.harmony.modalInterchange > 0.0f)
    {
        float interchangeEvidence = feedback.usedParams.harmony.modalInterchange * ratingMultiplier;
        profile.modalInterchangePreference = smoothPreferenceUpdate(
            profile.modalInterchangePreference, interchangeEvidence,
            0.8f, feedback.rating);
    }
}

void UserStyleLearningEngine::updateRhythmicPreferences(const PatternFeedback& feedback,
                                                       const PatternAnalysis& analysis,
                                                       UserStyleProfile& profile)
{
    float ratingMultiplier = 0.0f;
    switch (feedback.rating)
    {
        case PatternRating::Love: ratingMultiplier = STRONG_POSITIVE_MULTIPLIER; break;
        case PatternRating::Like: ratingMultiplier = POSITIVE_MULTIPLIER; break;
        case PatternRating::Neutral: ratingMultiplier = NEUTRAL_MULTIPLIER; break;
        case PatternRating::Discard: ratingMultiplier = NEGATIVE_MULTIPLIER; break;
    }
    
    // Update rhythmic complexity preference
    float complexityEvidence = analysis.rhythmicComplexity * ratingMultiplier;
    profile.rhythmicComplexityPreference = smoothPreferenceUpdate(
        profile.rhythmicComplexityPreference, complexityEvidence,
        analysis.rhythmicConfidence, feedback.rating);
    
    // Update syncopation preference
    float syncopationEvidence = analysis.syncopationLevel * ratingMultiplier;
    profile.syncopationPreference = smoothPreferenceUpdate(
        profile.syncopationPreference, syncopationEvidence,
        analysis.rhythmicConfidence, feedback.rating);
    
    // Update note density preference
    float densityEvidence = analysis.noteDensity * ratingMultiplier;
    profile.noteDensityPreference = smoothPreferenceUpdate(
        profile.noteDensityPreference, densityEvidence,
        analysis.rhythmicConfidence, feedback.rating);
}

void UserStyleLearningEngine::updateMelodicPreferences(const PatternFeedback& feedback,
                                                      const PatternAnalysis& analysis,
                                                      UserStyleProfile& profile)
{
    float ratingMultiplier = 0.0f;
    switch (feedback.rating)
    {
        case PatternRating::Love: ratingMultiplier = STRONG_POSITIVE_MULTIPLIER; break;
        case PatternRating::Like: ratingMultiplier = POSITIVE_MULTIPLIER; break;
        case PatternRating::Neutral: ratingMultiplier = NEUTRAL_MULTIPLIER; break;
        case PatternRating::Discard: ratingMultiplier = NEGATIVE_MULTIPLIER; break;
    }
    
    // Update interval variety preference
    float varietyEvidence = analysis.intervalVariety * ratingMultiplier;
    profile.intervalVarietyPreference = smoothPreferenceUpdate(
        profile.intervalVarietyPreference, varietyEvidence,
        analysis.melodicConfidence, feedback.rating);
    
    // Update chromatic usage preference
    float chromaticEvidence = analysis.chromaticUsage * ratingMultiplier;
    profile.melodicChromaticismPreference = smoothPreferenceUpdate(
        profile.melodicChromaticismPreference, chromaticEvidence,
        analysis.melodicConfidence, feedback.rating);
    
    // Update contour preference
    if (analysis.melodicConfidence > 0.5f)
    {
        auto& contourWeight = profile.contourPreferences[analysis.detectedContour];
        contourWeight = smoothPreferenceUpdate(contourWeight, ratingMultiplier * 0.1f,
                                             analysis.melodicConfidence, feedback.rating);
    }
}

void UserStyleLearningEngine::updateGenrePreferences(const PatternFeedback& feedback,
                                                    UserStyleProfile& profile)
{
    if (feedback.genreContext.isEmpty()) return;
    
    float ratingMultiplier = 0.0f;
    switch (feedback.rating)
    {
        case PatternRating::Love: ratingMultiplier = 0.2f; break;
        case PatternRating::Like: ratingMultiplier = 0.1f; break;
        case PatternRating::Neutral: ratingMultiplier = 0.0f; break;
        case PatternRating::Discard: ratingMultiplier = -0.1f; break;
    }
    
    std::string genreKey = feedback.genreContext.toStdString();
    auto& genreAffinity = profile.genreAffinities[genreKey];
    float oldValue = genreAffinity;
    genreAffinity = juce::jlimit(0.0f, 1.0f, genreAffinity + ratingMultiplier);
    
    // Debug output - we'll remove this later
    // std::cout << "Genre: " << genreKey << ", Rating: " << ratingMultiplier << ", Old: " << oldValue << ", New: " << genreAffinity << std::endl;
}

float UserStyleLearningEngine::smoothPreferenceUpdate(float currentWeight, float newEvidence, 
                                                     float confidence, PatternRating rating)
{
    // Apply learning rate and confidence weighting
    float learningAmount = learningRate * confidence;
    
    // Stronger learning for extreme ratings
    if (rating == PatternRating::Love || rating == PatternRating::Discard)
        learningAmount *= 1.5f;
    
    float updatedWeight = currentWeight + (newEvidence * learningAmount);
    return juce::jlimit(0.0f, 1.0f, updatedWeight);
}

//==============================================================================
// Parameter Adaptation Implementation

GenerationParameters UserStyleLearningEngine::adaptParameters(const GenerationParameters& baseParams, 
                                                             const UserStyleProfile& profile,
                                                             float adaptationStrength)
{
    GenerationParameters adaptedParams = baseParams;
    lastInfluence = AdaptationInfluence{}; // Reset influence tracking
    
    if (profile.learningConfidence < MIN_CONFIDENCE_FOR_ADAPTATION)
    {
        lastInfluence.confidenceLevel = "Low";
        lastInfluence.reasoning = "Insufficient learning data for confident adaptation";
        return adaptedParams;
    }
    
    float totalAdaptation = 0.0f;
    
    // Adapt harmonic parameters
    if (categoryEnabled["harmonic"] && calculateAdaptationConfidence(profile, "harmonic") > 0.3f)
    {
        float harmonicAdaptation = (profile.harmonicComplexityPreference - 0.5f) * adaptationStrength * 0.3f;
        adaptedParams.harmony.chromaticism = juce::jlimit(0.0f, 1.0f, 
            adaptedParams.harmony.chromaticism + harmonicAdaptation);
        
        if (std::abs(harmonicAdaptation) > 0.05f)
        {
            lastInfluence.adaptedParameters.push_back("Harmonic Complexity");
            lastInfluence.adaptationAmounts.push_back(harmonicAdaptation);
            totalAdaptation += std::abs(harmonicAdaptation);
        }
    }
    
    // Adapt rhythmic parameters
    if (categoryEnabled["rhythmic"] && calculateAdaptationConfidence(profile, "rhythmic") > 0.3f)
    {
        float rhythmicAdaptation = (profile.rhythmicComplexityPreference - 0.5f) * adaptationStrength * 0.3f;
        adaptedParams.rhythmicComplexity = juce::jlimit(0.0f, 1.0f,
            adaptedParams.rhythmicComplexity + rhythmicAdaptation);
        
        if (std::abs(rhythmicAdaptation) > 0.05f)
        {
            lastInfluence.adaptedParameters.push_back("Rhythmic Complexity");
            lastInfluence.adaptationAmounts.push_back(rhythmicAdaptation);
            totalAdaptation += std::abs(rhythmicAdaptation);
        }
    }
    
    lastInfluence.overallPersonalizationWeight = totalAdaptation;
    lastInfluence.confidenceLevel = profile.learningConfidence > HIGH_CONFIDENCE_THRESHOLD ? "High" : 
                                   profile.learningConfidence > 0.5f ? "Medium" : "Low";
    lastInfluence.reasoning = generateAdaptationReasoning(lastInfluence, profile);
    
    return adaptedParams;
}

float UserStyleLearningEngine::calculateAdaptationConfidence(const UserStyleProfile& profile,
                                                           const juce::String& category)
{
    return profile.getCategoryConfidence(category);
}

juce::String UserStyleLearningEngine::generateAdaptationReasoning(const AdaptationInfluence& influence,
                                                                const UserStyleProfile& profile)
{
    juce::String reasoning = "Personalization based on ";
    reasoning += juce::String(profile.totalRatings) + " feedback samples. ";
    
    if (influence.adaptedParameters.size() > 0)
    {
        juce::String paramList = "";
        for (size_t i = 0; i < influence.adaptedParameters.size(); ++i) {
            if (i > 0) paramList += ", ";
            paramList += juce::String(influence.adaptedParameters[i]);
        }
        reasoning += "Adjusted " + paramList + " ";
        reasoning += "based on your preferences.";
    }
    else
    {
        reasoning += "No significant adaptations applied due to insufficient confidence.";
    }
    
    return reasoning;
}

//==============================================================================
// Utility Methods

void UserStyleLearningEngine::batchUpdateProfile(const std::vector<PatternFeedback>& feedbackBatch,
                                                UserStyleProfile& profile)
{
    for (const auto& feedback : feedbackBatch)
        updateStyleProfile(feedback, profile);
}

juce::String UserStyleLearningEngine::classifyGenre(const PatternAnalysis& analysis)
{
    return analysis.inferredGenre;
}

UserStyleLearningEngine::LearningStats UserStyleLearningEngine::getLearningStats() const
{
    LearningStats stats;
    stats.totalPatternsAnalyzed = totalAnalyses;
    stats.totalFeedbackProcessed = totalUpdates;
    stats.currentLearningRate = learningRate;
    stats.lastUpdateTime = juce::Time::getCurrentTime().toMilliseconds();
    
    for (const auto& pair : categoryEnabled)
    {
        if (pair.second)
            stats.activeCategories.add(pair.first);
    }
    
    return stats;
}

void UserStyleLearningEngine::setLearningEnabled(const juce::String& category, bool enabled)
{
    categoryEnabled[category] = enabled;
}
