#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "../audio/InstrumentLibraryManager.h"
#include "../ai/PresetRecommendationEngine.h" // Re-enabled after fixing compatibility issues

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
// Epic 9.3: Recommendation Panel Component (Temporarily disabled for compilation)
/*
class RecommendationPanel : public juce::Component
{
public:
    struct RecommendationButton : public juce::TextButton
    {
        SpawnClone::PresetRecommendationEngine::Recommendation recommendation;
        
        RecommendationButton(const SpawnClone::PresetRecommendationEngine::Recommendation& rec)
            : recommendation(rec)
        {
            setButtonText(rec.presetName);
            setTooltip(rec.explanation);
        }
    };

    RecommendationPanel();
    ~RecommendationPanel() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;
    
    void setRecommendations(const std::vector<SpawnClone::PresetRecommendationEngine::Recommendation>& recommendations);
    void clearRecommendations();
    
    std::function<void(const std::string&)> onRecommendationSelected;

private:
    juce::Label titleLabel;
    juce::Viewport viewport;
    juce::Component contentComponent;
    std::vector<std::unique_ptr<RecommendationButton>> recommendationButtons;
    
    void buttonClicked(RecommendationButton* button);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RecommendationPanel)
};
*/

//==============================================================================
/**
 * Epic 9.3: Enhanced Preset Browser with Intelligent Recommendations
 * 
 * Provides an intuitive interface for browsing instrument presets with:
 * - AI-powered preset recommendations
 * - Smart complementary instrument suggestions  
 * - User preference learning
 * - Context-aware filtering
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
    // Integration with InstrumentLibraryManager & Recommendation Engine
    
    /** Set the instrument library manager to browse */
    void setInstrumentLibraryManager(InstrumentLibraryManager* manager);
    
    /** Set the recommendation engine for intelligent suggestions */
    // void setRecommendationEngine(std::shared_ptr<SpawnClone::PresetRecommendationEngine> engine); // Temporarily disabled
    
    /** Get currently selected preset */
    InstrumentLibraryManager::PresetData getCurrentlySelectedPreset() const;
    
    /** Programmatically select a preset by ID */
    void selectPreset(const juce::String& presetId);
    
    /** Refresh the preset list (e.g., after adding user presets) */
    void refreshPresetList();
    
    /** Handle preset list selection changes */
    void onPresetListSelectionChanged(int selectedIndex);

    //==============================================================================
    // Epic 9.3: Smart Recommendation Features (Temporarily disabled)
    
    // void updateRecommendations();
    // void setMusicalContext(const SpawnClone::PresetRecommendationEngine::MusicalContext& context);
    // void setRecommendationsVisible(bool visible);
    // void showComplementaryInstruments();

    //==============================================================================
    // Callback system
    
    /** Callback when a preset is selected */
    std::function<void(const InstrumentLibraryManager::PresetData&)> onPresetSelected;
    
    /** Callback when a preset is double-clicked for immediate loading */
    std::function<void(const InstrumentLibraryManager::PresetData&)> onPresetDoubleClicked;
    
    /** Callback when preview is requested */
    std::function<void(const InstrumentLibraryManager::PresetData&)> onPresetPreview;
    
    /** Callback when a recommendation is selected */
    std::function<void(const std::string&)> onRecommendationSelected;

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
    
    // Epic 9.3: Recommendation system UI (temporarily disabled)
    // std::unique_ptr<juce::TextButton> recommendationsToggle;
    // std::unique_ptr<juce::TextButton> refreshRecommendationsButton;
    // std::unique_ptr<RecommendationPanel> recommendationPanel;

    //==============================================================================
    // Internal data
    InstrumentLibraryManager* instrumentLibrary = nullptr;
    // std::shared_ptr<SpawnClone::PresetRecommendationEngine> recommendationEngine; // temporarily disabled
    juce::Array<InstrumentLibraryManager::PresetData> currentPresetList;
    InstrumentLibraryManager::PresetData selectedPreset;
    int selectedPresetIndex = -1;
    
    // Epic 9.3: Musical context and recommendations (temporarily disabled)
    // SpawnClone::PresetRecommendationEngine::MusicalContext currentContext;
    // std::vector<SpawnClone::PresetRecommendationEngine::Recommendation> currentRecommendations;
    // bool recommendationsVisible = true;
    
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
    
    // Epic 9.3: Recommendation callbacks (temporarily disabled)
    // void recommendationsToggleClicked();
    // void refreshRecommendationsClicked();
    // void onRecommendationChosen(const std::string& presetName);

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
