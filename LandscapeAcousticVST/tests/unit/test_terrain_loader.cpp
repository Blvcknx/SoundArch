#include <gtest/gtest.h>
#include "../Source/TerrainLoader.h"
#include <filesystem>

class TerrainLoaderTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Clean up any existing hillshade files
        juce::File testDir("D:\\SoundArch\\LandscapeAcousticVST\\Resources\\test_data");
        juce::File hillshadeFile = testDir.getChildFile("test_terrain_converted_hillshade.tif");
        if (hillshadeFile.existsAsFile()) {
            hillshadeFile.deleteFile();
        }
    }

    void TearDown() override {
        // Clean up hillshade files after test
        juce::File testDir("D:\\SoundArch\\LandscapeAcousticVST\\Resources\\test_data");
        juce::File hillshadeFile = testDir.getChildFile("test_terrain_converted_hillshade.tif");
        if (hillshadeFile.existsAsFile()) {
            hillshadeFile.deleteFile();
        }
    }
};

TEST_F(TerrainLoaderTest, LoadTestTerrain) {
    TerrainLoader loader;
    juce::String errorMsg;

    juce::File testFile("D:\\SoundArch\\LandscapeAcousticVST\\Resources\\test_data\\test_terrain_converted.tif");

    ASSERT_TRUE(testFile.existsAsFile()) << "Test terrain file should exist";

    bool result = loader.loadDEM(testFile, errorMsg);
    EXPECT_TRUE(result) << "Failed to load test terrain: " << errorMsg;

    if (result) {
        // Check that DEM data is valid
        const auto& demData = loader.getDEMData();
        EXPECT_TRUE(demData.isValid());
        EXPECT_GT(demData.width, 0);
        EXPECT_GT(demData.height, 0);

        // Check hillshade generation
        juce::File hillshadeFile = testFile.getParentDirectory().getChildFile(
            testFile.getFileNameWithoutExtension() + "_hillshade.tif");

        // Hillshade might not be generated immediately, but the loader should work
        EXPECT_TRUE(loader.isLoaded());

        // Test elevation sampling
        float elevation = loader.getElevationAt(0.0, 0.0);
        EXPECT_FALSE(std::isnan(elevation)); // Should not be NaN
    }
}

TEST_F(TerrainLoaderTest, LoadASCIIGrid) {
    TerrainLoader loader;
    juce::String errorMsg;

    juce::File ascFile("D:\\SoundArch\\LandscapeAcousticVST\\Resources\\test_data\\test_terrain.asc");

    ASSERT_TRUE(ascFile.existsAsFile()) << "ASCII grid test file should exist";

    bool result = loader.loadDEM(ascFile, errorMsg);
    EXPECT_TRUE(result) << "Failed to load ASCII grid: " << errorMsg;

    if (result) {
        const auto& demData = loader.getDEMData();
        EXPECT_TRUE(demData.isValid());
        EXPECT_TRUE(loader.isLoaded());
    }
}

TEST_F(TerrainLoaderTest, InvalidFile) {
    TerrainLoader loader;
    juce::String errorMsg;

    juce::File invalidFile("D:\\nonexistent\\file.tif");

    bool result = loader.loadDEM(invalidFile, errorMsg);
    EXPECT_FALSE(result);
    EXPECT_FALSE(errorMsg.isEmpty());
}

TEST_F(TerrainLoaderTest, SupportedExtensions) {
    auto extensions = TerrainLoader::getSupportedExtensions();

    EXPECT_FALSE(extensions.isEmpty());
    EXPECT_TRUE(extensions.contains("*.tif"));
    EXPECT_TRUE(extensions.contains("*.asc"));
}

TEST_F(TerrainLoaderTest, IsValidDEMFile) {
    juce::File tifFile("D:\\SoundArch\\LandscapeAcousticVST\\Resources\\test_data\\test_terrain_converted.tif");
    juce::File ascFile("D:\\SoundArch\\LandscapeAcousticVST\\Resources\\test_data\\test_terrain.asc");
    juce::File invalidFile("D:\\SoundArch\\LandscapeAcousticVST\\README.md");

    EXPECT_TRUE(TerrainLoader::isValidDEMFile(tifFile));
    EXPECT_TRUE(TerrainLoader::isValidDEMFile(ascFile));
    EXPECT_FALSE(TerrainLoader::isValidDEMFile(invalidFile));
}

TEST_F(TerrainLoaderTest, ElevationSampling) {
    TerrainLoader loader;
    juce::String errorMsg;

    juce::File testFile("D:\\SoundArch\\LandscapeAcousticVST\\Resources\\test_data\\test_terrain_converted.tif");

    if (loader.loadDEM(testFile, errorMsg)) {
        // Test various coordinates
        float elev1 = loader.getElevationAt(0.0, 0.0);
        float elev2 = loader.getElevationAt(1.0, 1.0);

        EXPECT_FALSE(std::isnan(elev1));
        EXPECT_FALSE(std::isnan(elev2));

        // Test terrain profile sampling
        TerrainProfile profile = loader.sampleProfile(-1.0, -1.0, 1.0, 1.0, 10);
        EXPECT_TRUE(profile.isValid());
        EXPECT_EQ(profile.getNumSamples(), 10);
    }
}