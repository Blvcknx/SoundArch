#include "FresnelDiffraction.h"
#include <algorithm>

double FresnelDiffraction::calculateFresnelParameter(
    double d1, double d2, double h, double f)
{
    if (f <= 0.0 || d1 <= 0.0 || d2 <= 0.0) {
        return 0.0;
    }
    
    double lambda = SPEED_OF_SOUND / f;
    double nu = (2.0 / lambda) * std::sqrt((d1 * d2) / (d1 + d2)) * h;
    
    return nu;
}

double FresnelDiffraction::calculateDiffraction(double nu) {
    if (nu < -0.2) {
        return 0.0; // No diffraction loss (clear line-of-sight)
    } else {
        return 10.0 + 13.0 * nu; // ISO 9613-2 knife-edge formula
    }
}