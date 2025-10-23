# 🎵 How to Use LandscapeAcousticVST Plugin

## Quick Start Guide

### 1. **Installation & Setup**

#### Step 1: Build the Plugin
```powershell
# Navigate to project directory
cd C:\Users\marco\OneDrive\Skrivebord\Sound\SoundArch\LandscapeAcousticVST

# Run setup (as Administrator)
.\setup_build_environment.ps1

# Build the plugin
.\build.ps1
```

#### Step 2: Install in Your DAW
After successful build, copy the plugin files:
- **VST3**: Copy `LandscapeAcousticVST.vst3` to `%COMMONPROGRAMFILES%\VST3\`
- **Standalone**: Run `LandscapeAcousticVST.exe` directly

---

## 🎚️ **Using the Plugin**

### **Method 1: Quick Start with Built-in Terrain**
1. **Load the plugin** in your DAW on an audio track
2. **Select "Demo Terrain"** from the preset dropdown
3. **Adjust source/receiver positions** using the XY pad
4. **Play audio** - you'll hear real-time acoustic simulation!

### **Method 2: Using Real Terrain Data**

#### A. **With QGIS Integration** (Recommended)
1. **Open QGIS** and load your DEM file
2. **Install the LandscapeAcoustic plugin**:
   - Copy `scripts/qgis_plugin/` folder to QGIS plugins directory
   - Enable in QGIS Plugin Manager
3. **Export terrain data**:
   - Select your DEM layer
   - Click "LandscapeAcoustic" → "Export for VST"
   - Set source and receiver points on the map
   - Save configuration file
4. **Load in VST**:
   - Click "Import QGIS Config" in plugin
   - Select the saved configuration file

#### B. **Direct DEM File Loading**
1. **Click "Load Terrain"** in the plugin interface
2. **Select DEM file** (supports .asc, .tif, .dem formats)
3. **Set analysis points**:
   - Click "Set Source" and click on terrain view
   - Click "Set Receiver" and click on terrain view
4. **Acoustic simulation starts automatically**

---

## 🎛️ **Plugin Controls**

### **Main Interface**
- **Terrain View**: Visual representation of loaded terrain
- **Source/Receiver Points**: Draggable markers for positioning
- **Profile View**: Shows terrain cross-section between points

### **Audio Parameters**
- **Dry/Wet Mix** (0-100%): Blend original and processed audio
- **Output Gain** (-20 to +20 dB): Final output level
- **Temperature** (-10 to 40°C): Air temperature for atmospheric absorption
- **Humidity** (0-100%): Relative humidity for atmospheric calculations
- **Ground Type**: Hard / Porous / Mixed surface types
- **Source Height** (0.1-10m): Height of sound source above terrain
- **Receiver Height** (0.1-10m): Height of listener above terrain

### **Analysis Tools**
- **Real-time IR Display**: Shows current impulse response
- **Frequency Response**: Displays attenuation across frequency bands
- **Distance Display**: Shows path distance and elevation profile

---

## 🌍 **Working with Terrain Data**

### **Supported File Formats**
- **ASCII Grid (.asc)**: Simple text-based elevation data
- **GeoTIFF (.tif/.tiff)**: Georeferenced raster format
- **DTED (.dt0/.dt1/.dt2)**: Digital terrain elevation data

### **Getting Terrain Data**
1. **Free Sources**:
   - USGS Earth Explorer (global coverage)
   - NASA SRTM data (30m resolution)
   - National mapping agencies
2. **QGIS Integration**:
   - Use QGIS to download, process, and crop terrain data
   - Export specific regions for acoustic analysis

### **Data Requirements**
- **Resolution**: 10-30m recommended for acoustic accuracy
- **Coverage**: Should include source and receiver locations
- **Size**: Smaller files process faster (< 1000x1000 pixels ideal)

---

## 🎵 **Creative Applications**

### **Music Production**
- **Environmental Reverb**: Create realistic outdoor spaces
- **Dynamic Processing**: Sound changes as you move positions
- **Atmospheric Effects**: Temperature/humidity affect tone
- **Landscape Composition**: Let terrain shape your music

### **Sound Design**
- **Film Scoring**: Match audio to landscape shots
- **Game Audio**: Realistic environmental acoustics
- **VR/AR**: Immersive spatial audio experiences
- **Architecture**: Preview building acoustics

### **Research Applications**
- **Acoustic Ecology**: Study natural soundscapes
- **Environmental Impact**: Assess noise propagation
- **Urban Planning**: Evaluate acoustic environments
- **Validation**: Compare with field measurements

---

## ⚙️ **Advanced Usage**

### **Custom Configurations**
1. **Save/Load Presets**: Store your favorite settings
2. **Automation**: Automate parameters in your DAW
3. **MIDI Control**: Map terrain positions to MIDI controllers
4. **Multiple Instances**: Different terrains on different tracks

### **Performance Optimization**
- **Buffer Size**: Larger buffers = more stable processing
- **Sample Rate**: 44.1/48 kHz recommended
- **CPU Load**: Reduce terrain resolution if needed
- **Memory**: Close unused terrain files

### **Expert Tips**
- **Terrain Scale**: Match real-world scale for accuracy
- **Source Types**: Different sounds behave differently
- **Atmospheric Conditions**: Change throughout the day
- **Ground Effects**: Wet vs dry surfaces sound different

---

## 🔧 **Troubleshooting**

### **Common Issues**

#### Plugin Won't Load
- ✅ Check VST3 installation path
- ✅ Verify DAW rescanned plugins
- ✅ Ensure all dependencies installed

#### No Sound Output
- ✅ Check dry/wet mix (set to ~50%)
- ✅ Verify terrain is loaded
- ✅ Ensure source/receiver points are set
- ✅ Check output gain level

#### Performance Issues
- ✅ Reduce terrain file size
- ✅ Increase audio buffer size
- ✅ Close other plugins/applications
- ✅ Use faster storage (SSD)

#### Terrain Won't Load
- ✅ Check file format compatibility
- ✅ Verify file isn't corrupted
- ✅ Ensure GDAL is properly installed
- ✅ Try smaller file size

### **Getting Help**
- **Documentation**: Check included README files
- **Validation**: Run physics validation scripts
- **Community**: GitHub repository for issues/questions
- **Support**: Contact developer for technical issues

---

## 📚 **Example Workflows**

### **Workflow 1: Film Scoring**
1. Load terrain matching film location
2. Set source at action position
3. Set receiver at camera position
4. Adjust atmospheric conditions to match scene
5. Automate positions during action sequences

### **Workflow 2: Music Production**
1. Load interesting terrain (mountains, valleys)
2. Create multiple source positions
3. Automate movement between positions
4. Layer different instruments at different positions
5. Create dynamic, evolving soundscape

### **Workflow 3: Research/Validation**
1. Load precise DEM of study area
2. Set exact GPS coordinates
3. Match atmospheric conditions to measurement day
4. Compare plugin output with field recordings
5. Export impulse responses for analysis

---

## 🎯 **Next Steps**

### **Learn More**
- **Physics**: Study ISO 9613-2 acoustic standard
- **GIS**: Learn QGIS for terrain processing
- **Audio**: Understand convolution and impulse responses
- **Research**: Read acoustic landscape literature

### **Contribute**
- **Bug Reports**: Help improve the plugin
- **Feature Requests**: Suggest new functionality
- **Terrain Data**: Share interesting locations
- **Validation**: Compare with real measurements

---

## 🎊 **Have Fun!**

The LandscapeAcousticVST plugin opens up new possibilities for:
- **Creative expression** through terrain-based audio
- **Scientific accuracy** in environmental acoustics
- **Real-time exploration** of acoustic landscapes
- **Educational** understanding of sound propagation

**Start exploring the acoustic properties of the world around you!** 🌄🎵

---

*For technical support, visit: https://github.com/MarcoJ03rgensen/SoundArch*