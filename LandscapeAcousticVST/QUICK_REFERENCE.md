# 🚀 LandscapeAcousticVST Quick Reference

## 📋 **3-Minute Quick Start**

### **Option A: Use Demo Mode**
1. Load plugin in DAW → Select "Demo Terrain" → Play audio ✅

### **Option B: Load Your Own Terrain**
1. Plugin: "Load Terrain" → Select DEM file
2. Click map to set Source → Click map to set Receiver  
3. Adjust Dry/Wet mix → Play audio ✅

### **Option C: QGIS Integration** (Most Powerful)
1. QGIS: Load DEM → LandscapeAcoustic → Export for VST
2. Set source/receiver points → Export
3. Plugin: "Import QGIS Config" → Select exported file ✅

---

## 🎛️ **Essential Controls**

| Control | Range | What It Does |
|---------|-------|-------------|
| **Dry/Wet** | 0-100% | Mix original vs processed audio |
| **Output Gain** | -20 to +20 dB | Final volume level |
| **Temperature** | -10 to 40°C | Affects high frequency absorption |
| **Humidity** | 0-100% | Changes atmospheric absorption |
| **Ground Type** | Hard/Porous/Mixed | Surface reflection properties |
| **Source Height** | 0.1-10m | Sound source elevation |
| **Receiver Height** | 0.1-10m | Listener elevation |

---

## 🎵 **Instant Creative Ideas**

### **🎸 Guitar Processing**
- Load mountain terrain → Set source in valley → Receiver on peak
- **Result**: Natural mountain echo and atmospheric filtering

### **🥁 Drum Ambience**  
- Load urban DEM → Source at street level → Receiver on rooftop
- **Result**: City reverb with realistic sound propagation

### **🎹 Pad Textures**
- Load forest terrain → Move receiver position over time
- **Result**: Evolving spatial textures as you "walk" through landscape

### **🎺 Orchestral Depth**
- Different instruments at different terrain positions
- **Result**: Natural orchestral spacing based on real geography

---

## ⚡ **Power User Shortcuts**

### **Best File Formats**
- ✅ **GeoTIFF (.tif)** - Best quality, fast loading
- ✅ **ASCII Grid (.asc)** - Simple, universal compatibility  
- ❌ **Avoid huge files** - Keep under 2000x2000 pixels

### **Performance Tips**
- 🚀 **Lower resolution** = faster processing
- 🚀 **Smaller terrain area** = more responsive
- 🚀 **Higher audio buffer** = more stable playback

### **Creative Automation**
- **Automate X/Y position** → Moving sound source
- **Automate temperature** → Day/night atmospheric changes
- **Automate humidity** → Weather condition changes
- **Automate ground type** → Surface transitions

---

## 🛠️ **Troubleshooting in 30 Seconds**

| Problem | 30-Second Fix |
|---------|---------------|
| **No sound** | Check Dry/Wet mix (set to 50%) + verify terrain loaded |
| **Plugin won't load** | Copy .vst3 to `%COMMONPROGRAMFILES%\VST3\` + rescan |
| **Terrain won't load** | Use smaller file + check format (.tif/.asc) |
| **Crackling audio** | Increase audio buffer size in DAW |
| **High CPU usage** | Reduce terrain resolution + close other plugins |

---

## 🌍 **Where to Get Terrain Data**

### **Free Sources** (30 seconds to download):
1. **USGS Earth Explorer** → usgs.gov/earthexplorer
2. **NASA SRTM** → earthdata.nasa.gov
3. **OpenTopography** → opentopography.org

### **What to Download**:
- **Format**: GeoTIFF or ASCII Grid
- **Resolution**: 10-30 meter (best balance speed/quality)
- **Size**: 10-50 km area (covers most acoustic scenarios)

---

## 🎯 **Common Use Cases**

### **🎵 Music Production**
```
Dry/Wet: 30-70% | Temperature: 15-25°C | Ground: Mixed
→ Subtle environmental enhancement
```

### **🎬 Film/Game Audio**
```  
Dry/Wet: 50-90% | Match scene conditions | Accurate terrain
→ Realistic environmental audio
```

### **🔬 Research/Validation**
```
Dry/Wet: 100% | Precise conditions | High-res terrain
→ Scientific acoustic modeling
```

### **🎨 Sound Design**
```
Dry/Wet: 70-100% | Extreme conditions | Interesting terrain
→ Creative atmospheric effects
```

---

## 📞 **Getting Help**

- 📖 **Full Guide**: `USER_GUIDE.md`
- 🗺️ **QGIS Help**: `QGIS_PLUGIN_GUIDE.md`  
- 🔧 **Build Issues**: `BUILD_INSTRUCTIONS.md`
- 🐛 **Report Bugs**: GitHub repository
- ❓ **Quick Questions**: Check troubleshooting sections

---

## 🎊 **You're Ready to Go!**

**Start with demo terrain → experiment with controls → load your own landscape → create amazing acoustic experiences!**

*The world is your acoustic playground!* 🌄🎵