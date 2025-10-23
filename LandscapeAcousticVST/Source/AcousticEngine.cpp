#include "AcousticEngine.h"
#include "Utils.h"
#include <algorithm>
#include <cmath>

double AcousticEngine::calculateTotalAttenuation(
    const TerrainProfile& profile,
    double sourceHeight_m,
    double receiverHeight_m,
    double frequency_Hz,
    const AtmosphericConditions& atmo,
    GroundType ground) const
{
    if (!profile.isValid() || frequency_Hz <= 0.0) {
        return 0.0;
    }
    
    double distance = profile.totalDistance;
    
    // 1. Divergence (geometric spreading)
    double A_div = computeDivergence(distance);
    
    // 2. Atmospheric absorption
    double A_atm = computeAtmosphericAbsorption(distance, frequency_Hz, atmo);
    
    // 3. Ground effect
    double A_gr = computeGroundEffect(distance, sourceHeight_m, receiverHeight_m, 
                                      frequency_Hz, ground);
    
    // 4. Barrier diffraction
    double A_bar = computeBarrierDiffraction(profile, sourceHeight_m, 
                                             receiverHeight_m, frequency_Hz);
    
    // Total attenuation (ISO 9613-2)
    double A_total = A_div + A_atm + A_gr + A_bar;
    
    return A_total;
}

std::vector<double> AcousticEngine::calculateFrequencyResponse(
    const TerrainProfile& profile,
    double sourceHeight_m,
    double receiverHeight_m,
    const std::vector<double>& frequencies,
    const AtmosphericConditions& atmo,
    GroundType ground) const
{
    std::vector<double> attenuations;
    attenuations.reserve(frequencies.size());
    
    for (double freq : frequencies) {
        double atten = calculateTotalAttenuation(
            profile, sourceHeight_m, receiverHeight_m, freq, atmo, ground);
        attenuations.push_back(atten);
    }
    
    return attenuations;
}

std::vector<double> AcousticEngine::getOctaveBands() {
    return {63.0, 125.0, 250.0, 500.0, 1000.0, 2000.0, 4000.0, 8000.0};
}

double AcousticEngine::computeDivergence(double distance_m) const {
    const double d0 = 1.0; // Reference distance (1 meter)
    double d = std::max(distance_m, d0); // Avoid log(0)
    
    // ISO 9613-2 divergence formula for hemispherical spreading
    return 20.0 * std::log10(d / d0) + 11.0;
}

double AcousticEngine::computeAtmosphericAbsorption(
    double distance_m,
    double frequency_Hz,
    const AtmosphericConditions& atmo) const
{
    // Get absorption coefficient from lookup table
    double alpha = lookupAtmosphericAlpha(frequency_Hz, atmo.temperature_C, 
                                          atmo.relativeHumidity);
    
    // Atmospheric attenuation = α * distance
    return alpha * distance_m;
}

double AcousticEngine::lookupAtmosphericAlpha(
    double frequency_Hz,
    double temperature_C,
    double relativeHumidity) const
{
    // Simplified ISO 9613-1 atmospheric absorption model
    // Full implementation would use detailed lookup tables
    
    double alpha = 0.0;
    
    // Base absorption (molecular relaxation effects)
    if (frequency_Hz < 500.0) {
        alpha = 0.0005 * (frequency_Hz / 500.0); // Low freq: minimal absorption
    } else if (frequency_Hz < 2000.0) {
        alpha = 0.0005 + 0.004 * ((frequency_Hz - 500.0) / 1500.0); // Mid freq
    } else {
        alpha = 0.0045 + 0.015 * ((frequency_Hz - 2000.0) / 6000.0); // High freq
    }
    
    // Temperature correction (higher temp = more absorption at mid-freq)
    double tempFactor = 1.0 + 0.01 * (temperature_C - 20.0);
    alpha *= tempFactor;
    
    // Humidity correction (affects molecular relaxation peaks)
    double humidityFactor = 1.0 + 0.003 * (70.0 - relativeHumidity) / 70.0;
    alpha *= humidityFactor;
    
    // Clamp to reasonable range
    return Utils::clamp(alpha, 0.0001, 0.1); // 0.0001 - 0.1 dB/m
}

double AcousticEngine::computeGroundEffect(
    double distance_m,
    double sourceHeight_m,
    double receiverHeight_m,
    double frequency_Hz,
    GroundType ground) const
{
    // Ground factor (0 = hard, 1 = porous)
    double G = getGroundFactor(ground);
    
    // Effective height for ground reflection
    double h_eff = calculateEffectiveHeight(sourceHeight_m, receiverHeight_m);
    
    // Frequency at which ground effect is maximum
    const double SPEED_OF_SOUND = 343.0; // m/s
    double f_peak = SPEED_OF_SOUND / (4.0 * h_eff);
    
    // Ground effect decreases as frequency moves away from peak
    double freqRatio = frequency_Hz / f_peak;
    double freqFactor = 1.0 / (1.0 + std::pow(freqRatio - 1.0, 2));
    
    // Maximum ground attenuation (simplified ISO 9613-2)
    double maxAttenuation = G * 8.0; // Up to 8 dB for porous ground
    
    return maxAttenuation * freqFactor;
}

double AcousticEngine::calculateEffectiveHeight(
    double sourceHeight_m,
    double receiverHeight_m) const
{
    // Geometric mean of source and receiver heights
    return std::sqrt(sourceHeight_m * receiverHeight_m);
}

double AcousticEngine::computeBarrierDiffraction(
    const TerrainProfile& profile,
    double sourceHeight_m,
    double receiverHeight_m,
    double frequency_Hz) const
{
    if (checkLineOfSight(profile, sourceHeight_m, receiverHeight_m)) {
        return 0.0; // No diffraction for clear LOS
    }
    
    // Find highest obstruction
    auto [barrierIdx, barrierHeight] = findHighestObstruction(
        profile, sourceHeight_m, receiverHeight_m);
    
    if (barrierHeight <= 0.0) {
        return 0.0; // No significant obstruction
    }
    
    // Calculate Fresnel parameter
    double d1 = profile.distances[barrierIdx];
    double d2 = profile.totalDistance - d1;
    
    double nu = FresnelDiffraction::calculateFresnelParameter(
        d1, d2, barrierHeight, frequency_Hz);
    
    // Calculate diffraction attenuation
    return FresnelDiffraction::calculateDiffraction(nu);
}

bool AcousticEngine::checkLineOfSight(
    const TerrainProfile& profile,
    double sourceHeight_m,
    double receiverHeight_m) const
{
    if (profile.elevations.size() < 3) {
        return true; // Too few points to obstruct
    }
    
    double sourceElevation = profile.elevations[0] + sourceHeight_m;
    double receiverElevation = profile.elevations.back() + receiverHeight_m;
    
    // Check each intermediate point against LOS line
    for (size_t i = 1; i < profile.elevations.size() - 1; ++i) {
        double t = profile.distances[i] / profile.totalDistance;
        double losHeight = Utils::lerp(sourceElevation, receiverElevation, t);
        
        if (profile.elevations[i] > losHeight) {
            return false; // LOS blocked
        }
    }
    
    return true; // Clear LOS
}

std::pair<size_t, double> AcousticEngine::findHighestObstruction(
    const TerrainProfile& profile,
    double sourceHeight_m,
    double receiverHeight_m) const
{
    return profile.findHighestObstruction(sourceHeight_m, receiverHeight_m);
}