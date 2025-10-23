# 🗺️ QGIS LandscapeAcoustic Plugin Usage Guide

## Installation

### Step 1: Copy Plugin Files
```bash
# Copy the plugin to QGIS plugins directory
# Windows: %APPDATA%\QGIS\QGIS3\profiles\default\python\plugins\
# macOS: ~/Library/Application Support/QGIS/QGIS3/profiles/default/python/plugins/
# Linux: ~/.local/share/QGIS/QGIS3/profiles/default/python/plugins/

cp -r scripts/qgis_plugin/landscape_acoustic/ [QGIS_PLUGINS_PATH]/
```

### Step 2: Enable Plugin in QGIS
1. Open QGIS
2. Go to **Plugins** → **Manage and Install Plugins**
3. Click **Installed** tab
4. Find **LandscapeAcoustic** and check the box to enable it
5. You'll see a new **LandscapeAcoustic** menu in the menu bar

## Using the QGIS Plugin

### Step 1: Load Your DEM Data
1. **Load DEM file** in QGIS:
   - **Layer** → **Add Layer** → **Add Raster Layer**
   - Select your DEM file (.tif, .asc, etc.)
   - The terrain will appear in the map canvas

### Step 2: Export for VST Plugin
1. **Select the DEM layer** in the Layers panel
2. Click **LandscapeAcoustic** → **Export for VST** in the menu
3. The **LandscapeAcoustic Export Dialog** will open

### Step 3: Set Analysis Points
In the export dialog:

#### **Source Point** (where sound originates):
1. Click **"Set Source Point"** button
2. Click anywhere on the map canvas
3. A red marker will appear at the source location
4. Coordinates will be displayed in the dialog

#### **Receiver Point** (where sound is heard):
1. Click **"Set Receiver Point"** button  
2. Click anywhere on the map canvas
3. A blue marker will appear at the receiver location
4. Coordinates will be displayed in the dialog

### Step 4: Configure Export Settings

#### **Crop Settings**:
- **Auto Crop**: Automatically crops DEM around source/receiver points
- **Buffer Distance**: How much extra terrain to include (meters)
- **Max Resolution**: Limit output size for performance

#### **Analysis Parameters**:
- **Source Height**: Height of sound source above terrain (meters)
- **Receiver Height**: Height of listener above terrain (meters)
- **Sample Points**: Number of points along the path for analysis

#### **Output Settings**:
- **Export Directory**: Where to save the processed files
- **Filename Prefix**: Prefix for output files

### Step 5: Export Data
1. Click **"Export"** button
2. The plugin will:
   - Crop the DEM around your points
   - Extract terrain profile along the path
   - Save configuration file for the VST plugin
   - Save cropped DEM file
   - Display export summary

### Step 6: Load in VST Plugin
1. Open your DAW and load LandscapeAcousticVST
2. Click **"Import QGIS Config"** in the VST interface
3. Select the exported configuration file
4. The terrain and analysis points will load automatically

## Export File Formats

### Configuration File (.json)
Contains all analysis parameters:
```json
{
  "dem_file": "path/to/cropped_dem.tif",
  "source": {"lon": -122.4194, "lat": 37.7749, "height": 2.0},
  "receiver": {"lon": -122.4094, "lat": 37.7849, "height": 2.0},
  "atmospheric": {"temperature": 15.0, "humidity": 70.0},
  "ground_type": "Mixed",
  "export_info": {
    "timestamp": "2025-10-24T10:30:00",
    "qgis_version": "3.34.0",
    "original_crs": "EPSG:4326"
  }
}
```

### Cropped DEM File (.tif)
- GeoTIFF format for maximum compatibility
- Includes only relevant terrain area
- Maintains georeferencing information
- Optimized size for real-time processing

## Advanced Features

### **Multiple Analysis Points**
- Export multiple source/receiver combinations
- Compare different scenarios
- Batch processing for large studies

### **Coordinate System Handling**
- Automatically handles CRS transformations
- Converts between geographic and projected coordinates
- Maintains accuracy across different projections

### **Quality Control**
- Validates DEM data quality
- Checks for missing data (NODATA values)
- Warns about resolution/size limitations
- Provides terrain statistics

## Tips for Best Results

### **DEM Selection**
- **Higher resolution** = more accurate acoustics (10-30m ideal)
- **Appropriate extent** = include source, receiver, and path
- **Recent data** = reflects current terrain conditions

### **Point Placement**
- **Source**: Place at actual sound emission location
- **Receiver**: Place at listener position
- **Line of sight**: Consider terrain blocking between points
- **Scale**: Use real-world coordinates when possible

### **Export Settings**
- **Buffer**: Include 500-1000m buffer around points
- **Resolution**: Balance accuracy vs. performance
- **File size**: Smaller files load faster in VST

### **Validation**
- **Preview**: Check terrain profile looks reasonable
- **Distance**: Verify source-receiver distance makes sense
- **Elevation**: Check heights above terrain are realistic

## Troubleshooting

### **Plugin Won't Load**
- Check QGIS Python console for error messages
- Verify all plugin files are in correct directory
- Ensure QGIS version compatibility (3.22+)

### **Can't Set Points**
- Make sure a DEM layer is selected
- Check that layer is visible and has valid data
- Verify coordinate system is properly defined

### **Export Fails**
- Check write permissions in export directory
- Verify DEM has valid data at source/receiver locations
- Try smaller buffer distance or lower resolution

### **VST Won't Load Config**
- Check file paths in configuration JSON
- Verify cropped DEM file exists
- Ensure GDAL is installed for VST plugin

## Example Workflow

### **Study Wind Farm Noise Impact**:
1. Load high-resolution DEM of study area
2. Place source points at each turbine location
3. Place receiver points at nearby residences
4. Export multiple configurations for each source-receiver pair
5. Load in VST to analyze cumulative acoustic impact
6. Generate impulse responses for different weather conditions

### **Architectural Acoustics**:
1. Load urban DEM including buildings
2. Place source at proposed construction site
3. Place receivers at sensitive locations (schools, hospitals)
4. Export with different atmospheric conditions
5. Use VST to evaluate noise propagation patterns
6. Optimize design based on acoustic modeling

---

## 🎯 **The QGIS plugin serves as the essential bridge between GIS terrain data and real-time acoustic simulation in your DAW!**

*This integration allows you to leverage the power of professional GIS tools with cutting-edge audio processing for unprecedented acoustic landscape analysis.*