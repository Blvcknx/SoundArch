# LandscapeAcousticVST Compilation Guide

## Quick Start

### Prerequisites Setup
1. **Run Build Environment Setup** (as Administrator):
   ```powershell
   Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
   .\setup_build_environment.ps1
   ```

2. **Restart PowerShell** to pick up new environment variables

### Compilation Steps

#### Option 1: Visual Studio (Recommended)
```powershell
# Navigate to project directory
cd C:\Users\marco\OneDrive\Skrivebord\Sound\SoundArch\LandscapeAcousticVST

# Create and enter build directory
mkdir build -Force
cd build

# Configure with Visual Studio generator
cmake -G "Visual Studio 17 2022" -A x64 `
  -DCMAKE_BUILD_TYPE=Release `
  -DGDAL_ROOT="C:\Program Files\GDAL" `
  -DGDAL_INCLUDE_DIR="C:\Program Files\GDAL" `
  -DGDAL_LIBRARY="C:\Program Files\GDAL\gdal.dll" `
  ..

# Build the project
cmake --build . --config Release --parallel 4

# Alternative: Open in Visual Studio
# cmake --open .
```

#### Option 2: MinGW (Alternative)
```powershell
# Set MinGW environment
$env:PATH = "C:\msys64\mingw64\bin;$env:PATH"
$env:CC = "gcc"
$env:CXX = "g++"

# Configure with MinGW
cmake -G "MinGW Makefiles" `
  -DCMAKE_BUILD_TYPE=Release `
  -DGDAL_ROOT="C:\Program Files\GDAL" `
  ..

# Build
cmake --build . --parallel 4
```

#### Option 3: LLVM/Clang
```powershell
# Configure with Clang
cmake -G "Ninja" `
  -DCMAKE_C_COMPILER=clang `
  -DCMAKE_CXX_COMPILER=clang++ `
  -DCMAKE_BUILD_TYPE=Release `
  -DGDAL_ROOT="C:\Program Files\GDAL" `
  ..

# Build
ninja
```

## Build Outputs

### Plugin Formats
Upon successful compilation, you'll find:

```
build/
├── LandscapeAcousticVST_artefacts/
│   ├── Release/
│   │   ├── VST3/
│   │   │   └── LandscapeAcousticVST.vst3/     # VST3 Plugin
│   │   ├── AU/
│   │   │   └── LandscapeAcousticVST.component/  # Audio Unit (macOS)
│   │   └── Standalone/
│   │       └── LandscapeAcousticVST.exe        # Standalone Application
│   └── JuceLibraryCode/
├── tests/
│   └── LandscapeAcousticVST_Tests.exe         # Unit Tests
└── scripts/
    └── validation/
        ├── compare_iso9613.py                 # Validation Scripts
        └── generate_test_ir.py
```

### Installation Locations

**VST3 Plugin:**
- Windows: `%COMMONPROGRAMFILES%\VST3\LandscapeAcousticVST.vst3`
- macOS: `/Library/Audio/Plug-Ins/VST3/LandscapeAcousticVST.vst3`

**AU Plugin (macOS):**
- `/Library/Audio/Plug-Ins/Components/LandscapeAcousticVST.component`

## Troubleshooting

### Common Issues

#### 1. GDAL Not Found
```powershell
# Set GDAL paths explicitly
$env:GDAL_ROOT = "C:\Program Files\GDAL"
$env:GDAL_DATA = "C:\Program Files\GDAL\gdal-data"
$env:PATH = "C:\Program Files\GDAL;$env:PATH"
```

#### 2. JUCE Submodule Missing
```powershell
git submodule update --init --recursive
```

#### 3. CMake Can't Find Compiler
```powershell
# For Visual Studio - run from Developer Command Prompt
# Or set compiler explicitly:
cmake -DCMAKE_C_COMPILER="C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.39.33519/bin/Hostx64/x64/cl.exe" `
      -DCMAKE_CXX_COMPILER="C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.39.33519/bin/Hostx64/x64/cl.exe" `
      ..
```

#### 4. Missing Dependencies
```powershell
# Install vcpkg for dependency management
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg install gdal:x64-windows gtest:x64-windows

# Use vcpkg with CMake
cmake -DCMAKE_TOOLCHAIN_FILE=path/to/vcpkg/scripts/buildsystems/vcpkg.cmake ..
```

## Testing the Build

### 1. Run Unit Tests
```powershell
cd build
.\tests\LandscapeAcousticVST_Tests.exe
```

### 2. Validate Physics Calculations
```powershell
cd scripts\validation
python compare_iso9613.py
python generate_test_ir.py
```

### 3. Test Plugin Loading
- Copy VST3 to your DAW's plugin directory
- Rescan plugins in your DAW
- Load LandscapeAcousticVST on an audio track
- Load a DEM file using the "Load Terrain" button
- Adjust source/receiver positions
- Observe real-time acoustic processing

## Advanced Build Options

### Debug Build
```powershell
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build . --config Debug
```

### Build with Profiling
```powershell
cmake -DENABLE_PROFILING=ON -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
```

### Build Documentation
```powershell
# Requires Doxygen
cmake -DBUILD_DOCUMENTATION=ON ..
cmake --build . --target doc
```

## Performance Optimization

### Compiler Optimizations
```powershell
# Maximum optimization
cmake -DCMAKE_BUILD_TYPE=Release `
      -DCMAKE_CXX_FLAGS_RELEASE="/O2 /Ob2 /DNDEBUG /GL" `
      -DCMAKE_EXE_LINKER_FLAGS_RELEASE="/LTCG" `
      ..
```

### SIMD Support
```powershell
# Enable AVX2 for faster audio processing
cmake -DENABLE_AVX2=ON ..
```

## Packaging for Distribution

### Create Installer
```powershell
cmake --build . --target package
```

### Create Portable Archive
```powershell
# Copy required files
mkdir LandscapeAcousticVST_Portable
cp build/LandscapeAcousticVST_artefacts/Release/VST3/* LandscapeAcousticVST_Portable/
cp build/LandscapeAcousticVST_artefacts/Release/Standalone/* LandscapeAcousticVST_Portable/
cp Resources/default_config.json LandscapeAcousticVST_Portable/
cp README.md LandscapeAcousticVST_Portable/
cp LICENSE LandscapeAcousticVST_Portable/

# Create archive
Compress-Archive -Path LandscapeAcousticVST_Portable -DestinationPath LandscapeAcousticVST_v1.0.0.zip
```

---

## Next Steps After Build

1. **Install the plugin** in your DAW
2. **Load terrain data** (DEM files)
3. **Experiment with acoustic simulation** in real-time
4. **Validate results** against known acoustic measurements
5. **Share feedback** for further development

For support and updates, visit: https://github.com/MarcoJ03rgensen/SoundArch