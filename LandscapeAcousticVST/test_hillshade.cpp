#include "TerrainLoader.h"
#include <iostream>
#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>

// Initialize JUCE application
class TestApplication : public juce::JUCEApplicationBase {
public:
    TestApplication() {}
    ~TestApplication() override {}

    const juce::String getApplicationName() override { return "HillshadeTest"; }
    const juce::String getApplicationVersion() override { return "1.0"; }
    bool moreThanOneInstanceAllowed() override { return false; }

    void initialise(const juce::String&) override {
        // Initialize JUCE message manager
        juce::MessageManager::getInstance();

        runTests();
        quit();
    }

    void shutdown() override {
        juce::MessageManager::deleteInstance();
    }

    void systemRequestedQuit() override { quit(); }
    void anotherInstanceStarted(const juce::String&) override {}

    void suspended() override {}
    void resumed() override {}
    void unhandledException(const std::exception*, const juce::String&, int) override {}

private:
    void runTests() {
        std::cout << "Running hillshade tests..." << std::endl;

        TerrainLoader loader;
        juce::String errorMsg;

        // Try to load the test terrain
        juce::File testFile("D:\\SoundArch\\LandscapeAcousticVST\\Resources\\test_data\\test_terrain_converted.tif");

        std::cout << "Loading DEM: " << testFile.getFullPathName() << std::endl;

        if (loader.loadDEM(testFile, errorMsg)) {
            std::cout << "DEM loaded successfully!" << std::endl;

            // Check DEM data
            const auto& demData = loader.getDEMData();
            std::cout << "DEM dimensions: " << demData.width << "x" << demData.height << std::endl;
            std::cout << "DEM filename: " << demData.filename << std::endl;

            // Check if hillshade was generated
            juce::File hillshadeFile(testFile.getParentDirectory().getChildFile(
                testFile.getFileNameWithoutExtension() + "_hillshade.tif"));

            if (hillshadeFile.existsAsFile()) {
                std::cout << "Hillshade file generated: " << hillshadeFile.getFullPathName() << std::endl;
            } else {
                std::cout << "Hillshade file not found" << std::endl;
            }

            // Test elevation sampling
            float elevation = loader.getElevationAt(0.0, 0.0);
            std::cout << "Elevation at (0,0): " << elevation << std::endl;

            // Test terrain profile
            TerrainProfile profile = loader.sampleProfile(-1.0, -1.0, 1.0, 1.0, 5);
            if (profile.isValid()) {
                std::cout << "Profile sampled successfully with " << profile.getNumSamples() << " points" << std::endl;
            } else {
                std::cout << "Profile sampling failed" << std::endl;
            }

        } else {
            std::cout << "Failed to load DEM: " << errorMsg << std::endl;
        }
    }
};

// Main entry point
START_JUCE_APPLICATION(TestApplication)