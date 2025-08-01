/*
  ==============================================================================

    PatternSearchComponent.cpp
    Created: 30 Jul 2025
    Author:  BMad Master

    Epic 3 Story 3.3: Pattern Search and Filtering Implementation

  ==============================================================================
*/

#include "PatternSearchComponent.h"
#include <regex>

//==============================================================================
PatternSearchComponent::PatternSearchComponent()
{
    setupSearchField();
    setupFilterButton();
    setupClearButton();
    setupResultCountLabel();
}

//==============================================================================
void PatternSearchComponent::paint(juce::Graphics& g)
{
    g.fillAll(backgroundColour);
    
    // Draw subtle border
    g.setColour(borderColour);
    g.drawRect(getLocalBounds(), 1);
}

void PatternSearchComponent::resized()
{
    auto bounds = getLocalBounds().reduced(5);
    
    // Clear button (right)
    auto clearArea = bounds.removeFromRight(30);
    clearButton.setBounds(clearArea);
    
    // Filter button (right)
    bounds.removeFromRight(5); // Spacing
    auto filterArea = bounds.removeFromRight(60);
    filterButton.setBounds(filterArea);
    
    // Result count label (right) 
    bounds.removeFromRight(5); // Spacing
    auto resultArea = bounds.removeFromRight(80);
    resultCountLabel.setBounds(resultArea);
    
    // Search field (remaining space)
    bounds.removeFromRight(5); // Spacing
    searchField.setBounds(bounds);
    
    // Position filter panel if visible
    if (filterPanel != nullptr && filterPanel->isVisible())
    {
        auto parentBounds = getParentComponent()->getLocalBounds();
        auto panelBounds = parentBounds.removeFromTop(200).withTrimmedTop(getBottom());
        filterPanel->setBounds(panelBounds);
    }
}

//==============================================================================
void PatternSearchComponent::setupSearchField()
{
    searchField.setTextToShowWhenEmpty("Search patterns...", juce::Colour(0xff888888));
    searchField.setColour(juce::TextEditor::backgroundColourId, fieldColour);
    searchField.setColour(juce::TextEditor::textColourId, textColour);
    searchField.setColour(juce::TextEditor::outlineColourId, borderColour);
    searchField.setColour(juce::TextEditor::focusedOutlineColourId, accentColour);
    searchField.setFont(juce::FontOptions(14.0f));
    searchField.addListener(this);
    addAndMakeVisible(searchField);
}

void PatternSearchComponent::setupFilterButton()
{
    filterButton.setButtonText("Filter");
    filterButton.setColour(juce::TextButton::buttonColourId, fieldColour);
    filterButton.setColour(juce::TextButton::textColourOffId, textColour);
    filterButton.setColour(juce::TextButton::buttonOnColourId, accentColour);
    filterButton.onClick = [this]() { showFilterPanel(); };
    addAndMakeVisible(filterButton);
}

void PatternSearchComponent::setupClearButton()
{
    clearButton.setButtonText("×");
    clearButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    clearButton.setColour(juce::TextButton::textColourOffId, textColour);
    clearButton.onClick = [this]() { clearSearch(); };
    addAndMakeVisible(clearButton);
}

void PatternSearchComponent::setupResultCountLabel()
{
    resultCountLabel.setText("", juce::dontSendNotification);
    resultCountLabel.setFont(juce::FontOptions(12.0f));
    resultCountLabel.setColour(juce::Label::textColourId, textColour.withAlpha(0.7f));
    resultCountLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(resultCountLabel);
}

//==============================================================================
void PatternSearchComponent::textEditorTextChanged(juce::TextEditor& editor)
{
    if (&editor == &searchField)
    {
        // Debounce search updates
        isSearchPending = true;
        startTimer(300); // 300ms delay
    }
}

void PatternSearchComponent::textEditorFocusLost(juce::TextEditor& editor)
{
    // Ensure search is triggered when focus is lost
    if (&editor == &searchField && isSearchPending)
    {
        stopTimer();
        triggerSearchUpdate();
    }
}

void PatternSearchComponent::textEditorReturnKeyPressed(juce::TextEditor& editor)
{
    if (&editor == &searchField)
    {
        stopTimer();
        triggerSearchUpdate();
    }
}

//==============================================================================
void PatternSearchComponent::timerCallback()
{
    stopTimer();
    triggerSearchUpdate();
}

void PatternSearchComponent::triggerSearchUpdate()
{
    isSearchPending = false;
    currentCriteria.searchText = searchField.getText();
    
    if (onSearchChanged)
    {
        onSearchChanged(currentCriteria);
    }
}

//==============================================================================
void PatternSearchComponent::clearSearch()
{
    searchField.clear();
    currentCriteria = FilterCriteria();
    
    if (filterPanel != nullptr)
    {
        filterPanel->updateFromCriteria(currentCriteria);
    }
    
    if (onSearchChanged)
    {
        onSearchChanged(currentCriteria);
    }
    
    resultCountLabel.setText("", juce::dontSendNotification);
}

void PatternSearchComponent::focusSearchField()
{
    searchField.grabKeyboardFocus();
}

void PatternSearchComponent::setFilterCriteria(const FilterCriteria& criteria)
{
    currentCriteria = criteria;
    searchField.setText(criteria.searchText, false);
    
    if (filterPanel != nullptr)
    {
        filterPanel->updateFromCriteria(criteria);
    }
}

//==============================================================================
bool PatternSearchComponent::matchesPattern(const MIDIPattern& pattern, int patternIndex) const
{
    // Check text search
    if (!currentCriteria.searchText.isEmpty())
    {
        auto searchableText = extractSearchableText(pattern, patternIndex);
        
        if (currentCriteria.useRegex)
        {
            if (!performRegexSearch(currentCriteria.searchText, searchableText))
                return false;
        }
        else
        {
            if (!searchableText.containsIgnoreCase(currentCriteria.searchText))
                return false;
        }
    }
    
    // Check note count range
    int noteCount = static_cast<int>(pattern.notes.size());
    if (!currentCriteria.noteCountRange.contains(noteCount))
        return false;
    
    // For now, we don't have access to tempo, favorites, or date information in MIDIPattern
    // These would need to be extended in the pattern data structure
    
    return true;
}

juce::String PatternSearchComponent::extractSearchableText(const MIDIPattern& pattern, int patternIndex)
{
    juce::String searchableText;
    
    // Add pattern index
    searchableText += "Pattern " + juce::String(patternIndex + 1) + " ";
    
    // Add note count
    searchableText += juce::String(pattern.notes.size()) + " notes ";
    
    // Add note information
    for (const auto& note : pattern.notes)
    {
        searchableText += "pitch" + juce::String(note.pitch) + " ";
        searchableText += "vel" + juce::String(note.velocity) + " ";
    }
    
    return searchableText;
}

void PatternSearchComponent::showFilterPanel()
{
    if (filterPanel == nullptr)
    {
        filterPanel = std::make_unique<FilterPanel>(*this);
        filterPanel->onFilterChanged = [this]()
        {
            currentCriteria = filterPanel->getCriteria();
            currentCriteria.searchText = searchField.getText();
            
            if (onSearchChanged)
            {
                onSearchChanged(currentCriteria);
            }
        };
        
        getParentComponent()->addAndMakeVisible(filterPanel.get());
        filterPanel->updateFromCriteria(currentCriteria);
    }
    
    filterPanel->setVisible(true);
    filterButton.setToggleState(true, juce::dontSendNotification);
    resized(); // Update layout
}

void PatternSearchComponent::hideFilterPanel()
{
    if (filterPanel != nullptr)
    {
        filterPanel->setVisible(false);
        filterButton.setToggleState(false, juce::dontSendNotification);
    }
}

bool PatternSearchComponent::performRegexSearch(const juce::String& pattern, const juce::String& text) const
{
    try
    {
        std::regex regexPattern(pattern.toStdString(), std::regex_constants::icase);
        return std::regex_search(text.toStdString(), regexPattern);
    }
    catch (const std::regex_error&)
    {
        // If regex is invalid, fall back to simple text search
        return text.containsIgnoreCase(pattern);
    }
}

//==============================================================================
// FilterPanel Implementation
PatternSearchComponent::FilterPanel::FilterPanel(PatternSearchComponent& parent)
    : parentComponent(parent)
{
    setupControls();
}

void PatternSearchComponent::FilterPanel::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff2a2a2a));
    g.setColour(juce::Colour(0xff404040));
    g.drawRect(getLocalBounds(), 1);
    
    // Title
    g.setColour(juce::Colour(0xffcccccc));
    g.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    g.drawText("Filter Options", getLocalBounds().removeFromTop(30), juce::Justification::centred);
}

void PatternSearchComponent::FilterPanel::resized()
{
    auto bounds = getLocalBounds().reduced(10);
    bounds.removeFromTop(30); // Title space
    
    auto rowHeight = 25;
    auto spacing = 5;
    
    // Favorites toggle
    auto favRow = bounds.removeFromTop(rowHeight);
    favoritesOnlyToggle.setBounds(favRow);
    bounds.removeFromTop(spacing);
    
    // Regex toggle
    auto regexRow = bounds.removeFromTop(rowHeight);
    regexToggle.setBounds(regexRow);
    bounds.removeFromTop(spacing);
    
    // Tempo range
    auto tempoLabelRow = bounds.removeFromTop(rowHeight);
    tempoLabel.setBounds(tempoLabelRow);
    auto tempoRow = bounds.removeFromTop(rowHeight);
    tempoMinSlider.setBounds(tempoRow.removeFromLeft(tempoRow.getWidth() / 2 - 5));
    tempoMaxSlider.setBounds(tempoRow.removeFromRight(tempoRow.getWidth()));
    bounds.removeFromTop(spacing);
    
    // Note count range
    auto noteCountLabelRow = bounds.removeFromTop(rowHeight);
    noteCountLabel.setBounds(noteCountLabelRow);
    auto noteCountRow = bounds.removeFromTop(rowHeight);
    noteCountMinSlider.setBounds(noteCountRow.removeFromLeft(noteCountRow.getWidth() / 2 - 5));
    noteCountMaxSlider.setBounds(noteCountRow.removeFromRight(noteCountRow.getWidth()));
    bounds.removeFromTop(spacing * 2);
    
    // Buttons
    auto buttonRow = bounds.removeFromTop(rowHeight);
    resetButton.setBounds(buttonRow.removeFromLeft(80));
    buttonRow.removeFromLeft(10);
    applyButton.setBounds(buttonRow.removeFromLeft(80));
}

void PatternSearchComponent::FilterPanel::setupControls()
{
    // Favorites toggle
    favoritesOnlyToggle.setButtonText("Favorites Only");
    favoritesOnlyToggle.setColour(juce::ToggleButton::textColourId, juce::Colour(0xffcccccc));
    addAndMakeVisible(favoritesOnlyToggle);
    
    // Regex toggle
    regexToggle.setButtonText("Use Regular Expressions");
    regexToggle.setColour(juce::ToggleButton::textColourId, juce::Colour(0xffcccccc));
    addAndMakeVisible(regexToggle);
    
    // Tempo range
    tempoLabel.setText("Tempo Range:", juce::dontSendNotification);
    tempoLabel.setColour(juce::Label::textColourId, juce::Colour(0xffcccccc));
    addAndMakeVisible(tempoLabel);
    
    tempoMinSlider.setRange(60, 200, 1);
    tempoMinSlider.setValue(60);
    tempoMinSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    tempoMinSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible(tempoMinSlider);
    
    tempoMaxSlider.setRange(60, 200, 1);
    tempoMaxSlider.setValue(200);
    tempoMaxSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    tempoMaxSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible(tempoMaxSlider);
    
    // Note count range
    noteCountLabel.setText("Note Count Range:", juce::dontSendNotification);
    noteCountLabel.setColour(juce::Label::textColourId, juce::Colour(0xffcccccc));
    addAndMakeVisible(noteCountLabel);
    
    noteCountMinSlider.setRange(1, 128, 1);
    noteCountMinSlider.setValue(1);
    noteCountMinSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    noteCountMinSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible(noteCountMinSlider);
    
    noteCountMaxSlider.setRange(1, 128, 1);
    noteCountMaxSlider.setValue(128);
    noteCountMaxSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    noteCountMaxSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible(noteCountMaxSlider);
    
    // Buttons
    resetButton.setButtonText("Reset");
    resetButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff666666));
    resetButton.onClick = [this]()
    {
        favoritesOnlyToggle.setToggleState(false, juce::dontSendNotification);
        regexToggle.setToggleState(false, juce::dontSendNotification);
        tempoMinSlider.setValue(60, juce::dontSendNotification);
        tempoMaxSlider.setValue(200, juce::dontSendNotification);
        noteCountMinSlider.setValue(1, juce::dontSendNotification);
        noteCountMaxSlider.setValue(128, juce::dontSendNotification);
        
        if (onFilterChanged)
            onFilterChanged();
    };
    addAndMakeVisible(resetButton);
    
    applyButton.setButtonText("Apply");
    applyButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff4a9eff));
    applyButton.onClick = [this]()
    {
        if (onFilterChanged)
            onFilterChanged();
        parentComponent.hideFilterPanel();
    };
    addAndMakeVisible(applyButton);
}

void PatternSearchComponent::FilterPanel::updateFromCriteria(const FilterCriteria& criteria)
{
    favoritesOnlyToggle.setToggleState(criteria.favoritesOnly, juce::dontSendNotification);
    regexToggle.setToggleState(criteria.useRegex, juce::dontSendNotification);
    tempoMinSlider.setValue(criteria.tempoRange.getStart(), juce::dontSendNotification);
    tempoMaxSlider.setValue(criteria.tempoRange.getEnd(), juce::dontSendNotification);
    noteCountMinSlider.setValue(criteria.noteCountRange.getStart(), juce::dontSendNotification);
    noteCountMaxSlider.setValue(criteria.noteCountRange.getEnd(), juce::dontSendNotification);
}

PatternSearchComponent::FilterCriteria PatternSearchComponent::FilterPanel::getCriteria() const
{
    FilterCriteria criteria;
    criteria.favoritesOnly = favoritesOnlyToggle.getToggleState();
    criteria.useRegex = regexToggle.getToggleState();
    criteria.tempoRange = juce::Range<int>(static_cast<int>(tempoMinSlider.getValue()),
                                          static_cast<int>(tempoMaxSlider.getValue()));
    criteria.noteCountRange = juce::Range<int>(static_cast<int>(noteCountMinSlider.getValue()),
                                              static_cast<int>(noteCountMaxSlider.getValue()));
    return criteria;
}
