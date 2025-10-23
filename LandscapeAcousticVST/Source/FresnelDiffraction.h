#pragma once

#include <cmath>

/**
 * @brief Fresnel diffraction calculations for knife-edge terrain obstructions
 * 
 * Implements ISO 9613-2 barrier diffraction calculations using Fresnel theory.
 * Used when terrain blocks line-of-sight between sound source and receiver.
 */
class FresnelDiffraction {
public:
    /**
     * @brief Calculate Fresnel parameter for given geometry and frequency
     * 
     * @param d1_m Distance from source to barrier (meters)
     * @param d2_m Distance from barrier to receiver (meters)  
     * @param h_barrier_m Height of barrier above line-of-sight (meters)
     * @param frequency_Hz Sound frequency (Hz)
     * @return Fresnel parameter ν (dimensionless)
     */
    static double calculateFresnelParameter(
        double d1_m,
        double d2_m,
        double h_barrier_m,
        double frequency_Hz
    );
    
    /**
     * @brief Calculate diffraction attenuation from Fresnel parameter
     * 
     * Uses ISO 9613-2 knife-edge formula:
     * A_bar = 10 + 13*ν (dB) for ν ≥ -0.2
     * A_bar = 0 (dB) for ν < -0.2 (clear line-of-sight)
     * 
     * @param nu Fresnel parameter
     * @return Attenuation due to diffraction (dB)
     */
    static double calculateDiffraction(double nu);
    
    /**
     * @brief Check if line-of-sight is blocked (ν > 0)
     * @param nu Fresnel parameter
     * @return True if LOS is blocked
     */
    static bool isLineOfSightBlocked(double nu) {
        return nu > 0.0;
    }
    
private:
    static constexpr double SPEED_OF_SOUND = 343.0; // m/s at 20°C
};