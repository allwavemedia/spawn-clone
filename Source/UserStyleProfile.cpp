/*
  ==============================================================================

    UserStyleProfile.cpp
    Created: 2 Aug 2025
    Author:  Epic 6 Story 6.4 Implementation

    Implementation of user preference learning and style adaptation system.

  ==============================================================================
*/

#include "UserStyleProfile.h"
#include <algorithm>
#include <chrono>

//==============================================================================
// UserStyleProfile Implementation

juce::ValueTree UserStyleProfile::toValueTree() const
{
    juce::ValueTree tree("UserStyleProfile");
    
    // Basic metadata
    tree.setProperty("profileId", profileId, nullptr);
    tree.setProperty("profileName", profileName, nullptr);
    tree.setProperty("createdAt", createdAt, nullptr);
    tree.setProperty("lastUpdated", lastUpdated, nullptr);
    tree.setProperty("totalGenerations", totalGenerations, nullptr);
    tree.setProperty("totalRatings", totalRatings, nullptr);
    tree.setProperty("averageRating", averageRating, nullptr);
    tree.setProperty("learningConfidence", learningConfidence, nullptr);
    
    // Harmonic preferences
    tree.setProperty("harmonicComplexityPreference", harmonicComplexityPreference, nullptr);
    tree.setProperty("modalInterchangePreference", modalInterchangePreference, nullptr);
    tree.setProperty("chromaticHarmonyPreference", chromaticHarmonyPreference, nullptr);
    
    // Rhythmic preferences
    tree.setProperty("rhythmicComplexityPreference", rhythmicComplexityPreference, nullptr);
    tree.setProperty("syncopationPreference", syncopationPreference, nullptr);
    tree.setProperty("swingPreference", swingPreference, nullptr);
    tree.setProperty("noteDensityPreference", noteDensityPreference, nullptr);
    
    // Melodic preferences
    tree.setProperty("intervalVarietyPreference", intervalVarietyPreference, nullptr);
    tree.setProperty("melodicChromaticismPreference", melodicChromaticismPreference, nullptr);
    tree.setProperty("melodicRangePreference", melodicRangePreference, nullptr);
    
    // Chord type preferences
    juce::ValueTree chordPrefs("ChordTypePreferences");
    for (const auto& pair : chordTypePreferences)
    {
        juce::ValueTree chordPref("ChordType");
        chordPref.setProperty("type", static_cast<int>(pair.first), nullptr);
        chordPref.setProperty("weight", pair.second, nullptr);
        chordPrefs.appendChild(chordPref, nullptr);
    }
    tree.appendChild(chordPrefs, nullptr);
    
    // Progression style preferences
    juce::ValueTree progressionPrefs("ProgressionStylePreferences");
    for (const auto& pair : progressionStylePreferences)
    {
        juce::ValueTree progPref("ProgressionStyle");
        progPref.setProperty("style", static_cast<int>(pair.first), nullptr);
        progPref.setProperty("weight", pair.second, nullptr);
        progressionPrefs.appendChild(progPref, nullptr);
    }
    tree.appendChild(progressionPrefs, nullptr);
    
    // Contour preferences
    juce::ValueTree contourPrefs("ContourPreferences");
    for (const auto& pair : contourPreferences)
    {
        juce::ValueTree contourPref("Contour");
        contourPref.setProperty("contour", static_cast<int>(pair.first), nullptr);
        contourPref.setProperty("weight", pair.second, nullptr);
        contourPrefs.appendChild(contourPref, nullptr);
    }
    tree.appendChild(contourPrefs, nullptr);
    
    // Genre affinities
    juce::ValueTree genrePrefs("GenreAffinities");
    for (const auto& pair : genreAffinities)
    {
        juce::ValueTree genrePref("Genre");
        genrePref.setProperty("name", juce::String(pair.first), nullptr);
        genrePref.setProperty("affinity", pair.second, nullptr);
        genrePrefs.appendChild(genrePref, nullptr);
    }
    tree.appendChild(genrePrefs, nullptr);
    
    // Key preferences
    juce::ValueTree keyPrefs("KeyPreferences");
    for (const auto& pair : keyPreferences)
    {
        juce::ValueTree keyPref("Key");
        keyPref.setProperty("key", pair.first, nullptr);
        keyPref.setProperty("frequency", pair.second, nullptr);
        keyPrefs.appendChild(keyPref, nullptr);
    }
    tree.appendChild(keyPrefs, nullptr);
    
    // Scale preferences
    juce::ValueTree scalePrefs("ScalePreferences");
    for (const auto& pair : scalePreferences)
    {
        juce::ValueTree scalePref("Scale");
        scalePref.setProperty("scale", static_cast<int>(pair.first), nullptr);
        scalePref.setProperty("frequency", pair.second, nullptr);
        scalePrefs.appendChild(scalePref, nullptr);
    }
    tree.appendChild(scalePrefs, nullptr);
    
    return tree;
}

void UserStyleProfile::fromValueTree(const juce::ValueTree& tree)
{
    if (!tree.hasType("UserStyleProfile"))
        return;
    
    // Basic metadata
    profileId = tree.getProperty("profileId", juce::Uuid().toString());
    profileName = tree.getProperty("profileName", "Default Style");
    createdAt = tree.getProperty("createdAt", 0.0);
    lastUpdated = tree.getProperty("lastUpdated", 0.0);
    totalGenerations = tree.getProperty("totalGenerations", 0);
    totalRatings = tree.getProperty("totalRatings", 0);
    averageRating = tree.getProperty("averageRating", 3.0f);
    learningConfidence = tree.getProperty("learningConfidence", 0.0f);
    
    // Harmonic preferences
    harmonicComplexityPreference = tree.getProperty("harmonicComplexityPreference", 0.5f);
    modalInterchangePreference = tree.getProperty("modalInterchangePreference", 0.3f);
    chromaticHarmonyPreference = tree.getProperty("chromaticHarmonyPreference", 0.2f);
    
    // Rhythmic preferences
    rhythmicComplexityPreference = tree.getProperty("rhythmicComplexityPreference", 0.5f);
    syncopationPreference = tree.getProperty("syncopationPreference", 0.5f);
    swingPreference = tree.getProperty("swingPreference", 0.0f);
    noteDensityPreference = tree.getProperty("noteDensityPreference", 0.6f);
    
    // Melodic preferences
    intervalVarietyPreference = tree.getProperty("intervalVarietyPreference", 0.5f);
    melodicChromaticismPreference = tree.getProperty("melodicChromaticismPreference", 0.3f);
    melodicRangePreference = tree.getProperty("melodicRangePreference", 1.5f);
    
    // Chord type preferences
    chordTypePreferences.clear();
    auto chordPrefs = tree.getChildWithName("ChordTypePreferences");
    if (chordPrefs.isValid())
    {
        for (int i = 0; i < chordPrefs.getNumChildren(); ++i)
        {
            auto chordPref = chordPrefs.getChild(i);
            auto type = static_cast<MusicTheoryEngine::ExtendedChordType>(
                static_cast<int>(chordPref.getProperty("type")));
            float weight = chordPref.getProperty("weight");
            chordTypePreferences[type] = weight;
        }
    }
    
    // Progression style preferences
    progressionStylePreferences.clear();
    auto progressionPrefs = tree.getChildWithName("ProgressionStylePreferences");
    if (progressionPrefs.isValid())
    {
        for (int i = 0; i < progressionPrefs.getNumChildren(); ++i)
        {
            auto progPref = progressionPrefs.getChild(i);
            auto style = static_cast<GenerationParameters::HarmonicSettings::ProgressionStyle>(
                static_cast<int>(progPref.getProperty("style")));
            float weight = progPref.getProperty("weight");
            progressionStylePreferences[style] = weight;
        }
    }
    
    // Contour preferences
    contourPreferences.clear();
    auto contourPrefs = tree.getChildWithName("ContourPreferences");
    if (contourPrefs.isValid())
    {
        for (int i = 0; i < contourPrefs.getNumChildren(); ++i)
        {
            auto contourPref = contourPrefs.getChild(i);
            auto contour = static_cast<GenerationParameters::MelodicContour>(
                static_cast<int>(contourPref.getProperty("contour")));
            float weight = contourPref.getProperty("weight");
            contourPreferences[contour] = weight;
        }
    }
    
    // Genre affinities
    genreAffinities.clear();
    auto genrePrefs = tree.getChildWithName("GenreAffinities");
    if (genrePrefs.isValid())
    {
        for (int i = 0; i < genrePrefs.getNumChildren(); ++i)
        {
            auto genrePref = genrePrefs.getChild(i);
            juce::String name = genrePref.getProperty("name");
            float affinity = genrePref.getProperty("affinity");
            genreAffinities[name.toStdString()] = affinity;
        }
    }
    
    // Key preferences
    keyPreferences.clear();
    auto keyPrefs = tree.getChildWithName("KeyPreferences");
    if (keyPrefs.isValid())
    {
        for (int i = 0; i < keyPrefs.getNumChildren(); ++i)
        {
            auto keyPref = keyPrefs.getChild(i);
            int key = keyPref.getProperty("key");
            float frequency = keyPref.getProperty("frequency");
            keyPreferences[key] = frequency;
        }
    }
    
    // Scale preferences
    scalePreferences.clear();
    auto scalePrefs = tree.getChildWithName("ScalePreferences");
    if (scalePrefs.isValid())
    {
        for (int i = 0; i < scalePrefs.getNumChildren(); ++i)
        {
            auto scalePref = scalePrefs.getChild(i);
            auto scale = static_cast<GenerationParameters::ScaleType>(
                static_cast<int>(scalePref.getProperty("scale")));
            float frequency = scalePref.getProperty("frequency");
            scalePreferences[scale] = frequency;
        }
    }
}

void UserStyleProfile::resetToDefaults()
{
    // Reset all preference maps
    chordTypePreferences.clear();
    progressionStylePreferences.clear();
    contourPreferences.clear();
    genreAffinities.clear();
    keyPreferences.clear();
    scalePreferences.clear();
    
    // Reset scalar preferences to defaults
    harmonicComplexityPreference = 0.5f;
    modalInterchangePreference = 0.3f;
    chromaticHarmonyPreference = 0.2f;
    rhythmicComplexityPreference = 0.5f;
    syncopationPreference = 0.5f;
    swingPreference = 0.0f;
    noteDensityPreference = 0.6f;
    intervalVarietyPreference = 0.5f;
    melodicChromaticismPreference = 0.3f;
    melodicRangePreference = 1.5f;
    
    // Reset learning metadata
    totalGenerations = 0;
    totalRatings = 0;
    averageRating = 3.0f;
    learningConfidence = 0.0f;
    lastUpdated = juce::Time::getCurrentTime().toMilliseconds();
}

float UserStyleProfile::getCategoryConfidence(const juce::String& category) const
{
    // Base confidence on number of samples for this category
    if (category == "harmonic")
    {
        int samples = static_cast<int>(chordTypePreferences.size() + progressionStylePreferences.size());
        return juce::jmin(1.0f, samples / 20.0f); // 20 samples for full confidence
    }
    else if (category == "rhythmic")
    {
        return juce::jmin(1.0f, totalGenerations / 30.0f); // 30 generations for rhythmic confidence
    }
    else if (category == "melodic")
    {
        int samples = static_cast<int>(contourPreferences.size());
        return juce::jmin(1.0f, samples / 15.0f); // 15 samples for melodic confidence
    }
    else if (category == "genre")
    {
        int samples = static_cast<int>(genreAffinities.size());
        return juce::jmin(1.0f, samples / 10.0f); // 10 genres for full confidence
    }
    
    return learningConfidence;
}

juce::StringArray UserStyleProfile::getPreferenceSummary() const
{
    juce::StringArray summary;
    
    // Add harmonic preferences
    if (harmonicComplexityPreference > 0.7f)
        summary.add("Prefers complex harmony");
    else if (harmonicComplexityPreference < 0.3f)
        summary.add("Prefers simple harmony");
    
    // Add rhythmic preferences
    if (rhythmicComplexityPreference > 0.7f)
        summary.add("Enjoys complex rhythms");
    else if (rhythmicComplexityPreference < 0.3f)
        summary.add("Prefers simple rhythms");
    
    if (syncopationPreference > 0.7f)
        summary.add("Loves syncopated patterns");
    
    if (swingPreference > 0.5f)
        summary.add("Prefers swing feel");
    
    // Add melodic preferences
    if (intervalVarietyPreference > 0.7f)
        summary.add("Enjoys varied melodic intervals");
    
    // Add top genre preferences
    std::vector<std::pair<juce::String, float>> sortedGenres;
    for (const auto& pair : genreAffinities)
        sortedGenres.push_back(pair);
    
    std::sort(sortedGenres.begin(), sortedGenres.end(),
        [](const auto& a, const auto& b) { return a.second > b.second; });
    
    if (!sortedGenres.empty() && sortedGenres[0].second > 0.6f)
        summary.add("Favors " + sortedGenres[0].first + " style");
    
    if (summary.isEmpty())
        summary.add("Still learning your preferences...");
    
    return summary;
}

//==============================================================================
// UserFeedbackCollector Implementation

UserFeedbackCollector::UserFeedbackCollector()
{
    feedbackHistory.reserve(MAX_FEEDBACK_HISTORY);
}

void UserFeedbackCollector::recordFeedback(const juce::String& patternId,
                                          PatternRating rating,
                                          const GenerationParameters& params,
                                          const juce::String& genreContext)
{
    PatternFeedback feedback;
    feedback.patternId = patternId;
    feedback.rating = rating;
    feedback.usedParams = params;
    feedback.genreContext = genreContext;
    feedback.timestamp = juce::Time::getCurrentTime().toMilliseconds();
    feedback.complexityLevel = params.rhythmicComplexity; // Simplified complexity calculation
    
    feedbackHistory.push_back(feedback);
    maintainHistorySize();
}

std::vector<PatternFeedback> UserFeedbackCollector::getRecentFeedback(int numEntries) const
{
    std::vector<PatternFeedback> recent;
    int startIndex = juce::jmax(0, static_cast<int>(feedbackHistory.size()) - numEntries);
    
    for (int i = startIndex; i < static_cast<int>(feedbackHistory.size()); ++i)
        recent.push_back(feedbackHistory[i]);
    
    return recent;
}

std::vector<PatternFeedback> UserFeedbackCollector::getFeedbackForGenre(const juce::String& genre) const
{
    std::vector<PatternFeedback> genreFeedback;
    
    for (const auto& feedback : feedbackHistory)
    {
        if (feedback.genreContext == genre)
            genreFeedback.push_back(feedback);
    }
    
    return genreFeedback;
}

float UserFeedbackCollector::getRecentAverageRating(int numEntries) const
{
    auto recent = getRecentFeedback(numEntries);
    if (recent.empty())
        return 3.0f; // Default neutral rating
    
    float sum = 0.0f;
    for (const auto& feedback : recent)
        sum += static_cast<float>(feedback.rating);
    
    return sum / static_cast<float>(recent.size());
}

juce::ValueTree UserFeedbackCollector::exportFeedbackData() const
{
    juce::ValueTree feedbackTree("FeedbackHistory");
    
    for (const auto& feedback : feedbackHistory)
    {
        juce::ValueTree feedbackNode("Feedback");
        feedbackNode.setProperty("patternId", feedback.patternId, nullptr);
        feedbackNode.setProperty("rating", static_cast<int>(feedback.rating), nullptr);
        feedbackNode.setProperty("timestamp", feedback.timestamp, nullptr);
        feedbackNode.setProperty("genreContext", feedback.genreContext, nullptr);
        feedbackNode.setProperty("complexityLevel", feedback.complexityLevel, nullptr);
        
        // Store simplified parameter data
        feedbackNode.setProperty("key", feedback.usedParams.key, nullptr);
        feedbackNode.setProperty("scale", static_cast<int>(feedback.usedParams.scale), nullptr);
        feedbackNode.setProperty("rhythmicComplexity", feedback.usedParams.rhythmicComplexity, nullptr);
        
        feedbackTree.appendChild(feedbackNode, nullptr);
    }
    
    return feedbackTree;
}

void UserFeedbackCollector::importFeedbackData(const juce::ValueTree& data)
{
    if (!data.hasType("FeedbackHistory"))
        return;
    
    feedbackHistory.clear();
    
    for (int i = 0; i < data.getNumChildren(); ++i)
    {
        auto feedbackNode = data.getChild(i);
        
        PatternFeedback feedback;
        feedback.patternId = feedbackNode.getProperty("patternId");
        feedback.rating = static_cast<PatternRating>(static_cast<int>(feedbackNode.getProperty("rating")));
        feedback.timestamp = feedbackNode.getProperty("timestamp");
        feedback.genreContext = feedbackNode.getProperty("genreContext");
        feedback.complexityLevel = feedbackNode.getProperty("complexityLevel");
        
        // Restore simplified parameter data
        feedback.usedParams.key = feedbackNode.getProperty("key");
        feedback.usedParams.scale = static_cast<GenerationParameters::ScaleType>(
            static_cast<int>(feedbackNode.getProperty("scale")));
        feedback.usedParams.rhythmicComplexity = feedbackNode.getProperty("rhythmicComplexity");
        
        feedbackHistory.push_back(feedback);
    }
}

void UserFeedbackCollector::clearHistory()
{
    feedbackHistory.clear();
}

void UserFeedbackCollector::maintainHistorySize()
{
    if (feedbackHistory.size() > MAX_FEEDBACK_HISTORY)
    {
        // Remove oldest entries
        int excess = static_cast<int>(feedbackHistory.size()) - MAX_FEEDBACK_HISTORY;
        feedbackHistory.erase(feedbackHistory.begin(), feedbackHistory.begin() + excess);
    }
}

juce::String UserFeedbackCollector::generateFeedbackId()
{
    return juce::Uuid().toString();
}
