#pragma once

#include "TerrainProfile.h"
#include <JuceHeader.h>
#include <gdal_priv.h>
#include <ogr_spatialref.h>
#include <memory>
#include <vector>

/**
 * @brief Data structure containing loaded DEM information
 */
struct DEMData {
    std::vector<float> elevations;      // Elevation values (row-major order)
    double geoTransform[6];             // GDAL geotransform coefficients
    int width, height;                  // Raster dimensions
    std::unique_ptr<OGRSpatialReference> srs; // Spatial reference system
    juce::String filename;              // Source filename
    double noDataValue;                 // No-data value
    bool hasNoDataValue;                // Whether no-data value is defined
    
    DEMData();
    ~DEMData() = default;
    
    // Move-only semantics
    DEMData(const DEMData&) = delete;
    DEMData& operator=(const DEMData&) = delete;
    DEMData(DEMData&&) = default;
    DEMData& operator=(DEMData&&) = default;
    
    /**
     * @brief Check if DEM data is valid
     */
    bool isValid() const;
    
    /**
     * @brief Get geographic bounds of DEM
     */
    juce::Rectangle<double> getBounds() const;
};

/**
 * @brief GDAL-based terrain data loader for Digital Elevation Models
 * 
 * Supports multiple DEM formats: GeoTIFF, ASC, DTED, HGT, SRTM
 * Provides elevation sampling and profile extraction for acoustic modeling.
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
    const DEMData& getDEMData() const { return demData; }
    
    /**
     * @brief Check if DEM is loaded
     */
    bool isLoaded() const { return dataset != nullptr && demData.isValid(); }
    
    /**
     * @brief Get supported file extensions
     */
    static juce::StringArray getSupportedExtensions();
    
    /**
     * @brief Validate DEM file format
     */
    static bool isValidDEMFile(const juce::File& file);
    
private:
    GDALDataset* dataset;
    DEMData demData;
    
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
};