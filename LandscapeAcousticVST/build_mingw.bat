@echo off
echo Building LandscapeAcousticVST with MinGW-w64...
echo.

:: Set up MinGW-w64 paths
set PATH=C:\msys64\mingw64\bin;%PATH%

:: Check if compiler is available
gcc --version >nul 2>&1
if errorlevel 1 (
    echo Installing MinGW-w64 components...
    C:\msys64\mingw64.exe -lc "pacman -S --noconfirm mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-ninja mingw-w64-x86_64-gdal"
    if errorlevel 1 (
        echo Failed to install MinGW-w64 components.
        pause
        exit /b 1
    )
)

echo Compiler ready. Configuring build...

:: Create and enter build directory
if not exist build mkdir build
cd build

:: Configure with MinGW Makefiles
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 (
    echo CMake configuration failed.
    pause
    exit /b 1
)

echo Building plugin...
cmake --build . --config Release -j %NUMBER_OF_PROCESSORS%
if errorlevel 1 (
    echo Build failed.
    pause
    exit /b 1
)

echo Build successful!

:: Install plugin
echo Installing plugin...
if not exist "%COMMONPROGRAMFILES%\VST3\" mkdir "%COMMONPROGRAMFILES%\VST3\"

:: Find the VST3 file and copy it
for /r . %%f in (*.vst3) do (
    echo Found VST3: %%f
    copy /Y "%%f" "%COMMONPROGRAMFILES%\VST3\"
    if errorlevel 1 (
        echo Failed to copy to system directory, trying user directory...
        if not exist "%APPDATA%\VST3\" mkdir "%APPDATA%\VST3\"
        copy /Y "%%f" "%APPDATA%\VST3\"
    )
)

echo.
echo ================================
echo LandscapeAcousticVST Installation Complete!
echo ================================
echo.
echo The plugin has been built and installed.
echo You can now load it in your DAW.
echo.
echo Plugin locations:
echo - System: %COMMONPROGRAMFILES%\VST3\
echo - User:   %APPDATA%\VST3\
echo.
pause