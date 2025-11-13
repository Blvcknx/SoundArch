#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_core/juce_core.h>
#include <juce_data_structures/juce_data_structures.h>
#include <juce_dsp/juce_dsp.h>
#include <atomic>
#include "TerrainLoader.h"
#include "AcousticEngine.h"
#include "ImpulseGenerator.h"

/**
 * @brief Main audio processor for LandscapeAcousticVST plugin
 * 
 * Coordinates terrain loading, acoustic calculations, and real-time convolution.
 * Implements JUCE AudioProcessor interface for VST3/AU/Standalone compatibility.
 */
class LandscapeAcousticVSTProcessor : public juce::AudioProcessor,
                                      public juce::AudioProcessorValueTreeState::Listener {
public:
    LandscapeAcousticVSTProcessor();
    ~LandscapeAcousticVSTProcessor() override;
    
    //==============================================================================
    // AudioProcessor interface
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    
#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif
    
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    
    //==============================================================================
    // Editor interface  
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;
    
    //==============================================================================
    // Program/preset interface
    const juce::String getName() const override;
    
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;
    
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;
    
    //==============================================================================
    // State management
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;
    
    //==============================================================================
    // Plugin-specific interface
    
    /**
     * @brief Load DEM file and initialize terrain data
     */
    bool loadTerrainFile(const juce::File& demFile, juce::String& errorMsg);
    
    /**
     * @brief Set source and receiver points for acoustic analysis
     */
    void setAnalysisPoints(juce::Point<double> source, juce::Point<double> receiver);
    
    /**
     * @brief Update impulse response based on current settings
     */
    void updateImpulseResponse();
    
    /**
     * @brief Get current terrain profile
     */
    TerrainProfile getCurrentProfile() const;
    
    /**
     * @brief Import configuration from QGIS export
     */
    bool importQGISConfig(const juce::File& configFile, juce::String& errorMsg);
    
    /**
     * @brief Export current impulse response
     */
    bool exportImpulseResponse(const juce::File& outputFile);
    
    // Public member access for editor
    std::unique_ptr<TerrainLoader> terrainLoader;
    std::unique_ptr<AcousticEngine> acousticEngine;
    std::unique_ptr<ImpulseGenerator> impulseGenerator;
    
    juce::AudioProcessorValueTreeState parameters;
    
private:
    //==============================================================================
    // DSP components
    juce::dsp::Convolution convolution;
    
    // Current analysis state
    juce::Point<double> sourcePoint {0.0, 0.0};
    juce::Point<double> receiverPoint {0.0, 0.0};
    bool pointsAreSet = false;
    
    // Processing state
    std::atomic<bool> needsIRUpdate {false};
    std::atomic<bool> isProcessingIR {false};
    
    /**
     * @brief Create parameter layout
     */
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    /**
     * @brief Parameter change callback
     */
    void parameterChanged(const juce::String& parameterID, float newValue);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LandscapeAcousticVSTProcessor)
};