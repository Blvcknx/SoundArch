#include <gtest/gtest.h>
#include "../Source/TerrainProfile.h"
#include <vector>

class TerrainProfileTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a valid test profile
        profile.distances = {0.0, 25.0, 50.0, 75.0, 100.0};
        profile.elevations = {100.0f, 102.0f, 98.0f, 101.0f, 99.0f};
        profile.totalDistance = 100.0;
        profile.sourceCoords = {-105.5, 36.0};
        profile.receiverCoords = {-105.4, 36.1};
    }
    
    TerrainProfile profile;
};

TEST_F(TerrainProfileTest, IsValid) {
    EXPECT_TRUE(profile.isValid());
    
    // Test invalid cases
    TerrainProfile invalid1;
    EXPECT_FALSE(invalid1.isValid());  // Empty profile
    
    TerrainProfile invalid2;
    invalid2.distances = {0.0, 50.0, 100.0};
    invalid2.elevations = {100.0f, 101.0f};  // Mismatched sizes
    invalid2.totalDistance = 100.0;
    EXPECT_FALSE(invalid2.isValid());
    
    TerrainProfile invalid3;
    invalid3.distances = {0.0};
    invalid3.elevations = {100.0f};  // Only one point
    invalid3.totalDistance = 0.0;
    EXPECT_FALSE(invalid3.isValid());
}

TEST_F(TerrainProfileTest, GetNumSamples) {
    EXPECT_EQ(profile.getNumSamples(), 5);
    
    TerrainProfile empty;
    EXPECT_EQ(empty.getNumSamples(), 0);
}

TEST_F(TerrainProfileTest, ElevationInterpolation) {
    // Test interpolation at exact sample points
    EXPECT_FLOAT_EQ(profile.getElevationAtDistance(0.0), 100.0f);
    EXPECT_FLOAT_EQ(profile.getElevationAtDistance(25.0), 102.0f);
    EXPECT_FLOAT_EQ(profile.getElevationAtDistance(100.0), 99.0f);
    
    // Test interpolation between points
    float elev_12_5 = profile.getElevationAtDistance(12.5);  // Halfway between 0 and 25
    EXPECT_FLOAT_EQ(elev_12_5, 101.0f);  // (100 + 102) / 2
    
    float elev_37_5 = profile.getElevationAtDistance(37.5);  // Halfway between 25 and 50  
    EXPECT_FLOAT_EQ(elev_37_5, 100.0f);  // (102 + 98) / 2
    
    // Test out of bounds
    EXPECT_FLOAT_EQ(profile.getElevationAtDistance(-10.0), 100.0f);  // Before start
    EXPECT_FLOAT_EQ(profile.getElevationAtDistance(150.0), 99.0f);   // After end
}

TEST_F(TerrainProfileTest, FindHighestObstruction) {
    // Create profile with clear obstruction
    TerrainProfile obstructed_profile;
    obstructed_profile.distances = {0.0, 25.0, 50.0, 75.0, 100.0};
    obstructed_profile.elevations = {100.0f, 101.0f, 110.0f, 101.0f, 100.0f};  // Peak at 50m
    obstructed_profile.totalDistance = 100.0;
    
    double sourceHeight = 2.0;
    double receiverHeight = 2.0;
    
    auto [idx, height] = obstructed_profile.findHighestObstruction(sourceHeight, receiverHeight);
    
    EXPECT_EQ(idx, 2);  // Peak is at index 2 (50m mark)
    EXPECT_GT(height, 0.0);  // Should be a positive obstruction height
    
    // Test clear line of sight
    auto [clear_idx, clear_height] = profile.findHighestObstruction(sourceHeight, receiverHeight);
    EXPECT_EQ(clear_height, 0.0);  // No significant obstruction
}

TEST_F(TerrainProfileTest, LineOfSightCalculation) {
    // Create profile with known obstruction
    TerrainProfile los_test;
    los_test.distances = {0.0, 50.0, 100.0};
    los_test.elevations = {100.0f, 120.0f, 100.0f};  // 20m high barrier in middle
    los_test.totalDistance = 100.0;
    
    double sourceHeight = 2.0;
    double receiverHeight = 2.0;
    
    // Line of sight from (100+2=102m) to (100+2=102m) should be blocked by 120m barrier
    auto [idx, obstruction] = los_test.findHighestObstruction(sourceHeight, receiverHeight);
    EXPECT_GT(obstruction, 15.0);  // Should be significantly obstructed
    
    // Test with higher source/receiver
    auto [idx_high, obstruction_high] = los_test.findHighestObstruction(15.0, 15.0);
    EXPECT_LT(obstruction_high, obstruction);  // Less obstruction with higher points
}

TEST_F(TerrainProfileTest, EdgeCases) {
    // Test with only two points (minimum valid profile)
    TerrainProfile minimal;
    minimal.distances = {0.0, 100.0};
    minimal.elevations = {100.0f, 105.0f};
    minimal.totalDistance = 100.0;
    
    EXPECT_TRUE(minimal.isValid());
    EXPECT_EQ(minimal.getNumSamples(), 2);
    
    // Interpolation should work
    EXPECT_FLOAT_EQ(minimal.getElevationAtDistance(50.0), 102.5f);
    
    // No obstruction with only two points
    auto [idx, height] = minimal.findHighestObstruction(2.0, 2.0);
    EXPECT_EQ(height, 0.0);
}

TEST_F(TerrainProfileTest, MonotonicDistance) {
    // Distances should be monotonically increasing
    for (size_t i = 1; i < profile.distances.size(); ++i) {
        EXPECT_GT(profile.distances[i], profile.distances[i-1]);
    }
    
    // First distance should be 0
    EXPECT_DOUBLE_EQ(profile.distances[0], 0.0);
    
    // Last distance should equal total distance
    EXPECT_DOUBLE_EQ(profile.distances.back(), profile.totalDistance);
}

TEST_F(TerrainProfileTest, CoordinateConsistency) {
    // Source and receiver coordinates should be reasonable geographic coordinates
    EXPECT_GE(profile.sourceCoords.x, -180.0);
    EXPECT_LE(profile.sourceCoords.x, 180.0);
    EXPECT_GE(profile.sourceCoords.y, -90.0);
    EXPECT_LE(profile.sourceCoords.y, 90.0);
    
    EXPECT_GE(profile.receiverCoords.x, -180.0);
    EXPECT_LE(profile.receiverCoords.x, 180.0);
    EXPECT_GE(profile.receiverCoords.y, -90.0);
    EXPECT_LE(profile.receiverCoords.y, 90.0);
    
    // Source and receiver should be different points
    EXPECT_FALSE(profile.sourceCoords.x == profile.receiverCoords.x && 
                 profile.sourceCoords.y == profile.receiverCoords.y);
}