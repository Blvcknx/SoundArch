@echo off
echo Building OpenCV Hillshade Viewer...
echo.

REM Set up Visual Studio environment
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" 2>nul
if errorlevel 1 (
    echo Warning: Could not set up Visual Studio environment automatically
    echo Please run this from a Visual Studio Developer Command Prompt
)

REM Create build directory
if not exist build mkdir build
cd build

REM Configure with CMake
echo Configuring with CMake...
cmake .. -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 (
    echo.
    echo CMake configuration failed!
    echo.
    echo Make sure you have:
    echo 1. OpenCV installed (try: vcpkg install opencv4)
    echo 2. Visual Studio 2022 with C++ support
    echo 3. CMake 3.15+
    echo.
    pause
    exit /b 1
)

REM Build
echo.
echo Building...
cmake --build . --config Release
if errorlevel 1 (
    echo.
    echo Build failed!
    pause
    exit /b 1
)

echo.
echo Build completed successfully!
echo.
echo To run the viewer:
echo   cd bin
echo   .\opencv_hillshade_viewer.exe path\to\hillshade.tif
echo.
echo To generate hillshade from DEM first:
echo   gdaldem hillshade input_dem.tif output_hillshade.tif
echo.
pause