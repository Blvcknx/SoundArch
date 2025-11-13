#pragma once

#include "TerrainProfile.h"
#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <memory>

/**
 * @brief Structure containing DEM metadata (no full data)
 */
struct DEMData {
    int width, height;
    double geoTransform[6];
    juce::String filename;
    double noDataValue;
    bool hasNoDataValue;

    DEMData();
    ~DEMData() = default;

    // Copyable
    DEMData(const DEMData&) = default;
    DEMData& operator=(const DEMData&) = default;
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
 * @brief Tile-based terrain data for large file handling
 */
struct TerrainTile {
    int level;      // LOD level (0 = full resolution)
    int x, y;       // Tile coordinates
    int size;       // Tile size in pixels
    std::vector<float> elevations;
    std::vector<uint8_t> hillshade;
    bool loaded;
    juce::Time lastAccessed;

    TerrainTile(int lvl, int tx, int ty, int tileSize);

    juce::String getKey() const;
    bool containsPixel(int px, int py) const;
    juce::Rectangle<int> getPixelBounds() const;
};

/**
 * @brief Virtual terrain renderer for large DEM files
 */
class TerrainRenderer {
public:
    TerrainRenderer();
    ~TerrainRenderer() = default;

    /**
     * @brief Set the DEM data source
     */
    void setDEMData(const DEMData& data);

    /**
     * @brief Set GDAL dataset for data access
     */
    void setGDALDataset(void* dataset, void* band);

    /**
     * @brief Set GDAL hillshade dataset for hillshade access
     */
    void setHillshadeDataset(void* dataset, void* band);

    /**
     * @brief Render terrain to a graphics context
     */
    void render(juce::Graphics& g, const juce::Rectangle<int>& bounds,
                double geoXMin, double geoXMax, double geoYMin, double geoYMax);

    /**
     * @brief Get elevation at geographic coordinates
     */
    float getElevationAt(double lon, double lat) const;

    /**
     * @brief Sample elevation profile between points
     */
    TerrainProfile sampleProfile(double lon1, double lat1, double lon2, double lat2, size_t numSamples) const;

    /**
     * @brief Clear cache and free memory
     */
    void clearCache();

    /**
     * @brief Set maximum cache size in MB
     */
    void setMaxCacheSize(size_t mb);

    /**
     * @brief Get the DEM data
     */
    const DEMData& getDEMData() const { return demData; }

private:
    DEMData demData;
    std::unordered_map<juce::String, std::unique_ptr<TerrainTile>> tileCache;
    mutable std::mutex cacheMutex;
    size_t maxCacheSizeBytes;
    int tileSize;
    void* gdalDataset;
    void* gdalBand;
    void* hillshadeDataset;
    void* hillshadeBand;

    /**
     * @brief Get appropriate LOD level for current zoom
     */
    int getLODLevel(double pixelsPerGeoUnit) const;

    /**
     * @brief Load or get tile for given coordinates and LOD
     */
    TerrainTile* getTile(int level, int tileX, int tileY);

    /**
     * @brief Load tile data from source
     */
    bool loadTileData(TerrainTile& tile);

    /**
     * @brief Generate hillshade for tile
     */
    void generateTileHillshade(TerrainTile& tile);

    /**
     * @brief Convert geo coordinates to pixel coordinates
     */
    std::pair<int, int> geoToPixel(double lon, double lat) const;

    /**
     * @brief Convert pixel coordinates to geo coordinates
     */
    std::pair<double, double> pixelToGeo(int px, int py) const;

    /**
     * @brief Evict old tiles if cache is too full
     */
    void evictOldTiles();

    /**
     * @brief Get tile key
     */
    static juce::String getTileKey(int level, int x, int y);
};