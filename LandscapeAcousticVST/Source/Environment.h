#pragma once

/**
 * @brief Atmospheric conditions for sound propagation calculations
 * 
 * Used in ISO 9613-2 atmospheric absorption calculations.
 * Temperature and humidity significantly affect high-frequency attenuation.
 */
struct AtmosphericConditions {
    double temperature_C = 15.0;        // Temperature (°C)
    double relativeHumidity = 70.0;     // Relative humidity (%)
    double pressure_kPa = 101.325;      // Atmospheric pressure (kPa)
    double windSpeed_ms = 0.0;          // Wind speed (m/s) - for future use
    
    /**
     * @brief Validate atmospheric conditions are within reasonable ranges
     */
    bool isValid() const {
        return temperature_C >= -40.0 && temperature_C <= 50.0 &&
               relativeHumidity >= 0.0 && relativeHumidity <= 100.0 &&
               pressure_kPa >= 80.0 && pressure_kPa <= 120.0;
    }
};

/**
 * @brief Ground surface types for acoustic impedance modeling
 * 
 * Ground impedance affects reflection/absorption characteristics
 * according to ISO 9613-2 ground effect calculations.
 */
enum class GroundType {
    Hard,       // Concrete, water, asphalt (G ≈ 0)
    Porous,     // Grass, soil, forest floor (G ≈ 1)
    Mixed       // Mixed hard/soft surfaces (G ≈ 0.5)
};

/**
 * @brief Convert GroundType to ground factor G (0-1)
 * @param ground Ground surface type
 * @return Ground factor for ISO 9613-2 calculations
 */
inline double getGroundFactor(GroundType ground) {
    switch (ground) {
        case GroundType::Hard: return 0.0;
        case GroundType::Porous: return 1.0;
        case GroundType::Mixed: return 0.5;
        default: return 0.5;
    }
}