#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

/**
 * @brief Editor GUI for LandscapeAcousticVST plugin
 * 
 * Provides interactive interface for:
 * - DEM file loading and terrain visualization
 * - Source/receiver point selection
 * - Acoustic parameter adjustment
 * - Real-time impulse response display
 */
class LandscapeAcousticVSTEditor : public juce::AudioProcessorEditor,
                                   public juce::FileDragAndDropTarget,
                                   public juce::Timer
{
public:
    LandscapeAcousticVSTEditor (LandscapeAcousticVSTProcessor&);
    ~LandscapeAcousticVSTEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    //==============================================================================
    // File drag and drop
    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void fileDragEnter(const juce::StringArray& files, int x, int y) override;
    void fileDragExit(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;
    
    //==============================================================================
    // Timer callback for UI updates
    void timerCallback() override;

private:
    // Reference to processor
    LandscapeAcousticVSTProcessor& processor;
    
    // UI Layout Constants
    static constexpr int WINDOW_WIDTH = 800;
    static constexpr int WINDOW_HEIGHT = 600;
    static constexpr int MARGIN = 10;
    static constexpr int BUTTON_HEIGHT = 30;
    static constexpr int SLIDER_HEIGHT = 25;
    static constexpr int LABEL_WIDTH = 120;
    
    //==============================================================================
    // GUI Components
    
    // File operations
    juce::TextButton loadDEMButton {"Load DEM File"};
    juce::TextButton importQGISButton {"Import QGIS Config"};
    juce::TextButton exportIRButton {"Export IR"};
    
    // Parameter controls
    juce::Slider dryWetSlider;
    juce::Label dryWetLabel {"", "Dry/Wet Mix:"};
    
    juce::Slider outputGainSlider;
    juce::Label outputGainLabel {"", "Output Gain (dB):"};
    
    juce::Slider temperatureSlider;
    juce::Label temperatureLabel {"", "Temperature (°C):"};
    
    juce::Slider humiditySlider;
    juce::Label humidityLabel {"", "Humidity (%):"};
    
    juce::ComboBox groundTypeCombo;
    juce::Label groundTypeLabel {"", "Ground Type:"};
    
    juce::Slider sourceHeightSlider;
    juce::Label sourceHeightLabel {"", "Source Height (m):"};
    
    juce::Slider receiverHeightSlider;
    juce::Label receiverHeightLabel {"", "Receiver Height (m):"};
    
    // Status display
    juce::Label statusLabel {"", "Ready - Load DEM file to begin"};
    juce::Label terrainInfoLabel {"", ""};
    juce::Label profileInfoLabel {"", ""};
    
    // Terrain visualization area
    juce::Rectangle<int> terrainViewArea;
    
    //==============================================================================
    // Parameter attachments
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    
    std::unique_ptr<SliderAttachment> dryWetAttachment;
    std::unique_ptr<SliderAttachment> outputGainAttachment;
    std::unique_ptr<SliderAttachment> temperatureAttachment;
    std::unique_ptr<SliderAttachment> humidityAttachment;
    std::unique_ptr<ComboAttachment> groundTypeAttachment;
    std::unique_ptr<SliderAttachment> sourceHeightAttachment;
    std::unique_ptr<SliderAttachment> receiverHeightAttachment;
    
    //==============================================================================
    // State
    bool isDraggingFile = false;
    juce::Point<double> sourcePoint {0.0, 0.0};
    juce::Point<double> receiverPoint {0.0, 0.0};
    enum class SelectionMode { None, Source, Receiver } selectionMode = SelectionMode::None;
    
    //==============================================================================
    // Methods
    
    /**
     * @brief Initialize all GUI components
     */
    void setupComponents();
    
    /**
     * @brief Setup parameter attachments
     */
    void setupAttachments();
    
    /**
     * @brief Handle file loading
     */
    void loadDEMFile();
    void loadQGISConfig();
    void exportImpulseResponse();
    
    /**
     * @brief Update terrain display
     */
    void updateTerrainDisplay();
    
    /**
     * @brief Update status labels
     */
    void updateStatusLabels();
    
    /**
     * @brief Draw terrain visualization
     */
    void drawTerrainMap(juce::Graphics& g);
    
    /**
     * @brief Handle mouse clicks on terrain map
     */
    void mouseDown(const juce::MouseEvent& event) override;
    
    /**
     * @brief Convert pixel coordinates to geographic coordinates
     */
    juce::Point<double> pixelToGeo(juce::Point<int> pixel) const;
    
    /**
     * @brief Convert geographic coordinates to pixel coordinates  
     */
    juce::Point<int> geoToPixel(juce::Point<double> geo) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LandscapeAcousticVSTEditor)
};