#include "StandaloneEditor.h"
#include "StandaloneMain.h"
#include "StandaloneMain.h"

//==============================================================================
LandscapeAcousticVSTStandaloneEditor::LandscapeAcousticVSTStandaloneEditor (LandscapeAcousticVSTProcessor& p)
    : LandscapeAcousticVSTEditor (p)
{
    // Get reference to standalone application
    if (auto* app = dynamic_cast<LandscapeAcousticVSTStandaloneApplication*> (
        juce::JUCEApplication::getInstance()))
    {
        standaloneApp = app;
    }

    // Add audio control components
    addAudioControls();
}

void LandscapeAcousticVSTStandaloneEditor::paint(juce::Graphics& g)
{
    // Call parent paint method to ensure modern UI is drawn
    LandscapeAcousticVSTEditor::paint(g);
}

void LandscapeAcousticVSTStandaloneEditor::setCurrentAudioFile (const juce::String& filename)
{
    currentAudioFileLabel.setText ("Audio: " + filename, juce::dontSendNotification);
    updatePlayButton();
}

void LandscapeAcousticVSTStandaloneEditor::addAudioControls()
{
    // Audio file info label
    addAndMakeVisible (currentAudioFileLabel);
    currentAudioFileLabel.setText ("No audio file loaded", juce::dontSendNotification);
    currentAudioFileLabel.setColour (juce::Label::textColourId, juce::Colour(0xffcccccc));
    currentAudioFileLabel.setColour (juce::Label::backgroundColourId, juce::Colour(0x20ffffff));

    // Load audio button with modern styling
    addAndMakeVisible (loadAudioButton);
    loadAudioButton.setButtonText ("Load Audio");
    loadAudioButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff00d4ff));
    loadAudioButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff0099cc));
    loadAudioButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    loadAudioButton.setColour(juce::TextButton::textColourOnId, juce::Colours::white);
    loadAudioButton.onClick = [this] { loadAudioFile(); };

    // Transport controls with modern styling
    addAndMakeVisible (playButton);
    playButton.setButtonText ("Play");
    playButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff44ff44));
    playButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff22cc22));
    playButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    playButton.setColour(juce::TextButton::textColourOnId, juce::Colours::white);
    playButton.onClick = [this] { togglePlayback(); };

    addAndMakeVisible (stopButton);
    stopButton.setButtonText ("Stop");
    stopButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xffff4444));
    stopButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xffcc2222));
    stopButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    stopButton.setColour(juce::TextButton::textColourOnId, juce::Colours::white);
    stopButton.onClick = [this] {
        if (standaloneApp) standaloneApp->stopAudio();
        updatePlayButton();
    };

    // Position slider with modern styling
    addAndMakeVisible (positionSlider);
    positionSlider.setRange (0.0, 1.0, 0.001);
    positionSlider.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
    positionSlider.setColour(juce::Slider::thumbColourId, juce::Colour(0xff00d4ff));
    positionSlider.setColour(juce::Slider::trackColourId, juce::Colour(0x60ffffff));
    positionSlider.setColour(juce::Slider::backgroundColourId, juce::Colour(0x20ffffff));
    positionSlider.onValueChange = [this] {
        if (standaloneApp && !positionSlider.isMouseButtonDown())
            standaloneApp->setPosition (positionSlider.getValue() * standaloneApp->getLengthInSeconds());
    };

    // Time display
    addAndMakeVisible (timeLabel);
    timeLabel.setText ("00:00 / 00:00", juce::dontSendNotification);
    timeLabel.setColour (juce::Label::textColourId, juce::Colour(0xffcccccc));
    timeLabel.setJustificationType (juce::Justification::centred);

    // Start timer for UI updates
    startTimer (50); // 20 FPS
}

void LandscapeAcousticVSTStandaloneEditor::loadAudioFile()
{
    juce::FileChooser chooser ("Select audio file",
                              juce::File{},
                              "*.wav;*.aiff;*.flac;*.mp3;*.ogg");

    // Try asynchronous file chooser
    chooser.launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                       [this](const juce::FileChooser& fc) {
        juce::File file = fc.getResult();
        if (file.existsAsFile() && standaloneApp)
        {
            if (standaloneApp->loadAudioFile (file))
            {
                setCurrentAudioFile (file.getFileName());
                positionSlider.setValue (0.0, juce::dontSendNotification);
            }
            else
            {
                juce::AlertWindow::showMessageBoxAsync (
                    juce::AlertWindow::WarningIcon,
                    "Load Error",
                    "Failed to load audio file: " + file.getFileName());
            }
        }
    });
}

void LandscapeAcousticVSTStandaloneEditor::togglePlayback()
{
    if (standaloneApp)
    {
        if (standaloneApp->isPlaying())
            standaloneApp->pauseAudio();
        else
            standaloneApp->playAudio();

        updatePlayButton();
    }
}

void LandscapeAcousticVSTStandaloneEditor::updatePlayButton()
{
    if (standaloneApp && standaloneApp->isPlaying())
        playButton.setButtonText ("Pause");
    else
        playButton.setButtonText ("Play");
}

void LandscapeAcousticVSTStandaloneEditor::timerCallback()
{
    LandscapeAcousticVSTEditor::timerCallback();

    if (standaloneApp)
    {
        // Update position slider
        double length = standaloneApp->getLengthInSeconds();
        if (length > 0.0)
        {
            double position = standaloneApp->getCurrentPosition();
            positionSlider.setValue (position / length, juce::dontSendNotification);

            // Update time display
            auto formatTime = [](double seconds) -> juce::String {
                int mins = static_cast<int>(seconds) / 60;
                int secs = static_cast<int>(seconds) % 60;
                return juce::String::formatted ("%02d:%02d", mins, secs);
            };

            timeLabel.setText (formatTime (position) + " / " + formatTime (length),
                             juce::dontSendNotification);
        }
    }
}

void LandscapeAcousticVSTStandaloneEditor::resized()
{
    LandscapeAcousticVSTEditor::resized();

    auto bounds = getLocalBounds();
    auto bottomSection = bounds.removeFromBottom (60);

    // Audio controls layout
    auto controlsArea = bottomSection.reduced (10);
    auto buttonWidth = 80;
    auto buttonHeight = 25;

    // Top row: file info and load button
    auto topRow = controlsArea.removeFromTop (25);
    currentAudioFileLabel.setBounds (topRow.removeFromLeft (controlsArea.getWidth() - buttonWidth - 10));
    loadAudioButton.setBounds (topRow);

    // Bottom row: transport controls
    controlsArea.removeFromTop (5);
    playButton.setBounds (controlsArea.getX(), controlsArea.getY(), buttonWidth, buttonHeight);
    stopButton.setBounds (playButton.getRight() + 5, controlsArea.getY(), buttonWidth, buttonHeight);

    auto sliderArea = controlsArea.withLeft (stopButton.getRight() + 15);
    positionSlider.setBounds (sliderArea.withHeight (buttonHeight));

    timeLabel.setBounds (sliderArea.withTop (sliderArea.getBottom() - 15).withHeight (15));
}