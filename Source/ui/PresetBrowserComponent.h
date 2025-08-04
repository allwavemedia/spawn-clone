/*
  ==============================================================================

    PresetBrowserComponent.h
    Created: 4 Aug 2025
    Author:  Epic 9 Story 9.1 Task 9.1.3 - UI Development

    Preset browser UI component for navigating and selecting instrument presets
    from the comprehensive InstrumentLibraryManager system.

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "../audio/InstrumentLibraryManager.h"

//==============================================================================
// Custom ListBox model for presets
class PresetListBoxModel : public juce::ListBoxModel
{
public:
    PresetListBoxModel(class PresetBrowserComponent& owner);
    ~PresetListBoxModel() override = default;

    //==============================================================================
    // ListBoxModel interface
    int getNumRows() override;
    void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
    void selectedRowsChanged(int lastRowSelected) override;
    
    //==============================================================================
    // Preset management
    void setPresets(const juce::Array<InstrumentLibraryManager::PresetData>& newPresets);
    const InstrumentLibraryManager::PresetData& getPreset(int index) const;

private:
    class PresetBrowserComponent& owner;
    juce::Array<InstrumentLibraryManager::PresetData> presets;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetListBoxModel)
};

//==============================================================================
/**
 * Epic 9 Story 9.1 Task 9.1.3: Preset Browser UI Component
 * 
 * Provides an intuitive interface for browsing instrument presets with:
 * - Category-based navigation
 * - Search functionality  
 * - Tag filtering
 * - Preset preview and selection
 * - Seamless integration with InstrumentLibraryManager
 */
class PresetBrowserComponent : public juce::Component,
                               public juce::TextEditor::Listener,
                               public juce::ComboBox::Listener
{
public:
    //==============================================================================
    PresetBrowserComponent();
    ~PresetBrowserComponent() override;

    //==============================================================================
    // Component overrides
    void paint(juce::Graphics& g) override;
    void resized() override;

    //==============================================================================
    // Integration with InstrumentLibraryManager
    
    /** Set the instrument library manager to browse */
    void setInstrumentLibraryManager(InstrumentLibraryManager* manager);
    
    /** Get currently selected preset */
    InstrumentLibraryManager::PresetData getCurrentlySelectedPreset() const;
    
    /** Programmatically select a preset by ID */
    void selectPreset(const juce::String& presetId);
    
    /** Refresh the preset list (e.g., after adding user presets) */
    void refreshPresetList();
    
    /** Handle preset list selection changes */
    void onPresetListSelectionChanged(int selectedIndex);

    //==============================================================================
    // Callback system
    
    /** Callback when a preset is selected */
    std::function<void(const InstrumentLibraryManager::PresetData&)> onPresetSelected;
    
    /** Callback when a preset is double-clicked for immediate loading */
    std::function<void(const InstrumentLibraryManager::PresetData&)> onPresetDoubleClicked;
    
    /** Callback when preview is requested */
    std::function<void(const InstrumentLibraryManager::PresetData&)> onPresetPreview;

private:
    //==============================================================================
    // UI Components
    
    // Search and filtering
    std::unique_ptr<juce::Label> searchLabel;
    std::unique_ptr<juce::TextEditor> searchEditor;
    std::unique_ptr<juce::Label> categoryLabel;
    std::unique_ptr<juce::ComboBox> categoryComboBox;
    std::unique_ptr<juce::Label> tagLabel;
    std::unique_ptr<juce::ComboBox> tagComboBox;
    std::unique_ptr<juce::TextButton> clearFiltersButton;
    
    // Preset list
    std::unique_ptr<juce::Label> presetListLabel;
    std::unique_ptr<juce::ListBox> presetListBox;
    std::unique_ptr<PresetListBoxModel> presetListModel;
    
    // Preset details
    std::unique_ptr<juce::GroupComponent> detailsGroup;
    std::unique_ptr<juce::Label> presetNameLabel;
    std::unique_ptr<juce::Label> presetCategoryLabel;
    std::unique_ptr<juce::Label> presetDescriptionLabel;
    std::unique_ptr<juce::Label> presetTagsLabel;
    
    // Action buttons
    std::unique_ptr<juce::TextButton> loadPresetButton;
    std::unique_ptr<juce::TextButton> previewButton;
    std::unique_ptr<juce::ToggleButton> autoPreviewToggle;

    //==============================================================================
    // Internal data
    InstrumentLibraryManager* instrumentLibrary = nullptr;
    juce::Array<InstrumentLibraryManager::PresetData> currentPresetList;
    InstrumentLibraryManager::PresetData selectedPreset;
    int selectedPresetIndex = -1;
    
    // Filter state
    juce::String currentSearchText;
    juce::String currentCategoryFilter;
    juce::String currentTagFilter;

    //==============================================================================
    // Event handlers
    void textEditorTextChanged(juce::TextEditor& editor) override;
    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;
    void listBoxItemClicked(int row, const juce::MouseEvent& e);
    void listBoxItemDoubleClicked(int row, const juce::MouseEvent& e);
    
    // Button callbacks
    void clearFiltersClicked();
    void loadPresetClicked();
    void previewButtonClicked();
    void autoPreviewToggled();

    //==============================================================================
    // Internal methods
    void initializeComponents();
    void updatePresetList();
    void updateCategoryComboBox();
    void updateTagComboBox();
    void updatePresetDetails();
    void applyFilters();
    
    juce::Array<InstrumentLibraryManager::PresetData> getFilteredPresets() const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetBrowserComponent)
};
