/*
  ==============================================================================

    PatternHistoryListBox.cpp
    Created: 30 Jul 2025
    Author:  BMad Master

    Implementation of Pattern History Panel UI Component.

  ==============================================================================
*/

#include "PatternHistoryListBox.h"
#include <regex>

//==============================================================================
PatternHistoryListBox::PatternHistoryListBox(PatternManager& pm)
    : patternManager(pm), searchComponent()
{
    setupListBox();
    
    // Setup search component
    addAndMakeVisible(searchComponent);
    searchComponent.onSearchChanged = [this](const PatternSearchComponent::FilterCriteria& criteria) {
        setSearchCriteria(criteria);
    };
    
    // Register for pattern manager updates
    patternManager.addChangeListener(this);
    
    // Initial pattern list update
    updatePatternList();
}

PatternHistoryListBox::~PatternHistoryListBox()
{
    patternManager.removeChangeListener(this);
}

//==============================================================================
void PatternHistoryListBox::paint(juce::Graphics& g)
{
    g.fillAll(backgroundColour);
    
    // Draw header
    auto headerArea = getLocalBounds().removeFromTop(30);
    g.setColour(textColour);
    g.setFont(juce::FontOptions(16.0f, juce::Font::bold));
    g.drawText("Pattern History", headerArea, juce::Justification::centred);
    
    // Draw border
    g.setColour(juce::Colour(0xff404040));
    g.drawRect(getLocalBounds(), 1);
}

void PatternHistoryListBox::resized()
{
    auto bounds = getLocalBounds();
    
    // Header space
    auto headerArea = bounds.removeFromTop(30);
    
    // Search component area (100px height)
    auto searchArea = bounds.removeFromTop(100);
    searchArea.reduce(2, 2);
    searchComponent.setBounds(searchArea);
    
    // Remaining space for list
    bounds.reduce(2, 2); // Border padding
    listBox.setBounds(bounds);
}

//==============================================================================
void PatternHistoryListBox::setupListBox()
{
    listBox.setModel(this);
    listBox.setColour(juce::ListBox::backgroundColourId, backgroundColour);
    listBox.setColour(juce::ListBox::outlineColourId, juce::Colours::transparentBlack);
    listBox.setRowHeight(60);
    listBox.setMultipleSelectionEnabled(false);
    
    addAndMakeVisible(listBox);
}

//==============================================================================
// ListBoxModel implementation
int PatternHistoryListBox::getNumRows()
{
    return static_cast<int>(filteredPatterns.size());
}

void PatternHistoryListBox::paintListBoxItem(int rowNumber, juce::Graphics& g, 
                                           int width, int height, bool rowIsSelected)
{
    // Background handled by PatternRowComponent
}

juce::Component* PatternHistoryListBox::refreshComponentForRow(int rowNumber, bool isRowSelected,
                                                            juce::Component* existingComponentToUpdate)
{
    if (rowNumber >= 0 && rowNumber < static_cast<int>(filteredPatterns.size()))
    {
        auto* rowComponent = dynamic_cast<PatternRowComponent*>(existingComponentToUpdate);
        
        if (rowComponent == nullptr)
        {
            rowComponent = new PatternRowComponent(*this, rowNumber);
        }
        
        rowComponent->updatePattern(filteredPatterns[rowNumber], rowNumber, isRowSelected || rowNumber == selectedPatternIndex);
        return rowComponent;
    }
    
    delete existingComponentToUpdate;
    return nullptr;
}

void PatternHistoryListBox::listBoxItemClicked(int row, const juce::MouseEvent& e)
{
    if (row >= 0 && row < static_cast<int>(filteredPatterns.size()))
    {
        selectPattern(row);
    }
}

//==============================================================================
void PatternHistoryListBox::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &patternManager)
    {
        // Update on message thread
        juce::MessageManager::callAsync([this]()
        {
            updatePatternList();
        });
    }
}

//==============================================================================
void PatternHistoryListBox::buttonClicked(juce::Button* button)
{
    // Button handling is delegated to PatternRowComponent
}

//==============================================================================
void PatternHistoryListBox::updatePatternList()
{
    // Get recent patterns from manager (last 20 for UI display)
    currentPatterns = patternManager.getRecentPatterns(20);
    
    // Reverse order so newest patterns appear at top
    std::reverse(currentPatterns.begin(), currentPatterns.end());
    
    // Update filtered patterns based on current search criteria
    updateFilteredPatterns();
    
    // Update list box
    listBox.updateContent();
    listBox.repaint();
    
    // Maintain selection if valid
    if (selectedPatternIndex >= static_cast<int>(filteredPatterns.size()))
    {
        selectedPatternIndex = -1;
    }
}

void PatternHistoryListBox::selectPattern(int index)
{
    if (index >= 0 && index < static_cast<int>(currentPatterns.size()))
    {
        selectedPatternIndex = index;
        listBox.selectRow(index);
        listBox.repaint();
        
        if (onPatternSelect)
        {
            onPatternSelect(index);
        }
    }
}

void PatternHistoryListBox::handlePatternAction(int patternIndex, const juce::String& action)
{
    if (patternIndex < 0 || patternIndex >= static_cast<int>(currentPatterns.size()))
        return;
        
    if (action == "preview" && onPatternPreview)
    {
        onPatternPreview(patternIndex);
    }
    else if (action == "delete" && onPatternDelete)
    {
        // Show confirmation dialog
        auto options = juce::MessageBoxOptions()
            .withIconType(juce::AlertWindow::QuestionIcon)
            .withTitle("Delete Pattern")
            .withMessage("Are you sure you want to delete this pattern from history?")
            .withButton("Delete")
            .withButton("Cancel");
            
        juce::AlertWindow::showAsync(options, [this, patternIndex](int result)
        {
            if (result == 1) // Delete button
            {
                onPatternDelete(patternIndex);
            }
        });
    }
}

//==============================================================================
// PatternRowComponent Implementation
PatternHistoryListBox::PatternRowComponent::PatternRowComponent(PatternHistoryListBox& parent, int rowIndex)
    : parentList(parent), patternIndex(rowIndex)
{
    // Setup buttons
    previewButton.setButtonText("▶");
    previewButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff4a9eff));
    previewButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    previewButton.onClick = [this]() { parentList.handlePatternAction(patternIndex, "preview"); };
    addAndMakeVisible(previewButton);
    
    favoriteButton.setButtonText("♡");
    favoriteButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff555555));
    favoriteButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    favoriteButton.onClick = [this]() 
    { 
        isFavorited = !isFavorited;
        favoriteButton.setButtonText(isFavorited ? "♥" : "♡");
        favoriteButton.setColour(juce::TextButton::buttonColourId, 
                                isFavorited ? juce::Colour(0xffff4444) : juce::Colour(0xff555555));
    };
    addAndMakeVisible(favoriteButton);
    
    deleteButton.setButtonText("×");
    deleteButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff666666));
    deleteButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    deleteButton.onClick = [this]() { parentList.handlePatternAction(patternIndex, "delete"); };
    addAndMakeVisible(deleteButton);
}

void PatternHistoryListBox::PatternRowComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    
    // Background
    if (isSelectedRow)
    {
        g.setColour(parentList.selectedColour);
        g.fillRect(bounds);
    }
    else if (isDragging)
    {
        // Semi-transparent when being dragged
        g.setColour(parentList.backgroundColour.withAlpha(0.7f));
        g.fillRect(bounds);
    }
    else
    {
        g.setColour(parentList.backgroundColour);
        g.fillRect(bounds);
    }
    
    // Border
    g.setColour(juce::Colour(0xff404040));
    g.drawRect(bounds, 1);
    
    // Draw drop indicator if this is a drop target
    drawDropIndicator(g);
    
    // Pattern info text
    auto textArea = bounds.reduced(10, 5);
    textArea.removeFromRight(150); // Space for buttons
    
    g.setColour(parentList.textColour.withAlpha(isDragging ? 0.7f : 1.0f));
    g.setFont(juce::FontOptions(12.0f));
    g.drawText(formatPatternInfo(), textArea.removeFromTop(20), juce::Justification::topLeft);
    
    // Mini piano roll visualization
    if (textArea.getHeight() > 10)
    {
        auto pianoRollArea = textArea.reduced(0, 2);
        drawMiniPianoRoll(g, pianoRollArea);
    }
}

void PatternHistoryListBox::PatternRowComponent::resized()
{
    auto bounds = getLocalBounds();
    auto buttonArea = bounds.removeFromRight(140);
    buttonArea = buttonArea.reduced(5);
    
    // Arrange buttons horizontally
    auto buttonWidth = (buttonArea.getWidth() - 10) / 3;
    previewButton.setBounds(buttonArea.removeFromLeft(buttonWidth));
    buttonArea.removeFromLeft(5);
    favoriteButton.setBounds(buttonArea.removeFromLeft(buttonWidth));
    buttonArea.removeFromLeft(5);
    deleteButton.setBounds(buttonArea);
}

void PatternHistoryListBox::PatternRowComponent::updatePattern(const MIDIPattern& pattern, int index, bool isSelected)
{
    currentPattern = pattern;
    patternIndex = index;
    isSelectedRow = isSelected;
    repaint();
}

void PatternHistoryListBox::PatternRowComponent::drawMiniPianoRoll(juce::Graphics& g, juce::Rectangle<int> area)
{
    if (area.getWidth() < 10 || area.getHeight() < 10)
        return;
        
    // Simple visualization - draw note blocks
    g.setColour(juce::Colour(0xff666666));
    g.drawRect(area, 1);
    
    if (currentPattern.notes.empty())
        return;
    
    // Find note range for scaling
    int minNote = 127, maxNote = 0;
    double maxTime = 0.0;
    
    for (const auto& note : currentPattern.notes)
    {
        minNote = juce::jmin(minNote, note.pitch);
        maxNote = juce::jmax(maxNote, note.pitch);
        maxTime = juce::jmax(maxTime, note.startTime + note.duration);
    }
    
    if (maxNote <= minNote || maxTime <= 0.0)
        return;
    
    // Draw notes
    g.setColour(parentList.accentColour);
    auto drawArea = area.reduced(2);
    
    for (const auto& note : currentPattern.notes)
    {
        // Scale position
        float x = static_cast<float>(note.startTime / maxTime) * drawArea.getWidth();
        float width = juce::jmax(2.0f, static_cast<float>(note.duration / maxTime) * drawArea.getWidth());
        float y = static_cast<float>(maxNote - note.pitch) / (maxNote - minNote) * drawArea.getHeight();
        float height = juce::jmax(1.0f, drawArea.getHeight() / 12.0f); // Approximate note height
        
        juce::Rectangle<float> noteRect(drawArea.getX() + x, drawArea.getY() + y, width, height);
        g.fillRect(noteRect);
    }
}

juce::String PatternHistoryListBox::PatternRowComponent::formatPatternInfo() const
{
    auto timeStr = juce::Time::getCurrentTime().toString(true, true, false, true);
    auto noteCount = static_cast<int>(currentPattern.notes.size());
    
    return juce::String("Pattern #") + juce::String(patternIndex + 1) + 
           " - " + juce::String(noteCount) + " notes - " + timeStr;
}

//==============================================================================
// Epic 3 Story 3.3: Drag & Drop Implementation

void PatternHistoryListBox::reorderPattern(int fromIndex, int toIndex)
{
    if (fromIndex == toIndex || fromIndex < 0 || toIndex < 0 || 
        fromIndex >= static_cast<int>(currentPatterns.size()) || 
        toIndex >= static_cast<int>(currentPatterns.size()))
        return;
    
    // Reorder patterns in our local list
    auto patternToMove = currentPatterns[fromIndex];
    currentPatterns.erase(currentPatterns.begin() + fromIndex);
    currentPatterns.insert(currentPatterns.begin() + toIndex, patternToMove);
    
    // Update the list box display
    listBox.updateContent();
    listBox.repaint();
    
    // Update selection to follow the moved pattern
    if (selectedPatternIndex == fromIndex)
    {
        selectedPatternIndex = toIndex;
        listBox.selectRow(toIndex);
    }
    else if (selectedPatternIndex > fromIndex && selectedPatternIndex <= toIndex)
    {
        selectedPatternIndex--;
    }
    else if (selectedPatternIndex < fromIndex && selectedPatternIndex >= toIndex)
    {
        selectedPatternIndex++;
    }
}

// PatternRowComponent Drag & Drop Implementation
void PatternHistoryListBox::PatternRowComponent::mouseDown(const juce::MouseEvent& e)
{
    if (e.mods.isLeftButtonDown() && !e.mods.isRightButtonDown())
    {
        // Don't start drag if clicking on buttons
        if (previewButton.getBounds().contains(e.getPosition()) ||
            favoriteButton.getBounds().contains(e.getPosition()) ||
            deleteButton.getBounds().contains(e.getPosition()))
        {
            return;
        }
        
        // Prepare for potential drag operation
        isDragging = false;
    }
}

void PatternHistoryListBox::PatternRowComponent::mouseDrag(const juce::MouseEvent& e)
{
    if (e.mods.isLeftButtonDown() && !isDragging)
    {
        // Start drag operation if moved sufficient distance
        if (e.getDistanceFromDragStart() > 10)
        {
            isDragging = true;
            repaint();
            
            // For now, just enable visual feedback
            // TODO: Implement actual drag and drop in future iteration
        }
    }
}

bool PatternHistoryListBox::PatternRowComponent::isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails)
{
    // Only accept pattern row drags
    return dragSourceDetails.description.toString().startsWith("PatternRow:");
}

void PatternHistoryListBox::PatternRowComponent::itemDragEnter(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails)
{
    auto draggedIndex = dragSourceDetails.description.toString().fromFirstOccurrenceOf(":", false, false).getIntValue();
    
    // Don't accept drops on self
    if (draggedIndex != patternIndex)
    {
        isDropTarget = true;
        
        // Determine if drop indicator should be above or below
        auto localMousePos = dragSourceDetails.localPosition;
        showDropIndicatorAbove = localMousePos.getY() < getHeight() / 2;
        
        repaint();
    }
}

void PatternHistoryListBox::PatternRowComponent::itemDragExit(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails)
{
    isDropTarget = false;
    repaint();
}

void PatternHistoryListBox::PatternRowComponent::itemDropped(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails)
{
    auto draggedIndex = dragSourceDetails.description.toString().fromFirstOccurrenceOf(":", false, false).getIntValue();
    
    if (draggedIndex != patternIndex)
    {
        // Calculate target index based on drop position
        int targetIndex = patternIndex;
        if (!showDropIndicatorAbove)
        {
            targetIndex++;
        }
        
        // Adjust for source removal
        if (draggedIndex < targetIndex)
        {
            targetIndex--;
        }
        
        // Perform the reorder
        parentList.reorderPattern(draggedIndex, targetIndex);
    }
    
    isDropTarget = false;
    isDragging = false;
    repaint();
}

void PatternHistoryListBox::PatternRowComponent::drawDropIndicator(juce::Graphics& g)
{
    if (!isDropTarget)
        return;
        
    // Draw drop indicator line
    g.setColour(juce::Colour(0xff4a9eff));
    
    auto bounds = getLocalBounds();
    int y = showDropIndicatorAbove ? bounds.getY() : bounds.getBottom() - 1;
    
    // Draw thick line with some glow effect
    for (int i = 0; i < 3; ++i)
    {
        g.setOpacity(0.7f - i * 0.2f);
        g.drawHorizontalLine(y + i - 1, static_cast<float>(bounds.getX() + 5), static_cast<float>(bounds.getRight() - 5));
    }
}

//==============================================================================
// Epic 3 Story 3.3: Search and filtering implementation

void PatternHistoryListBox::setSearchCriteria(const PatternSearchComponent::FilterCriteria& criteria)
{
    currentFilter = criteria;
    updateFilteredPatterns();
}

void PatternHistoryListBox::refreshFilteredPatterns()
{
    updateFilteredPatterns();
}

bool PatternHistoryListBox::matchesFilter(const MIDIPattern& pattern) const
{
    // Search text filter - search in pattern ID or metadata
    if (!currentFilter.searchText.isEmpty())
    {
        juce::String searchText = currentFilter.searchText.toLowerCase();
        juce::String patternId = pattern.id.toString().toLowerCase();
        
        if (currentFilter.useRegex)
        {
            try 
            {
                std::regex searchRegex(searchText.toStdString(), std::regex_constants::icase);
                if (!std::regex_search(patternId.toStdString(), searchRegex))
                    return false;
            }
            catch (const std::regex_error&)
            {
                // Fall back to simple text search if regex is invalid
                if (!patternId.contains(searchText))
                    return false;
            }
        }
        else
        {
            if (!patternId.contains(searchText))
                return false;
        }
    }
    
    // Note count range filter
    int noteCount = static_cast<int>(pattern.notes.size());
    if (!currentFilter.noteCountRange.contains(noteCount))
        return false;
    
    // Tempo range filter using metadata tempo
    if (pattern.metadata.tempo != 0.0f)
    {
        int patternBpm = static_cast<int>(pattern.metadata.tempo);
        if (!currentFilter.tempoRange.contains(patternBpm))
            return false;
    }
    
    return true;
}

void PatternHistoryListBox::updateFilteredPatterns()
{
    filteredPatterns.clear();
    
    for (const auto& pattern : currentPatterns)
    {
        if (matchesFilter(pattern))
        {
            filteredPatterns.push_back(pattern);
        }
    }
    
    // Update UI
    listBox.updateContent();
    listBox.repaint();
    
    // Reset selection if it's no longer valid
    if (selectedPatternIndex >= static_cast<int>(filteredPatterns.size()))
    {
        selectedPatternIndex = -1;
    }
}
