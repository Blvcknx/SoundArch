@echo off
REM Build script for DEM Viewer with Qt5

echo Setting up Visual Studio Developer Environment...
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

if %errorlevel% neq 0 (
    echo Failed to initialize Visual Studio environment
    pause
    exit /b 1
)

echo Visual Studio environment initialized successfully

REM Set Qt5 path - ADJUST THIS TO YOUR QT INSTALLATION
set Qt5_DIR=C:\Qt\5.15.2\msvc2019_64\lib\cmake\Qt5
set CMAKE_PREFIX_PATH=C:\Qt\5.15.2\msvc2019_64

echo Qt5 path: %Qt5_DIR%

REM Create build directory
if not exist build mkdir build
cd build

echo Configuring CMake with Visual Studio 2022...
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH=%CMAKE_PREFIX_PATH%

if %errorlevel% neq 0 (
    echo CMake configuration failed
    cd ..
    pause
    exit /b 1
)

echo Configuration successful. Building DEM Viewer...
cmake --build . --config Release

if %errorlevel% neq 0 (
    echo Build failed
    cd ..
    pause
    exit /b 1
)

cd ..

echo.
echo ================================================
echo   DEM Viewer Build Complete!
echo ================================================
echo.
echo Executable location:
echo   build\Release\DEMViewer.exe
echo.
echo To run the viewer:
echo   cd build\Release
echo   .\DEMViewer.exe
echo.

pause
