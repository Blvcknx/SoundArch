# DEM Viewer - Qt-based Terrain Visualization

A standalone Qt application for viewing and analyzing Digital Elevation Models (DEMs) with professional GDAL hillshade rendering.

## Features

- **GDAL Hillshade Generation**: Professional terrain visualization using GDAL's proven algorithms
- **Interactive Viewing**: Mouse wheel zoom and pan with scroll bars
- **Multiple Formats**: Support for GeoTIFF, ASCII Grid, DTED, SRTM, and more
- **Automatic Processing**: Generates and caches hillshade files automatically
- **Color Mapping**: Applies terrain-appropriate color gradients for better visualization
- **Cross-platform**: Works on Windows, macOS, and Linux

## Prerequisites

### Windows
- Qt5 (5.15.2 recommended) - [Download](https://www.qt.io/download)
- GDAL library (already configured if you built the main VST plugin)
- Visual Studio 2022 with C++ support
- CMake 3.15+

### macOS
```bash
brew install qt@5 gdal cmake
```

### Linux (Ubuntu/Debian)
```bash
sudo apt-get install qtbase5-dev libgdal-dev cmake build-essential
```

### Qt5 Installation Steps (Windows)

1. **Download Qt Online Installer**: Go to [qt.io/download](https://www.qt.io/download)
2. **Run the installer** and select Qt 5.15.2 → MSVC 2019 64-bit
3. **Installation path**: Typically `C:\Qt\5.15.2\msvc2019_64\`
4. **Update build.bat**: Edit the `Qt5_DIR` path in `build.bat` to match your installation

### Alternative: Use Qt Creator

If you prefer a complete IDE:
1. Download Qt Creator from the same installer
2. Open `CMakeLists.txt` in Qt Creator
3. Configure with your Qt kit
4. Build and run

## Building

### Windows

1. **Install Qt5**: Download and install Qt 5.15.2 with MSVC 2019 64-bit compiler
   
2. **Update build script**: Edit `build.bat` and set your Qt installation path:
   ```batch
   set Qt5_DIR=C:\Qt\5.15.2\msvc2019_64\lib\cmake\Qt5
   set CMAKE_PREFIX_PATH=C:\Qt\5.15.2\msvc2019_64
   ```

3. **Build**:
   ```batch
   .\build.bat
   ```

4. **Run**:
   ```batch
   cd build\Release
   .\DEMViewer.exe
   ```

### macOS / Linux

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
./DEMViewer
```

## Troubleshooting Qt Installation

### "Qt5 not found" error
```
CMake Error: Could not find a package configuration file provided by "Qt5"
```

**Solutions:**
1. **Check Qt installation**: Ensure Qt 5.15.2 is installed
2. **Update PATH**: Add Qt bin directory to system PATH
3. **Set CMAKE_PREFIX_PATH**: Point to Qt installation root
4. **Use Qt Creator**: Import the project for automatic configuration

### Alternative: Use Command Line GDAL

If Qt installation is problematic, you can still generate hillshade files manually:

```bash
# Generate hillshade using GDAL command line
gdaldem hillshade input_dem.tif output_hillshade.tif -z 1.0 -s 1.0 -az 315 -alt 45

# View with any image viewer
start output_hillshade.tif
```

### Alternative: Use QGIS

For the most user-friendly experience:
1. Install [QGIS](https://qgis.org/)
2. Load your DEM as a raster layer
3. Right-click layer → Properties → Symbology → Hillshade
4. Adjust parameters as needed

## Usage

1. **Open DEM File**: File → Open DEM... or Ctrl+O
   - Select any supported DEM format (GeoTIFF, ASC, DTED, etc.)
   - The viewer will automatically generate a hillshade if one doesn't exist

2. **Navigation**:
   - **Zoom In/Out**: Mouse wheel or View menu
   - **Pan**: Click and drag scroll bars
   - **Reset Zoom**: View → Reset Zoom or Ctrl+0

3. **File Management**:
   - Hillshade files are saved as `<filename>_hillshade.tif` in the same directory
   - Subsequent loads reuse existing hillshade files for faster display

## Hillshade Parameters

The viewer uses GDAL's standard hillshade parameters:
- **Azimuth**: 315° (northwest light source)
- **Altitude**: 45° (mid-level sun angle)
- **Z-factor**: 1.0 (no vertical exaggeration)
- **Scale**: 1.0

These match QGIS defaults for consistent visualization.

## Color Scheme

The viewer applies a terrain-appropriate color gradient:
- **Low elevations**: Brown tones
- **Mid elevations**: Green tones
- **High elevations**: White/snow tones

This can be customized in the `loadHillshadeImage()` function in `main.cpp`.

## Performance

- **Large DEMs**: The viewer supports DEMs up to 32,768 × 32,768 pixels
- **Memory Usage**: Approximately 3 bytes per pixel for colored display
- **Generation Time**: Hillshade generation time depends on DEM size (typically seconds to minutes)

## Troubleshooting

### Qt not found
```
CMake Error: Could not find Qt5
```
**Solution**: Set `CMAKE_PREFIX_PATH` to your Qt installation directory

### GDAL not found
```
Failed to open DEM file
```
**Solution**: Ensure GDAL DLL is in your PATH or same directory as executable

### Image too large
```
Warning: DEM is very large (...)
```
**Solution**: Downsample your DEM using GDAL:
```bash
gdalwarp -tr 30 30 input.tif output.tif
```

## Integration with VST Plugin

This viewer can be used alongside the Landscape Acoustic VST plugin:

1. **Preview DEMs**: Visualize terrain before loading into the plugin
2. **Verify Hillshade**: Ensure hillshade generation is working correctly
3. **Select Areas**: Identify regions of interest for acoustic analysis

The viewer uses the same GDAL hillshade algorithm as the plugin, ensuring consistent visualization.

## License

MIT License - Same as the main LandscapeAcousticVST project

## See Also

- [Main VST Plugin Documentation](../README.md)
- [GDAL Documentation](https://gdal.org/)
- [Qt Documentation](https://doc.qt.io/)
