#include "TerrainLoader.h"
#include "Utils.h"
#include <algorithm>

// DEMData Implementation
DEMData::DEMData() : width(0), height(0), noDataValue(0.0), hasNoDataValue(false) {
    std::fill(geoTransform, geoTransform + 6, 0.0);
}

bool DEMData::isValid() const {
    return width > 0 && height > 0 && !elevations.empty() && 
           elevations.size() == static_cast<size_t>(width * height);
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

// TerrainLoader Implementation
TerrainLoader::TerrainLoader() : dataset(nullptr) {
    initializeGDAL();
}

TerrainLoader::~TerrainLoader() {
    closeDEM();
}

void TerrainLoader::initializeGDAL() {
    static bool initialized = false;
    if (!initialized) {
        GDALAllRegister(); // Register all GDAL drivers
        initialized = true;
    }
}

bool TerrainLoader::loadDEM(const juce::File& demFile, juce::String& errorMsg) {
    // Close previous dataset
    closeDEM();
    
    // Check file exists
    if (!demFile.existsAsFile()) {
        errorMsg = "DEM file does not exist: " + demFile.getFileName();
        return false;
    }
    
    // Open DEM file
    dataset = static_cast<GDALDataset*>(
        GDALOpen(demFile.getFullPathName().toRawUTF8(), GA_ReadOnly));
    
    if (dataset == nullptr) {
        errorMsg = "Failed to open DEM file: " + demFile.getFileName() + 
                  " (GDAL error: " + juce::String(CPLGetLastErrorMsg()) + ")";
        return false;
    }
    
    // Validate raster properties
    if (dataset->GetRasterCount() < 1) {
        errorMsg = "DEM file has no raster bands";
        closeDEM();
        return false;
    }
    
    GDALRasterBand* band = dataset->GetRasterBand(1);
    if (band == nullptr) {
        errorMsg = "Cannot access raster band 1";
        closeDEM();
        return false;
    }
    
    // Store metadata
    demData.width = dataset->GetRasterXSize();
    demData.height = dataset->GetRasterYSize();
    
    if (demData.width <= 0 || demData.height <= 0) {
        errorMsg = "Invalid raster dimensions";
        closeDEM();
        return false;
    }
    
    // Get geotransform
    CPLErr err = dataset->GetGeoTransform(demData.geoTransform);
    if (err != CE_None) {
        errorMsg = "DEM file has no geotransform information";
        closeDEM();
        return false;
    }
    
    // Get spatial reference
    const char* proj = dataset->GetProjectionRef();
    if (proj && strlen(proj) > 0) {
        demData.srs = std::make_unique<OGRSpatialReference>(proj);
    }
    
    // Get no-data value
    int hasNoData = 0;
    demData.noDataValue = band->GetNoDataValue(&hasNoData);
    demData.hasNoDataValue = (hasNoData != 0);
    
    // Read elevation data
    size_t totalPixels = static_cast<size_t>(demData.width) * demData.height;
    demData.elevations.resize(totalPixels);
    
    err = band->RasterIO(
        GF_Read, 0, 0, demData.width, demData.height,
        demData.elevations.data(), demData.width, demData.height,
        GDT_Float32, 0, 0);
    
    if (err != CE_None) {
        errorMsg = "Failed to read elevation data: " + juce::String(CPLGetLastErrorMsg());
        closeDEM();
        return false;
    }
    
    demData.filename = demFile.getFileName();
    
    // Validate loaded data
    if (!validateDEM()) {
        errorMsg = "DEM validation failed";
        closeDEM();
        return false;
    }
    
    return true;
}

bool TerrainLoader::validateDEM() const {
    if (!demData.isValid()) {
        return false;
    }
    
    // Check geotransform validity
    if (Utils::isApproximatelyZero(demData.geoTransform[1]) || 
        Utils::isApproximatelyZero(demData.geoTransform[5])) {
        return false; // Invalid pixel size
    }
    
    return true;
}

std::pair<int, int> TerrainLoader::geoToPixel(double lon, double lat) const {
    const double* gt = demData.geoTransform;
    
    // Apply inverse geotransform
    double det = gt[1] * gt[5] - gt[2] * gt[4];
    if (Utils::isApproximatelyZero(det)) {
        return {-1, -1}; // Invalid transform
    }
    
    double temp = lon - gt[0];
    double temp2 = lat - gt[3];
    
    int px = static_cast<int>((gt[5] * temp - gt[2] * temp2) / det);
    int py = static_cast<int>((gt[1] * temp2 - gt[4] * temp) / det);
    
    return {px, py};
}

float TerrainLoader::getElevationAt(double lon, double lat) const {
    if (!isLoaded()) {
        return 0.0f;
    }
    
    return interpolateElevation(lon, lat);
}

float TerrainLoader::interpolateElevation(double lon, double lat) const {
    auto [px, py] = geoToPixel(lon, lat);
    
    // Bounds check
    if (px < 0 || px >= demData.width - 1 || py < 0 || py >= demData.height - 1) {
        return 0.0f;
    }
    
    // Get fractional part for interpolation
    const double* gt = demData.geoTransform;
    double exactX = (lon - gt[0]) / gt[1];
    double exactY = (lat - gt[3]) / gt[5];
    
    double fracX = exactX - std::floor(exactX);
    double fracY = exactY - std::floor(exactY);
    
    // Bilinear interpolation
    int idx00 = py * demData.width + px;
    int idx01 = py * demData.width + (px + 1);
    int idx10 = (py + 1) * demData.width + px;
    int idx11 = (py + 1) * demData.width + (px + 1);
    
    float z00 = demData.elevations[idx00];
    float z01 = demData.elevations[idx01];
    float z10 = demData.elevations[idx10];
    float z11 = demData.elevations[idx11];
    
    // Handle no-data values
    if (demData.hasNoDataValue) {
        if (z00 == demData.noDataValue || z01 == demData.noDataValue ||
            z10 == demData.noDataValue || z11 == demData.noDataValue) {
            return 0.0f; // Return 0 for no-data areas
        }
    }
    
    // Bilinear interpolation
    float z0 = Utils::lerp(z00, z01, fracX);
    float z1 = Utils::lerp(z10, z11, fracX);
    return Utils::lerp(z0, z1, fracY);
}

TerrainProfile TerrainLoader::sampleProfile(
    double lon1, double lat1,
    double lon2, double lat2,
    size_t numSamples) const 
{
    TerrainProfile profile;
    profile.sourceCoords = {lon1, lat1};
    profile.receiverCoords = {lon2, lat2};
    
    if (!isLoaded() || numSamples < 2) {
        return profile;
    }
    
    // Calculate total distance using Haversine formula
    profile.totalDistance = Utils::haversineDistance(lat1, lon1, lat2, lon2);
    
    // Sample along path
    profile.distances.reserve(numSamples);
    profile.elevations.reserve(numSamples);
    
    for (size_t i = 0; i < numSamples; ++i) {
        double t = static_cast<double>(i) / (numSamples - 1);
        
        // Linear interpolation in geographic space (great circle approximation)
        double lon = Utils::lerp(lon1, lon2, t);
        double lat = Utils::lerp(lat1, lat2, t);
        
        float elevation = getElevationAt(lon, lat);
        
        profile.distances.push_back(t * profile.totalDistance);
        profile.elevations.push_back(elevation);
    }
    
    return profile;
}

void TerrainLoader::closeDEM() {
    if (dataset != nullptr) {
        GDALClose(dataset);
        dataset = nullptr;
    }
    
    demData = DEMData(); // Reset to empty state
}

juce::StringArray TerrainLoader::getSupportedExtensions() {
    return juce::StringArray{
        ".tif", ".tiff",    // GeoTIFF
        ".asc",             // ESRI ASCII Grid  
        ".dted", ".dt0", ".dt1", ".dt2", // DTED
        ".hgt",             // SRTM HGT
        ".img",             // ERDAS Imagine
        ".dem"              // USGS DEM
    };
}

bool TerrainLoader::isValidDEMFile(const juce::File& file) {
    if (!file.existsAsFile()) {
        return false;
    }
    
    juce::String extension = file.getFileExtension().toLowerCase();
    return getSupportedExtensions().contains(extension);
}