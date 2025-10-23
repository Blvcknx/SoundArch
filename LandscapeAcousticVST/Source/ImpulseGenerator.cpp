#include "ImpulseGenerator.h"
#include "Utils.h"
#include <algorithm>

ImpulseGenerator::ImpulseGenerator() {
    impulseResponse.setSize(2, 0); // Start with empty stereo buffer
}

juce::AudioBuffer<float> ImpulseGenerator::generateImpulseResponse(
    const TerrainProfile& profile,
    double sourceHeight_m,
    double receiverHeight_m,
    const AtmosphericConditions& atmo,
    GroundType ground,
    double sampleRate,
    double maxLength_s)
{
    auto startTime = juce::Time::getMillisecondCounterHiRes();
    
    if (!profile.isValid() || sampleRate <= 0.0) {
        return juce::AudioBuffer<float>(2, 0);
    }
    
    // Calculate IR length in samples
    int irLength = static_cast<int>(maxLength_s * sampleRate);
    irLength = std::min(irLength, 32768); // Maximum 32k samples
    
    // Clear and resize impulse response buffer
    impulseResponse.setSize(2, irLength, true, true, false);
    
    // Get octave band frequencies for analysis
    auto frequencies = AcousticEngine::getOctaveBands();
    
    // Calculate frequency-dependent attenuations
    auto attenuations = acousticEngine.calculateFrequencyResponse(
        profile, sourceHeight_m, receiverHeight_m, frequencies, atmo, ground);
    
    // Calculate direct arrival delay
    const double SPEED_OF_SOUND = 343.0; // m/s at 20°C
    double distance = profile.totalDistance;
    double delay_s = distance / SPEED_OF_SOUND;
    
    // Ensure delay fits within IR length
    if (delay_s >= maxLength_s) {
        delay_s = maxLength_s * 0.9; // Leave some room
    }
    
    int delaySamples = static_cast<int>(delay_s * sampleRate);
    
    // Synthesize direct arrival
    synthesizeDirectArrival(frequencies, attenuations, delay_s, sampleRate,
                           impulseResponse, delaySamples);
    
    // Apply windowing and normalization
    applyWindowing(impulseResponse, sampleRate);
    normalizeIR(impulseResponse);
    
    // Store metadata
    metadata.distance_m = distance;
    metadata.totalAttenuation_dB = *std::max_element(attenuations.begin(), attenuations.end());
    metadata.frequencyResponse = attenuations;
    metadata.atmosphere = atmo;
    metadata.groundType = ground;
    metadata.generationTime_ms = juce::Time::getMillisecondCounterHiRes() - startTime;
    
    return impulseResponse;
}

void ImpulseGenerator::synthesizeDirectArrival(
    const std::vector<double>& frequencies,
    const std::vector<double>& attenuations,
    double delay_s,
    double sampleRate,
    juce::AudioBuffer<float>& buffer,
    int startSample)
{
    if (startSample >= buffer.getNumSamples()) {
        return; // Delay exceeds buffer length
    }
    
    // Synthesize each frequency component
    for (size_t i = 0; i < frequencies.size(); ++i) {
        double freq = frequencies[i];
        double atten_dB = attenuations[i];
        double amplitude = juce::Decibels::decibelsToGain(static_cast<float>(-atten_dB));
        
        // Window size based on frequency (at least one period, max 512 samples)
        int windowSize = static_cast<int>(std::min(sampleRate / freq, 512.0));
        windowSize = std::max(windowSize, 8); // Minimum window size
        
        // Add frequency component to buffer
        addFrequencyComponent(buffer, freq, amplitude, startSample, 
                             windowSize, sampleRate);
    }
}

void ImpulseGenerator::addFrequencyComponent(
    juce::AudioBuffer<float>& buffer,
    double frequency_Hz,
    double amplitude,
    int startSample,
    int windowSize,
    double sampleRate)
{
    // Generate windowed sinc function
    auto sinc = createWindowedSinc(frequency_Hz, windowSize, sampleRate);
    
    // Add to both channels with slight stereo spread
    for (int ch = 0; ch < std::min(2, buffer.getNumChannels()); ++ch) {
        auto* channelData = buffer.getWritePointer(ch);
        
        // Slight amplitude variation for stereo width
        float chAmplitude = amplitude * (ch == 0 ? 1.0f : 0.95f);
        
        for (int i = 0; i < static_cast<int>(sinc.size()); ++i) {
            int sampleIdx = startSample + i - windowSize / 2;
            if (sampleIdx >= 0 && sampleIdx < buffer.getNumSamples()) {
                channelData[sampleIdx] += sinc[i] * chAmplitude;
            }
        }
    }
}

std::vector<float> ImpulseGenerator::createWindowedSinc(
    double frequency_Hz,
    int windowSize,
    double sampleRate)
{
    std::vector<float> sinc(windowSize);
    
    double nyquist = sampleRate * 0.5;
    double normalizedFreq = frequency_Hz / nyquist;
    
    int center = windowSize / 2;
    
    for (int i = 0; i < windowSize; ++i) {
        int n = i - center;
        double t = static_cast<double>(n) / sampleRate;
        
        double sincValue;
        if (n == 0) {
            sincValue = 2.0 * normalizedFreq; // sinc(0) = 1, scaled by bandwidth
        } else {
            double x = 2.0 * M_PI * normalizedFreq * n;
            sincValue = std::sin(x) / x * 2.0 * normalizedFreq;
        }
        
        // Apply Hann window
        double window = 0.5 * (1.0 + std::cos(2.0 * M_PI * i / (windowSize - 1)));
        
        sinc[i] = static_cast<float>(sincValue * window);
    }
    
    return sinc;
}

void ImpulseGenerator::applyWindowing(juce::AudioBuffer<float>& buffer, double sampleRate) {
    int numSamples = buffer.getNumSamples();
    
    // Fade-in samples
    int fadeInSamples = static_cast<int>(fadeInDuration * sampleRate);
    fadeInSamples = std::min(fadeInSamples, numSamples / 4);
    
    // Fade-out samples  
    int fadeOutSamples = static_cast<int>(fadeOutDuration * sampleRate);
    fadeOutSamples = std::min(fadeOutSamples, numSamples / 4);
    
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
        auto* channelData = buffer.getWritePointer(ch);
        
        // Apply fade-in
        for (int i = 0; i < fadeInSamples; ++i) {
            double gain = static_cast<double>(i) / fadeInSamples;
            gain = 0.5 * (1.0 - std::cos(M_PI * gain)); // Cosine fade
            channelData[i] *= static_cast<float>(gain);
        }
        
        // Apply fade-out
        for (int i = 0; i < fadeOutSamples; ++i) {
            int sampleIdx = numSamples - fadeOutSamples + i;
            double gain = static_cast<double>(fadeOutSamples - i) / fadeOutSamples;
            gain = 0.5 * (1.0 - std::cos(M_PI * gain)); // Cosine fade
            channelData[sampleIdx] *= static_cast<float>(gain);
        }
    }
}

void ImpulseGenerator::normalizeIR(juce::AudioBuffer<float>& buffer) {
    float maxMagnitude = 0.0f;
    
    // Find peak amplitude across all channels
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
        float channelMax = buffer.getMagnitude(ch, 0, buffer.getNumSamples());
        maxMagnitude = std::max(maxMagnitude, channelMax);
    }
    
    // Apply normalization to prevent clipping (leave 1 dB headroom)
    if (maxMagnitude > 0.001f) {
        float targetLevel = 0.89f; // -1 dB
        float gain = targetLevel / maxMagnitude;
        buffer.applyGain(gain);
    }
}

bool ImpulseGenerator::exportIR(const juce::File& outputFile) const {
    if (!hasImpulseResponse()) {
        return false;
    }
    
    // Create WAV format writer
    juce::WavAudioFormat wavFormat;
    std::unique_ptr<juce::FileOutputStream> fileStream(
        outputFile.createOutputStream());
    
    if (fileStream == nullptr) {
        return false;
    }
    
    std::unique_ptr<juce::AudioFormatWriter> writer(
        wavFormat.createWriterFor(
            fileStream.release(), // Writer takes ownership
            48000.0, // Fixed sample rate for export
            impulseResponse.getNumChannels(),
            24, // 24-bit depth
            {},  // Metadata
            0)); // Quality hint
    
    if (writer == nullptr) {
        return false;
    }
    
    // Write audio data
    return writer->writeFromAudioSampleBuffer(
        impulseResponse, 0, impulseResponse.getNumSamples());
}

juce::var ImpulseGenerator::getMetadata() const {
    juce::var metadataObj = juce::var(new juce::DynamicObject());
    auto* obj = metadataObj.getDynamicObject();
    
    obj->setProperty("distance_m", metadata.distance_m);
    obj->setProperty("totalAttenuation_dB", metadata.totalAttenuation_dB);
    obj->setProperty("temperature_C", metadata.atmosphere.temperature_C);
    obj->setProperty("humidity", metadata.atmosphere.relativeHumidity);
    obj->setProperty("groundType", static_cast<int>(metadata.groundType));
    obj->setProperty("generationTime_ms", metadata.generationTime_ms);
    obj->setProperty("irLength_samples", impulseResponse.getNumSamples());
    
    // Add frequency response array
    juce::Array<juce::var> freqResponse;
    for (double atten : metadata.frequencyResponse) {
        freqResponse.add(atten);
    }
    obj->setProperty("frequencyResponse_dB", freqResponse);
    
    return metadataObj;
}

void ImpulseGenerator::setParameters(double fadeIn_ms, double fadeOut_ms) {
    fadeInDuration = fadeIn_ms * 0.001; // Convert to seconds
    fadeOutDuration = fadeOut_ms * 0.001;
}