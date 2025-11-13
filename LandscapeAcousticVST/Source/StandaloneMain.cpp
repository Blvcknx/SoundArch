#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "StandaloneMain.h"

//==============================================================================
void LandscapeAcousticVSTStandaloneApplication::initialise (const juce::String& commandLine)
{
    // Create the processor
    processor = std::make_unique<LandscapeAcousticVSTProcessor>();
    
    // Create standalone editor
    editor = std::make_unique<LandscapeAcousticVSTStandaloneEditor>(*processor);
    
    // Create main window
    mainWindow.reset (new MainWindow (getApplicationName(), editor.get()));

    // Set up audio device manager
    audioDeviceManager.initialiseWithDefaultDevices (2, 2);

    // Create audio source player
    audioSourcePlayer.setSource (&transportSource);

    // Connect to audio device
    audioDeviceManager.addAudioCallback (&audioSourcePlayer);

    // Load default audio file if specified
    if (commandLine.isNotEmpty())
    {
        juce::File audioFile (commandLine.unquoted());
        if (audioFile.existsAsFile())
            loadAudioFile (audioFile);
    }
}

void LandscapeAcousticVSTStandaloneApplication::shutdown()
{
    // Stop audio playback
    transportSource.stop();
    transportSource.setSource (nullptr);
    audioDeviceManager.removeAudioCallback (&audioSourcePlayer);

    // Close main window
    mainWindow = nullptr;
    editor = nullptr;
    processor = nullptr;
}

void LandscapeAcousticVSTStandaloneApplication::systemRequestedQuit()
{
    quit();
}

void LandscapeAcousticVSTStandaloneApplication::anotherInstanceStarted (const juce::String& commandLine)
{
    // Handle multiple instances if needed
}

bool LandscapeAcousticVSTStandaloneApplication::loadAudioFile (const juce::File& audioFile)
{
    // Stop current playback
    transportSource.stop();
    transportSource.setSource (nullptr);

    // Create audio format reader
    auto* reader = formatManager.createReaderFor (audioFile);

    if (reader != nullptr)
    {
        // Create audio format reader source
        currentAudioFileSource = std::make_unique<juce::AudioFormatReaderSource> (reader, true);

        // Set up transport source
        transportSource.setSource (currentAudioFileSource.get(), 0, nullptr, reader->sampleRate);

        // Update UI
        if (editor != nullptr)
            editor->setCurrentAudioFile (audioFile.getFileName());

        return true;
    }

    return false;
}

void LandscapeAcousticVSTStandaloneApplication::playAudio()
{
    if (transportSource.getCurrentPosition() >= transportSource.getLengthInSeconds())
        transportSource.setPosition (0.0);

    transportSource.start();
}

void LandscapeAcousticVSTStandaloneApplication::pauseAudio()
{
    transportSource.stop();
}

void LandscapeAcousticVSTStandaloneApplication::stopAudio()
{
    transportSource.stop();
    transportSource.setPosition (0.0);
}

bool LandscapeAcousticVSTStandaloneApplication::isPlaying() const
{
    return transportSource.isPlaying();
}

double LandscapeAcousticVSTStandaloneApplication::getCurrentPosition() const
{
    return transportSource.getCurrentPosition();
}

double LandscapeAcousticVSTStandaloneApplication::getLengthInSeconds() const
{
    return transportSource.getLengthInSeconds();
}

void LandscapeAcousticVSTStandaloneApplication::setPosition (double positionInSeconds)
{
    transportSource.setPosition (positionInSeconds);
}

//==============================================================================
// MainWindow implementation
LandscapeAcousticVSTStandaloneApplication::MainWindow::MainWindow (juce::String name, LandscapeAcousticVSTEditor* editor)
    : DocumentWindow (name,
                    juce::Desktop::getInstance().getDefaultLookAndFeel()
                                                  .findColour (juce::ResizableWindow::backgroundColourId),
                    DocumentWindow::allButtons)
{
    setUsingNativeTitleBar (true);
    setContentOwned (editor, true);
    setResizable (true, true);
    centreWithSize (getWidth(), getHeight());
    setVisible (true);
}

void LandscapeAcousticVSTStandaloneApplication::MainWindow::closeButtonPressed()
{
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
}

//==============================================================================
// Extended Plugin Editor with audio controls
// (Implementation moved to StandaloneEditor.cpp)

//==============================================================================
START_JUCE_APPLICATION (LandscapeAcousticVSTStandaloneApplication)

// Override main with no arguments
int main()
{
    return juce::JUCEApplicationBase::main();
}