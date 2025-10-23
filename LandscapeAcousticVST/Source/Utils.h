#pragma once

#include <cmath>
#include <vector>

/**
 * @brief Mathematical and conversion utilities for acoustic calculations
 */
namespace Utils {
    
    /**
     * @brief Convert decibels to linear amplitude
     */
    inline double dbToLinear(double db) {
        return std::pow(10.0, db / 20.0);
    }
    
    /**
     * @brief Convert linear amplitude to decibels
     */
    inline double linearToDb(double linear) {
        return 20.0 * std::log10(std::abs(linear));
    }
    
    /**
     * @brief Calculate great circle distance between two geographic points
     * @param lat1 Latitude of point 1 (degrees)
     * @param lon1 Longitude of point 1 (degrees) 
     * @param lat2 Latitude of point 2 (degrees)
     * @param lon2 Longitude of point 2 (degrees)
     * @return Distance in meters
     */
    double haversineDistance(double lat1, double lon1, double lat2, double lon2);
    
    /**
     * @brief Linear interpolation between two values
     */
    template<typename T>
    T lerp(T a, T b, double t) {
        return a + t * (b - a);
    }
    
    /**
     * @brief Clamp value between min and max
     */
    template<typename T>
    T clamp(T value, T min_val, T max_val) {
        return std::max(min_val, std::min(value, max_val));
    }
    
    /**
     * @brief Generate Hann window for signal processing
     */
    std::vector<double> generateHannWindow(size_t length);
    
    /**
     * @brief Convert degrees to radians
     */
    inline double degreesToRadians(double degrees) {
        return degrees * M_PI / 180.0;
    }
    
    /**
     * @brief Convert radians to degrees  
     */
    inline double radiansToDegrees(double radians) {
        return radians * 180.0 / M_PI;
    }
    
    /**
     * @brief Check if value is approximately zero
     */
    inline bool isApproximatelyZero(double value, double epsilon = 1e-9) {
        return std::abs(value) < epsilon;
    }
    
    /**
     * @brief Safe division (returns 0 if denominator is zero)
     */
    inline double safeDivide(double numerator, double denominator, double fallback = 0.0) {
        return isApproximatelyZero(denominator) ? fallback : numerator / denominator;
    }
    
} // namespace Utils