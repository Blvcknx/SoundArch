#pragma once

#include "TerrainProfile.h"
#include "TerrainRenderer.h"
#include <juce_core/juce_core.h>
// #include <gdal_priv.h>
// #include <ogr_spatialref.h>
#include <memory>
#include <vector>

// Forward declaration for GDAL types
#ifdef GDAL_FOUND
class GDALDataset;
class GDALRasterBand;
#else
// Stub for when GDAL is not available
using GDALDataset = void;
using GDALRasterBand = void;
#endif

/**
 * @brief GDAL-based terrain data loader for Digital Elevation Models
 * 
 * Supports multiple DEM formats: GeoTIFF, ASC, DTED, HGT, SRTM
 * Provides elevation sampling and profile extraction for acoustic modeling.
 * Uses virtual rendering for large files through TerrainRenderer.
 */
class TerrainLoader {
public:
    TerrainLoader();
    ~TerrainLoader();
    
    /**
     * @brief Load DEM from file
     * @param demFile Path to DEM file
     * @param errorMsg Output error message if loading fails
     * @return True if loaded successfully
     */
    bool loadDEM(const juce::File& demFile, juce::String& errorMsg);
    
    /**
     * @brief Load ASCII grid format DEM
     */
    bool loadASCIIGrid(const juce::File& ascFile, juce::String& errorMsg);
    
    /**
     * @brief Get elevation at geographic coordinates (with interpolation)
     * @param lon Longitude (degrees)
     * @param lat Latitude (degrees) 
     * @return Elevation (meters) or 0.0 if out of bounds
     */
    float getElevationAt(double lon, double lat) const;
    
    /**
     * @brief Sample elevation profile between two points
     * @param lon1 Source longitude (degrees)
     * @param lat1 Source latitude (degrees)
     * @param lon2 Receiver longitude (degrees) 
     * @param lat2 Receiver latitude (degrees)
     * @param numSamples Number of elevation samples along path
     * @return Terrain profile with distances and elevations
     */
    TerrainProfile sampleProfile(
        double lon1, double lat1,
        double lon2, double lat2, 
        size_t numSamples = 500) const;
    
    /**
     * @brief Get loaded DEM data
     */
    const DEMData& getDEMData() const { return renderer.getDEMData(); }
    
    /**
     * @brief Get terrain renderer for visualization
     */
    TerrainRenderer& getRenderer() { return renderer; }
    const TerrainRenderer& getRenderer() const { return renderer; }
    
    /**
     * @brief Check if DEM is loaded
     */
    bool isLoaded() const {
#ifdef GDAL_FOUND
        return dataset != nullptr && renderer.getDEMData().isValid();
#else
        return renderer.getDEMData().isValid();
#endif
    }
    
    /**
     * @brief Get supported file extensions
     */
    static juce::StringArray getSupportedExtensions();
    
    /**
     * @brief Validate DEM file format
     */
    static bool isValidDEMFile(const juce::File& file);
    
private:
#ifdef GDAL_FOUND
    GDALDataset* dataset;
    GDALRasterBand* band;
    GDALDataset* hillshadeDataset;
    GDALRasterBand* hillshadeBand;
#endif
    TerrainRenderer renderer;
    bool gdalInitialized;
    
    /**
     * @brief Initialize GDAL library
     */
    void initializeGDAL();
    
    /**
     * @brief Validate loaded DEM data
     */
    bool validateDEM() const;
    
    /**
     * @brief Convert geographic coordinates to pixel coordinates
     */
    std::pair<int, int> geoToPixel(double lon, double lat) const;
    
    /**
     * @brief Bilinear interpolation for elevation sampling
     */
    float interpolateElevation(double lon, double lat) const;
    
    /**
     * @brief Close current dataset
     */
    void closeDEM();
    
    /**
     * @brief Generate hillshade for the loaded DEM using GDAL
     */
    bool generateHillshade(const juce::File& demFile, juce::String& errorMsg);
    
    /**
     * @brief Load hillshade dataset
     */
    bool loadHillshade(const juce::File& hillshadeFile, juce::String& errorMsg);
};