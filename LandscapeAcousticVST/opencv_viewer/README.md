# OpenCV DEM Hillshade Viewer

A lightweight C++ application for viewing DEM hillshade images using OpenCV. This provides a simple alternative to the Qt-based viewer with full pan/zoom functionality.

## Features

- **Fast Loading**: Direct OpenCV image loading for GeoTIFF and other formats
- **Interactive Viewing**: Mouse wheel zoom and click-drag pan
- **Keyboard Controls**: Reset view, fit to window, quit
- **Cross-platform**: Works on Windows, macOS, and Linux
- **No Dependencies**: Only requires OpenCV (much lighter than Qt)

## Prerequisites

### Windows
- **OpenCV 4.x** - Install via vcpkg: `vcpkg install opencv4[contrib]`
- **Visual Studio 2022** with C++ support
- **CMake 3.15+**
- **GDAL** (for hillshade generation) - already available from main VST plugin

### macOS
```bash
brew install opencv cmake
```

### Linux (Ubuntu/Debian)
```bash
sudo apt-get install libopencv-dev cmake build-essential
```

## Building

### Windows

1. **Install OpenCV via vcpkg:**
   ```bash
   vcpkg install opencv4[contrib] --triplet x64-windows
   ```

2. **Build the viewer:**
   ```bash
   cd opencv_viewer
   .\build.bat
   ```

3. **Run:**
   ```bash
   cd build\bin
   .\opencv_hillshade_viewer.exe path\to\hillshade.tif
   ```

### macOS / Linux

```bash
cd opencv_viewer
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
./opencv_hillshade_viewer ../path/to/hillshade.tif
```

## Usage

### Generate Hillshade First

Before viewing, generate a hillshade from your DEM:

```bash
# Basic hillshade (matches QGIS defaults)
gdaldem hillshade input_dem.tif output_hillshade.tif

# Custom parameters
gdaldem hillshade input_dem.tif output_hillshade.tif -z 1.0 -s 1.0 -az 315 -alt 45
```

### View the Hillshade

```bash
# Windows
.\opencv_hillshade_viewer.exe output_hillshade.tif

# macOS/Linux
./opencv_hillshade_viewer output_hillshade.tif
```

### Controls

- **Mouse Wheel**: Zoom in/out
- **Left Click + Drag**: Pan around the image
- **'r'**: Reset view to original zoom and position
- **'f'**: Fit image to window size
- **ESC or 'q'**: Quit the viewer

## Integration with VST Plugin

This viewer complements the main LandscapeAcousticVST plugin:

1. **Preview DEMs**: Use this viewer to quickly check hillshade quality
2. **Verify Processing**: Ensure GDAL hillshade generation works correctly
3. **Select Regions**: Identify areas of interest for acoustic analysis
4. **Fast Iteration**: Much faster startup than loading DEMs in the VST plugin

## Performance

- **Loading**: Near-instantaneous for most hillshade images
- **Memory Usage**: Minimal - only loads the display image
- **Zoom/Pan**: Smooth interaction even with large images (tested with 10k×10k pixels)
- **File Formats**: Supports TIFF, PNG, JPEG, and other OpenCV-compatible formats

## Troubleshooting

### "OpenCV not found" error
```
CMake Error: Could not find OpenCV
```

**Solutions:**
1. **Install OpenCV**: `vcpkg install opencv4[contrib]`
2. **Set CMAKE_PREFIX_PATH**: Point to OpenCV installation
3. **Check environment**: Ensure vcpkg is in PATH

### Image doesn't load
- Check file path and format
- Ensure GDAL generated the hillshade correctly
- Try converting to PNG: `gdal_translate -of PNG input.tif output.png`

### Poor performance with large images
- The viewer handles large images well, but very large ones (>50MP) may be slow to zoom
- Consider downsampling: `gdal_translate -outsize 50% 50% input.tif output_small.tif`

## Comparison with Qt Viewer

| Feature | OpenCV Viewer | Qt Viewer |
|---------|---------------|-----------|
| **Setup Complexity** | Simple (just OpenCV) | Complex (full Qt SDK) |
| **Build Time** | Fast | Slower |
| **Startup Time** | Instant | Fast |
| **Features** | Pan/Zoom only | Full GUI with file dialogs |
| **Dependencies** | Minimal | Heavy |
| **Use Case** | Quick viewing | Integrated workflow |

## Technical Details

- **Image Processing**: Automatic conversion to 8-bit grayscale
- **Zoom Limits**: 0.1x to 10x magnification
- **Pan Bounds**: Constrained to image boundaries
- **Window Management**: Resizable window with aspect ratio preservation
- **Memory Management**: Efficient display buffer management

## License

MIT License - Same as the main LandscapeAcousticVST project