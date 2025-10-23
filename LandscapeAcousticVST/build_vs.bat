@echo off
echo Setting up Visual Studio Developer Environment...

:: Initialize Visual Studio Developer Command Prompt
call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"
if errorlevel 1 (
    echo Failed to initialize Visual Studio environment
    echo Trying alternative path...
    call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat"
    if errorlevel 1 (
        echo Visual Studio Build Tools not found
        echo Please install Visual Studio Build Tools with C++ workload
        pause
        exit /b 1
    )
)

echo Visual Studio environment initialized successfully

:: Add CMake to PATH
set PATH=%PATH%;C:\Program Files\CMake\bin

:: Navigate to build directory and clean
if not exist build mkdir build
cd build
del CMakeCache.txt 2>nul
rmdir /s /q CMakeFiles 2>nul

:: Configure with Visual Studio generator
echo Configuring CMake with Visual Studio 2022...
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 (
    echo CMake configuration failed
    pause
    exit /b 1
)

echo Configuration successful. Building plugin...
cmake --build . --config Release --parallel
if errorlevel 1 (
    echo Build failed
    pause
    exit /b 1
)

echo Build completed successfully!

:: Find and install the VST3 file
echo Installing plugin...
for /r . %%f in (*.vst3) do (
    echo Found VST3: %%f
    echo Copying to system directory...
    if not exist "%COMMONPROGRAMFILES%\VST3\" mkdir "%COMMONPROGRAMFILES%\VST3\"
    xcopy /Y /E "%%f" "%COMMONPROGRAMFILES%\VST3\" >nul
    if errorlevel 1 (
        echo Failed to copy to system directory, trying user directory...
        if not exist "%APPDATA%\VST3\" mkdir "%APPDATA%\VST3\"
        xcopy /Y /E "%%f" "%APPDATA%\VST3\" >nul
    )
)

echo.
echo ================================================
echo   LandscapeAcousticVST Installation Complete!
echo ================================================
echo.
echo The plugin has been successfully built and installed.
echo You can now load it in any compatible DAW.
echo.
echo Plugin locations:
echo - System: %COMMONPROGRAMFILES%\VST3\
echo - User:   %APPDATA%\VST3\
echo.
echo Next steps:
echo 1. Open your DAW (Reaper, Ableton, FL Studio, etc.)
echo 2. Rescan for plugins
echo 3. Load "LandscapeAcousticVST" as an audio effect
echo 4. Use demo terrain or load your own DEM files
echo.
pause