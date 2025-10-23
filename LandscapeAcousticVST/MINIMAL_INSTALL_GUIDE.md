# 🎯 **Minimal Visual Studio Components for LandscapeAcousticVST**

## ❓ **Do I Need the Full C++ Workload?**

**No!** You only need these specific components:

### 🎯 **Required Components (Minimal Install):**

#### **Essential (Must Have):**
- ✅ **MSVC v143 - VS 2022 C++ x64/x86 build tools (latest)**
- ✅ **Windows 11 SDK (10.0.22621.0 or latest)**
- ✅ **CMake tools for C++**

#### **Recommended (Small additions):**
- ✅ **IntelliSense** (for better development experience)
- ✅ **VC++ 2022 Redistributable Update**

### 📦 **What You DON'T Need:**
- ❌ ATL/MFC libraries
- ❌ Game development tools
- ❌ Linux development
- ❌ Clang compiler
- ❌ Address Sanitizer
- ❌ Graphics debugger
- ❌ Performance profiling tools

---

## 🛠️ **How to Install Minimal Components**

### **Option 1: Individual Components (Recommended)**
In Visual Studio Installer:

1. **Click "Modify"** for Visual Studio Community 2022
2. **Go to "Individual components" tab** (not Workloads)
3. **Search and check these boxes:**
   - `MSVC v143 - VS 2022 C++ x64/x86 build tools`
   - `Windows 11 SDK (10.0.22621.0)`
   - `CMake tools for C++`
4. **Click "Modify"**

**Download size: ~2-3 GB** (vs. 6+ GB for full workload)

### **Option 2: Custom Workload**
1. **Click "Modify"**
2. **Check "Desktop development with C++"**
3. **In the right panel, UNCHECK:**
   - ❌ Blend for Visual Studio
   - ❌ Live Share
   - ❌ IntelliCode
   - ❌ Windows 10/11 SDK (older versions)
   - ❌ Address Sanitizer
   - ❌ Test Adapter for Boost.Test
   - ❌ Test Adapter for Google Test

**Download size: ~4-5 GB**

---

## ⚡ **What JUCE Actually Needs**

```cpp
// JUCE requirements for Windows:
1. C++ compiler (MSVC v143)
2. Windows SDK (for Windows API)
3. CMake (for build system)
4. Standard C++ libraries
```

**That's it!** No special graphics tools, no debugging tools, no extra frameworks.

---

## 🚀 **Fastest Installation Method**

**PowerShell command (run as Administrator):**
```powershell
winget install Microsoft.VisualStudio.2022.BuildTools --override "--add Microsoft.VisualStudio.Component.VC.Tools.x86.x64 --add Microsoft.VisualStudio.Component.Windows11SDK.22621 --add Microsoft.VisualStudio.Component.VC.CMake.Project --quiet --wait"
```

This installs **only** what you need: **~2 GB download**.

---

## 🎯 **Verification**

After installing the minimal components, test:
```powershell
# Open new PowerShell
& "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\Launch-VsDevShell.ps1"
cl
# Should show: "Microsoft (R) C/C++ Optimizing Compiler"

cmake --version
# Should show: "cmake version 3.x.x"
```

---

## ⏱️ **Time Comparison**

| Method | Download Size | Install Time | What You Get |
|--------|--------------|--------------|--------------|
| **Full Workload** | ~6-8 GB | 20-30 min | Everything + extras |
| **Minimal Components** | ~2-3 GB | 8-12 min | Just what JUCE needs |
| **Build Tools Only** | ~1-2 GB | 5-8 min | Command-line only |

---

## 🎵 **Bottom Line**

**For LandscapeAcousticVST, you need:**
- C++ compiler ✅
- Windows SDK ✅  
- CMake ✅
- **Total: ~2-3 GB**

**You can skip all the IDE features, debugging tools, and extra frameworks!**

Choose **Individual Components** method for the smallest, fastest installation that will build your plugin perfectly! 🎯