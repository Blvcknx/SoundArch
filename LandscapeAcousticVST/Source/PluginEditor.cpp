#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
LandscapeAcousticVSTEditor::LandscapeAcousticVSTEditor (LandscapeAcousticVSTProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    setSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    setupComponents();
    setupAttachments();
    
    // Start timer for regular UI updates
    startTimer(100); // 10 FPS
}

LandscapeAcousticVSTEditor::~LandscapeAcousticVSTEditor()
{
    stopTimer();
}

//==============================================================================
void LandscapeAcousticVSTEditor::paint (juce::Graphics& g)
{
    // Background
    g.fillAll(juce::Colour(0xff2d2d2d));
    
    // Header
    g.setColour(juce::Colours::white);
    g.setFont(18.0f);
    g.drawText("Landscape Acoustic VST", MARGIN, 5, getWidth() - 2*MARGIN, 25, 
               juce::Justification::centred);
    
    // Draw terrain visualization
    if (processor.terrainLoader->isLoaded()) {
        drawTerrainMap(g);
    } else {
        // Draw placeholder for terrain view
        g.setColour(juce::Colour(0xff1a1a1a));
        g.fillRect(terrainViewArea);
        g.setColour(juce::Colours::grey);
        g.drawRect(terrainViewArea, 1);
        
        g.setColour(juce::Colours::lightgrey);
        g.setFont(14.0f);
        g.drawText("Drag DEM file here or click 'Load DEM File'", 
                   terrainViewArea, juce::Justification::centred);
    }
    
    // Drag overlay
    if (isDraggingFile) {
        g.setColour(juce::Colour(0x80ffffff));
        g.fillRect(getLocalBounds());
        g.setColour(juce::Colours::white);
        g.setFont(20.0f);
        g.drawText("Drop DEM file to load", getLocalBounds(), juce::Justification::centred);
    }
}

void LandscapeAcousticVSTEditor::resized()
{
    auto bounds = getLocalBounds().reduced(MARGIN);
    
    // Header space
    bounds.removeFromTop(30);
    
    // Top row - file operation buttons
    auto buttonRow = bounds.removeFromTop(BUTTON_HEIGHT + MARGIN);
    auto buttonWidth = buttonRow.getWidth() / 3 - MARGIN;
    
    loadDEMButton.setBounds(buttonRow.removeFromLeft(buttonWidth));
    buttonRow.removeFromLeft(MARGIN);
    importQGISButton.setBounds(buttonRow.removeFromLeft(buttonWidth));
    buttonRow.removeFromLeft(MARGIN);
    exportIRButton.setBounds(buttonRow.removeFromLeft(buttonWidth));
    
    bounds.removeFromTop(MARGIN);
    
    // Split remaining area: left for controls, right for terrain view
    auto controlsArea = bounds.removeFromLeft(250);
    bounds.removeFromLeft(MARGIN);
    terrainViewArea = bounds.reduced(1); // Leave space for border
    
    // Parameter controls in left column
    auto setupSlider = [&](juce::Slider& slider, juce::Label& label) {
        auto row = controlsArea.removeFromTop(SLIDER_HEIGHT + 5);
        label.setBounds(row.removeFromLeft(LABEL_WIDTH));
        slider.setBounds(row);
        controlsArea.removeFromTop(5); // Spacing
    };
    
    setupSlider(dryWetSlider, dryWetLabel);
    setupSlider(outputGainSlider, outputGainLabel);
    setupSlider(temperatureSlider, temperatureLabel);
    setupSlider(humiditySlider, humidityLabel);
    
    // Ground type combo box
    auto comboRow = controlsArea.removeFromTop(SLIDER_HEIGHT + 5);
    groundTypeLabel.setBounds(comboRow.removeFromLeft(LABEL_WIDTH));
    groundTypeCombo.setBounds(comboRow);
    controlsArea.removeFromTop(5);
    
    setupSlider(sourceHeightSlider, sourceHeightLabel);
    setupSlider(receiverHeightSlider, receiverHeightLabel);
    
    controlsArea.removeFromTop(10); // Extra space
    
    // Status labels at bottom of controls
    statusLabel.setBounds(controlsArea.removeFromTop(20));
    controlsArea.removeFromTop(5);
    terrainInfoLabel.setBounds(controlsArea.removeFromTop(20));
    controlsArea.removeFromTop(5);
    profileInfoLabel.setBounds(controlsArea.removeFromTop(20));
}

//==============================================================================
void LandscapeAcousticVSTEditor::setupComponents()
{
    // File operation buttons
    addAndMakeVisible(loadDEMButton);
    addAndMakeVisible(importQGISButton);
    addAndMakeVisible(exportIRButton);
    
    loadDEMButton.onClick = [this] { loadDEMFile(); };
    importQGISButton.onClick = [this] { loadQGISConfig(); };
    exportIRButton.onClick = [this] { exportImpulseResponse(); };
    
    // Parameter sliders
    auto setupSlider = [this](juce::Slider& slider, juce::Label& label, const juce::String& text) {
        addAndMakeVisible(slider);
        addAndMakeVisible(label);
        
        slider.setSliderStyle(juce::Slider::LinearHorizontal);
        slider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
        
        label.setText(text, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centredRight);
        label.attachToComponent(&slider, false);
    };
    
    setupSlider(dryWetSlider, dryWetLabel, "Dry/Wet:");
    setupSlider(outputGainSlider, outputGainLabel, "Gain (dB):");
    setupSlider(temperatureSlider, temperatureLabel, "Temp (°C):");
    setupSlider(humiditySlider, humidityLabel, "Humidity (%):");
    setupSlider(sourceHeightSlider, sourceHeightLabel, "Source (m):");
    setupSlider(receiverHeightSlider, receiverHeightLabel, "Receiver (m):");
    
    // Ground type combo
    addAndMakeVisible(groundTypeCombo);
    addAndMakeVisible(groundTypeLabel);
    
    groundTypeCombo.addItem("Hard", 1);
    groundTypeCombo.addItem("Porous", 2);
    groundTypeCombo.addItem("Mixed", 3);
    groundTypeCombo.setSelectedId(2); // Default to Porous
    
    groundTypeLabel.setText("Ground:", juce::dontSendNotification);
    groundTypeLabel.setJustificationType(juce::Justification::centredRight);
    
    // Status labels
    addAndMakeVisible(statusLabel);
    addAndMakeVisible(terrainInfoLabel);
    addAndMakeVisible(profileInfoLabel);
    
    statusLabel.setFont(juce::Font(12.0f));
    terrainInfoLabel.setFont(juce::Font(11.0f));
    profileInfoLabel.setFont(juce::Font(11.0f));
    
    statusLabel.setColour(juce::Label::textColourId, juce::Colours::lightblue);
    terrainInfoLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    profileInfoLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
}

void LandscapeAcousticVSTEditor::setupAttachments()
{
    // Create parameter attachments for automatic UI synchronization
    dryWetAttachment = std::make_unique<SliderAttachment>(
        processor.parameters, "dry_wet", dryWetSlider);
    
    outputGainAttachment = std::make_unique<SliderAttachment>(
        processor.parameters, "output_gain", outputGainSlider);
    
    temperatureAttachment = std::make_unique<SliderAttachment>(
        processor.parameters, "temperature", temperatureSlider);
    
    humidityAttachment = std::make_unique<SliderAttachment>(
        processor.parameters, "humidity", humiditySlider);
    
    groundTypeAttachment = std::make_unique<ComboAttachment>(
        processor.parameters, "ground_type", groundTypeCombo);
    
    sourceHeightAttachment = std::make_unique<SliderAttachment>(
        processor.parameters, "source_height", sourceHeightSlider);
    
    receiverHeightAttachment = std::make_unique<SliderAttachment>(
        processor.parameters, "receiver_height", receiverHeightSlider);
}

//==============================================================================
// File drag and drop

bool LandscapeAcousticVSTEditor::isInterestedInFileDrag(const juce::StringArray& files)
{
    for (const auto& file : files) {
        juce::File f(file);
        if (TerrainLoader::isValidDEMFile(f) || 
            f.getFileExtension().toLowerCase() == ".json") {
            return true;
        }
    }
    return false;
}

void LandscapeAcousticVSTEditor::fileDragEnter(const juce::StringArray& files, int x, int y)
{
    isDraggingFile = true;
    repaint();
}

void LandscapeAcousticVSTEditor::fileDragExit(const juce::StringArray& files)
{
    isDraggingFile = false;
    repaint();
}

void LandscapeAcousticVSTEditor::filesDropped(const juce::StringArray& files, int x, int y)
{
    isDraggingFile = false;
    repaint();
    
    if (files.size() > 0) {
        juce::File file(files[0]);
        juce::String extension = file.getFileExtension().toLowerCase();
        
        if (extension == ".json") {
            // Load QGIS config
            juce::String errorMsg;
            if (!processor.importQGISConfig(file, errorMsg)) {
                juce::AlertWindow::showMessageBoxAsync(
                    juce::AlertWindow::WarningIcon,
                    "Import Error",
                    "Failed to import QGIS config: " + errorMsg);
            } else {
                statusLabel.setText("Loaded QGIS config: " + file.getFileName(), 
                                   juce::dontSendNotification);
            }
        } else if (TerrainLoader::isValidDEMFile(file)) {
            // Load DEM file
            juce::String errorMsg;
            if (!processor.loadTerrainFile(file, errorMsg)) {
                juce::AlertWindow::showMessageBoxAsync(
                    juce::AlertWindow::WarningIcon,
                    "Load Error", 
                    "Failed to load DEM file: " + errorMsg);
            } else {
                statusLabel.setText("Loaded DEM: " + file.getFileName(), 
                                   juce::dontSendNotification);
            }
        }
    }
}

//==============================================================================
// File operations

void LandscapeAcousticVSTEditor::loadDEMFile()
{
    juce::FileChooser chooser("Select DEM file", juce::File{}, 
                              "*.tif;*.tiff;*.asc;*.dted;*.hgt;*.img;*.dem");
    
    if (chooser.browseForFileToOpen()) {
        juce::File file = chooser.getResult();
        juce::String errorMsg;
        
        if (processor.loadTerrainFile(file, errorMsg)) {
            statusLabel.setText("Loaded DEM: " + file.getFileName(), 
                               juce::dontSendNotification);
        } else {
            juce::AlertWindow::showMessageBoxAsync(
                juce::AlertWindow::WarningIcon,
                "Load Error",
                "Failed to load DEM file: " + errorMsg);
        }
    }
}

void LandscapeAcousticVSTEditor::loadQGISConfig()
{
    juce::FileChooser chooser("Select QGIS config file", juce::File{}, "*.json");
    
    if (chooser.browseForFileToOpen()) {
        juce::File file = chooser.getResult();
        juce::String errorMsg;
        
        if (processor.importQGISConfig(file, errorMsg)) {
            statusLabel.setText("Loaded QGIS config: " + file.getFileName(),
                               juce::dontSendNotification);
        } else {
            juce::AlertWindow::showMessageBoxAsync(
                juce::AlertWindow::WarningIcon,
                "Import Error",
                "Failed to import QGIS config: " + errorMsg);
        }
    }
}

void LandscapeAcousticVSTEditor::exportImpulseResponse()
{
    if (!processor.impulseGenerator->hasImpulseResponse()) {
        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::InfoIcon,
            "No Impulse Response",
            "Generate an impulse response first by loading DEM and setting analysis points.");
        return;
    }
    
    juce::FileChooser chooser("Export impulse response", juce::File{}, "*.wav");
    
    if (chooser.browseForFileToSave(true)) {
        juce::File file = chooser.getResult();
        
        if (processor.exportImpulseResponse(file)) {
            statusLabel.setText("Exported IR to: " + file.getFileName(),
                               juce::dontSendNotification);
        } else {
            juce::AlertWindow::showMessageBoxAsync(
                juce::AlertWindow::WarningIcon,
                "Export Error",
                "Failed to export impulse response");
        }
    }
}

//==============================================================================
// Terrain visualization

void LandscapeAcousticVSTEditor::drawTerrainMap(juce::Graphics& g)
{
    if (!processor.terrainLoader->isLoaded()) {
        return;
    }
    
    const auto& demData = processor.terrainLoader->getDEMData();
    auto bounds = terrainViewArea;
    
    // Draw terrain as grayscale heightmap (simplified visualization)
    // In a full implementation, this would use OpenGL or more sophisticated rendering
    
    g.setColour(juce::Colour(0xff404040));
    g.fillRect(bounds);
    
    // Draw border
    g.setColour(juce::Colours::grey);
    g.drawRect(bounds, 1);
    
    // Draw simplified terrain representation
    if (demData.isValid()) {
        // Sample terrain at lower resolution for display
        int displayWidth = bounds.getWidth();
        int displayHeight = bounds.getHeight();
        
        for (int y = 0; y < displayHeight; y += 4) {
            for (int x = 0; x < displayWidth; x += 4) {
                // Map display coordinates to geographic coordinates
                double geoX = demData.geoTransform[0] + 
                             (static_cast<double>(x) / displayWidth) * 
                             (demData.width * demData.geoTransform[1]);
                double geoY = demData.geoTransform[3] + 
                             (static_cast<double>(y) / displayHeight) * 
                             (demData.height * demData.geoTransform[5]);
                
                float elevation = processor.terrainLoader->getElevationAt(geoX, geoY);
                
                // Simple elevation-based coloring (adjust range as needed)
                float normalizedElevation = juce::jlimit(0.0f, 1.0f, elevation / 1000.0f);
                juce::Colour elevationColor = juce::Colour::fromHSV(
                    0.6f - normalizedElevation * 0.3f, // Blue to brown
                    0.7f,
                    0.3f + normalizedElevation * 0.5f,
                    1.0f);
                
                g.setColour(elevationColor);
                g.fillRect(bounds.getX() + x, bounds.getY() + y, 4, 4);
            }
        }
    }
    
    // Draw source and receiver points if set
    g.setColour(juce::Colours::red);
    if (sourcePoint.x != 0.0 || sourcePoint.y != 0.0) {
        auto sourcePx = geoToPixel(sourcePoint);
        g.fillEllipse(sourcePx.x - 3, sourcePx.y - 3, 6, 6);
        g.drawText("S", sourcePx.x - 10, sourcePx.y - 15, 20, 10, juce::Justification::centred);
    }
    
    g.setColour(juce::Colours::green);
    if (receiverPoint.x != 0.0 || receiverPoint.y != 0.0) {
        auto receiverPx = geoToPixel(receiverPoint);
        g.fillEllipse(receiverPx.x - 3, receiverPx.y - 3, 6, 6);
        g.drawText("R", receiverPx.x - 10, receiverPx.y - 15, 20, 10, juce::Justification::centred);
    }
    
    // Draw path between points
    if ((sourcePoint.x != 0.0 || sourcePoint.y != 0.0) && 
        (receiverPoint.x != 0.0 || receiverPoint.y != 0.0)) {
        g.setColour(juce::Colours::white);
        auto sourcePx = geoToPixel(sourcePoint);
        auto receiverPx = geoToPixel(receiverPoint);
        g.drawLine(sourcePx.x, sourcePx.y, receiverPx.x, receiverPx.y, 1.0f);
    }
}

//==============================================================================
// Mouse interaction

void LandscapeAcousticVSTEditor::mouseDown(const juce::MouseEvent& event)
{
    if (!processor.terrainLoader->isLoaded()) {
        return;
    }
    
    // Check if click is in terrain view area
    if (terrainViewArea.contains(event.getPosition())) {
        // Convert pixel to geographic coordinates
        auto geoPoint = pixelToGeo(event.getPosition());
        
        // Alternate between setting source and receiver
        if (event.mods.isRightButtonDown()) {
            // Right click sets receiver
            receiverPoint = geoPoint;
            selectionMode = SelectionMode::Receiver;
        } else {
            // Left click sets source
            sourcePoint = geoPoint;
            selectionMode = SelectionMode::Source;
        }
        
        // Update processor with new points
        processor.setAnalysisPoints(sourcePoint, receiverPoint);
        
        repaint();
    }
}

//==============================================================================
// Coordinate conversion

juce::Point<double> LandscapeAcousticVSTEditor::pixelToGeo(juce::Point<int> pixel) const
{
    if (!processor.terrainLoader->isLoaded()) {
        return {0.0, 0.0};
    }
    
    const auto& demData = processor.terrainLoader->getDEMData();
    auto bounds = demData.getBounds();
    
    // Normalize pixel coordinates to [0,1] within terrain view area
    double normX = static_cast<double>(pixel.x - terrainViewArea.getX()) / terrainViewArea.getWidth();
    double normY = static_cast<double>(pixel.y - terrainViewArea.getY()) / terrainViewArea.getHeight();
    
    // Map to geographic coordinates
    double geoX = bounds.getX() + normX * bounds.getWidth();
    double geoY = bounds.getY() + (1.0 - normY) * bounds.getHeight(); // Flip Y axis
    
    return {geoX, geoY};
}

juce::Point<int> LandscapeAcousticVSTEditor::geoToPixel(juce::Point<double> geo) const
{
    if (!processor.terrainLoader->isLoaded()) {
        return {0, 0};
    }
    
    const auto& demData = processor.terrainLoader->getDEMData();
    auto bounds = demData.getBounds();
    
    // Normalize geographic coordinates
    double normX = (geo.x - bounds.getX()) / bounds.getWidth();
    double normY = (geo.y - bounds.getY()) / bounds.getHeight();
    normY = 1.0 - normY; // Flip Y axis
    
    // Map to pixel coordinates
    int pixelX = terrainViewArea.getX() + static_cast<int>(normX * terrainViewArea.getWidth());
    int pixelY = terrainViewArea.getY() + static_cast<int>(normY * terrainViewArea.getHeight());
    
    return {pixelX, pixelY};
}

//==============================================================================
// Timer callback

void LandscapeAcousticVSTEditor::timerCallback()
{
    updateStatusLabels();
}

void LandscapeAcousticVSTEditor::updateStatusLabels()
{
    // Update terrain info
    if (processor.terrainLoader->isLoaded()) {
        const auto& demData = processor.terrainLoader->getDEMData();
        auto bounds = demData.getBounds();
        
        terrainInfoLabel.setText(
            juce::String::formatted("DEM: %dx%d, %.3f°-%.3f° lon, %.3f°-%.3f° lat",
                                   demData.width, demData.height,
                                   bounds.getX(), bounds.getRight(),
                                   bounds.getY(), bounds.getBottom()),
            juce::dontSendNotification);
        
        // Update profile info if points are set
        if ((sourcePoint.x != 0.0 || sourcePoint.y != 0.0) && 
            (receiverPoint.x != 0.0 || receiverPoint.y != 0.0)) {
            
            auto profile = processor.getCurrentProfile();
            if (profile.isValid()) {
                profileInfoLabel.setText(
                    juce::String::formatted("Path: %.1f m, %d elevation samples, IR: %s",
                                           profile.totalDistance,
                                           static_cast<int>(profile.elevations.size()),
                                           processor.impulseGenerator->hasImpulseResponse() ? "Ready" : "Pending"),
                    juce::dontSendNotification);
            }
        } else {
            profileInfoLabel.setText("Click on terrain to set source (left) and receiver (right)",
                                    juce::dontSendNotification);
        }
    } else {
        terrainInfoLabel.setText("No DEM loaded", juce::dontSendNotification);
        profileInfoLabel.setText("", juce::dontSendNotification);
    }
}