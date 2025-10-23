@echo off
echo Setting up build environment...

:: Set up Visual Studio environment
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
if errorlevel 1 (
    echo Visual Studio 2022 Community not found, trying Professional...
    call "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat"
    if errorlevel 1 (
        echo Visual Studio 2022 Professional not found, trying Enterprise...
        call "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
        if errorlevel 1 (
            echo Visual Studio 2022 not found. Please install Visual Studio 2022 with C++ tools.
            pause
            exit /b 1
        )
    )
)

echo Visual Studio environment set up successfully.

:: Configure CMake
echo Configuring CMake...
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
if errorlevel 1 (
    echo CMake configuration failed.
    pause
    exit /b 1
)

echo Configuration successful.

:: Build the plugin
echo Building plugin...
cmake --build . --config Release
if errorlevel 1 (
    echo Build failed.
    pause
    exit /b 1
)

echo Build successful!

:: Copy to VST3 directory
echo Installing plugin...
if not exist "%COMMONPROGRAMFILES%\VST3\" (
    mkdir "%COMMONPROGRAMFILES%\VST3\"
)

copy /Y ".\LandscapeAcousticVST_artefacts\Release\VST3\LandscapeAcousticVST.vst3" "%COMMONPROGRAMFILES%\VST3\"
if errorlevel 1 (
    echo Failed to install VST3 plugin. Trying to copy to user directory...
    if not exist "%APPDATA%\VST3\" (
        mkdir "%APPDATA%\VST3\"
    )
    copy /Y ".\LandscapeAcousticVST_artefacts\Release\VST3\LandscapeAcousticVST.vst3" "%APPDATA%\VST3\"
)

echo Plugin installation complete!
echo.
echo LandscapeAcousticVST has been built and installed successfully!
echo You can now load it in your DAW.
echo.
pause