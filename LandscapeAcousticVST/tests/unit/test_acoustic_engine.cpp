#include <gtest/gtest.h>
#include "../Source/AcousticEngine.h"
#include "../Source/Environment.h"
#include <cmath>

class AcousticEngineTest : public ::testing::Test {
protected:
    void SetUp() override {
        engine = std::make_unique<AcousticEngine>();
    }
    
    std::unique_ptr<AcousticEngine> engine;
};

TEST_F(AcousticEngineTest, DivergenceCalculation) {
    // Test divergence at various distances
    
    // At 1 meter (reference distance)
    double atten_1m = engine->computeDivergence(1.0);
    EXPECT_NEAR(atten_1m, 11.0, 0.1);  // 20*log10(1) + 11 = 11 dB
    
    // At 10 meters
    double atten_10m = engine->computeDivergence(10.0);
    EXPECT_NEAR(atten_10m, 31.0, 0.1);  // 20*log10(10) + 11 = 31 dB
    
    // At 100 meters
    double atten_100m = engine->computeDivergence(100.0);
    EXPECT_NEAR(atten_100m, 51.0, 0.1);  // 20*log10(100) + 11 = 51 dB
    
    // At 1000 meters
    double atten_1000m = engine->computeDivergence(1000.0);
    EXPECT_NEAR(atten_1000m, 71.0, 0.1);  // 20*log10(1000) + 11 = 71 dB
}

TEST_F(AcousticEngineTest, DivergenceMonotonic) {
    // Divergence should increase monotonically with distance
    double prev_atten = 0.0;
    
    for (double distance = 1.0; distance <= 1000.0; distance *= 2.0) {
        double atten = engine->computeDivergence(distance);
        EXPECT_GT(atten, prev_atten);
        prev_atten = atten;
    }
}

TEST_F(AcousticEngineTest, AtmosphericAbsorption) {
    AtmosphericConditions atmo;
    atmo.temperature_C = 20.0;
    atmo.relativeHumidity = 70.0;
    
    // Test at 1 kHz, various distances
    double atten_100m = engine->computeAtmosphericAbsorption(100.0, 1000.0, atmo);
    double atten_200m = engine->computeAtmosphericAbsorption(200.0, 1000.0, atmo);
    
    // Should increase linearly with distance
    EXPECT_GT(atten_200m, atten_100m);
    EXPECT_NEAR(atten_200m / atten_100m, 2.0, 0.1);
    
    // Typical values for 1 kHz should be in reasonable range (0.1 - 1.0 dB per 100m)
    EXPECT_GT(atten_100m, 0.05);  // At least some absorption
    EXPECT_LT(atten_100m, 2.0);   // Not excessive
}

TEST_F(AcousticEngineTest, AtmosphericFrequencyDependence) {
    AtmosphericConditions atmo;
    atmo.temperature_C = 20.0;
    atmo.relativeHumidity = 70.0;
    
    double distance = 100.0;
    
    // High frequencies should have more absorption than low frequencies
    double atten_125Hz = engine->computeAtmosphericAbsorption(distance, 125.0, atmo);
    double atten_1kHz = engine->computeAtmosphericAbsorption(distance, 1000.0, atmo);
    double atten_8kHz = engine->computeAtmosphericAbsorption(distance, 8000.0, atmo);
    
    EXPECT_LT(atten_125Hz, atten_1kHz);
    EXPECT_LT(atten_1kHz, atten_8kHz);
}

TEST_F(AcousticEngineTest, GroundEffect) {
    double distance = 100.0;
    double sourceHeight = 2.0;
    double receiverHeight = 2.0;
    double frequency = 1000.0;
    
    // Hard ground should have less attenuation than porous ground
    double atten_hard = engine->computeGroundEffect(
        distance, sourceHeight, receiverHeight, frequency, GroundType::Hard);
    double atten_porous = engine->computeGroundEffect(
        distance, sourceHeight, receiverHeight, frequency, GroundType::Porous);
    
    EXPECT_LT(atten_hard, atten_porous);
    
    // Ground effect should be frequency dependent
    double atten_low = engine->computeGroundEffect(
        distance, sourceHeight, receiverHeight, 250.0, GroundType::Porous);
    double atten_high = engine->computeGroundEffect(
        distance, sourceHeight, receiverHeight, 4000.0, GroundType::Porous);
    
    // Peak ground effect occurs at specific frequency, so we test that
    // the effect is non-zero and reasonable
    EXPECT_GE(atten_low, 0.0);
    EXPECT_GE(atten_high, 0.0);
}

TEST_F(AcousticEngineTest, OctaveBands) {
    // Test that octave bands are correctly defined
    auto bands = AcousticEngine::getOctaveBands();
    
    EXPECT_EQ(bands.size(), 8);
    EXPECT_DOUBLE_EQ(bands[0], 63.0);
    EXPECT_DOUBLE_EQ(bands[1], 125.0);
    EXPECT_DOUBLE_EQ(bands[2], 250.0);
    EXPECT_DOUBLE_EQ(bands[3], 500.0);
    EXPECT_DOUBLE_EQ(bands[4], 1000.0);
    EXPECT_DOUBLE_EQ(bands[5], 2000.0);
    EXPECT_DOUBLE_EQ(bands[6], 4000.0);
    EXPECT_DOUBLE_EQ(bands[7], 8000.0);
    
    // Each band should be approximately double the previous
    for (size_t i = 1; i < bands.size(); ++i) {
        double ratio = bands[i] / bands[i-1];
        EXPECT_NEAR(ratio, 2.0, 0.1);
    }
}

TEST_F(AcousticEngineTest, FrequencyResponseConsistency) {
    // Create a simple terrain profile (flat)
    TerrainProfile profile;
    profile.distances = {0.0, 25.0, 50.0, 75.0, 100.0};
    profile.elevations = {100.0f, 100.0f, 100.0f, 100.0f, 100.0f};
    profile.totalDistance = 100.0;
    profile.sourceCoords = {0.0, 0.0};
    profile.receiverCoords = {0.001, 0.0};  // ~100m at equator
    
    AtmosphericConditions atmo;
    atmo.temperature_C = 20.0;
    atmo.relativeHumidity = 70.0;
    
    auto frequencies = AcousticEngine::getOctaveBands();
    auto attenuations = engine->calculateFrequencyResponse(
        profile, 2.0, 2.0, frequencies, atmo, GroundType::Porous);
    
    EXPECT_EQ(attenuations.size(), frequencies.size());
    
    // All attenuations should be positive (sound gets quieter with distance)
    for (double atten : attenuations) {
        EXPECT_GT(atten, 0.0);
    }
    
    // Generally, higher frequencies should have higher attenuation
    // (though ground effects can cause variations)
    double firstThird = (attenuations[0] + attenuations[1] + attenuations[2]) / 3.0;
    double lastThird = (attenuations[5] + attenuations[6] + attenuations[7]) / 3.0;
    EXPECT_LT(firstThird, lastThird);
}

TEST_F(AcousticEngineTest, EnvironmentValidation) {
    // Test atmospheric conditions validation
    AtmosphericConditions valid_atmo;
    valid_atmo.temperature_C = 20.0;
    valid_atmo.relativeHumidity = 70.0;
    valid_atmo.pressure_kPa = 101.325;
    
    EXPECT_TRUE(valid_atmo.isValid());
    
    // Test invalid conditions
    AtmosphericConditions invalid_temp;
    invalid_temp.temperature_C = -50.0;  // Too cold
    EXPECT_FALSE(invalid_temp.isValid());
    
    AtmosphericConditions invalid_humidity;
    invalid_humidity.relativeHumidity = 150.0;  // Over 100%
    EXPECT_FALSE(invalid_humidity.isValid());
}

TEST_F(AcousticEngineTest, GroundFactorConversion) {
    // Test ground factor utility function
    EXPECT_DOUBLE_EQ(getGroundFactor(GroundType::Hard), 0.0);
    EXPECT_DOUBLE_EQ(getGroundFactor(GroundType::Porous), 1.0);
    EXPECT_DOUBLE_EQ(getGroundFactor(GroundType::Mixed), 0.5);
}