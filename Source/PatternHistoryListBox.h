/*
  ==============================================================================

    PatternHistoryListBox.h
    Created: 30 Jul 2025
    Author:  BMad Master

    Pattern History Panel UI Component - displays pattern history with
    interactive preview, favorite, and delete functionality.

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_data_structures/juce_data_structures.h>
#include "PatternManager.h"
#include "MIDIPattern.h"
#include "PatternSearchComponent.h"

//==============================================================================
/**
    Custom ListBox component for displaying pattern history with interactive
    elements. Provides real-time updates when patterns are added/removed.
*/
class PatternHistoryListBox : public juce::Component,
                             public juce::ListBoxModel,
                             public juce::ChangeListener,
                             public juce::Button::Listener,
                             public juce::DragAndDropContainer
{
public:
    PatternHistoryListBox(PatternManager& pm);
    ~PatternHistoryListBox() override;

    //==============================================================================
    // Component overrides
    void paint(juce::Graphics& g) override;
    void resized() override;

    //==============================================================================
    // ListBoxModel implementation
    int getNumRows() override;
    void paintListBoxItem(int rowNumber, juce::Graphics& g, 
                         int width, int height, bool rowIsSelected) override;
    juce::Component* refreshComponentForRow(int rowNumber, bool isRowSelected,
                                          juce::Component* existingComponentToUpdate) override;
    void listBoxItemClicked(int row, const juce::MouseEvent& e) override;

    //==============================================================================
    // ChangeListener for PatternManager updates
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    //==============================================================================
    // Button handling for preview/favorite/delete
    void buttonClicked(juce::Button* button) override;

    //==============================================================================
    // Public interface
    void updatePatternList();
    void selectPattern(int index);
    void reorderPattern(int fromIndex, int toIndex); // Epic 3 Story 3.3: Drag & Drop
    int getSelectedPatternIndex() const { return selectedPatternIndex; }
    
    // Epic 3 Story 3.3: Search and filtering
    void setSearchCriteria(const PatternSearchComponent::FilterCriteria& criteria);
    void refreshFilteredPatterns();

    //==============================================================================
    // Callback for parent component to handle pattern actions
    std::function<void(int patternIndex)> onPatternPreview;
    std::function<void(int patternIndex)> onPatternSelect;
    std::function<void(int patternIndex)> onPatternDelete;

private:
    //==============================================================================
    // Pattern Row Component
    class PatternRowComponent : public juce::Component,
                              public juce::DragAndDropContainer,
                              public juce::DragAndDropTarget
    {
    public:
        PatternRowComponent(PatternHistoryListBox& parent, int rowIndex);
        ~PatternRowComponent() override = default;

        void paint(juce::Graphics& g) override;
        void resized() override;
        void updatePattern(const MIDIPattern& pattern, int index, bool isSelected);
        
        // Mouse handling for drag initiation
        void mouseDown(const juce::MouseEvent& e) override;
        void mouseDrag(const juce::MouseEvent& e) override;
        
        // DragAndDropTarget implementation
        bool isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override;
        void itemDragEnter(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override;
        void itemDragExit(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override;
        void itemDropped(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override;

    private:
        PatternHistoryListBox& parentList;
        int patternIndex;
        MIDIPattern currentPattern;
        bool isSelectedRow = false;
        bool isFavorited = false;
        
        // Drag & Drop state
        bool isDragging = false;
        bool isDropTarget = false;
        bool showDropIndicatorAbove = false;

        juce::TextButton previewButton;
        juce::TextButton favoriteButton;
        juce::TextButton deleteButton;

        void drawMiniPianoRoll(juce::Graphics& g, juce::Rectangle<int> area);
        void drawDropIndicator(juce::Graphics& g);
        juce::String formatPatternInfo() const;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PatternRowComponent)
    };

    //==============================================================================
    PatternManager& patternManager;
    juce::ListBox listBox;
    PatternSearchComponent searchComponent;
    
    // Pattern storage for filtering
    std::vector<MIDIPattern> currentPatterns;
    std::vector<MIDIPattern> filteredPatterns;
    PatternSearchComponent::FilterCriteria currentFilter;
    
    int selectedPatternIndex = -1;

    // UI Colors (matching professional theme from Story 3.1)
    juce::Colour backgroundColour = juce::Colour(0xff2a2a2a);
    juce::Colour selectedColour = juce::Colour(0xff404040);
    juce::Colour textColour = juce::Colour(0xffcccccc);
    juce::Colour accentColour = juce::Colour(0xff4a9eff);

    void setupListBox();
    void handlePatternAction(int patternIndex, const juce::String& action);
    bool matchesFilter(const MIDIPattern& pattern) const;
    void updateFilteredPatterns();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PatternHistoryListBox)
};
