#include <gtest/gtest.h>
#include "../Source/FresnelDiffraction.h"
#include <cmath>

class FresnelDiffractionTest : public ::testing::Test {
protected:
    static constexpr double TOLERANCE = 0.01;
};

TEST_F(FresnelDiffractionTest, FresnelParameterCalculation) {
    // Test Fresnel parameter calculation with known values
    
    double d1 = 150.0;  // 150m to barrier
    double d2 = 150.0;  // 150m from barrier to receiver
    double h = 5.0;     // 5m barrier height above LOS
    double freq = 1000.0; // 1 kHz
    
    double nu = FresnelDiffraction::calculateFresnelParameter(d1, d2, h, freq);
    
    // For these values: nu = (2/λ) * √[(d1*d2)/(d1+d2)] * h
    // λ = 343/1000 = 0.343 m
    // nu = (2/0.343) * √[(150*150)/(150+150)] * 5
    // nu = 5.831 * √[75] * 5 = 5.831 * 8.66 * 5 ≈ 252.5
    
    EXPECT_GT(nu, 200.0);  // Should be a large positive value for blocked LOS
    EXPECT_LT(nu, 300.0);
}

TEST_F(FresnelDiffractionTest, FresnelParameterFrequencyDependence) {
    // Fresnel parameter should be inversely proportional to wavelength (proportional to frequency)
    
    double d1 = 100.0, d2 = 100.0, h = 2.0;
    
    double nu_500Hz = FresnelDiffraction::calculateFresnelParameter(d1, d2, h, 500.0);
    double nu_1000Hz = FresnelDiffraction::calculateFresnelParameter(d1, d2, h, 1000.0);
    double nu_2000Hz = FresnelDiffraction::calculateFresnelParameter(d1, d2, h, 2000.0);
    
    // Higher frequency should give higher Fresnel parameter
    EXPECT_LT(nu_500Hz, nu_1000Hz);
    EXPECT_LT(nu_1000Hz, nu_2000Hz);
    
    // Should be approximately linear with frequency
    EXPECT_NEAR(nu_1000Hz / nu_500Hz, 2.0, 0.1);
    EXPECT_NEAR(nu_2000Hz / nu_1000Hz, 2.0, 0.1);
}

TEST_F(FresnelDiffractionTest, FresnelParameterGeometryDependence) {
    double freq = 1000.0;
    double h = 3.0;
    
    // Varying distance should affect Fresnel parameter
    double nu_near = FresnelDiffraction::calculateFresnelParameter(50.0, 50.0, h, freq);
    double nu_far = FresnelDiffraction::calculateFresnelParameter(200.0, 200.0, h, freq);
    
    // For same barrier height but different distances, near field should have larger nu
    EXPECT_GT(nu_near, nu_far);
    
    // Varying barrier height
    double nu_low = FresnelDiffraction::calculateFresnelParameter(100.0, 100.0, 1.0, freq);
    double nu_high = FresnelDiffraction::calculateFresnelParameter(100.0, 100.0, 5.0, freq);
    
    // Higher barrier should give larger Fresnel parameter
    EXPECT_LT(nu_low, nu_high);
    EXPECT_NEAR(nu_high / nu_low, 5.0, 0.1);  // Linear with height
}

TEST_F(FresnelDiffractionTest, DiffractionAttenuationFormula) {
    // Test ISO 9613-2 diffraction formula: A = 10 + 13*nu for nu ≥ -0.2
    
    // Clear line of sight (nu < -0.2)
    EXPECT_DOUBLE_EQ(FresnelDiffraction::calculateDiffraction(-0.3), 0.0);
    EXPECT_DOUBLE_EQ(FresnelDiffraction::calculateDiffraction(-0.5), 0.0);
    
    // Grazing incidence (nu = 0)
    EXPECT_DOUBLE_EQ(FresnelDiffraction::calculateDiffraction(0.0), 10.0);
    
    // Blocked line of sight
    EXPECT_DOUBLE_EQ(FresnelDiffraction::calculateDiffraction(1.0), 23.0);  // 10 + 13*1
    EXPECT_DOUBLE_EQ(FresnelDiffraction::calculateDiffraction(2.0), 36.0);  // 10 + 13*2
    
    // Boundary case
    EXPECT_DOUBLE_EQ(FresnelDiffraction::calculateDiffraction(-0.2), 7.4);  // 10 + 13*(-0.2)
}

TEST_F(FresnelDiffractionTest, LineOfSightBlocked) {
    // Test LOS blocking detection
    
    EXPECT_FALSE(FresnelDiffraction::isLineOfSightBlocked(-1.0));
    EXPECT_FALSE(FresnelDiffraction::isLineOfSightBlocked(-0.1));
    EXPECT_FALSE(FresnelDiffraction::isLineOfSightBlocked(0.0));
    
    EXPECT_TRUE(FresnelDiffraction::isLineOfSightBlocked(0.1));
    EXPECT_TRUE(FresnelDiffraction::isLineOfSightBlocked(1.0));
    EXPECT_TRUE(FresnelDiffraction::isLineOfSightBlocked(5.0));
}

TEST_F(FresnelDiffractionTest, EdgeCaseHandling) {
    // Test handling of edge cases and invalid inputs
    
    // Zero frequency should return 0 (avoid division by zero)
    double nu_zero_freq = FresnelDiffraction::calculateFresnelParameter(100.0, 100.0, 2.0, 0.0);
    EXPECT_DOUBLE_EQ(nu_zero_freq, 0.0);
    
    // Negative frequency should return 0
    double nu_neg_freq = FresnelDiffraction::calculateFresnelParameter(100.0, 100.0, 2.0, -100.0);
    EXPECT_DOUBLE_EQ(nu_neg_freq, 0.0);
    
    // Zero distances should return 0
    double nu_zero_d1 = FresnelDiffraction::calculateFresnelParameter(0.0, 100.0, 2.0, 1000.0);
    EXPECT_DOUBLE_EQ(nu_zero_d1, 0.0);
    
    double nu_zero_d2 = FresnelDiffraction::calculateFresnelParameter(100.0, 0.0, 2.0, 1000.0);
    EXPECT_DOUBLE_EQ(nu_zero_d2, 0.0);
    
    // Negative distances should return 0
    double nu_neg_d1 = FresnelDiffraction::calculateFresnelParameter(-100.0, 100.0, 2.0, 1000.0);
    EXPECT_DOUBLE_EQ(nu_neg_d1, 0.0);
}

TEST_F(FresnelDiffractionTest, PhysicallyReasonableResults) {
    // Test that results are physically reasonable for realistic scenarios
    
    struct Scenario {
        double d1, d2, h, freq;
        double expected_min_atten, expected_max_atten;
        const char* description;
    };
    
    std::vector<Scenario> scenarios = {
        {100.0, 100.0, 1.0, 1000.0, 5.0, 25.0, "Small hill"},
        {300.0, 300.0, 10.0, 1000.0, 20.0, 50.0, "Large building"},
        {50.0, 50.0, 5.0, 500.0, 10.0, 30.0, "Close obstacle"},
        {1000.0, 1000.0, 2.0, 2000.0, 5.0, 20.0, "Distant small obstacle"}
    };
    
    for (const auto& scenario : scenarios) {
        double nu = FresnelDiffraction::calculateFresnelParameter(
            scenario.d1, scenario.d2, scenario.h, scenario.freq);
        double attenuation = FresnelDiffraction::calculateDiffraction(nu);
        
        EXPECT_GE(attenuation, scenario.expected_min_atten) 
            << "Scenario: " << scenario.description;
        EXPECT_LE(attenuation, scenario.expected_max_atten) 
            << "Scenario: " << scenario.description;
    }
}

TEST_F(FresnelDiffractionTest, ReciprocalGeometry) {
    // Fresnel parameter should be symmetric with respect to d1 and d2
    // (swapping source and receiver shouldn't change the result)
    
    double freq = 1000.0;
    double h = 3.0;
    
    double nu1 = FresnelDiffraction::calculateFresnelParameter(100.0, 200.0, h, freq);
    double nu2 = FresnelDiffraction::calculateFresnelParameter(200.0, 100.0, h, freq);
    
    EXPECT_NEAR(nu1, nu2, TOLERANCE);
    
    double atten1 = FresnelDiffraction::calculateDiffraction(nu1);
    double atten2 = FresnelDiffraction::calculateDiffraction(nu2);
    
    EXPECT_NEAR(atten1, atten2, TOLERANCE);
}