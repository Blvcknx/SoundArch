#include "TerrainRenderer.h"
#include "TerrainProfile.h"
#include "TerrainProfile.h"
#include "Utils.h"
#include <algorithm>
#include <cmath>

// GDAL includes (conditionally included)
#ifdef GDAL_FOUND
#include <gdal_priv.h>
#include <cpl_conv.h>
#endif

// DEMData Implementation
DEMData::DEMData() : width(0), height(0), noDataValue(0.0), hasNoDataValue(false) {
    std::fill(geoTransform, geoTransform + 6, 0.0);
}

bool DEMData::isValid() const {
    return width > 0 && height > 0;
}

juce::Rectangle<double> DEMData::getBounds() const {
    if (!isValid()) return {};

    // Calculate bounds from geotransform
    double minX = geoTransform[0];
    double maxX = geoTransform[0] + width * geoTransform[1];
    double maxY = geoTransform[3];
    double minY = geoTransform[3] + height * geoTransform[5];

    return juce::Rectangle<double>(minX, minY, maxX - minX, maxY - minY);
}

// TerrainTile Implementation
TerrainTile::TerrainTile(int lvl, int tx, int ty, int tileSize)
    : level(lvl), x(tx), y(ty), size(tileSize), loaded(false), lastAccessed(juce::Time::getCurrentTime()) {
}

juce::String TerrainTile::getKey() const {
    return juce::String::formatted("%d_%d_%d", level, x, y);
}

bool TerrainTile::containsPixel(int px, int py) const {
    return px >= x * size && px < (x + 1) * size &&
           py >= y * size && py < (y + 1) * size;
}

juce::Rectangle<int> TerrainTile::getPixelBounds() const {
    return juce::Rectangle<int>(x * size, y * size, size, size);
}

// TerrainRenderer Implementation
TerrainRenderer::TerrainRenderer()
    : maxCacheSizeBytes(512 * 1024 * 1024), // 512 MB default
      tileSize(256), gdalDataset(nullptr), gdalBand(nullptr),
      hillshadeDataset(nullptr), hillshadeBand(nullptr) {
}

void TerrainRenderer::setDEMData(const DEMData& data) {
    demData = data;
    clearCache();
}

void TerrainRenderer::setGDALDataset(void* dataset, void* band) {
    gdalDataset = dataset;
    gdalBand = band;
}

void TerrainRenderer::setHillshadeDataset(void* dataset, void* band) {
    hillshadeDataset = dataset;
    hillshadeBand = band;
}

void TerrainRenderer::render(juce::Graphics& g, const juce::Rectangle<int>& bounds,
                           double geoXMin, double geoXMax, double geoYMin, double geoYMax) {
    if (!demData.isValid()) return;

    // Calculate pixels per geo unit for LOD determination
    double geoWidth = geoXMax - geoXMin;
    double geoHeight = geoYMax - geoYMin;
    double pixelsPerGeoX = bounds.getWidth() / geoWidth;
    double pixelsPerGeoY = bounds.getHeight() / geoHeight;
    double pixelsPerGeoUnit = std::min(pixelsPerGeoX, pixelsPerGeoY);

    int lodLevel = getLODLevel(pixelsPerGeoUnit);

    // Calculate visible pixel bounds
    auto [minPx, minPy] = geoToPixel(geoXMin, geoYMax); // Top-left (note Y flip)
    auto [maxPx, maxPy] = geoToPixel(geoXMax, geoYMin); // Bottom-right

    int startTileX = minPx / tileSize;
    int startTileY = minPy / tileSize;
    int endTileX = (maxPx + tileSize - 1) / tileSize;
    int endTileY = (maxPy + tileSize - 1) / tileSize;

    // Render tiles
    for (int ty = startTileY; ty <= endTileY; ++ty) {
        for (int tx = startTileX; tx <= endTileX; ++tx) {
            TerrainTile* tile = getTile(lodLevel, tx, ty);
            if (!tile || !tile->loaded) continue;

            // Calculate screen bounds for this tile
            double tileGeoMinX = pixelToGeo(tx * tileSize, ty * tileSize).first;
            double tileGeoMaxX = pixelToGeo((tx + 1) * tileSize, (ty + 1) * tileSize).first;
            double tileGeoMinY = pixelToGeo(tx * tileSize, (ty + 1) * tileSize).second;
            double tileGeoMaxY = pixelToGeo((tx + 1) * tileSize, ty * tileSize).second;

            // Convert to screen coordinates
            int screenMinX = bounds.getX() + static_cast<int>((tileGeoMinX - geoXMin) / geoWidth * bounds.getWidth());
            int screenMaxX = bounds.getX() + static_cast<int>((tileGeoMaxX - geoXMin) / geoWidth * bounds.getWidth());
            int screenMinY = bounds.getY() + static_cast<int>((geoYMax - tileGeoMaxY) / geoHeight * bounds.getHeight());
            int screenMaxY = bounds.getY() + static_cast<int>((geoYMax - tileGeoMinY) / geoHeight * bounds.getHeight());

            juce::Rectangle<int> screenBounds(screenMinX, screenMinY, screenMaxX - screenMinX, screenMaxY - screenMinY);

            // Render tile pixels
            int tilePixelSize = tileSize >> lodLevel; // Divide by 2^level
            for (int py = 0; py < tilePixelSize; ++py) {
                for (int px = 0; px < tilePixelSize; ++px) {
                    int tileIdx = py * tilePixelSize + px;
                    if (tileIdx >= tile->hillshade.size()) continue;

                    uint8_t shade = tile->hillshade[tileIdx];
                    float normalizedShade = shade / 255.0f;

                    // Enhanced hillshade coloring
                    juce::Colour hillshadeColor = juce::Colour::fromHSV(
                        0.08f + normalizedShade * 0.15f,
                        0.4f + normalizedShade * 0.3f,
                        0.2f + normalizedShade * 0.6f,
                        1.0f);

                    // Calculate screen position
                    int screenX = screenMinX + (px * (screenMaxX - screenMinX)) / tilePixelSize;
                    int screenY = screenMinY + (py * (screenMaxY - screenMinY)) / tilePixelSize;

                    g.setColour(hillshadeColor);
                    g.fillRect(screenX, screenY, 1, 1);
                }
            }
        }
    }
}

float TerrainRenderer::getElevationAt(double lon, double lat) const {
    if (!demData.isValid()) return 0.0f;

    auto [px, py] = geoToPixel(lon, lat);
    if (px < 0 || px >= demData.width || py < 0 || py >= demData.height)
        return 0.0f;

    // Try to get from cache first
    int tileX = px / tileSize;
    int tileY = py / tileSize;
    int lodLevel = 0; // Full resolution for elevation queries

    {
        std::lock_guard<std::mutex> lock(cacheMutex);
        auto it = tileCache.find(getTileKey(lodLevel, tileX, tileY));
        if (it != tileCache.end() && it->second->loaded) {
            const TerrainTile& tile = *it->second;
            int localPx = px - tileX * tileSize;
            int localPy = py - tileY * tileSize;
            int tilePixelSize = tileSize >> lodLevel;
            if (localPx < tilePixelSize && localPy < tilePixelSize) {
                int idx = localPy * tilePixelSize + localPx;
                if (idx < tile.elevations.size()) {
                    return tile.elevations[idx];
                }
            }
        }
    }

    // Fallback: load directly from GDAL
#ifdef GDAL_FOUND
    if (gdalBand) {
        float value;
        GDALRasterBand* band = static_cast<GDALRasterBand*>(gdalBand);
        if (band->RasterIO(GF_Read, px, py, 1, 1, &value, 1, 1, GDT_Float32, 0, 0) == CE_None) {
            return value;
        }
    }
#endif

    return 0.0f;
}

TerrainProfile TerrainRenderer::sampleProfile(double lon1, double lat1, double lon2, double lat2, size_t numSamples) const {
    TerrainProfile profile;

    if (!demData.isValid() || numSamples == 0) return profile;

    // Calculate total distance
    double totalDistance = Utils::haversineDistance(lat1, lon1, lat2, lon2);

    profile.distances.reserve(numSamples);
    profile.elevations.reserve(numSamples);

    for (size_t i = 0; i < numSamples; ++i) {
        double t = static_cast<double>(i) / (numSamples - 1);
        double lon = lon1 + t * (lon2 - lon1);
        double lat = lat1 + t * (lat2 - lat1);

        profile.distances.push_back(t * totalDistance);
        profile.elevations.push_back(getElevationAt(lon, lat));
    }

    return profile;
}

void TerrainRenderer::clearCache() {
    std::lock_guard<std::mutex> lock(cacheMutex);
    tileCache.clear();
}

void TerrainRenderer::setMaxCacheSize(size_t mb) {
    maxCacheSizeBytes = mb * 1024 * 1024;
}

int TerrainRenderer::getLODLevel(double pixelsPerGeoUnit) const {
    // Determine LOD based on pixels per geo unit
    // Higher LOD = lower resolution
    if (pixelsPerGeoUnit < 0.1) return 3;      // Very zoomed out
    else if (pixelsPerGeoUnit < 1.0) return 2; // Moderately zoomed out
    else if (pixelsPerGeoUnit < 10.0) return 1; // Slightly zoomed out
    else return 0;                             // Full resolution
}

TerrainTile* TerrainRenderer::getTile(int level, int tileX, int tileY) {
    juce::String key = getTileKey(level, tileX, tileY);

    std::lock_guard<std::mutex> lock(cacheMutex);

    // Check if tile exists in cache
    auto it = tileCache.find(key);
    if (it != tileCache.end()) {
        it->second->lastAccessed = juce::Time::getCurrentTime();
        return it->second.get();
    }

    // Create new tile
    auto tile = std::make_unique<TerrainTile>(level, tileX, tileY, tileSize);
    TerrainTile* tilePtr = tile.get();

    // Try to load tile data
    if (loadTileData(*tile)) {
        tile->loaded = true;
        generateTileHillshade(*tile);
    }

    // Add to cache
    tileCache[key] = std::move(tile);

    // Evict old tiles if needed
    evictOldTiles();

    return tilePtr;
}

bool TerrainRenderer::loadTileData(TerrainTile& tile) {
#ifdef GDAL_FOUND
    if (!gdalBand) return false;

    GDALRasterBand* band = static_cast<GDALRasterBand*>(gdalBand);

    // Calculate pixel bounds for this tile at this LOD level
    int pixelSize = tileSize >> tile.level; // 256, 128, 64, 32, etc.
    int startX = tile.x * pixelSize;
    int startY = tile.y * pixelSize;

    // Check bounds
    if (startX >= demData.width || startY >= demData.height) return false;

    int readWidth = std::min(pixelSize, demData.width - startX);
    int readHeight = std::min(pixelSize, demData.height - startY);

    // Allocate tile data
    tile.elevations.resize(readWidth * readHeight);

    // Read data from GDAL
    CPLErr err = band->RasterIO(GF_Read, startX, startY, readWidth, readHeight,
                               tile.elevations.data(), readWidth, readHeight,
                               GDT_Float32, 0, 0);

    if (err != CE_None) {
        DBG("Failed to read tile data: " + juce::String(CPLGetLastErrorMsg()));
        return false;
    }

    // If we read less than full tile, pad with zeros
    if (readWidth < pixelSize || readHeight < pixelSize) {
        std::vector<float> fullData(pixelSize * pixelSize, 0.0f);
        for (int y = 0; y < readHeight; ++y) {
            for (int x = 0; x < readWidth; ++x) {
                fullData[y * pixelSize + x] = tile.elevations[y * readWidth + x];
            }
        }
        tile.elevations = std::move(fullData);
    }

    return true;
#else
    // Fallback: generate test data
    int pixelSize = tileSize >> tile.level;
    tile.elevations.resize(pixelSize * pixelSize);

    for (int y = 0; y < pixelSize; ++y) {
        for (int x = 0; x < pixelSize; ++x) {
            float elevation = 100.0f * sinf((tile.x * pixelSize + x) * 0.01f) *
                             sinf((tile.y * pixelSize + y) * 0.01f);
            tile.elevations[y * pixelSize + x] = elevation;
        }
    }

    return true;
#endif
}

void TerrainRenderer::generateTileHillshade(TerrainTile& tile) {
    int pixelSize = tileSize >> tile.level;
    if (tile.elevations.size() != static_cast<size_t>(pixelSize * pixelSize)) return;

    tile.hillshade.resize(pixelSize * pixelSize);

#ifdef GDAL_FOUND
    // If we have pre-computed hillshade data, use it
    if (hillshadeBand) {
        GDALRasterBand* band = static_cast<GDALRasterBand*>(hillshadeBand);

        // Calculate pixel bounds for this tile at this LOD level
        int startX = tile.x * pixelSize;
        int startY = tile.y * pixelSize;

        // Check bounds
        if (startX >= demData.width || startY >= demData.height) {
            std::fill(tile.hillshade.begin(), tile.hillshade.end(), 128); // Neutral gray
            return;
        }

        int readWidth = std::min(pixelSize, demData.width - startX);
        int readHeight = std::min(pixelSize, demData.height - startY);

        // Read hillshade data from GDAL
        std::vector<uint8_t> tempHillshade(readWidth * readHeight);
        CPLErr err = band->RasterIO(GF_Read, startX, startY, readWidth, readHeight,
                                   tempHillshade.data(), readWidth, readHeight,
                                   GDT_Byte, 0, 0);

        if (err == CE_None) {
            // Copy data to tile, padding if necessary
            for (int y = 0; y < readHeight; ++y) {
                for (int x = 0; x < readWidth; ++x) {
                    tile.hillshade[y * pixelSize + x] = tempHillshade[y * readWidth + x];
                }
            }
            // Fill remaining with neutral gray
            for (int y = 0; y < pixelSize; ++y) {
                for (int x = readWidth; x < pixelSize; ++x) {
                    tile.hillshade[y * pixelSize + x] = 128;
                }
            }
            for (int y = readHeight; y < pixelSize; ++y) {
                for (int x = 0; x < pixelSize; ++x) {
                    tile.hillshade[y * pixelSize + x] = 128;
                }
            }
            return;
        }
    }
#endif

    // Fallback: generate hillshade using elevation data
    // Hillshade parameters
    const float zenithRad = juce::degreesToRadians(45.0f);
    const float azimuthRad = juce::degreesToRadians(315.0f);
    const float zFactor = 1.0f;

    // Use appropriate pixel size for this LOD level
    float pixelSizeX = static_cast<float>(std::abs(demData.geoTransform[1])) * (1 << tile.level);
    float pixelSizeY = static_cast<float>(std::abs(demData.geoTransform[5])) * (1 << tile.level);

    for (int y = 1; y < pixelSize - 1; ++y) {
        for (int x = 1; x < pixelSize - 1; ++x) {
            float center = tile.elevations[y * pixelSize + x];
            float left = tile.elevations[y * pixelSize + (x - 1)];
            float right = tile.elevations[y * pixelSize + (x + 1)];
            float top = tile.elevations[(y - 1) * pixelSize + x];
            float bottom = tile.elevations[(y + 1) * pixelSize + x];

            float dzdx = (right - left) / (2.0f * pixelSizeX);
            float dzdy = (bottom - top) / (2.0f * pixelSizeY);

            dzdx *= zFactor;
            dzdy *= zFactor;

            float slope = atanf(sqrtf(dzdx * dzdx + dzdy * dzdy));
            float aspect = atan2f(dzdy, -dzdx);

            float hillshade = cosf(zenithRad) * cosf(slope) +
                            sinf(zenithRad) * sinf(slope) * cosf(azimuthRad - aspect);

            hillshade = juce::jlimit(0.0f, 1.0f, hillshade);
            tile.hillshade[y * pixelSize + x] = static_cast<uint8_t>(hillshade * 255.0f);
        }
    }

    // Fill edges
    for (int x = 0; x < pixelSize; ++x) {
        tile.hillshade[x] = tile.hillshade[pixelSize + x]; // Top
        tile.hillshade[(pixelSize - 1) * pixelSize + x] = tile.hillshade[(pixelSize - 2) * pixelSize + x]; // Bottom
    }
    for (int y = 0; y < pixelSize; ++y) {
        tile.hillshade[y * pixelSize] = tile.hillshade[y * pixelSize + 1]; // Left
        tile.hillshade[y * pixelSize + (pixelSize - 1)] = tile.hillshade[y * pixelSize + (pixelSize - 2)]; // Right
    }
}

std::pair<int, int> TerrainRenderer::geoToPixel(double lon, double lat) const {
    double pixelX = (lon - demData.geoTransform[0]) / demData.geoTransform[1];
    double pixelY = (lat - demData.geoTransform[3]) / demData.geoTransform[5];
    return {static_cast<int>(pixelX), static_cast<int>(pixelY)};
}

std::pair<double, double> TerrainRenderer::pixelToGeo(int px, int py) const {
    double lon = demData.geoTransform[0] + px * demData.geoTransform[1];
    double lat = demData.geoTransform[3] + py * demData.geoTransform[5];
    return {lon, lat};
}

void TerrainRenderer::evictOldTiles() {
    if (tileCache.size() * tileSize * tileSize * sizeof(float) < maxCacheSizeBytes) return;

    // Sort tiles by last access time
    std::vector<std::pair<juce::Time, juce::String>> tilesByAge;
    for (const auto& pair : tileCache) {
        tilesByAge.emplace_back(pair.second->lastAccessed, pair.first);
    }

    std::sort(tilesByAge.begin(), tilesByAge.end());

    // Remove oldest tiles until we're under the limit
    size_t targetSize = maxCacheSizeBytes * 3 / 4; // Target 75% of max
    for (const auto& tile : tilesByAge) {
        if (tileCache.size() * tileSize * tileSize * sizeof(float) < targetSize) break;
        tileCache.erase(tile.second);
    }
}

juce::String TerrainRenderer::getTileKey(int level, int x, int y) {
    return juce::String::formatted("%d_%d_%d", level, x, y);
}