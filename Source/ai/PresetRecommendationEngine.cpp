#include "PresetRecommendationEngine.h"
#include <algorithm>
#include <random>
#include <cmath>

namespace SpawnClone {

PresetRecommendationEngine::PresetRecommendationEngine()
{
    // Initialize default user preferences
    userPreferences.tagWeights = {
        {"electronic", 0.5f}, {"acoustic", 0.3f}, {"synthetic", 0.6f},
        {"warm", 0.4f}, {"bright", 0.5f}, {"dark", 0.3f},
        {"lead", 0.7f}, {"bass", 0.6f}, {"pad", 0.4f}, {"percussion", 0.5f}
    };
    
    // Initialize category preferences (neutral) for common categories
    userPreferences.categoryPreferences = {
        {"Bass", 0.5f}, {"Bell", 0.5f}, {"Brass", 0.5f}, {"Guitar", 0.5f},
        {"Lead", 0.5f}, {"Pad", 0.5f}, {"Percussion", 0.5f}, {"Pluck", 0.5f},
        {"String", 0.5f}, {"Synth", 0.5f}, {"Vocal", 0.5f}, {"Wind", 0.5f}
    };
}

std::vector<PresetRecommendationEngine::Recommendation> 
PresetRecommendationEngine::getRecommendations(
    const MusicalContext& context,
    const RecommendationSettings& settings)
{
    if (!instrumentLibrary) {
        jassertfalse; // Must have instrument library
        return {};
    }

    std::vector<Recommendation> allRecommendations;
    
    // Generate recommendations from different algorithms
    int perAlgorithm = settings.maxRecommendations / 3;
    
    auto harmonicRecs = generateHarmonicRecommendations(context, perAlgorithm);
    auto genreRecs = generateGenreBasedRecommendations(context, perAlgorithm);
    auto userRecs = generateUserBasedRecommendations(perAlgorithm);
    
    // Combine all recommendations
    allRecommendations.insert(allRecommendations.end(), harmonicRecs.begin(), harmonicRecs.end());
    allRecommendations.insert(allRecommendations.end(), genreRecs.begin(), genreRecs.end());
    allRecommendations.insert(allRecommendations.end(), userRecs.begin(), userRecs.end());
    
    // Rank, filter, and finalize
    rankAndFilterRecommendations(allRecommendations, settings);
    
    if (settings.diversifyResults) {
        ensureCategoryDiversity(allRecommendations, 2);
    }
    
    // Limit to requested count
    if (allRecommendations.size() > static_cast<size_t>(settings.maxRecommendations)) {
        allRecommendations.resize(settings.maxRecommendations);
    }
    
    // Generate explanations
    for (auto& rec : allRecommendations) {
        rec.explanation = generateExplanation(rec, context);
    }
    
    return allRecommendations;
}

std::vector<PresetRecommendationEngine::Recommendation> 
PresetRecommendationEngine::getRecommendations(const MusicalContext& context)
{
    RecommendationSettings defaultSettings;
    return getRecommendations(context, defaultSettings);
}

std::vector<PresetRecommendationEngine::Recommendation>
PresetRecommendationEngine::getComplementaryInstruments(
    const std::vector<std::string>& currentPresets,
    const MusicalContext& context,
    const RecommendationSettings& settings)
{
    auto recommendations = generateComplementaryRecommendations(currentPresets, context, settings.maxRecommendations * 2);
    
    rankAndFilterRecommendations(recommendations, settings);
    
    if (settings.diversifyResults) {
        ensureCategoryDiversity(recommendations, 2);
    }
    
    if (recommendations.size() > static_cast<size_t>(settings.maxRecommendations)) {
        recommendations.resize(settings.maxRecommendations);
    }
    
    for (auto& rec : recommendations) {
        rec.explanation = generateExplanation(rec, context);
    }
    
    return recommendations;
}

std::vector<PresetRecommendationEngine::Recommendation>
PresetRecommendationEngine::getComplementaryInstruments(
    const std::vector<std::string>& currentPresets,
    const MusicalContext& context)
{
    RecommendationSettings defaultSettings;
    return getComplementaryInstruments(currentPresets, context, defaultSettings);
}

PresetRecommendationEngine::MusicalContext 
PresetRecommendationEngine::analyzeCurrentContext(
    const std::vector<std::string>& activePresets,
    int currentTempo,
    const std::string& detectedKey)
{
    MusicalContext context;
    context.tempo = currentTempo;
    
    // Parse key
    if (musicTheory) {
        // Convert string key to MusicTheoryEngine::Key enum
        // This would need actual implementation based on MusicTheoryEngine API
        context.currentKey = MusicTheoryEngine::Key::C_MAJOR; // Default
    }
    
    if (!instrumentLibrary) return context;
    
    // Analyze active presets to determine context
    std::unordered_map<std::string, int> genreCounts;
    std::unordered_map<std::string, int> categoryCounts;
    
    for (const auto& presetName : activePresets) {
        auto preset = instrumentLibrary->getPreset(presetName);
        if (preset.name.isEmpty()) continue;
        
        // Count genres
        for (const auto& tag : preset.tags) {
            if (tag.startsWith("genre:")) {
                juce::String genre = tag.substring(6); // Remove "genre:" prefix
                genreCounts[genre.toStdString()]++;
            }
        }
        
        // Count categories
        categoryCounts[preset.category.toStdString()]++;
    }
    
    // Determine primary genre
    if (!genreCounts.empty()) {
        auto maxGenre = std::max_element(genreCounts.begin(), genreCounts.end(),
            [](const auto& a, const auto& b) { return a.second < b.second; });
        context.primaryGenre = maxGenre->first;
    }
    
    // Analyze instrument balance
    context.hasLeadInstrument = categoryCounts["Lead"] > 0;
    context.hasBasInstrument = categoryCounts["Bass"] > 0;
    context.hasPercussion = categoryCounts["Percussion"] > 0;
    context.hasPads = categoryCounts["Pad"] > 0;
    
    // Estimate intensity from tempo and active instruments
    float tempoIntensity = std::clamp((currentTempo - 60.0f) / 140.0f, 0.0f, 1.0f);
    float instrumentIntensity = static_cast<float>(activePresets.size()) / 8.0f; // Assume 8 is "full"
    context.intensity = (tempoIntensity + instrumentIntensity) / 2.0f;
    
    return context;
}

void PresetRecommendationEngine::recordPresetUsage(const std::string& presetName, const MusicalContext& context)
{
    userPreferences.presetUsageCount[presetName]++;
    
    if (userPreferences.enableLearning) {
        updateUserPreferences(presetName, context);
    }
}

void PresetRecommendationEngine::recordPresetRating(const std::string& presetName, float rating)
{
    userPreferences.presetRatings[presetName] = std::clamp(rating, 0.0f, 1.0f);
}

void PresetRecommendationEngine::recordPresetPairing(const std::string& preset1, const std::string& preset2, bool successful)
{
    if (successful) {
        // Record successful pairings for future recommendations
        userPreferences.contextualPairings[preset1].push_back(preset2);
        userPreferences.contextualPairings[preset2].push_back(preset1);
    }
}

float PresetRecommendationEngine::calculateHarmonicCompatibility(
    const InstrumentLibraryManager::PresetData& preset,
    const MusicalContext& context)
{
    float compatibility = 0.5f; // Neutral base
    
    // Key compatibility (simplified)
    // In a real implementation, this would analyze harmonic relationships
    compatibility += 0.2f; // Assume basic compatibility
    
    // Tempo compatibility - instruments work better in certain tempo ranges
    if (preset.category == "Bass") {
        // Bass instruments work well in most tempos
        compatibility += 0.2f;
    } else if (preset.category == "Lead") {
        // Lead instruments prefer mid to high tempos
        if (context.tempo >= 100 && context.tempo <= 140) {
            compatibility += 0.3f;
        }
    }
    
    // Genre compatibility through tags
    for (const auto& tag : preset.tags) {
        if (tag.startsWith("genre:")) {
            juce::String presetGenre = tag.substring(6);
            if (presetGenre.toStdString() == context.primaryGenre) {
                compatibility += 0.2f;
                break;
            }
        }
    }
    
    return std::clamp(compatibility, 0.0f, 1.0f);
}

float PresetRecommendationEngine::calculateGenreCompatibility(
    const InstrumentLibraryManager::PresetData& preset,
    const MusicalContext& context)
{
    float compatibility = 0.0f;
    
    // Direct genre match
    for (const auto& tag : preset.tags) {
        if (tag.startsWith("genre:")) {
            juce::String presetGenre = tag.substring(6);
            if (presetGenre.toStdString() == context.primaryGenre) {
                compatibility = 0.9f;
                break;
            }
            // Partial matches for related genres
            else if ((presetGenre == "Electronic" && context.primaryGenre == "Synthwave") ||
                     (presetGenre == "Rock" && context.primaryGenre == "Alternative")) {
                compatibility = 0.7f;
            }
        }
    }
    
    // Mood compatibility
    for (const auto& tag : preset.tags) {
        if (tag == "energetic" && context.mood == "Energetic") {
            compatibility += 0.2f;
        } else if (tag == "calm" && context.mood == "Calm") {
            compatibility += 0.2f;
        } else if (tag == "dark" && context.mood == "Dark") {
            compatibility += 0.2f;
        } else if (tag == "bright" && context.mood == "Bright") {
            compatibility += 0.2f;
        }
    }
    
    return std::clamp(compatibility, 0.0f, 1.0f);
}

std::vector<PresetRecommendationEngine::Recommendation>
PresetRecommendationEngine::generateHarmonicRecommendations(const MusicalContext& context, int count)
{
    std::vector<Recommendation> recommendations;
    if (!instrumentLibrary) return recommendations;
    
    auto allPresets = instrumentLibrary->getAllPresets();
    
    for (const auto& preset : allPresets) {
        Recommendation rec;
        rec.presetName = preset.name.toStdString();
        rec.category = preset.category.toStdString();
        rec.presetData = preset;
        rec.compatibilityScore = calculateHarmonicCompatibility(preset, context);
        rec.reasonCode = "harmonic_match";
        
        if (rec.compatibilityScore > 0.3f) { // Threshold for consideration
            recommendations.push_back(rec);
        }
    }
    
    // Sort by compatibility score
    std::sort(recommendations.begin(), recommendations.end(),
              [](const auto& a, const auto& b) { return a.compatibilityScore > b.compatibilityScore; });
    
    if (recommendations.size() > static_cast<size_t>(count)) {
        recommendations.resize(count);
    }
    
    return recommendations;
}

std::vector<PresetRecommendationEngine::Recommendation>
PresetRecommendationEngine::generateGenreBasedRecommendations(const MusicalContext& context, int count)
{
    std::vector<Recommendation> recommendations;
    if (!instrumentLibrary) return recommendations;
    
    auto allPresets = instrumentLibrary->getAllPresets();
    
    for (const auto& preset : allPresets) {
        Recommendation rec;
        rec.presetName = preset.name.toStdString();
        rec.category = preset.category.toStdString();
        rec.presetData = preset;
        rec.compatibilityScore = calculateGenreCompatibility(preset, context);
        rec.reasonCode = "genre_compatible";
        
        if (rec.compatibilityScore > 0.4f) {
            recommendations.push_back(rec);
        }
    }
    
    std::sort(recommendations.begin(), recommendations.end(),
              [](const auto& a, const auto& b) { return a.compatibilityScore > b.compatibilityScore; });
    
    if (recommendations.size() > static_cast<size_t>(count)) {
        recommendations.resize(count);
    }
    
    return recommendations;
}

std::vector<PresetRecommendationEngine::Recommendation>
PresetRecommendationEngine::generateUserBasedRecommendations(int count)
{
    std::vector<Recommendation> recommendations;
    if (!instrumentLibrary) return recommendations;
    
    auto allPresets = instrumentLibrary->getAllPresets();
    
    for (const auto& preset : allPresets) {
        Recommendation rec;
        rec.presetName = preset.name.toStdString();
        rec.category = preset.category.toStdString();
        rec.presetData = preset;
        rec.userPreferenceScore = calculateUserPreferenceScore(preset);
        rec.reasonCode = "user_favorite";
        
        if (rec.userPreferenceScore > 0.3f) {
            recommendations.push_back(rec);
        }
    }
    
    std::sort(recommendations.begin(), recommendations.end(),
              [](const auto& a, const auto& b) { return a.userPreferenceScore > b.userPreferenceScore; });
    
    if (recommendations.size() > static_cast<size_t>(count)) {
        recommendations.resize(count);
    }
    
    return recommendations;
}

std::vector<PresetRecommendationEngine::Recommendation>
PresetRecommendationEngine::generateComplementaryRecommendations(
    const std::vector<std::string>& currentPresets,
    const MusicalContext& context,
    int count)
{
    std::vector<Recommendation> recommendations;
    if (!instrumentLibrary) return recommendations;
    
    // Determine what's missing from current setup
    auto neededCategories = getNeededCategories(currentPresets);
    
    auto allPresets = instrumentLibrary->getAllPresets();
    
    for (const auto& preset : allPresets) {
        // Skip if this preset is already active
        if (std::find(currentPresets.begin(), currentPresets.end(), preset.name) != currentPresets.end()) {
            continue;
        }
        
        // Prioritize needed categories
        bool isNeededCategory = std::find(neededCategories.begin(), neededCategories.end(), preset.category.toStdString()) != neededCategories.end();
        
        Recommendation rec;
        rec.presetName = preset.name.toStdString();
        rec.category = preset.category.toStdString();
        rec.presetData = preset;
        rec.compatibilityScore = calculateHarmonicCompatibility(preset, context);
        rec.userPreferenceScore = calculateUserPreferenceScore(preset);
        
        // Boost score for needed categories
        if (isNeededCategory) {
            rec.compatibilityScore += 0.3f;
        }
        
        rec.finalScore = (rec.compatibilityScore + rec.userPreferenceScore) / 2.0f;
        rec.reasonCode = isNeededCategory ? "fills_gap" : "complementary";
        
        if (rec.finalScore > 0.2f) {
            recommendations.push_back(rec);
        }
    }
    
    std::sort(recommendations.begin(), recommendations.end(),
              [](const auto& a, const auto& b) { return a.finalScore > b.finalScore; });
    
    if (recommendations.size() > static_cast<size_t>(count)) {
        recommendations.resize(count);
    }
    
    return recommendations;
}

std::vector<std::string>
PresetRecommendationEngine::getNeededCategories(const std::vector<std::string>& currentPresets)
{
    std::vector<std::string> needed;
    
    if (!instrumentLibrary) return needed;
    
    std::set<std::string> presentCategories;
    
    // Analyze current presets
    for (const auto& presetName : currentPresets) {
        auto preset = instrumentLibrary->getPreset(presetName);
        if (!preset.name.isEmpty()) {
            presentCategories.insert(preset.category.toStdString());
        }
    }
    
    // Suggest missing essential categories
    if (presentCategories.find("Bass") == presentCategories.end()) {
        needed.push_back("Bass");
    }
    
    if (presentCategories.find("Lead") == presentCategories.end()) {
        needed.push_back("Lead");
    }
    
    if (presentCategories.find("Pad") == presentCategories.end()) {
        needed.push_back("Pad");
    }
    
    if (presentCategories.find("Percussion") == presentCategories.end()) {
        needed.push_back("Percussion");
    }
    
    return needed;
}

float PresetRecommendationEngine::calculateUserPreferenceScore(const InstrumentLibraryManager::PresetData& preset)
{
    float score = 0.0f;
    
    // Category preference
    auto catIt = userPreferences.categoryPreferences.find(preset.category.toStdString());
    if (catIt != userPreferences.categoryPreferences.end()) {
        score += catIt->second * 0.3f;
    }
    
    // Tag preferences
    for (const auto& tag : preset.tags) {
        auto tagIt = userPreferences.tagWeights.find(tag.toStdString());
        if (tagIt != userPreferences.tagWeights.end()) {
            score += tagIt->second * 0.1f; // Each tag contributes less
        }
    }
    
    // Usage history
    auto usageIt = userPreferences.presetUsageCount.find(preset.name.toStdString());
    if (usageIt != userPreferences.presetUsageCount.end()) {
        score += std::min(static_cast<float>(usageIt->second) / 10.0f, 0.3f); // Cap at 0.3
    }
    
    // User rating
    auto ratingIt = userPreferences.presetRatings.find(preset.name.toStdString());
    if (ratingIt != userPreferences.presetRatings.end()) {
        score += ratingIt->second * 0.4f; // Strong weight for explicit ratings
    }
    
    return std::clamp(score, 0.0f, 1.0f);
}

void PresetRecommendationEngine::rankAndFilterRecommendations(
    std::vector<Recommendation>& recommendations,
    const RecommendationSettings& settings)
{
    // Calculate final scores
    for (auto& rec : recommendations) {
        rec.finalScore = (rec.compatibilityScore * settings.harmonicWeight) +
                        (rec.compatibilityScore * settings.genreWeight) + // Note: using compatibility for genre too
                        (rec.userPreferenceScore * settings.userPrefWeight);
        
        // Apply novelty factor
        float novelty = calculateNoveltyScore(rec.presetName);
        rec.finalScore = (rec.finalScore * (1.0f - settings.noveltyFactor)) + (novelty * settings.noveltyFactor);
    }
    
    // Sort by final score
    std::sort(recommendations.begin(), recommendations.end(),
              [](const auto& a, const auto& b) { return a.finalScore > b.finalScore; });
    
    // Remove duplicates
    recommendations.erase(
        std::unique(recommendations.begin(), recommendations.end(),
                   [](const auto& a, const auto& b) { return a.presetName == b.presetName; }),
        recommendations.end()
    );
}

void PresetRecommendationEngine::ensureCategoryDiversity(
    std::vector<Recommendation>& recommendations,
    int maxPerCategory)
{
    std::unordered_map<std::string, int> categoryCount;
    
    auto it = recommendations.begin();
    while (it != recommendations.end()) {
        if (categoryCount[it->category] >= maxPerCategory) {
            it = recommendations.erase(it);
        } else {
            categoryCount[it->category]++;
            ++it;
        }
    }
}

float PresetRecommendationEngine::calculateNoveltyScore(const std::string& presetName)
{
    auto usageIt = userPreferences.presetUsageCount.find(presetName);
    if (usageIt == userPreferences.presetUsageCount.end() || usageIt->second == 0) {
        return 1.0f; // Never used = high novelty
    }
    
    // Less used = more novel
    return std::max(0.0f, 1.0f - (static_cast<float>(usageIt->second) / 20.0f));
}

std::string PresetRecommendationEngine::generateExplanation(const Recommendation& rec, const MusicalContext& context)
{
    if (rec.reasonCode == "harmonic_match") {
        return "Perfect harmonic match for " + context.primaryGenre + " in " + std::to_string(context.tempo) + " BPM";
    } else if (rec.reasonCode == "genre_compatible") {
        return "Excellent fit for " + context.primaryGenre + " style";
    } else if (rec.reasonCode == "user_favorite") {
        return "Based on your preferences and usage history";
    } else if (rec.reasonCode == "fills_gap") {
        return "Completes your arrangement with essential " + rec.category + " layer";
    } else if (rec.reasonCode == "complementary") {
        return "Works beautifully with your current selection";
    }
    
    return "Great choice for your current context";
}

void PresetRecommendationEngine::updateUserPreferences(const std::string& presetName, const MusicalContext& context)
{
    if (!instrumentLibrary) return;
    
    auto preset = instrumentLibrary->getPreset(presetName);
    if (preset.name.isEmpty()) return;
    
    // Boost preference for used category
    userPreferences.categoryPreferences[preset.category.toStdString()] += 0.01f;
    userPreferences.categoryPreferences[preset.category.toStdString()] = std::min(1.0f, userPreferences.categoryPreferences[preset.category.toStdString()]);
    
    // Boost preference for used tags
    for (const auto& tag : preset.tags) {
        userPreferences.tagWeights[tag.toStdString()] += 0.005f;
        userPreferences.tagWeights[tag.toStdString()] = std::min(1.0f, userPreferences.tagWeights[tag.toStdString()]);
    }
    
    // Record genre preference
    if (std::find(userPreferences.preferredGenres.begin(), userPreferences.preferredGenres.end(), context.primaryGenre) == userPreferences.preferredGenres.end()) {
        if (userPreferences.preferredGenres.size() < 5) { // Limit to top 5
            userPreferences.preferredGenres.push_back(context.primaryGenre);
        }
    }
}

juce::XmlElement* PresetRecommendationEngine::saveUserPreferences()
{
    auto xml = std::make_unique<juce::XmlElement>("UserPreferences");
    
    // Save tag weights
    auto tagWeightsXml = xml->createNewChildElement("TagWeights");
    for (const auto& [tag, weight] : userPreferences.tagWeights) {
        auto tagXml = tagWeightsXml->createNewChildElement("Tag");
        tagXml->setAttribute("name", tag);
        tagXml->setAttribute("weight", weight);
    }
    
    // Save category preferences
    auto categoryPrefsXml = xml->createNewChildElement("CategoryPreferences");
    for (const auto& [category, pref] : userPreferences.categoryPreferences) {
        auto catXml = categoryPrefsXml->createNewChildElement("Category");
        catXml->setAttribute("name", category);
        catXml->setAttribute("preference", pref);
    }
    
    // Save usage counts
    auto usageXml = xml->createNewChildElement("UsageCounts");
    for (const auto& [preset, count] : userPreferences.presetUsageCount) {
        auto presetXml = usageXml->createNewChildElement("Preset");
        presetXml->setAttribute("name", preset);
        presetXml->setAttribute("count", count);
    }
    
    // Save ratings
    auto ratingsXml = xml->createNewChildElement("Ratings");
    for (const auto& [preset, rating] : userPreferences.presetRatings) {
        auto ratingXml = ratingsXml->createNewChildElement("Preset");
        ratingXml->setAttribute("name", preset);
        ratingXml->setAttribute("rating", rating);
    }
    
    return xml.release();
}

void PresetRecommendationEngine::loadUserPreferences(const juce::XmlElement& xml)
{
    userPreferences = UserPreference{}; // Reset
    
    // Load tag weights
    if (auto tagWeightsXml = xml.getChildByName("TagWeights")) {
        for (auto* tagXml : tagWeightsXml->getChildIterator()) {
            if (tagXml->hasTagName("Tag")) {
                std::string tag = tagXml->getStringAttribute("name").toStdString();
                float weight = static_cast<float>(tagXml->getDoubleAttribute("weight"));
                userPreferences.tagWeights[tag] = weight;
            }
        }
    }
    
    // Load category preferences
    if (auto categoryPrefsXml = xml.getChildByName("CategoryPreferences")) {
        for (auto* catXml : categoryPrefsXml->getChildIterator()) {
            if (catXml->hasTagName("Category")) {
                std::string category = catXml->getStringAttribute("name").toStdString();
                float pref = static_cast<float>(catXml->getDoubleAttribute("preference"));
                userPreferences.categoryPreferences[category] = pref;
            }
        }
    }
    
    // Load usage counts
    if (auto usageXml = xml.getChildByName("UsageCounts")) {
        for (auto* presetXml : usageXml->getChildIterator()) {
            if (presetXml->hasTagName("Preset")) {
                std::string preset = presetXml->getStringAttribute("name").toStdString();
                int count = presetXml->getIntAttribute("count");
                userPreferences.presetUsageCount[preset] = count;
            }
        }
    }
    
    // Load ratings
    if (auto ratingsXml = xml.getChildByName("Ratings")) {
        for (auto* ratingXml : ratingsXml->getChildIterator()) {
            if (ratingXml->hasTagName("Preset")) {
                std::string preset = ratingXml->getStringAttribute("name").toStdString();
                float rating = static_cast<float>(ratingXml->getDoubleAttribute("rating"));
                userPreferences.presetRatings[preset] = rating;
            }
        }
    }
}

} // namespace SpawnClone
