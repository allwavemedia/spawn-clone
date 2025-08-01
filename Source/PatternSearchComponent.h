/*
  ==============================================================================

    PatternSearchComponent.h
    Created: 30 Jul 2025
    Author:  BMad Master

    Epic 3 Story 3.3: Pattern Search and Filtering Component
    Provides real-time search and filtering for pattern history.

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_data_structures/juce_data_structures.h>
#include "MIDIPattern.h"

//==============================================================================
/**
    Search and filter component for pattern history. Provides real-time search
    with text matching and advanced filtering options.
*/
class PatternSearchComponent : public juce::Component,
                              public juce::TextEditor::Listener,
                              public juce::Timer
{
public:
    //==============================================================================
    public:
    //==============================================================================
    struct FilterCriteria
    {
        juce::String searchText;
        bool favoritesOnly = false;
        bool useRegex = false;
        juce::Range<int> tempoRange{60, 200};
        juce::Range<int> noteCountRange{1, 128};
        juce::Time dateFrom = juce::Time();
        juce::Time dateTo = juce::Time::getCurrentTime();
        
        FilterCriteria() = default;
    };

    //==============================================================================
    PatternSearchComponent();
    ~PatternSearchComponent() override = default;
    
    // Callback for filter changes
    std::function<void(const FilterCriteria&)> onSearchChanged;

    void paint(juce::Graphics& g) override;
    void resized() override;
    
    //==============================================================================
    // TextEditor::Listener implementation
    void textEditorTextChanged(juce::TextEditor& editor) override;
    void textEditorFocusLost(juce::TextEditor& editor) override;
    void textEditorReturnKeyPressed(juce::TextEditor& editor) override;
    
    //==============================================================================
    // Timer implementation for search debouncing
    void timerCallback() override;
    
    //==============================================================================
    // Public interface
    void clearSearch();
    void focusSearchField();
    void setFilterCriteria(const FilterCriteria& criteria);
    FilterCriteria getFilterCriteria() const { return currentCriteria; }
    
    //==============================================================================
    // Pattern filtering
    bool matchesPattern(const MIDIPattern& pattern, int patternIndex) const;
    static juce::String extractSearchableText(const MIDIPattern& pattern, int patternIndex);
    
    //==============================================================================
    // Callbacks
    std::function<void(int matchCount)> onResultCountChanged;

private:
    //==============================================================================
    // Filter Panel Component
    class FilterPanel : public juce::Component
    {
    public:
        FilterPanel(PatternSearchComponent& parent);
        
        void paint(juce::Graphics& g) override;
        void resized() override;
        void updateFromCriteria(const FilterCriteria& criteria);
        FilterCriteria getCriteria() const;
        
        std::function<void()> onFilterChanged;
        
    private:
        PatternSearchComponent& parentComponent;
        
        juce::ToggleButton favoritesOnlyToggle;
        juce::ToggleButton regexToggle;
        juce::Label tempoLabel;
        juce::Slider tempoMinSlider, tempoMaxSlider;
        juce::Label noteCountLabel;
        juce::Slider noteCountMinSlider, noteCountMaxSlider;
        juce::TextButton applyButton;
        juce::TextButton resetButton;
        
        void setupControls();
    };

    //==============================================================================
    FilterCriteria currentCriteria;
    bool isSearchPending = false;
    
    // UI Components
    juce::TextEditor searchField;
    juce::TextButton filterButton;
    juce::TextButton clearButton;
    juce::Label resultCountLabel;
    std::unique_ptr<FilterPanel> filterPanel;
    
    // UI Colors (matching professional theme)
    juce::Colour backgroundColour = juce::Colour(0xff2a2a2a);
    juce::Colour fieldColour = juce::Colour(0xff363636);
    juce::Colour textColour = juce::Colour(0xffcccccc);
    juce::Colour accentColour = juce::Colour(0xff4a9eff);
    juce::Colour borderColour = juce::Colour(0xff404040);
    
    //==============================================================================
    void setupSearchField();
    void setupFilterButton();
    void setupClearButton();
    void setupResultCountLabel();
    void triggerSearchUpdate();
    void showFilterPanel();
    void hideFilterPanel();
    bool performRegexSearch(const juce::String& pattern, const juce::String& text) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PatternSearchComponent)
};
