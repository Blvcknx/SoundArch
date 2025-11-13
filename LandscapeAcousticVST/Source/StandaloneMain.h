#pragma once

#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "PluginProcessor.h"
#include "StandaloneEditor.h"

//==============================================================================
class LandscapeAcousticVSTStandaloneApplication : public juce::JUCEApplication
{
public:
    LandscapeAcousticVSTStandaloneApplication() = default;

    const juce::String getApplicationName() override       { return "Landscape Acoustic VST"; }
    const juce::String getApplicationVersion() override    { return "1.0.0"; }
    bool moreThanOneInstanceAllowed() override             { return true; }

    //==============================================================================
    void initialise (const juce::String& commandLine) override;
    void shutdown() override;

    //==============================================================================
    void systemRequestedQuit() override;
    void anotherInstanceStarted (const juce::String& commandLine) override;

    //==============================================================================
    bool loadAudioFile (const juce::File& audioFile);
    void playAudio();
    void pauseAudio();
    void stopAudio();
    bool isPlaying() const;
    double getCurrentPosition() const;
    double getLengthInSeconds() const;
    void setPosition (double positionInSeconds);

    LandscapeAcousticVSTProcessor* getProcessor() { return processor.get(); }

private:
    //==============================================================================
    class MainWindow : public juce::DocumentWindow
    {
    public:
        MainWindow (juce::String name, LandscapeAcousticVSTEditor* editor);
        void closeButtonPressed() override;

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

    std::unique_ptr<LandscapeAcousticVSTProcessor> processor;
    std::unique_ptr<LandscapeAcousticVSTEditor> editor;
    std::unique_ptr<MainWindow> mainWindow;

    // Audio playback components
    juce::AudioDeviceManager audioDeviceManager;
    juce::AudioSourcePlayer audioSourcePlayer;
    juce::AudioTransportSource transportSource;
    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> currentAudioFileSource;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LandscapeAcousticVSTStandaloneApplication)
};