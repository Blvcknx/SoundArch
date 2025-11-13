#pragma once

#include "TerrainProfile.h"
#include "Environment.h"
#include "AcousticEngine.h"
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_core/juce_core.h>
#include <vector>

/**
 * @brief Synthesizes acoustic impulse responses from terrain profiles
 * 
 * Converts terrain-based acoustic calculations into time-domain impulse responses
 * suitable for real-time convolution. Uses frequency-dependent attenuation from
 * ISO 9613-2 calculations to create realistic outdoor propagation effects.
 */
class ImpulseGenerator {
public:
    ImpulseGenerator();
    ~ImpulseGenerator() = default;
    
    /**
     * @brief Generate impulse response from terrain and acoustic parameters
     * @param profile Terrain elevation profile
     * @param sourceHeight_m Height of source above terrain (meters)
     * @param receiverHeight_m Height of receiver above terrain (meters)
     * @param atmo Atmospheric conditions
     * @param ground Ground surface type
     * @param sampleRate Audio sample rate (Hz)
     * @param maxLength_s Maximum IR length in seconds (default: 0.5s)
     * @return Stereo impulse response buffer
     */
    juce::AudioBuffer<float> generateImpulseResponse(
        const TerrainProfile& profile,
        double sourceHeight_m,
        double receiverHeight_m,
        const AtmosphericConditions& atmo,
        GroundType ground,
        double sampleRate,
        double maxLength_s = 0.5);
    
    /**
     * @brief Get most recently generated impulse response
     */
    const juce::AudioBuffer<float>& getImpulseResponse() const { 
        return impulseResponse; 
    }
    
    /**
     * @brief Check if impulse response is available
     */
    bool hasImpulseResponse() const { 
        return impulseResponse.getNumSamples() > 0; 
    }
    
    /**
     * @brief Export impulse response to WAV file
     * @param outputFile Output WAV file
     * @return True if export successful
     */
    bool exportIR(const juce::File& outputFile) const;
    
    /**
     * @brief Get impulse response metadata as JSON
     */
    juce::var getMetadata() const;
    
    /**
     * @brief Set IR generation parameters
     */
    void setParameters(double fadeIn_ms = 5.0, double fadeOut_ms = 50.0);
    
private:
    juce::AudioBuffer<float> impulseResponse;
    AcousticEngine acousticEngine;
    
    // Generation parameters
    double fadeInDuration = 0.005;  // 5ms fade-in
    double fadeOutDuration = 0.05;  // 50ms fade-out
    
    // Metadata for last generated IR
    struct IRMetadata {
        double distance_m;
        double totalAttenuation_dB;
        std::vector<double> frequencyResponse;
        juce::String terrainFile;
        AtmosphericConditions atmosphere;
        GroundType groundType;
        double generationTime_ms;
    } metadata;
    
    /**
     * @brief Synthesize direct arrival impulse
     * @param frequencies Octave band frequencies
     * @param attenuations Attenuation for each frequency band (dB)
     * @param delay_s Time delay (seconds)
     * @param sampleRate Sample rate (Hz)
     * @param buffer Output buffer to write to
     * @param startSample Starting sample index
     */
    void synthesizeDirectArrival(
        const std::vector<double>& frequencies,
        const std::vector<double>& attenuations,
        double delay_s,
        double sampleRate,
        juce::AudioBuffer<float>& buffer,
        int startSample);
    
    /**
     * @brief Apply windowing to impulse response
     */
    void applyWindowing(juce::AudioBuffer<float>& buffer, double sampleRate);
    
    /**
     * @brief Normalize impulse response to prevent clipping
     */
    void normalizeIR(juce::AudioBuffer<float>& buffer);
    
    /**
     * @brief Generate band-limited impulse for specific frequency
     */
    void addFrequencyComponent(
        juce::AudioBuffer<float>& buffer,
        double frequency_Hz,
        double amplitude,
        int startSample,
        int windowSize,
        double sampleRate);
    
    /**
     * @brief Create Hann-windowed sinc function
     */
    std::vector<float> createWindowedSinc(
        double frequency_Hz,
        int windowSize,
        double sampleRate);
};