@echo off
echo ================================================
echo    LandscapeAcousticVST - Final Setup Step
echo ================================================
echo.
echo Visual Studio is installed but missing C++ tools.
echo We need to add the C++ Desktop Development workload.
echo.
echo OPTION 1 - Automatic Installation (Run as Administrator):
echo.
echo    Right-click this file and select "Run as administrator"
echo    OR
echo    Open PowerShell as Administrator and run:
echo    winget install Microsoft.VisualStudio.2022.Community --override "--add Microsoft.VisualStudio.Workload.NativeDesktop --includeRecommended --quiet --wait"
echo.
echo OPTION 2 - Manual Installation (Recommended):
echo.
echo    1. Press Windows key and type "Visual Studio Installer"
echo    2. Click "Modify" next to Visual Studio Community 2022
echo    3. Check "Desktop development with C++" workload
echo    4. Click "Modify" and wait for installation to complete
echo    5. Restart this script
echo.
echo ================================================

:: Check if running as administrator
net session >nul 2>&1
if %errorLevel% == 0 (
    echo Running as Administrator - attempting automatic installation...
    echo.
    
    :: Try to modify the existing installation
    "C:\Program Files (x86)\Microsoft Visual Studio\Installer\vs_installer.exe" modify --installPath "C:\Program Files\Microsoft Visual Studio\2022\Community" --add Microsoft.VisualStudio.Workload.NativeDesktop --quiet --wait
    
    if errorlevel 1 (
        echo Installation failed. Please use Manual Installation method above.
        pause
        exit /b 1
    )
    
    echo C++ workload installed successfully!
    echo Waiting for installation to complete...
    timeout /t 30 /nobreak >nul
    
    goto :build_plugin
) else (
    echo Not running as Administrator.
    echo Please use one of the options above to install C++ tools.
    echo.
    echo After installing C++ tools, run this script again.
    pause
    exit /b 0
)

:build_plugin
echo.
echo ================================================
echo    Building LandscapeAcousticVST Plugin...
echo ================================================
echo.

:: Set up Visual Studio environment
call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64

:: Add CMake to PATH
set PATH=%PATH%;C:\Program Files\CMake\bin

:: Verify compiler is available
cl >nul 2>&1
if errorlevel 1 (
    echo ERROR: Visual Studio C++ compiler not found!
    echo Please ensure "Desktop development with C++" is installed.
    echo.
    echo To install:
    echo 1. Open "Visual Studio Installer"  
    echo 2. Click "Modify" for Visual Studio Community 2022
    echo 3. Check "Desktop development with C++"
    echo 4. Click "Modify"
    pause
    exit /b 1
)

echo C++ compiler found! Proceeding with build...

:: Clean and create build directory
if exist build rmdir /s /q build
mkdir build
cd build

:: Configure CMake with Visual Studio generator
echo Configuring CMake...
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 (
    echo CMake configuration failed!
    echo.
    echo Troubleshooting:
    echo - Ensure Visual Studio Community 2022 is fully installed
    echo - Verify "Desktop development with C++" workload is installed
    echo - Check that CMake can find Visual Studio
    pause
    exit /b 1
)

:: Build the plugin
echo Building LandscapeAcousticVST...
cmake --build . --config Release --parallel %NUMBER_OF_PROCESSORS%
if errorlevel 1 (
    echo Build failed!
    pause
    exit /b 1
)

echo.
echo ================================================
echo      Build Successful! Installing Plugin...
echo ================================================

:: Find and install VST3
for /r . %%f in (*.vst3) do (
    echo Found VST3 plugin: %%f
    
    :: Try system directory first
    if not exist "%COMMONPROGRAMFILES%\VST3\" mkdir "%COMMONPROGRAMFILES%\VST3\"
    xcopy /Y /E "%%f" "%COMMONPROGRAMFILES%\VST3\" >nul 2>&1
    
    if not errorlevel 1 (
        echo ✓ Installed to system directory: %COMMONPROGRAMFILES%\VST3\
    ) else (
        :: Fall back to user directory
        if not exist "%APPDATA%\VST3\" mkdir "%APPDATA%\VST3\"
        xcopy /Y /E "%%f" "%APPDATA%\VST3\" >nul
        echo ✓ Installed to user directory: %APPDATA%\VST3\
    )
)

echo.
echo ================================================
echo    🎉 LandscapeAcousticVST Installation Complete! 
echo ================================================
echo.
echo ✓ Plugin built successfully
echo ✓ Plugin installed to VST3 directory  
echo ✓ Ready to use in your DAW
echo.
echo Next Steps:
echo 1. Open your DAW (Reaper, Ableton, FL Studio, etc.)
echo 2. Rescan for new plugins
echo 3. Load "LandscapeAcousticVST" as an audio effect
echo 4. Try the demo terrain or load your own DEM files
echo.
echo Documentation:
echo - USER_GUIDE.md - Complete usage instructions
echo - QUICK_REFERENCE.md - Quick start guide
echo - QGIS_PLUGIN_GUIDE.md - Advanced terrain workflows
echo.
echo Happy music making with real landscapes! 🌄🎵
echo.
pause