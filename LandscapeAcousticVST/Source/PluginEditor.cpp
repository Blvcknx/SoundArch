#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
LandscapeAcousticVSTEditor::LandscapeAcousticVSTEditor (LandscapeAcousticVSTProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    setSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    setupComponents();
    setupAttachments();
    updateGeoBounds(); // Initialize zoom bounds
    
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
    // Modern gradient background
    juce::ColourGradient backgroundGradient(
        juce::Colour(0xff1a1a2e), juce::Point<float>(0, 0),
        juce::Colour(0xff16213e), juce::Point<float>(0, getHeight()),
        false);
    g.setGradientFill(backgroundGradient);
    g.fillRect(getLocalBounds());

    // Add subtle pattern overlay
    g.setColour(juce::Colour(0x10ffffff));
    for (int i = 0; i < getWidth(); i += 20) {
        g.drawLine(i, 0, i, getHeight(), 1.0f);
    }
    for (int i = 0; i < getHeight(); i += 20) {
        g.drawLine(0, i, getWidth(), i, 1.0f);
    }

    // Header with modern styling
    auto headerBounds = juce::Rectangle<int>(MARGIN, 5, getWidth() - 2*MARGIN, 30);

    // Header background with rounded corners
    g.setColour(juce::Colour(0x20ffffff));
    g.fillRoundedRectangle(headerBounds.toFloat(), 8.0f);

    // Header border
    g.setColour(juce::Colour(0x40ffffff));
    g.drawRoundedRectangle(headerBounds.toFloat(), 8.0f, 1.5f);

    // Header text with shadow
    g.setColour(juce::Colours::black.withAlpha(0.5f));
    g.setFont(18.0f);
    g.drawText("Landscape Acoustic VST", headerBounds.getX() + 1, headerBounds.getY() + 1,
               headerBounds.getWidth(), headerBounds.getHeight(), juce::Justification::centred);

    g.setColour(juce::Colour(0xff00d4ff));
    g.setFont(18.0f);
    g.drawText("Landscape Acoustic VST", headerBounds.getX(), headerBounds.getY(),
               headerBounds.getWidth(), headerBounds.getHeight(), juce::Justification::centred);

    // Draw terrain visualization
    if (processor.terrainLoader->isLoaded()) {
        drawTerrainMap(g);
    } else {
        // Modern placeholder for terrain view
        auto placeholderBounds = terrainViewArea.expanded(2);

        // Background with gradient
        juce::ColourGradient placeholderGradient(
            juce::Colour(0xff2a2a3a), placeholderBounds.getTopLeft().toFloat(),
            juce::Colour(0xff1a1a2a), placeholderBounds.getBottomRight().toFloat(),
            false);
        g.setGradientFill(placeholderGradient);
        g.fillRoundedRectangle(placeholderBounds.toFloat(), 6.0f);

        // Border
        g.setColour(juce::Colour(0x60ffffff));
        g.drawRoundedRectangle(placeholderBounds.toFloat(), 6.0f, 2.0f);

        // Icon (simplified mountain symbol)
        g.setColour(juce::Colour(0x80ffffff));
        auto iconBounds = placeholderBounds.reduced(20);
        juce::Path mountainPath;
        mountainPath.startNewSubPath(iconBounds.getX(), iconBounds.getBottom());
        mountainPath.lineTo(iconBounds.getCentreX(), iconBounds.getY());
        mountainPath.lineTo(iconBounds.getRight(), iconBounds.getBottom());
        mountainPath.closeSubPath();
        g.fillPath(mountainPath);

        // Text
        g.setColour(juce::Colour(0xffcccccc));
        g.setFont(16.0f);
        g.drawText("Load DEM File to Begin", placeholderBounds.getX(), placeholderBounds.getY() + 40,
                   placeholderBounds.getWidth(), 25, juce::Justification::centred);

        g.setColour(juce::Colour(0xff888888));
        g.setFont(12.0f);
        g.drawText("Drag & drop or click 'Load DEM File'", placeholderBounds.getX(), placeholderBounds.getY() + 65,
                   placeholderBounds.getWidth(), 20, juce::Justification::centred);
    }

    // Drag overlay with modern styling
    if (isDraggingFile) {
        g.setColour(juce::Colour(0x8000d4ff));
        g.fillRoundedRectangle(getLocalBounds().toFloat(), 10.0f);

        g.setColour(juce::Colour(0xff00d4ff));
        g.setFont(24.0f);
        g.drawText("Drop DEM file here", getLocalBounds(), juce::Justification::centred);
    }
}

void LandscapeAcousticVSTEditor::resized()
{
    auto bounds = getLocalBounds().reduced(MARGIN);
    
    // Header space
    bounds.removeFromTop(30);
    
    // Top row - file operation buttons
    auto buttonRow = bounds.removeFromTop(BUTTON_HEIGHT + MARGIN);
    auto buttonWidth = buttonRow.getWidth() / 5 - MARGIN; // Changed from 3 to 5 buttons
    
    loadDEMButton.setBounds(buttonRow.removeFromLeft(buttonWidth));
    buttonRow.removeFromLeft(MARGIN);
    importQGISButton.setBounds(buttonRow.removeFromLeft(buttonWidth));
    buttonRow.removeFromLeft(MARGIN);
    exportIRButton.setBounds(buttonRow.removeFromLeft(buttonWidth));
    buttonRow.removeFromLeft(MARGIN);
    zoomInButton.setBounds(buttonRow.removeFromLeft(buttonWidth));
    buttonRow.removeFromLeft(MARGIN);
    zoomOutButton.setBounds(buttonRow.removeFromLeft(buttonWidth));
    
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
    // File operation buttons with modern styling
    auto setupButton = [this](juce::TextButton& button, const juce::String& text) {
        addAndMakeVisible(button);
        button.setButtonText(text);

        // Modern button styling
        button.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff00d4ff));
        button.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff0099cc));
        button.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        button.setColour(juce::TextButton::textColourOnId, juce::Colours::white);
    };

    setupButton(loadDEMButton, "Load DEM");
    setupButton(importQGISButton, "Import QGIS");
    setupButton(exportIRButton, "Export IR");
    setupButton(zoomInButton, "+");
    setupButton(zoomOutButton, "-");

    loadDEMButton.onClick = [this] { 
        DBG("Load DEM button clicked");
        loadDEMFile(); 
    };
    importQGISButton.onClick = [this] { 
        DBG("Import QGIS button clicked");
        loadQGISConfig(); 
    };
    exportIRButton.onClick = [this] { 
        DBG("Export IR button clicked");
        exportImpulseResponse(); 
    };
    zoomInButton.onClick = [this] { 
        DBG("Zoom in button clicked");
        zoomIn(); 
    };
    zoomOutButton.onClick = [this] { 
        DBG("Zoom out button clicked");
        zoomOut(); 
    };
    
    // Parameter sliders with modern styling
    auto setupSlider = [this](juce::Slider& slider, juce::Label& label, const juce::String& text) {
        addAndMakeVisible(slider);
        addAndMakeVisible(label);

        slider.setSliderStyle(juce::Slider::LinearHorizontal);
        slider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);

        // Modern slider colors
        slider.setColour(juce::Slider::thumbColourId, juce::Colour(0xff00d4ff));
        slider.setColour(juce::Slider::trackColourId, juce::Colour(0x60ffffff));
        slider.setColour(juce::Slider::backgroundColourId, juce::Colour(0x20ffffff));
        slider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
        slider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0x20ffffff));
        slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0x40ffffff));

        label.setText(text, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centredRight);
        label.setColour(juce::Label::textColourId, juce::Colour(0xffcccccc));
        label.attachToComponent(&slider, false);
    };
    
    setupSlider(dryWetSlider, dryWetLabel, "Dry/Wet:");
    setupSlider(outputGainSlider, outputGainLabel, "Gain (dB):");
    setupSlider(temperatureSlider, temperatureLabel, "Temp (°C):");
    setupSlider(humiditySlider, humidityLabel, "Humidity (%):");
    setupSlider(sourceHeightSlider, sourceHeightLabel, "Source (m):");
    setupSlider(receiverHeightSlider, receiverHeightLabel, "Receiver (m):");
    
    // Ground type combo with modern styling
    addAndMakeVisible(groundTypeCombo);
    addAndMakeVisible(groundTypeLabel);

    groundTypeCombo.addItem("Hard", 1);
    groundTypeCombo.addItem("Porous", 2);
    groundTypeCombo.addItem("Mixed", 3);
    groundTypeCombo.setSelectedId(2); // Default to Porous

    // Modern combo box colors
    groundTypeCombo.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0x20ffffff));
    groundTypeCombo.setColour(juce::ComboBox::textColourId, juce::Colours::white);
    groundTypeCombo.setColour(juce::ComboBox::outlineColourId, juce::Colour(0x40ffffff));
    groundTypeCombo.setColour(juce::ComboBox::buttonColourId, juce::Colour(0xff00d4ff));
    groundTypeCombo.setColour(juce::ComboBox::arrowColourId, juce::Colours::white);

    groundTypeLabel.setText("Ground:", juce::dontSendNotification);
    groundTypeLabel.setJustificationType(juce::Justification::centredRight);
    groundTypeLabel.setColour(juce::Label::textColourId, juce::Colour(0xffcccccc));
    
    // Status labels with modern styling
    addAndMakeVisible(statusLabel);
    addAndMakeVisible(terrainInfoLabel);
    addAndMakeVisible(profileInfoLabel);

    statusLabel.setFont(juce::Font(12.0f));
    terrainInfoLabel.setFont(juce::Font(11.0f));
    profileInfoLabel.setFont(juce::Font(11.0f));

    statusLabel.setColour(juce::Label::textColourId, juce::Colour(0xff00d4ff));
    terrainInfoLabel.setColour(juce::Label::textColourId, juce::Colour(0xffcccccc));
    profileInfoLabel.setColour(juce::Label::textColourId, juce::Colour(0xffcccccc));
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
                updateGeoBounds(); // Update zoom bounds for new DEM
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

    chooser.launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                       [this](const juce::FileChooser& fc) {
        juce::File file = fc.getResult();
        if (file.existsAsFile()) {
            juce::String errorMsg;

            if (processor.loadTerrainFile(file, errorMsg)) {
                statusLabel.setText("Loaded DEM: " + file.getFileName(),
                                   juce::dontSendNotification);
                updateGeoBounds(); // Update zoom bounds for new DEM
            } else {
                juce::AlertWindow::showMessageBoxAsync(
                    juce::AlertWindow::WarningIcon,
                    "Load Error",
                    "Failed to load DEM file: " + errorMsg);
            }
        }
    });
}

void LandscapeAcousticVSTEditor::loadQGISConfig()
{
    juce::FileChooser chooser("Select QGIS config file", juce::File{}, "*.json");

    chooser.launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                       [this](const juce::FileChooser& fc) {
        juce::File file = fc.getResult();
        if (file.existsAsFile()) {
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
    });
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

    chooser.launchAsync(juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
                       [this](const juce::FileChooser& fc) {
        juce::File file = fc.getResult();
        if (file.existsAsFile() || file.getParentDirectory().exists()) {
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
    });
}

//==============================================================================
// Zoom functionality

void LandscapeAcousticVSTEditor::zoomIn()
{
    if (!processor.terrainLoader->isLoaded()) {
        return;
    }

    const auto& demData = processor.terrainLoader->getDEMData();
    auto demBounds = demData.getBounds();

    // Calculate center of current view
    double centerX = (geoXMin + geoXMax) / 2.0;
    double centerY = (geoYMin + geoYMax) / 2.0;

    // Increase zoom level
    zoomLevel = juce::jmin(10.0f, zoomLevel * 1.5f);

    // Calculate new zoomed bounds centered on current view
    double width = demBounds.getWidth() / zoomLevel;
    double height = demBounds.getHeight() / zoomLevel;

    geoXMin = centerX - width / 2.0;
    geoXMax = centerX + width / 2.0;
    geoYMin = centerY - height / 2.0;
    geoYMax = centerY + height / 2.0;

    // Clamp to DEM bounds
    geoXMin = juce::jmax(demBounds.getX(), geoXMin);
    geoXMax = juce::jmin(demBounds.getRight(), geoXMax);
    geoYMin = juce::jmax(demBounds.getY(), geoYMin);
    geoYMax = juce::jmin(demBounds.getBottom(), geoYMax);

    repaint();
}

void LandscapeAcousticVSTEditor::zoomOut()
{
    if (!processor.terrainLoader->isLoaded()) {
        return;
    }

    const auto& demData = processor.terrainLoader->getDEMData();
    auto demBounds = demData.getBounds();

    // Calculate center of current view
    double centerX = (geoXMin + geoXMax) / 2.0;
    double centerY = (geoYMin + geoYMax) / 2.0;

    // Decrease zoom level
    zoomLevel = juce::jmax(0.1f, zoomLevel / 1.5f);

    // Calculate new zoomed bounds centered on current view
    double width = demBounds.getWidth() / zoomLevel;
    double height = demBounds.getHeight() / zoomLevel;

    geoXMin = centerX - width / 2.0;
    geoXMax = centerX + width / 2.0;
    geoYMin = centerY - height / 2.0;
    geoYMax = centerY + height / 2.0;

    // Clamp to DEM bounds
    geoXMin = juce::jmax(demBounds.getX(), geoXMin);
    geoXMax = juce::jmin(demBounds.getRight(), geoXMax);
    geoYMin = juce::jmax(demBounds.getY(), geoYMin);
    geoYMax = juce::jmin(demBounds.getBottom(), geoYMax);

    repaint();
}

void LandscapeAcousticVSTEditor::updateGeoBounds()
{
    if (!processor.terrainLoader->isLoaded()) {
        return;
    }

    const auto& demData = processor.terrainLoader->getDEMData();
    auto demBounds = demData.getBounds();

    // Set zoom bounds to show the entire DEM
    geoXMin = demBounds.getX();
    geoXMax = demBounds.getRight();
    geoYMin = demBounds.getY();
    geoYMax = demBounds.getBottom();

    // Calculate zoom level based on how much we need to scale to fit
    double demWidth = demBounds.getWidth();
    double demHeight = demBounds.getHeight();
    double viewWidth = terrainViewArea.getWidth();
    double viewHeight = terrainViewArea.getHeight();

    // Calculate zoom level to fit entire DEM (with some padding)
    double zoomX = viewWidth / demWidth;
    double zoomY = viewHeight / demHeight;
    zoomLevel = juce::jmin(zoomX, zoomY) * 0.95; // 95% to add some margin

    // Don't clamp zoom level - allow showing entire DEM even if it requires zooming out
    // zoomLevel = juce::jmax(0.01f, zoomLevel); // Allow very small zoom levels for large DEMs
}

//==============================================================================
// Terrain visualization

void LandscapeAcousticVSTEditor::drawTerrainMap(juce::Graphics& g)
{
    if (!processor.terrainLoader->isLoaded()) {
        return;
    }

    auto bounds = terrainViewArea;

    // Modern terrain view background with subtle gradient
    juce::ColourGradient terrainGradient(
        juce::Colour(0xff1e1e2e), bounds.getTopLeft().toFloat(),
        juce::Colour(0xff2a2a3a), bounds.getBottomRight().toFloat(),
        false);
    g.setGradientFill(terrainGradient);
    g.fillRoundedRectangle(bounds.toFloat(), 6.0f);

    // Border with glow effect
    g.setColour(juce::Colour(0x60ffffff));
    g.drawRoundedRectangle(bounds.toFloat(), 6.0f, 2.0f);

    // Inner border
    g.setColour(juce::Colour(0x30ffffff));
    g.drawRoundedRectangle(bounds.reduced(1).toFloat(), 5.0f, 1.0f);

    // Use TerrainRenderer for virtual terrain rendering
    processor.terrainLoader->getRenderer().render(g, bounds, geoXMin, geoXMax, geoYMin, geoYMax);

    // Draw source and receiver points with modern styling
    if (sourcePoint.x != 0.0 || sourcePoint.y != 0.0) {
        auto sourcePx = geoToPixel(sourcePoint);

        // Outer glow
        g.setColour(juce::Colour(0x80ff4444));
        g.fillEllipse(sourcePx.x - 6.0f, sourcePx.y - 6.0f, 12.0f, 12.0f);

        // Inner point
        g.setColour(juce::Colour(0xffff4444));
        g.fillEllipse(sourcePx.x - 3.0f, sourcePx.y - 3.0f, 6.0f, 6.0f);

        // Label with background
        g.setColour(juce::Colours::black.withAlpha(0.7f));
        g.fillRoundedRectangle(sourcePx.x - 12, sourcePx.y - 18, 24, 12, 3.0f);
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(10.0f, juce::Font::bold));
        g.drawText("S", sourcePx.x - 10, sourcePx.y - 16, 20, 10, juce::Justification::centred);
    }

    if (receiverPoint.x != 0.0 || receiverPoint.y != 0.0) {
        auto receiverPx = geoToPixel(receiverPoint);

        // Outer glow
        g.setColour(juce::Colour(0x8044ff44));
        g.fillEllipse(receiverPx.x - 6.0f, receiverPx.y - 6.0f, 12.0f, 12.0f);

        // Inner point
        g.setColour(juce::Colour(0xff44ff44));
        g.fillEllipse(receiverPx.x - 3.0f, receiverPx.y - 3.0f, 6.0f, 6.0f);

        // Label with background
        g.setColour(juce::Colours::black.withAlpha(0.7f));
        g.fillRoundedRectangle(receiverPx.x - 12, receiverPx.y - 18, 24, 12, 3.0f);
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(10.0f, juce::Font::bold));
        g.drawText("R", receiverPx.x - 10, receiverPx.y - 16, 20, 10, juce::Justification::centred);
    }

    // Draw path between points with modern styling
    if ((sourcePoint.x != 0.0 || sourcePoint.y != 0.0) &&
        (receiverPoint.x != 0.0 || receiverPoint.y != 0.0)) {
        auto sourcePx = geoToPixel(sourcePoint);
        auto receiverPx = geoToPixel(receiverPoint);

        // Glow effect
        g.setColour(juce::Colour(0x60ffffff));
        g.drawLine(sourcePx.x, sourcePx.y, receiverPx.x, receiverPx.y, 3.0f);

        // Main line
        g.setColour(juce::Colour(0xffffff00));
        g.drawLine(sourcePx.x, sourcePx.y, receiverPx.x, receiverPx.y, 2.0f);
    }

    // Draw zoom level indicator
    if (zoomLevel != 1.0f) {
        g.setColour(juce::Colours::black.withAlpha(0.7f));
        g.fillRoundedRectangle(bounds.getRight() - 60, bounds.getY() + 5, 55, 20, 5.0f);
        g.setColour(juce::Colour(0xff00d4ff));
        g.setFont(juce::Font(11.0f, juce::Font::bold));
        g.drawText(juce::String::formatted("%.1fx", zoomLevel),
                   bounds.getRight() - 58, bounds.getY() + 7, 51, 16, juce::Justification::centred);
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
    
    // Normalize pixel coordinates to [0,1] within terrain view area
    double normX = static_cast<double>(pixel.x - terrainViewArea.getX()) / terrainViewArea.getWidth();
    double normY = static_cast<double>(pixel.y - terrainViewArea.getY()) / terrainViewArea.getHeight();
    
    // Map to geographic coordinates using zoom bounds
    double geoX = geoXMin + normX * (geoXMax - geoXMin);
    double geoY = geoYMax - normY * (geoYMax - geoYMin); // Flip Y axis
    
    return {geoX, geoY};
}

juce::Point<int> LandscapeAcousticVSTEditor::geoToPixel(juce::Point<double> geo) const
{
    if (!processor.terrainLoader->isLoaded()) {
        return {0, 0};
    }
    
    // Normalize geographic coordinates using zoom bounds
    double normX = (geo.x - geoXMin) / (geoXMax - geoXMin);
    double normY = (geo.y - geoYMin) / (geoYMax - geoYMin);
    normY = 1.0 - normY; // Flip Y axis
    
    // Map to pixel coordinates
    int pixelX = terrainViewArea.getX() + static_cast<int>(normX * terrainViewArea.getWidth());
    int pixelY = terrainViewArea.getY() + static_cast<int>(normY * terrainViewArea.getHeight());
    
    return {pixelX, pixelY};
}

//==============================================================================
// Virtual method implementations

void LandscapeAcousticVSTEditor::setCurrentAudioFile (const juce::String& filename)
{
    // Default implementation - do nothing for VST plugin
    // Standalone version overrides this
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
        auto demBounds = demData.getBounds();
        
        terrainInfoLabel.setText(
            juce::String::formatted("DEM: %dx%d, %.3f°-%.3f° lon, %.3f°-%.3f° lat",
                                   demData.width, demData.height,
                                   demBounds.getX(), demBounds.getRight(),
                                   demBounds.getY(), demBounds.getBottom()),
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