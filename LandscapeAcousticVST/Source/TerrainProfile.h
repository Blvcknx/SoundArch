#pragma once

#include <vector>
#include <JuceHeader.h>

/**
 * @brief Structure representing a terrain elevation profile between two points
 * 
 * Contains distances and elevations sampled along a path from source to receiver,
 * used for acoustic propagation calculations according to ISO 9613-2.
 */
struct TerrainProfile {
    std::vector<double> distances;      // Distance from source (meters)
    std::vector<float> elevations;      // Elevation (meters above datum)
    double totalDistance;               // Total path length (meters)
    
    juce::Point<double> sourceCoords;   // Longitude, Latitude of source
    juce::Point<double> receiverCoords; // Longitude, Latitude of receiver
    
    /**
     * @brief Check if profile is valid
     */
    bool isValid() const {
        return distances.size() == elevations.size() && 
               distances.size() > 1 && 
               totalDistance > 0.0;
    }
    
    /**
     * @brief Get number of sample points
     */
    size_t getNumSamples() const {
        return distances.size();
    }
    
    /**
     * @brief Get elevation at specific distance (with interpolation)
     */
    float getElevationAtDistance(double distance) const;
    
    /**
     * @brief Find highest point above line-of-sight
     * @return Pair of (index, height_above_los) of highest obstruction
     */
    std::pair<size_t, double> findHighestObstruction(
        double sourceHeight, 
        double receiverHeight) const;
};