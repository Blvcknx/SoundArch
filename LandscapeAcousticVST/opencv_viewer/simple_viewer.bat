@echo off
echo ================================================
echo   DEM Hillshade Viewer - Simple Workflow
echo ================================================
echo.

if "%~1"=="" (
    echo Usage: %0 ^<dem_file.asc^> or drag and drop a DEM file onto this script
    echo.
    echo This script will:
    echo 1. Generate a hillshade using GDAL
    echo 2. Open the result in your default image viewer
    echo.
    echo Supported DEM formats: .asc, .tif, .tiff
    echo.
    pause
    exit /b 1
)

set INPUT_FILE=%~1
set OUTPUT_FILE=%~dpn1_hillshade.tif

echo Input DEM: %INPUT_FILE%
echo Output hillshade: %OUTPUT_FILE%
echo.

REM Add GDAL to PATH
set PATH=%PATH%;"C:\Program Files\GDAL"

echo Generating hillshade using GDAL...
echo Command: gdaldem hillshade "%INPUT_FILE%" "%OUTPUT_FILE%"

gdaldem hillshade "%INPUT_FILE%" "%OUTPUT_FILE%"

if errorlevel 1 (
    echo.
    echo ERROR: Failed to generate hillshade!
    echo.
    echo Make sure:
    echo - GDAL is installed in "C:\Program Files\GDAL"
    echo - The input file exists and is a valid DEM
    echo - You have write permissions in the output directory
    echo.
    pause
    exit /b 1
)

echo.
echo Hillshade generated successfully!
echo.

REM Check if output file exists
if not exist "%OUTPUT_FILE%" (
    echo ERROR: Output file was not created!
    pause
    exit /b 1
)

echo Opening hillshade in default viewer...
start "" "%OUTPUT_FILE%"

echo.
echo Hillshade opened in your default image viewer.
echo You can now view the terrain relief visualization.
echo.
echo To view with different parameters, use:
echo gdaldem hillshade input.tif output.tif -z 1.5 -az 315 -alt 60
echo.
echo Press any key to close...
pause >nul