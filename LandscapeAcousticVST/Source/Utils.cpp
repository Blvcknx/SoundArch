#include "Utils.h"
#include <cmath>

namespace Utils {

double haversineDistance(double lat1, double lon1, double lat2, double lon2) {
    constexpr double EARTH_RADIUS_M = 6371000.0; // Earth radius in meters
    
    double dLat = degreesToRadians(lat2 - lat1);
    double dLon = degreesToRadians(lon2 - lon1);
    
    lat1 = degreesToRadians(lat1);
    lat2 = degreesToRadians(lat2);
    
    double a = std::sin(dLat/2) * std::sin(dLat/2) +
               std::cos(lat1) * std::cos(lat2) *
               std::sin(dLon/2) * std::sin(dLon/2);
    
    double c = 2.0 * std::atan2(std::sqrt(a), std::sqrt(1-a));
    
    return EARTH_RADIUS_M * c;
}

std::vector<double> generateHannWindow(size_t length) {
    std::vector<double> window(length);
    
    for (size_t i = 0; i < length; ++i) {
        double t = static_cast<double>(i) / (length - 1);
        window[i] = 0.5 * (1.0 - std::cos(2.0 * M_PI * t));
    }
    
    return window;
}

} // namespace Utils