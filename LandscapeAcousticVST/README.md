# LandscapeAcousticVST

A real-time terrain-based sound propagation VST plugin that simulates outdoor acoustic environments using Digital Elevation Models (DEMs) and ISO 9613-2 propagation calculations.

## Features

- **Real-time Convolution**: Load terrain-derived impulse responses for immersive outdoor audio processing
- **ISO 9613-2 Compliance**: Scientifically accurate acoustic propagation modeling
- **DEM Support**: Import GeoTIFF, ASC, DTED, and other elevation formats via GDAL
- **Virtual Terrain Rendering**: Efficient rendering of massive DEM files (15-20 GB) without memory constraints
- **GDAL Hillshade Integration**: Professional terrain visualization compatible with QGIS
- **QGIS Integration**: Seamless workflow with PyQGIS export scripts
- **Cross-platform**: VST3, AU, and Standalone formats for Windows, macOS, and Linux
- **Interactive GUI**: Visual terrain map with point-and-click source/receiver selection

## Quick Start

### Prerequisites

- **JUCE Framework** (8.0+)
- **GDAL Library** (3.0+)
- **CMake** (3.15+)
- **C++17** compatible compiler
- **Visual Studio 2022** (Windows) or **Xcode** (macOS) or **GCC/Clang** (Linux)

### Build Instructions

1. **Clone Repository**
   ```bash
   git clone https://github.com/soundarch/LandscapeAcousticVST.git
   cd LandscapeAcousticVST
   git submodule update --init --recursive
   ```

2. **Install Dependencies**

   **Windows (vcpkg):**
   ```bash
   vcpkg install gdal:x64-windows
   ```

   **macOS (Homebrew):**
   ```bash
   brew install gdal cmake
   ```

   **Linux (Ubuntu/Debian):**
   ```bash
   sudo apt-get install libgdal-dev cmake build-essential
   ```

3. **Build Plugin**
   ```bash
   # Windows
   .\build_vs.bat

   # Or manually:
   mkdir build && cd build
   cmake .. -DCMAKE_BUILD_TYPE=Release
   cmake --build . --config Release
   ```

4. **Install Plugin**
   - VST3: Copy to your system's VST3 folder
   - AU: Copy to `~/Library/Audio/Plug-Ins/Components/` (macOS)
   - Standalone: Run executable directly

### Usage

1. **Load DEM Data**
   - Drag & drop GeoTIFF file into plugin GUI
   - Or use "Load DEM File" button to browse
   - Plugin automatically generates hillshade visualization

2. **Set Analysis Points**
   - Left-click on terrain map to set sound source
   - Right-click to set listener position
   - Plugin automatically generates impulse response

3. **Adjust Parameters**
   - Temperature and humidity for atmospheric absorption
   - Ground type (Hard/Porous/Mixed) for surface effects
   - Source and receiver heights above terrain
   - Dry/wet mix and output gain

4. **Real-time Processing**
   - Process audio through terrain-based convolution
   - Export impulse responses as WAV files

## DEM Viewer

Two options are available for visualizing DEM hillshade images:

### Option 1: OpenCV Viewer (Recommended - Lightweight)

A simple, fast C++ application using OpenCV for hillshade visualization:

#### Features
- **Fast Loading**: Direct image loading with minimal dependencies
- **Interactive Controls**: Mouse wheel zoom and click-drag pan
- **Cross-platform**: Works on Windows, macOS, and Linux
- **No Qt Required**: Only needs OpenCV (much lighter than Qt)

#### Quick Start
```bash
# Generate hillshade
gdaldem hillshade input_dem.tif hillshade.tif

# View with OpenCV
cd opencv_viewer
.\build.bat  # Windows
cd build/bin
.\opencv_hillshade_viewer.exe ../hillshade.tif
```

#### Prerequisites
- **OpenCV 4.x**: Install via `vcpkg install opencv4[contrib]` (Windows)
- **CMake 3.15+**
- **C++17 compiler**

See `opencv_viewer/README.md` for detailed setup instructions.

### Option 2: Qt Viewer (Full GUI)

A complete Qt-based application with advanced features:

#### Features
- **Professional GUI**: File dialogs, menus, and toolbars
- **Advanced Visualization**: Color mapping and terrain gradients
- **Integrated Workflow**: Seamless DEM loading and processing
- **QGIS Compatibility**: Identical hillshade rendering

#### Prerequisites
- **Qt 5.15.2**: Download from qt.io/download
- **GDAL Library**
- **CMake 3.15+**

See `viewer/README.md` for installation and usage details.

## QGIS Integration

### Export DEM and Analysis Points

1. **Install QGIS Plugin** (optional)
   ```bash
   cp -r scripts/qgis_plugin/ ~/.local/share/QGIS/QGIS3/profiles/default/python/plugins/landscape_acoustic/
   ```

2. **Manual Export Script**
   - Open DEM as raster layer in QGIS
   - Create point layer with source/receiver points
   - Add 'type' attribute: 'source' and 'receiver'
   - Run `scripts/qgis_export.py` in QGIS Python Console

3. **Import Configuration**
   - Use "Import QGIS Config" button in plugin
   - Automatically loads DEM and sets analysis points

## Technical Architecture

### Virtual Terrain Rendering System

The plugin implements a sophisticated virtual terrain rendering system designed to handle massive DEM files efficiently:

- **Tile-Based Loading**: DEM data is divided into 256×256 pixel tiles loaded on-demand
- **Multi-Level LOD**: Automatic level-of-detail selection based on zoom level and screen resolution
- **Intelligent Caching**: LRU cache with configurable memory limits (default 512MB)
- **Asynchronous Processing**: Non-blocking tile loading and rendering for smooth UI interaction

### GDAL Hillshade Integration

Professional terrain visualization using GDAL's proven algorithms:

- **Automatic Generation**: Hillshade files are generated automatically when loading DEMs
- **QGIS Compatibility**: Uses identical parameters as QGIS for consistent visualization
- **File Naming**: Hillshade files named `<dem_filename>_hillshade.tif`
- **Fallback Rendering**: Custom hillshade algorithm available if GDAL processing fails

### Memory Management

- **Virtual Rendering**: No memory limit on DEM file size (tested with 15-20 GB files)
- **Efficient Caching**: Smart cache eviction prevents memory bloat
- **GDAL Integration**: Direct access to compressed raster data without full file loading

## Scientific Background

### ISO 9613-2 Acoustic Propagation

The plugin implements the international standard for outdoor sound attenuation:

- **Divergence**: Geometric spreading loss (20log₁₀(d) + 11 dB)
- **Atmospheric Absorption**: Temperature/humidity-dependent high-frequency loss
- **Ground Effect**: Surface reflection and absorption modeling
- **Barrier Diffraction**: Fresnel knife-edge theory for terrain obstructions

### Terrain Analysis

- **Line-of-Sight Calculation**: Determines direct and obstructed sound paths
- **Fresnel Diffraction**: Models sound bending around terrain features
- **Elevation Profiling**: Samples terrain cross-sections between source and receiver
- **Hillshade Visualization**: Enhances terrain understanding for acoustic analysis

### Archaeoacoustic Applications

- **Soundscape Reconstruction**: Model historical acoustic environments
- **Site Analysis**: Understand acoustic properties of archaeological locations
- **Cultural Interpretation**: Explore sound-landscape relationships in past societies

## File Formats

### Supported DEM Formats
- **GeoTIFF** (.tif, .tiff) - Recommended
- **ESRI ASCII Grid** (.asc)
- **DTED** (.dted, .dt0, .dt1, .dt2)
- **SRTM HGT** (.hgt)
- **ERDAS Imagine** (.img)

### Configuration Format (JSON)
```json
{
  "format_version": "1.0",
  "files": {
    "dem_file": "path/to/elevation.tif"
  },
  "analysis_points": {
    "source": {"lon": -105.5, "lat": 36.0},
    "receiver": {"lon": -105.4, "lat": 36.1}
  },
  "default_parameters": {
    "temperature_C": 15.0,
    "humidity_percent": 70.0,
    "ground_type": "porous"
  }
}
```

## Development

### Project Structure
```
LandscapeAcousticVST/
├── Source/                 # C++ plugin source code
│   ├── PluginProcessor.cpp # Main audio processing logic
│   ├── PluginEditor.cpp    # GUI implementation
│   ├── TerrainLoader.cpp   # GDAL-based DEM loading and hillshade generation
│   ├── TerrainRenderer.cpp # Virtual terrain rendering system
│   ├── AcousticEngine.cpp  # ISO 9613-2 propagation calculations
│   ├── TerrainProfile.cpp  # Elevation profile sampling
│   └── Utils.cpp          # Utility functions
├── opencv_viewer/         # Lightweight OpenCV-based hillshade viewer
│   ├── main.cpp           # Interactive hillshade viewer with pan/zoom
│   ├── CMakeLists.txt     # OpenCV build configuration
│   ├── build.bat          # Windows build script
│   └── README.md          # Setup and usage instructions
├── viewer/                # Qt-based DEM visualization application
│   ├── main.cpp           # Complete Qt application with GDAL integration
│   ├── CMakeLists.txt     # Qt5 + GDAL build configuration
│   ├── build.bat          # Windows build script
│   └── README.md          # Detailed setup and usage instructions
├── Resources/             # UI assets and sample data
│   └── test_data/         # Sample DEM files for testing
├── scripts/               # QGIS integration and validation
│   ├── qgis_export.py     # Export configurations from QGIS
│   └── validation/        # Reference calculation validation
├── tests/                 # Unit and integration tests
├── JUCE/                  # JUCE framework (submodule)
├── CMakeLists.txt        # Build configuration
├── build_vs.bat          # Windows build script
└── README.md             # This file
```

### Key Components

#### TerrainLoader
- GDAL integration for DEM file loading
- Automatic hillshade generation using GDALDEMProcessing
- Support for GeoTIFF, ASCII Grid, DTED, SRTM formats
- Virtual tile-based data access for large files

#### TerrainRenderer
- Virtual terrain rendering with LOD support
- Tile-based caching system (256×256 pixel tiles)
- GDAL hillshade data integration
- Custom hillshade algorithm fallback
- Geographic coordinate transformations

#### AcousticEngine
- ISO 9613-2 propagation modeling
- Atmospheric absorption calculations
- Ground effect modeling
- Fresnel diffraction for terrain barriers

### Building and Testing

#### Windows Build
```bash
# Automated build script
.\build_vs.bat
```

#### Manual Build
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

#### Running Tests
```bash
cd build
ctest --output-on-failure
```

### Performance Characteristics

- **Real-time Processing**: <5% CPU usage for typical scenarios
- **IR Length**: Up to 16,384 samples (370ms @ 44.1kHz)
- **DEM Size**: Tested with up to 10k×10k elevation grids (no theoretical limit with virtual rendering)
- **Latency**: Plugin latency + convolution delay (typically <10ms)
- **Memory Usage**: Configurable cache (default 512MB) + base application memory

## Changelog

### Version 1.0.0 (November 2025)

#### Major Features
- **Virtual Terrain Rendering**: Complete rewrite of terrain rendering system to support massive DEM files (15-20 GB) without memory constraints
- **GDAL Hillshade Integration**: Professional terrain visualization using GDAL's algorithms for QGIS-compatible results
- **Tile-Based Architecture**: 256×256 pixel tile system with intelligent caching and LOD support
- **Enhanced GUI**: Modern interface with gradient backgrounds and improved terrain visualization

#### Technical Improvements
- **TerrainLoader**: Added automatic hillshade generation and loading
- **TerrainRenderer**: Implemented virtual rendering with GDAL dataset integration
- **Memory Management**: LRU cache system with configurable memory limits
- **Build System**: Updated to JUCE 8.0.10 and improved CMake configuration

#### Bug Fixes
- Fixed DEM display to show entire terrain at once instead of partial views
- Resolved button functionality issues in standalone application
- Improved file dialog handling and error reporting

#### Development
- Added comprehensive unit tests for terrain loading and rendering
- Implemented automated build scripts for Windows
- Enhanced documentation and project structure

## Contributing

1. Fork the repository
2. Create feature branch (`git checkout -b feature/amazing-feature`)
3. Commit changes (`git commit -am 'Add amazing feature'`)
4. Push to branch (`git push origin feature/amazing-feature`)
5. Open Pull Request

### Development Guidelines
- Follow C++17 standards
- Include unit tests for new features
- Document public APIs with Doxygen comments
- Validate against ISO 9613-2 reference calculations

## Performance

- **Real-time Processing**: <5% CPU usage for typical scenarios
- **IR Length**: Up to 16,384 samples (370ms @ 44.1kHz)
- **DEM Size**: Tested with up to 10k×10k elevation grids
- **Latency**: Plugin latency + convolution delay (typically <10ms)

## Contributing

1. Fork the repository
2. Create feature branch (`git checkout -b feature/amazing-feature`)
3. Commit changes (`git commit -am 'Add amazing feature'`)
4. Push to branch (`git push origin feature/amazing-feature`)
5. Open Pull Request

### Development Guidelines
- Follow C++17 standards
- Include unit tests for new features
- Document public APIs with Doxygen comments
- Validate against ISO 9613-2 reference calculations

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Citation

If using this plugin for research, please cite:

```bibtex
@software{landscape_acoustic_vst,
  title={LandscapeAcousticVST: Real-time Terrain-Based Sound Propagation Modeling},
  author={SoundArch Development Team},
  year={2025},
  url={https://github.com/soundarch/LandscapeAcousticVST},
  version={1.0.0}
}
```

## Support

- **Documentation**: [Wiki](https://github.com/soundarch/LandscapeAcousticVST/wiki)
- **Issues**: [GitHub Issues](https://github.com/soundarch/LandscapeAcousticVST/issues)
- **Discussions**: [GitHub Discussions](https://github.com/soundarch/LandscapeAcousticVST/discussions)

## Acknowledgments

- R. Murray Schafer for acoustic ecology foundations
- ISO 9613-2 working group for standardized outdoor acoustics
- JUCE Framework for cross-platform audio development
- GDAL/OGR for geospatial data processing
- QGIS community for GIS integration capabilities