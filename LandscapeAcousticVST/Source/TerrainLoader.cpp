#include "TerrainLoader.h"
#include "Utils.h"
#include <algorithm>
#include <memory>
#include <vector>

// GDAL includes (conditionally included)
#ifdef GDAL_FOUND
#include <gdal_priv.h>
#include <ogr_spatialref.h>
#include <cpl_conv.h>
#endif

// TerrainLoader Implementation
TerrainLoader::TerrainLoader() : gdalInitialized(false) {
    // initializeGDAL(); // Moved to loadDEM to avoid startup crashes
}

TerrainLoader::~TerrainLoader() {
    closeDEM();
}

void TerrainLoader::initializeGDAL() {
#ifdef GDAL_FOUND
    GDALAllRegister();
    CPLSetConfigOption("GDAL_DISABLE_READDIR_ON_OPEN", "YES");
    CPLSetConfigOption("GDAL_CACHEMAX", "512");
#endif
}

bool TerrainLoader::loadDEM(const juce::File& demFile, juce::String& errorMsg) {
    // Close previous dataset
    closeDEM();

    // Check file exists
    if (!demFile.existsAsFile()) {
        errorMsg = "DEM file does not exist: " + demFile.getFileName();
        return false;
    }

#ifdef GDAL_FOUND
    // Initialize GDAL if needed
    initializeGDAL();
    
    // Try to open with GDAL
    dataset = static_cast<GDALDataset*>(GDALOpen(demFile.getFullPathName().toRawUTF8(), GA_ReadOnly));
    if (dataset == nullptr) {
        errorMsg = "Failed to open DEM file with GDAL: " + CPLGetLastErrorMsg();
        return false;
    }

    // Get raster band (assume first band contains elevation)
    band = dataset->GetRasterBand(1);
    if (band == nullptr) {
        errorMsg = "No raster band found in DEM file";
        closeDEM();
        return false;
    }

    // Create DEMData structure
    DEMData demData;
    demData.width = band->GetXSize();
    demData.height = band->GetYSize();

    // Get geotransform
    if (dataset->GetGeoTransform(demData.geoTransform) != CE_None) {
        // No geotransform, create a simple one
        demData.geoTransform[0] = 0.0;
        demData.geoTransform[1] = 1.0;
        demData.geoTransform[2] = 0.0;
        demData.geoTransform[3] = demData.height;
        demData.geoTransform[4] = 0.0;
        demData.geoTransform[5] = -1.0;
    }

    // Get no-data value
    demData.hasNoDataValue = false;
    double noDataValue;
    if (band->GetNoDataValue(&noDataValue) == CE_None) {
        demData.noDataValue = static_cast<float>(noDataValue);
        demData.hasNoDataValue = true;
    }

    demData.filename = demFile.getFileName();

    // Generate hillshade using GDAL
    juce::File hillshadeFile = demFile.getParentDirectory().getChildFile(
        demFile.getFileNameWithoutExtension() + "_hillshade.tif");
    
    if (!generateHillshade(demFile, errorMsg)) {
        // Hillshade generation failed, but we can still load the DEM
        DBG("Hillshade generation failed: " + errorMsg);
    } else if (!loadHillshade(hillshadeFile, errorMsg)) {
        DBG("Hillshade loading failed: " + errorMsg);
    }

    // Set up TerrainRenderer with DEM data and GDAL pointers
    renderer.setDEMData(demData);
    renderer.setGDALDataset(dataset, band);
    if (hillshadeDataset && hillshadeBand) {
        renderer.setHillshadeDataset(hillshadeDataset, hillshadeBand);
    }

    return validateDEM();

#else
    // Fallback: Try to load ASCII grid (.asc) files directly
    if (demFile.hasFileExtension(".asc")) {
        return loadASCIIGrid(demFile, errorMsg);
    }

    // Fallback: Create test terrain when GDAL is not available and not .asc
    DEMData demData;
    demData.filename = demFile.getFileName();
    demData.width = 512;
    demData.height = 512;

    // Set up geotransform
    demData.geoTransform[0] = 0.0;
    demData.geoTransform[1] = 1.0;
    demData.geoTransform[2] = 0.0;
    demData.geoTransform[3] = 512.0;
    demData.geoTransform[4] = 0.0;
    demData.geoTransform[5] = -1.0;

    demData.noDataValue = -9999.0;
    demData.hasNoDataValue = false;

    // Set up TerrainRenderer with test data
    renderer.setDEMData(demData);
    // No GDAL dataset for test data

    return validateDEM();
#endif
}

bool TerrainLoader::loadASCIIGrid(const juce::File& ascFile, juce::String& errorMsg) {
    juce::FileInputStream inputStream(ascFile);
    if (!inputStream.openedOk()) {
        errorMsg = "Failed to open ASCII grid file";
        return false;
    }

    // Read header
    juce::String line;
    int ncols = 0, nrows = 0;
    double xllcorner = 0.0, yllcorner = 0.0, cellsize = 1.0, nodata = -9999.0;

    // Read header lines
    for (int i = 0; i < 6; ++i) {
        line = inputStream.readNextLine();
        if (line.isEmpty()) {
            errorMsg = "Invalid ASCII grid header";
            return false;
        }

        auto tokens = juce::StringArray::fromTokens(line, " \t", "");
        if (tokens.size() < 2) continue;

        juce::String key = tokens[0].toLowerCase();
        double value = tokens[1].getDoubleValue();

        if (key == "ncols") ncols = static_cast<int>(value);
        else if (key == "nrows") nrows = static_cast<int>(value);
        else if (key == "xllcorner") xllcorner = value;
        else if (key == "yllcorner") yllcorner = value;
        else if (key == "cellsize") cellsize = value;
        else if (key == "nodata_value") nodata = value;
    }

    if (ncols <= 0 || nrows <= 0) {
        errorMsg = "Invalid dimensions in ASCII grid";
        return false;
    }

    // Check size limit
    size_t totalPixels = static_cast<size_t>(ncols) * nrows;
    const size_t maxPixels = 50 * 1024 * 1024;
    if (totalPixels > maxPixels) {
        errorMsg = juce::String::formatted("ASCII grid too large: %dx%d (%d MPixels). Maximum allowed: %d MPixels",
                                         ncols, nrows, totalPixels / (1024 * 1024), maxPixels / (1024 * 1024));
        return false;
    }

    // Allocate data
    DEMData demData;
    demData.width = ncols;
    demData.height = nrows;
    demData.hasNoDataValue = true;
    demData.noDataValue = static_cast<float>(nodata);

    // Set geotransform
    demData.geoTransform[0] = xllcorner;
    demData.geoTransform[1] = cellsize;
    demData.geoTransform[2] = 0.0;
    demData.geoTransform[3] = yllcorner + nrows * cellsize;
    demData.geoTransform[4] = 0.0;
    demData.geoTransform[5] = -cellsize;

    // Read elevation data
    for (int y = 0; y < nrows; ++y) {
        line = inputStream.readNextLine();
        if (line.isEmpty()) {
            errorMsg = "Unexpected end of file in ASCII grid data";
            return false;
        }

        auto tokens = juce::StringArray::fromTokens(line, " \t", "");
        if (tokens.size() != ncols) {
            errorMsg = juce::String::formatted("Wrong number of values in row %d: expected %d, got %d",
                                             y, ncols, tokens.size());
            return false;
        }

        for (int x = 0; x < ncols; ++x) {
            double value = tokens[x].getDoubleValue();
            // For ASCII grids, we store data in TerrainRenderer, not here
        }
    }

    demData.filename = ascFile.getFileName();

    // Set up TerrainRenderer with ASCII grid data
    renderer.setDEMData(demData);
    // ASCII grids don't use GDAL, so no dataset pointer

    return validateDEM();
}

bool TerrainLoader::validateDEM() const {
    return renderer.getDEMData().isValid();
}

float TerrainLoader::getElevationAt(double lon, double lat) const {
    if (!isLoaded()) return 0.0f;
    return renderer.getElevationAt(lon, lat);
}

TerrainProfile TerrainLoader::sampleProfile(
    double lon1, double lat1,
    double lon2, double lat2,
    size_t numSamples) const {

    if (!isLoaded() || numSamples == 0) return TerrainProfile{};
    return renderer.sampleProfile(lon1, lat1, lon2, lat2, numSamples);
}

std::pair<int, int> TerrainLoader::geoToPixel(double lon, double lat) const {
    const auto& demData = renderer.getDEMData();
    // Use geotransform to convert geo coordinates to pixel coordinates
    // geotransform[0] = top-left x
    // geotransform[1] = pixel width in x
    // geotransform[2] = rotation (usually 0)
    // geotransform[3] = top-left y
    // geotransform[4] = rotation (usually 0)
    // geotransform[5] = pixel height in y (negative for north-up)

    double pixelX = (lon - demData.geoTransform[0]) / demData.geoTransform[1];
    double pixelY = (lat - demData.geoTransform[3]) / demData.geoTransform[5];

    return {static_cast<int>(pixelX), static_cast<int>(pixelY)};
}

float TerrainLoader::interpolateElevation(double lon, double lat) const {
    // Simple nearest neighbor for now
    return getElevationAt(lon, lat);
}

void TerrainLoader::closeDEM() {
#ifdef GDAL_FOUND
    if (dataset) {
        GDALClose(dataset);
        dataset = nullptr;
        band = nullptr;
    }
    if (hillshadeDataset) {
        GDALClose(hillshadeDataset);
        hillshadeDataset = nullptr;
        hillshadeBand = nullptr;
    }
#endif
    renderer.clearCache(); // Clear renderer cache
}

juce::StringArray TerrainLoader::getSupportedExtensions() {
    return juce::StringArray({ "*.tif", "*.tiff", "*.asc", "*.hgt", "*.dt0", "*.dt1", "*.dt2" });
}

bool TerrainLoader::isValidDEMFile(const juce::File& file) {
    if (!file.existsAsFile()) return false;

    juce::StringArray extensions = getSupportedExtensions();
    for (auto& ext : extensions) {
        if (file.hasFileExtension(ext.substring(1))) // Remove the *
            return true;
    }

    return false;
}

bool TerrainLoader::generateHillshade(const juce::File& demFile, juce::String& errorMsg) {
#ifdef GDAL_FOUND
    if (!dataset || !band) {
        errorMsg = "No DEM loaded to generate hillshade";
        return false;
    }

    // Generate hillshade filename
    juce::File hillshadeFile = demFile.getParentDirectory().getChildFile(
        demFile.getFileNameWithoutExtension() + "_hillshade.tif");

    // Use GDALDEMProcessing to generate hillshade
    // Parameters: hillshade algorithm, azimuth=315, altitude=45, scale=1, zfactor=1
    CPLErr err = GDALDEMProcessing(hillshadeFile.getFullPathName().toRawUTF8(),
                                   dataset,
                                   "hillshade",
                                   nullptr, // options
                                   nullptr); // progress callback

    if (err != CE_None) {
        errorMsg = "GDAL hillshade generation failed: " + juce::String(CPLGetLastErrorMsg());
        return false;
    }

    DBG("Hillshade generated successfully: " + hillshadeFile.getFullPathName());
    return true;
#else
    errorMsg = "GDAL not available for hillshade generation";
    return false;
#endif
}

bool TerrainLoader::loadHillshade(const juce::File& hillshadeFile, juce::String& errorMsg) {
#ifdef GDAL_FOUND
    if (!hillshadeFile.existsAsFile()) {
        errorMsg = "Hillshade file does not exist: " + hillshadeFile.getFileName();
        return false;
    }

    // Close previous hillshade dataset
    if (hillshadeDataset) {
        GDALClose(hillshadeDataset);
        hillshadeDataset = nullptr;
        hillshadeBand = nullptr;
    }

    // Open hillshade file
    hillshadeDataset = static_cast<GDALDataset*>(GDALOpen(hillshadeFile.getFullPathName().toRawUTF8(), GA_ReadOnly));
    if (hillshadeDataset == nullptr) {
        errorMsg = "Failed to open hillshade file: " + CPLGetLastErrorMsg();
        return false;
    }

    // Get hillshade band
    hillshadeBand = hillshadeDataset->GetRasterBand(1);
    if (hillshadeBand == nullptr) {
        errorMsg = "No raster band found in hillshade file";
        GDALClose(hillshadeDataset);
        hillshadeDataset = nullptr;
        return false;
    }

    DBG("Hillshade loaded successfully: " + hillshadeFile.getFullPathName());
    return true;
#else
    errorMsg = "GDAL not available for hillshade loading";
    return false;
#endif
}