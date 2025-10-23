#include "TerrainProfile.h"
#include <algorithm>
#include <cmath>

float TerrainProfile::getElevationAtDistance(double distance) const {
    if (distances.empty() || distance < 0.0) {
        return 0.0f;
    }
    
    // Find surrounding points
    auto it = std::lower_bound(distances.begin(), distances.end(), distance);
    
    if (it == distances.begin()) {
        return elevations[0];
    }
    if (it == distances.end()) {
        return elevations.back();
    }
    
    // Linear interpolation
    size_t idx = std::distance(distances.begin(), it);
    size_t idx1 = idx - 1;
    size_t idx2 = idx;
    
    double d1 = distances[idx1];
    double d2 = distances[idx2];
    float e1 = elevations[idx1];
    float e2 = elevations[idx2];
    
    double t = (distance - d1) / (d2 - d1);
    return static_cast<float>(e1 + t * (e2 - e1));
}

std::pair<size_t, double> TerrainProfile::findHighestObstruction(
    double sourceHeight, 
    double receiverHeight) const 
{
    if (distances.size() < 3) {
        return {0, 0.0}; // No obstruction possible
    }
    
    double sourceElevation = elevations[0] + sourceHeight;
    double receiverElevation = elevations.back() + receiverHeight;
    
    size_t maxObstructionIdx = 0;
    double maxObstructionHeight = 0.0;
    
    // Check each intermediate point
    for (size_t i = 1; i < distances.size() - 1; ++i) {
        // Calculate line-of-sight height at this distance
        double t = distances[i] / totalDistance;
        double losHeight = sourceElevation + t * (receiverElevation - sourceElevation);
        
        // Check if terrain exceeds LOS
        double terrainHeight = elevations[i];
        if (terrainHeight > losHeight) {
            double obstructionHeight = terrainHeight - losHeight;
            if (obstructionHeight > maxObstructionHeight) {
                maxObstructionHeight = obstructionHeight;
                maxObstructionIdx = i;
            }
        }
    }
    
    return {maxObstructionIdx, maxObstructionHeight};
}