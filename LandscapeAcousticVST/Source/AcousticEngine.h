#pragma once

#include "Environment.h"
#include "TerrainProfile.h"
#include "FresnelDiffraction.h"
#include <vector>

/**
 * @brief ISO 9613-2 acoustic propagation engine
 * 
 * Calculates outdoor sound attenuation including:
 * - Divergence (geometric spreading)
 * - Atmospheric absorption (temperature/humidity dependent)  
 * - Ground effect (impedance-based reflection/absorption)
 * - Barrier diffraction (Fresnel knife-edge theory)
 * 
 * Implements equations from ISO 9613-2:2024 standard for
 * engineering prediction of outdoor sound propagation.
 */
class AcousticEngine {
public:
    AcousticEngine() = default;
    ~AcousticEngine() = default;
    
    /**
     * @brief Calculate total attenuation for given frequency
     * @param profile Terrain elevation profile
     * @param sourceHeight_m Height of source above terrain (meters)
     * @param receiverHeight_m Height of receiver above terrain (meters) 
     * @param frequency_Hz Sound frequency (Hz)
     * @param atmo Atmospheric conditions
     * @param ground Ground surface type
     * @return Total attenuation (dB)
     */
    double calculateTotalAttenuation(
        const TerrainProfile& profile,
        double sourceHeight_m,
        double receiverHeight_m, 
        double frequency_Hz,
        const AtmosphericConditions& atmo,
        GroundType ground) const;
    
    /**
     * @brief Calculate attenuation for multiple frequency bands
     * @param profile Terrain elevation profile
     * @param sourceHeight_m Height of source above terrain (meters)
     * @param receiverHeight_m Height of receiver above terrain (meters)
     * @param frequencies Vector of frequencies (Hz)
     * @param atmo Atmospheric conditions
     * @param ground Ground surface type
     * @return Vector of attenuations (dB) for each frequency
     */
    std::vector<double> calculateFrequencyResponse(
        const TerrainProfile& profile,
        double sourceHeight_m,
        double receiverHeight_m,
        const std::vector<double>& frequencies,
        const AtmosphericConditions& atmo, 
        GroundType ground) const;
    
    /**
     * @brief Get standard octave band center frequencies (63 Hz - 8 kHz)
     */
    static std::vector<double> getOctaveBands();
    
    // Individual attenuation component calculations
    
    /**
     * @brief Divergence attenuation (geometric spreading)
     * A_div = 20*log10(d/d0) + 11 dB
     */
    double computeDivergence(double distance_m) const;
    
    /**
     * @brief Atmospheric absorption attenuation
     * Based on ISO 9613-1 with temperature/humidity corrections
     */
    double computeAtmosphericAbsorption(
        double distance_m,
        double frequency_Hz, 
        const AtmosphericConditions& atmo) const;
    
    /**
     * @brief Ground effect attenuation
     * Frequency-dependent reflection/absorption by terrain surface
     */
    double computeGroundEffect(
        double distance_m,
        double sourceHeight_m,
        double receiverHeight_m,
        double frequency_Hz,
        GroundType ground) const;
    
    /**
     * @brief Barrier diffraction attenuation
     * Uses highest terrain obstruction and Fresnel theory
     */
    double computeBarrierDiffraction(
        const TerrainProfile& profile,
        double sourceHeight_m, 
        double receiverHeight_m,
        double frequency_Hz) const;
    
    /**
     * @brief Check if line-of-sight is clear
     */
    bool checkLineOfSight(
        const TerrainProfile& profile,
        double sourceHeight_m,
        double receiverHeight_m) const;
    
private:
    /**
     * @brief Lookup atmospheric absorption coefficient from ISO 9613-1 tables
     */
    double lookupAtmosphericAlpha(
        double frequency_Hz,
        double temperature_C, 
        double relativeHumidity) const;
    
    /**
     * @brief Find highest terrain obstruction for diffraction calculation
     */
    std::pair<size_t, double> findHighestObstruction(
        const TerrainProfile& profile,
        double sourceHeight_m,
        double receiverHeight_m) const;
    
    /**
     * @brief Calculate effective height for ground effect
     */
    double calculateEffectiveHeight(
        double sourceHeight_m,
        double receiverHeight_m) const;
};