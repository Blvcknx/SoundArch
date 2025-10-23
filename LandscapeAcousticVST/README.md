# LandscapeAcousticVST

A real-time terrain-based sound propagation VST plugin that simulates outdoor acoustic environments using Digital Elevation Models (DEMs) and ISO 9613-2 propagation calculations.

## Features

- **Real-time Convolution**: Load terrain-derived impulse responses for immersive outdoor audio processing
- **ISO 9613-2 Compliance**: Scientifically accurate acoustic propagation modeling
- **DEM Support**: Import GeoTIFF, ASC, DTED, and other elevation formats via GDAL
- **QGIS Integration**: Seamless workflow with PyQGIS export scripts
- **Cross-platform**: VST3, AU, and Standalone formats for Windows, macOS, and Linux
- **Interactive GUI**: Visual terrain map with point-and-click source/receiver selection

## Quick Start

### Prerequisites

- **JUCE Framework** (7.0+)
- **GDAL Library** (3.0+) 
- **CMake** (3.15+)
- **C++17** compatible compiler

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

## Scientific Background

### ISO 9613-2 Acoustic Propagation

The plugin implements the international standard for outdoor sound attenuation:

- **Divergence**: Geometric spreading loss (20log₁₀(d) + 11 dB)
- **Atmospheric Absorption**: Temperature/humidity-dependent high-frequency loss
- **Ground Effect**: Surface reflection and absorption modeling
- **Barrier Diffraction**: Fresnel knife-edge theory for terrain obstructions

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
├── Resources/             # UI assets and sample data
├── scripts/               # QGIS integration and validation
├── tests/                 # Unit and integration tests
├── docs/                  # Documentation
└── CMakeLists.txt        # Build configuration
```

### Running Tests
```bash
cd build
ctest --output-on-failure
```

### Code Coverage
```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON
make coverage
```

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