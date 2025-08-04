# Epic 6 Story 6.4: User Style Profiles

## **Story Overview**
**As a music producer, I want personalized AI generation that learns from my preferences and adapts to my musical style, so that generated patterns become increasingly aligned with my creative vision over time.**

## **Acceptance Criteria**

### **AC1: Style Profile System**
- GIVEN I use SpawnClone regularly
- WHEN I generate patterns and provide feedback (keep/discard)
- THEN the system learns my preferences and adapts future generations
- AND my style profile persists across sessions

### **AC2: Musical Preference Learning**
- GIVEN I have generated multiple patterns
- WHEN I consistently favor certain harmonic progressions, rhythmic patterns, or melodic contours
- THEN the AI weights these elements more heavily in future generations
- AND the system can identify my preferred musical characteristics

### **AC3: Adaptive Generation Parameters**
- GIVEN I have an established style profile
- WHEN I request new pattern generation
- THEN the AI automatically adjusts generation parameters based on my learned preferences
- AND I can see which preferences are influencing the generation

### **AC4: Style Profile Management**
- GIVEN I want to manage my musical preferences
- WHEN I access the style profile interface
- THEN I can view my learned preferences, reset specific categories, or start fresh
- AND I can export/import style profiles for sharing or backup

### **AC5: Multi-Genre Adaptation**
- GIVEN I work in multiple musical genres
- WHEN I switch between different musical contexts
- THEN the system adapts its recommendations while maintaining cross-genre learning
- AND genre-specific preferences are tracked separately when appropriate

## **Technical Requirements**

### **TR1: User Preference Data Structure**
```cpp
struct UserStyleProfile
{
    // Harmonic preferences
    std::unordered_map<ExtendedChordType, float> chordTypePreferences;
    std::unordered_map<ProgressionStyle, float> progressionStylePreferences;
    float harmonicComplexityPreference = 0.5f;
    float modalInterchangePreference = 0.3f;
    
    // Rhythmic preferences  
    std::unordered_map<RhythmicPattern, float> rhythmicPatternPreferences;
    float rhythmicComplexityPreference = 0.5f;
    float syncopationPreference = 0.5f;
    float swingPreference = 0.0f;
    
    // Melodic preferences
    std::unordered_map<MelodicContour, float> contourPreferences;
    float intervalVarietyPreference = 0.5f;
    float chromaticismPreference = 0.3f;
    
    // Genre preferences
    std::unordered_map<juce::String, float> genreAffinities;
    
    // Learning metadata
    int totalGenerations = 0;
    int totalRatings = 0;
    float averageRating = 0.0f;
    double lastUpdated = 0.0;
    juce::String profileId;
};
```

### **TR2: Feedback Collection System**
```cpp
enum class PatternRating
{
    Discard = 1,     // User deleted/ignored
    Neutral = 3,     // User kept but no strong preference
    Like = 4,        // User actively used
    Love = 5         // User favorited/saved
};

struct PatternFeedback
{
    juce::String patternId;
    PatternRating rating;
    GenerationParameters usedParams;
    MIDIPattern generatedPattern;
    double timestamp;
};
```

### **TR3: Style Learning Engine**
```cpp
class UserStyleLearningEngine
{
public:
    /** Update style profile based on user feedback */
    void updateStyleProfile(const PatternFeedback& feedback, UserStyleProfile& profile);
    
    /** Adapt generation parameters based on learned style */
    GenerationParameters adaptParameters(const GenerationParameters& baseParams, 
                                       const UserStyleProfile& profile);
    
    /** Calculate preference weights for various musical elements */
    float calculateElementWeight(const juce::String& elementType, 
                               const juce::var& elementValue,
                               const UserStyleProfile& profile);
    
    /** Analyze pattern characteristics for learning */
    struct PatternAnalysis
    {
        std::vector<ExtendedChordType> chordsUsed;
        std::vector<int> rhythmicPatterns;
        MelodicContour overallContour;
        float harmonicComplexity;
        float rhythmicComplexity;
        juce::String inferredGenre;
    };
    
    PatternAnalysis analyzePattern(const MIDIPattern& pattern, 
                                 const GenerationParameters& params);
private:
    /** Learning rate for preference updates */
    static constexpr float LEARNING_RATE = 0.1f;
    
    /** Minimum samples needed for confident preference */
    static constexpr int MIN_SAMPLES_FOR_CONFIDENCE = 10;
};
```

### **TR4: Personalized Generation Pipeline**
```cpp
class PersonalizedGenerationEngine
{
public:
    PersonalizedGenerationEngine(MusicTheoryEngine* engine, 
                               UserStyleLearningEngine* learningEngine);
    
    /** Generate pattern with personalized adaptations */
    MIDIPattern generatePersonalizedPattern(const GenerationParameters& baseParams,
                                          const UserStyleProfile& profile);
    
    /** Get explanation of how personalization influenced generation */
    struct PersonalizationInfluence
    {
        std::vector<juce::String> adaptedParameters;
        std::vector<juce::String> influencingPreferences;
        float personalizedWeight = 0.0f;
        juce::String reasoning;
    };
    
    PersonalizationInfluence getLastInfluenceReport() const;
    
private:
    MusicTheoryEngine* musicEngine;
    UserStyleLearningEngine* learningEngine;
    PersonalizationInfluence lastInfluence;
};
```

## **Implementation Plan**

### **Phase 1: Core Infrastructure (Tasks 6.4.1-6.4.3)**
1. **Task 6.4.1:** Create `UserStyleProfile` data structure with serialization
2. **Task 6.4.2:** Implement `UserStyleLearningEngine` with basic preference learning
3. **Task 6.4.3:** Add pattern feedback collection system to PluginProcessor

### **Phase 2: Learning Algorithms (Tasks 6.4.4-6.4.6)**
4. **Task 6.4.4:** Implement pattern analysis for automatic characteristic extraction
5. **Task 6.4.5:** Create preference weight calculation and adaptation algorithms
6. **Task 6.4.6:** Build personalized parameter adaptation system

### **Phase 3: Integration & Testing (Tasks 6.4.7-6.4.9)**
7. **Task 6.4.7:** Integrate personalized generation into main AI pipeline
8. **Task 6.4.8:** Add style profile management interface (ready for UI)
9. **Task 6.4.9:** Create comprehensive test suite for learning and adaptation

### **Phase 4: Advanced Features (Tasks 6.4.10-6.4.12)**
10. **Task 6.4.10:** Implement multi-genre adaptation and context switching
11. **Task 6.4.11:** Add profile export/import functionality
12. **Task 6.4.12:** Create influence reporting and transparency features

## **Success Metrics**

- **Learning Effectiveness**: User satisfaction with generated patterns increases by 25% after 50+ generations
- **Adaptation Speed**: System demonstrates measurable preference learning within 20 user interactions
- **Persistence**: Style profiles maintain consistency across plugin sessions
- **Performance**: Personalized generation adds <100ms to total generation time
- **Accuracy**: Pattern analysis correctly identifies musical characteristics with >80% accuracy

## **Technical Integration Points**

- **MusicTheoryEngine**: Enhanced with style-aware generation methods
- **ParameterManager**: Extended with style profile parameters
- **PatternManager**: Integrated with feedback collection
- **State Management**: Style profiles persist in plugin state
- **AI Pipeline**: Personalized generation seamlessly integrated

This story establishes SpawnClone as an intelligent, adaptive music creation partner that grows with the user's musical journey.
