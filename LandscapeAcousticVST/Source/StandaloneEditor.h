#pragma once

#include "PluginEditor.h"

// Forward declaration
class LandscapeAcousticVSTStandaloneApplication;

/**
 * @brief Extended Plugin Editor with audio controls for standalone version
 */
class LandscapeAcousticVSTStandaloneEditor : public LandscapeAcousticVSTEditor
{
public:
    LandscapeAcousticVSTStandaloneEditor (LandscapeAcousticVSTProcessor& p);
    ~LandscapeAcousticVSTStandaloneEditor() override = default;

    void setCurrentAudioFile (const juce::String& filename) override;

private:
    void addAudioControls();
    void loadAudioFile();
    void togglePlayback();
    void updatePlayButton();

    void paint(juce::Graphics& g) override;
    void timerCallback() override;
    void resized() override;

    LandscapeAcousticVSTStandaloneApplication* standaloneApp = nullptr;

    juce::Label currentAudioFileLabel;
    juce::TextButton loadAudioButton;
    juce::TextButton playButton;
    juce::TextButton stopButton;
    juce::Slider positionSlider;
    juce::Label timeLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LandscapeAcousticVSTStandaloneEditor)
};