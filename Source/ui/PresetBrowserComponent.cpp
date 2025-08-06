/*
  ==============================================================================

    PresetBrowserComponent.cpp
    Created: 4 Aug 2025
    Author:  Epic 9 Story 9.1 Task 9.1.3 - UI Development

    Implementation of preset browser UI component for navigating and selecting
    instrument presets from the InstrumentLibraryManager system.

  ==============================================================================
*/

#include "PresetBrowserComponent.h"

//==============================================================================
// PresetBrowserComponent Implementation

PresetBrowserComponent::PresetBrowserComponent()
{
    initializeComponents();
}

PresetBrowserComponent::~PresetBrowserComponent()
{
}

void PresetBrowserComponent::paint(juce::Graphics& g)
{
    // Background
    g.fillAll(juce::Colour(0xff2a2a2a));
    
    // Header
    auto headerArea = getLocalBounds().removeFromTop(30);
    g.setColour(juce::Colour(0xff404040));
    g.fillRect(headerArea);
    
    g.setColour(juce::Colours::white);
    g.setFont(juce::FontOptions(16.0f, juce::Font::bold));
    g.drawText("Instrument Presets", headerArea, juce::Justification::centred);
    
    // Subtle borders
    g.setColour(juce::Colour(0xff505050));
    g.drawRect(getLocalBounds(), 1);
}

void PresetBrowserComponent::resized()
{
    auto area = getLocalBounds();
    area.removeFromTop(30); // Header space
    area.reduce(8, 8);
    
    // Search and filter section (top)
    auto filterArea = area.removeFromTop(100);
    
    // Search box
    auto searchRow = filterArea.removeFromTop(25);
    searchLabel->setBounds(searchRow.removeFromLeft(60));
    searchEditor->setBounds(searchRow);
    
    filterArea.removeFromTop(5); // Spacing
    
    // Category and tag filters
    auto filterRow = filterArea.removeFromTop(25);
    categoryLabel->setBounds(filterRow.removeFromLeft(70));
    categoryComboBox->setBounds(filterRow.removeFromLeft(150));
    
    filterRow.removeFromLeft(10); // Spacing
    tagLabel->setBounds(filterRow.removeFromLeft(40));
    tagComboBox->setBounds(filterRow.removeFromLeft(120));
    
    filterRow.removeFromLeft(10);
    clearFiltersButton->setBounds(filterRow.removeFromLeft(80));
    
    area.removeFromTop(10); // Spacing
    
    // Main content area
    auto contentArea = area;
    
    // Preset list (left side)
    auto listArea = contentArea.removeFromLeft(300);
    presetListLabel->setBounds(listArea.removeFromTop(20));
    presetListBox->setBounds(listArea);
    
    contentArea.removeFromLeft(10); // Spacing
    
    // Details and actions (right side)
    auto detailsArea = contentArea;
    
    // Details group
    auto detailsBounds = detailsArea.removeFromTop(200);
    detailsGroup->setBounds(detailsBounds);
    
    // Position details labels inside the group
    auto innerArea = detailsBounds.reduced(10, 25); // Account for group border and title
    presetNameLabel->setBounds(innerArea.removeFromTop(25));
    innerArea.removeFromTop(5);
    presetCategoryLabel->setBounds(innerArea.removeFromTop(20));
    innerArea.removeFromTop(5);
    presetTagsLabel->setBounds(innerArea.removeFromTop(20));
    innerArea.removeFromTop(5);
    presetDescriptionLabel->setBounds(innerArea);
    
    detailsArea.removeFromTop(15); // Spacing
    
    // Action buttons
    auto buttonArea = detailsArea.removeFromTop(30);
    loadPresetButton->setBounds(buttonArea.removeFromLeft(100));
    buttonArea.removeFromLeft(10);
    previewButton->setBounds(buttonArea.removeFromLeft(80));
    buttonArea.removeFromLeft(10);
    autoPreviewToggle->setBounds(buttonArea.removeFromLeft(100));
    
    // Epic 9.3: Recommendation panel (bottom area)
    if (detailsArea.getHeight() > 120)
    {
        detailsArea.removeFromTop(15); // Spacing
        recommendationPanel->setBounds(detailsArea);
    }
}

//==============================================================================
// Integration with InstrumentLibraryManager

void PresetBrowserComponent::setInstrumentLibraryManager(InstrumentLibraryManager* manager)
{
    instrumentLibrary = manager;
    
    if (instrumentLibrary != nullptr)
    {
        updateCategoryComboBox();
        updateTagComboBox();
        updatePresetList();
    }
}

InstrumentLibraryManager::PresetData PresetBrowserComponent::getCurrentlySelectedPreset() const
{
    return selectedPreset;
}

void PresetBrowserComponent::selectPreset(const juce::String& presetId)
{
    if (instrumentLibrary == nullptr)
        return;
        
    // Find preset in current list
    for (int i = 0; i < currentPresetList.size(); ++i)
    {
        if (currentPresetList[i].presetId == presetId)
        {
            presetListBox->selectRow(i);
            selectedPresetIndex = i;
            selectedPreset = currentPresetList[i];
            updatePresetDetails();
            break;
        }
    }
}

void PresetBrowserComponent::refreshPresetList()
{
    updatePresetList();
}

//==============================================================================
// Event handlers

void PresetBrowserComponent::textEditorTextChanged(juce::TextEditor& editor)
{
    if (&editor == searchEditor.get())
    {
        currentSearchText = editor.getText();
        updatePresetList();
    }
}

void PresetBrowserComponent::comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == categoryComboBox.get())
    {
        currentCategoryFilter = categoryComboBox->getText();
        updatePresetList();
    }
    else if (comboBoxThatHasChanged == tagComboBox.get())
    {
        currentTagFilter = tagComboBox->getText();
        updatePresetList();
    }
}

void PresetBrowserComponent::listBoxItemClicked(int row, const juce::MouseEvent& e)
{
    if (row >= 0 && row < currentPresetList.size())
    {
        selectedPresetIndex = row;
        selectedPreset = currentPresetList[row];
        updatePresetDetails();
        
        // Auto-preview if enabled
        if (autoPreviewToggle->getToggleState() && onPresetPreview)
        {
            onPresetPreview(selectedPreset);
        }
        
        // Notify selection callback
        if (onPresetSelected)
        {
            onPresetSelected(selectedPreset);
        }
    }
}

void PresetBrowserComponent::listBoxItemDoubleClicked(int row, const juce::MouseEvent& e)
{
    if (row >= 0 && row < currentPresetList.size())
    {
        selectedPresetIndex = row;
        selectedPreset = currentPresetList[row];
        
        // Double-click loads the preset immediately
        if (onPresetDoubleClicked)
        {
            onPresetDoubleClicked(selectedPreset);
        }
    }
}

void PresetBrowserComponent::onPresetListSelectionChanged(int selectedIndex)
{
    if (selectedIndex >= 0 && selectedIndex < currentPresetList.size())
    {
        selectedPresetIndex = selectedIndex;
        selectedPreset = currentPresetList[selectedIndex];
        updatePresetDetails();
        
        // Auto-preview if enabled
        if (autoPreviewToggle->getToggleState() && onPresetPreview)
        {
            onPresetPreview(selectedPreset);
        }
        
        // Notify selection callback
        if (onPresetSelected)
        {
            onPresetSelected(selectedPreset);
        }
    }
}

//==============================================================================
// Button callbacks

void PresetBrowserComponent::clearFiltersClicked()
{
    // Clear all filters
    searchEditor->clear();
    categoryComboBox->setSelectedId(1); // "All Categories"
    tagComboBox->setSelectedId(1); // "All Tags"
    
    currentSearchText.clear();
    currentCategoryFilter.clear();
    currentTagFilter.clear();
    
    updatePresetList();
}

void PresetBrowserComponent::loadPresetClicked()
{
    if (selectedPresetIndex >= 0 && onPresetDoubleClicked)
    {
        onPresetDoubleClicked(selectedPreset);
    }
}

void PresetBrowserComponent::previewButtonClicked()
{
    if (selectedPresetIndex >= 0 && onPresetPreview)
    {
        onPresetPreview(selectedPreset);
    }
}

void PresetBrowserComponent::autoPreviewToggled()
{
    // Auto-preview setting is stored in the toggle state
    // Preview current selection if auto-preview is now enabled
    if (autoPreviewToggle->getToggleState() && selectedPresetIndex >= 0 && onPresetPreview)
    {
        onPresetPreview(selectedPreset);
    }
}

//==============================================================================
// Internal methods

void PresetBrowserComponent::initializeComponents()
{
    // Search components
    searchLabel = std::make_unique<juce::Label>("searchLabel", "Search:");
    searchLabel->setFont(juce::FontOptions(14.0f));
    searchLabel->setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(*searchLabel);
    
    searchEditor = std::make_unique<juce::TextEditor>("searchEditor");
    searchEditor->setTextToShowWhenEmpty("Type to search presets...", juce::Colours::grey);
    searchEditor->addListener(this);
    addAndMakeVisible(*searchEditor);
    
    // Category filter
    categoryLabel = std::make_unique<juce::Label>("categoryLabel", "Category:");
    categoryLabel->setFont(juce::FontOptions(14.0f));
    categoryLabel->setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(*categoryLabel);
    
    categoryComboBox = std::make_unique<juce::ComboBox>("categoryComboBox");
    categoryComboBox->addListener(this);
    addAndMakeVisible(*categoryComboBox);
    
    // Tag filter
    tagLabel = std::make_unique<juce::Label>("tagLabel", "Tag:");
    tagLabel->setFont(juce::FontOptions(14.0f));
    tagLabel->setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(*tagLabel);
    
    tagComboBox = std::make_unique<juce::ComboBox>("tagComboBox");
    tagComboBox->addListener(this);
    addAndMakeVisible(*tagComboBox);
    
    // Clear filters button
    clearFiltersButton = std::make_unique<juce::TextButton>("Clear");
    clearFiltersButton->onClick = [this] { clearFiltersClicked(); };
    addAndMakeVisible(*clearFiltersButton);
    
    // Preset list
    presetListLabel = std::make_unique<juce::Label>("presetListLabel", "Presets:");
    presetListLabel->setFont(juce::FontOptions(14.0f, juce::Font::bold));
    presetListLabel->setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(*presetListLabel);
    
    presetListModel = std::make_unique<PresetListBoxModel>(*this);
    presetListBox = std::make_unique<juce::ListBox>("presetListBox", presetListModel.get());
    presetListBox->setMultipleSelectionEnabled(false);
    presetListBox->setRowHeight(22);
    presetListBox->setColour(juce::ListBox::backgroundColourId, juce::Colour(0xff1a1a1a));
    presetListBox->setColour(juce::ListBox::outlineColourId, juce::Colour(0xff505050));
    addAndMakeVisible(*presetListBox);
    
    // Details group
    detailsGroup = std::make_unique<juce::GroupComponent>("detailsGroup", "Preset Details");
    detailsGroup->setColour(juce::GroupComponent::textColourId, juce::Colours::white);
    detailsGroup->setColour(juce::GroupComponent::outlineColourId, juce::Colour(0xff505050));
    addAndMakeVisible(*detailsGroup);
    
    // Detail labels
    presetNameLabel = std::make_unique<juce::Label>("presetNameLabel", "No preset selected");
    presetNameLabel->setFont(juce::FontOptions(16.0f, juce::Font::bold));
    presetNameLabel->setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(*presetNameLabel);
    
    presetCategoryLabel = std::make_unique<juce::Label>("presetCategoryLabel", "");
    presetCategoryLabel->setFont(juce::FontOptions(14.0f));
    presetCategoryLabel->setColour(juce::Label::textColourId, juce::Colour(0xffaaaaaa));
    addAndMakeVisible(*presetCategoryLabel);
    
    presetTagsLabel = std::make_unique<juce::Label>("presetTagsLabel", "");
    presetTagsLabel->setFont(juce::FontOptions(12.0f));
    presetTagsLabel->setColour(juce::Label::textColourId, juce::Colour(0xff888888));
    addAndMakeVisible(*presetTagsLabel);
    
    presetDescriptionLabel = std::make_unique<juce::Label>("presetDescriptionLabel", "");
    presetDescriptionLabel->setFont(juce::FontOptions(13.0f));
    presetDescriptionLabel->setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    presetDescriptionLabel->setJustificationType(juce::Justification::topLeft);
    addAndMakeVisible(*presetDescriptionLabel);
    
    // Action buttons
    loadPresetButton = std::make_unique<juce::TextButton>("Load");
    loadPresetButton->setColour(juce::TextButton::buttonColourId, juce::Colour(0xff4a90e2));
    loadPresetButton->onClick = [this] { loadPresetClicked(); };
    addAndMakeVisible(*loadPresetButton);
    
    previewButton = std::make_unique<juce::TextButton>("Preview");
    previewButton->onClick = [this] { previewButtonClicked(); };
    addAndMakeVisible(*previewButton);
    
    autoPreviewToggle = std::make_unique<juce::ToggleButton>("Auto Preview");
    autoPreviewToggle->setColour(juce::ToggleButton::textColourId, juce::Colours::white);
    autoPreviewToggle->onClick = [this] { autoPreviewToggled(); };
    addAndMakeVisible(*autoPreviewToggle);
    
    // Epic 9.3: Initialize recommendation panel
    recommendationPanel = std::make_unique<RecommendationPanel>();
    recommendationPanel->onRecommendationSelected = [this](const std::string& presetName) {
        // Find and load the selected preset
        juce::String jucePresetName(presetName);
        for (int i = 0; i < presetListModel->getNumRows(); ++i) {
            const auto& preset = presetListModel->getPreset(i);
            if (preset.name == jucePresetName) {
                selectedPresetIndex = i;
                selectedPreset = preset;
                presetListBox->selectRow(i);
                if (onPresetDoubleClicked)
                    onPresetDoubleClicked(preset);
                break;
            }
        }
    };
    addAndMakeVisible(*recommendationPanel);
}

void PresetBrowserComponent::updatePresetList()
{
    currentPresetList = getFilteredPresets();
    presetListModel->setPresets(currentPresetList);
    presetListBox->updateContent();
    
    // Clear selection if current selection is no longer valid
    if (selectedPresetIndex >= currentPresetList.size())
    {
        selectedPresetIndex = -1;
        selectedPreset = InstrumentLibraryManager::PresetData{};
        updatePresetDetails();
        presetListBox->deselectAllRows();
    }
    
    // Update list label with count
    presetListLabel->setText("Presets (" + juce::String(currentPresetList.size()) + "):", 
                            juce::dontSendNotification);
}

void PresetBrowserComponent::updateCategoryComboBox()
{
    if (instrumentLibrary == nullptr)
        return;
        
    categoryComboBox->clear();
    categoryComboBox->addItem("All Categories", 1);
    
    auto categories = instrumentLibrary->getAvailableCategories();
    for (int i = 0; i < categories.size(); ++i)
    {
        categoryComboBox->addItem(categories[i], i + 2);
    }
    
    categoryComboBox->setSelectedId(1);
}

void PresetBrowserComponent::updateTagComboBox()
{
    if (instrumentLibrary == nullptr)
        return;
        
    tagComboBox->clear();
    tagComboBox->addItem("All Tags", 1);
    
    auto tags = instrumentLibrary->getAvailableTags();
    for (int i = 0; i < tags.size(); ++i)
    {
        tagComboBox->addItem(tags[i], i + 2);
    }
    
    tagComboBox->setSelectedId(1);
}

void PresetBrowserComponent::updatePresetDetails()
{
    if (selectedPresetIndex < 0 || selectedPreset.presetId.isEmpty())
    {
        presetNameLabel->setText("No preset selected", juce::dontSendNotification);
        presetCategoryLabel->setText("", juce::dontSendNotification);
        presetTagsLabel->setText("", juce::dontSendNotification);
        presetDescriptionLabel->setText("", juce::dontSendNotification);
        
        loadPresetButton->setEnabled(false);
        previewButton->setEnabled(false);
    }
    else
    {
        presetNameLabel->setText(selectedPreset.name, juce::dontSendNotification);
        presetCategoryLabel->setText("Category: " + selectedPreset.category, juce::dontSendNotification);
        
        // Format tags
        juce::String tagsText = "Tags: ";
        for (int i = 0; i < selectedPreset.tags.size(); ++i)
        {
            if (i > 0) tagsText += ", ";
            tagsText += selectedPreset.tags[i];
        }
        presetTagsLabel->setText(tagsText, juce::dontSendNotification);
        
        presetDescriptionLabel->setText(selectedPreset.description, juce::dontSendNotification);
        
        loadPresetButton->setEnabled(true);
        previewButton->setEnabled(true);
    }
}

juce::Array<InstrumentLibraryManager::PresetData> PresetBrowserComponent::getFilteredPresets() const
{
    if (instrumentLibrary == nullptr)
        return {};
    
    juce::Array<InstrumentLibraryManager::PresetData> results;
    
    // Start with all presets or category-filtered presets
    if (currentCategoryFilter.isEmpty() || currentCategoryFilter == "All Categories")
    {
        // Get all presets from all categories
        auto categories = instrumentLibrary->getAvailableCategories();
        for (const auto& category : categories)
        {
            auto categoryPresets = instrumentLibrary->getPresetsForCategory(category);
            results.addArray(categoryPresets);
        }
    }
    else
    {
        results = instrumentLibrary->getPresetsForCategory(currentCategoryFilter);
    }
    
    // Apply tag filter
    if (!currentTagFilter.isEmpty() && currentTagFilter != "All Tags")
    {
        juce::StringArray tagArray;
        tagArray.add(currentTagFilter);
        results = instrumentLibrary->getPresetsForTags(tagArray);
    }
    
    // Apply search filter
    if (!currentSearchText.isEmpty())
    {
        auto searchResults = instrumentLibrary->searchPresets(currentSearchText);
        
        // Intersect with current results
        juce::Array<InstrumentLibraryManager::PresetData> filteredResults;
        for (const auto& preset : results)
        {
            for (const auto& searchResult : searchResults)
            {
                if (preset.presetId == searchResult.presetId)
                {
                    filteredResults.add(preset);
                    break;
                }
            }
        }
        results = filteredResults;
    }
    
    return results;
}

//==============================================================================
// PresetListBoxModel Implementation

PresetListBoxModel::PresetListBoxModel(PresetBrowserComponent& owner)
    : owner(owner)
{
}

int PresetListBoxModel::getNumRows()
{
    return presets.size();
}

void PresetListBoxModel::paintListBoxItem(int rowNumber, juce::Graphics& g, 
                                                                 int width, int height, bool rowIsSelected)
{
    if (rowNumber < 0 || rowNumber >= presets.size())
        return;
    
    const auto& preset = presets[rowNumber];
    
    // Background
    if (rowIsSelected)
        g.fillAll(juce::Colour(0xff4a90e2));
    else if (rowNumber % 2 == 0)
        g.fillAll(juce::Colour(0xff202020));
    else
        g.fillAll(juce::Colour(0xff1a1a1a));
    
    // Text
    g.setColour(rowIsSelected ? juce::Colours::white : juce::Colours::lightgrey);
    
    auto textArea = juce::Rectangle<int>(4, 0, width - 8, height);
    
    // Preset name (main text)
    g.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    auto nameArea = textArea.removeFromTop(height / 2);
    g.drawText(preset.name, nameArea, juce::Justification::centredLeft, true);
    
    // Category and tags (smaller text)
    g.setFont(juce::FontOptions(11.0f));
    g.setColour(rowIsSelected ? juce::Colour(0xffcccccc) : juce::Colour(0xff888888));
    
    juce::String subtitleText = preset.category;
    if (!preset.tags.isEmpty())
    {
        subtitleText += " • " + preset.tags.joinIntoString(", ");
    }
    
    g.drawText(subtitleText, textArea, juce::Justification::centredLeft, true);
}

void PresetListBoxModel::selectedRowsChanged(int lastRowSelected)
{
    if (lastRowSelected >= 0 && lastRowSelected < presets.size())
    {
        owner.onPresetListSelectionChanged(lastRowSelected);
    }
}

void PresetListBoxModel::setPresets(const juce::Array<InstrumentLibraryManager::PresetData>& newPresets)
{
    presets = newPresets;
}

const InstrumentLibraryManager::PresetData& PresetListBoxModel::getPreset(int index) const
{
    static InstrumentLibraryManager::PresetData emptyPreset;
    if (index >= 0 && index < presets.size())
        return presets[index];
    return emptyPreset;
}

//==============================================================================
// Epic 9.3: AI Recommendation Implementation

void PresetBrowserComponent::setRecommendationEngine(std::shared_ptr<SpawnClone::PresetRecommendationEngine> engine)
{
    recommendationEngine = engine;
    updateRecommendations();
}

void PresetBrowserComponent::updateRecommendations()
{
    if (!recommendationEngine || !instrumentLibrary)
        return;
        
    // Get current selection context
    auto context = currentContext;
    if (selectedPresetIndex >= 0 && selectedPresetIndex < currentPresetList.size())
    {
        // Note: PresetRecommendationEngine doesn't use presetId in context
        // context.currentPresetName = currentPresetList[selectedPresetIndex].name;
    }
    
    // Generate recommendations using the correct method name
    currentRecommendations = recommendationEngine->getRecommendations(context);
    
    // Update the recommendation panel
    if (recommendationPanel)
    {
        recommendationPanel->setRecommendations(currentRecommendations);
    }
}

void PresetBrowserComponent::setMusicalContext(const SpawnClone::PresetRecommendationEngine::MusicalContext& context)
{
    currentContext = context;
    updateRecommendations();
}

//==============================================================================
// RecommendationPanel Implementation

RecommendationPanel::RecommendationPanel()
{
    titleLabel.setText("Smart Recommendations", juce::dontSendNotification);
    titleLabel.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);
    
    addAndMakeVisible(viewport);
    viewport.setViewedComponent(&contentComponent, false);
}

void RecommendationPanel::paint(juce::Graphics& g)
{
    // Background
    g.setColour(juce::Colour(0xff2d2d2d));
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 4.0f);
    
    // Border
    g.setColour(juce::Colour(0xff505050));
    g.drawRoundedRectangle(getLocalBounds().toFloat(), 4.0f, 1.0f);
}

void RecommendationPanel::resized()
{
    auto area = getLocalBounds().reduced(8);
    
    // Title
    titleLabel.setBounds(area.removeFromTop(25));
    area.removeFromTop(5);
    
    // Viewport takes remaining space
    viewport.setBounds(area);
    
    // Layout buttons in content component
    auto contentArea = juce::Rectangle<int>(0, 0, area.getWidth(), 0);
    const int buttonHeight = 35;
    const int spacing = 5;
    
    for (auto& button : recommendationButtons)
    {
        button->setBounds(0, contentArea.getHeight(), contentArea.getWidth(), buttonHeight);
        contentArea = contentArea.withHeight(contentArea.getHeight() + buttonHeight + spacing);
    }
    
    contentComponent.setSize(contentArea.getWidth(), contentArea.getHeight());
}

void RecommendationPanel::setRecommendations(
    const std::vector<SpawnClone::PresetRecommendationEngine::Recommendation>& recommendations)
{
    // Clear existing buttons
    recommendationButtons.clear();
    
    // Create new buttons for recommendations
    for (const auto& rec : recommendations)
    {
        auto button = std::make_unique<RecommendationButton>(rec);
        button->onClick = [this, rec] { 
            if (onRecommendationSelected)
                onRecommendationSelected(rec.presetName);
        };
        contentComponent.addAndMakeVisible(*button);
        recommendationButtons.push_back(std::move(button));
    }
    
    resized();
}

void RecommendationPanel::clearRecommendations()
{
    recommendationButtons.clear();
    repaint();
}

void RecommendationPanel::buttonClicked(RecommendationButton* button)
{
    if (button && onRecommendationSelected)
        onRecommendationSelected(button->recommendation.presetName);
}
