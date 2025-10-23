#!/usr/bin/env powershell
# LandscapeAcousticVST Automated Build Script
# This script attempts to build the project with multiple fallback options

param(
    [string]$BuildType = "Release",
    [switch]$SkipTests = $false,
    [switch]$Verbose = $false
)

$ErrorActionPreference = "Continue"
$ProjectRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$BuildDir = Join-Path $ProjectRoot "build"

Write-Host "LandscapeAcousticVST Automated Build" -ForegroundColor Green
Write-Host "====================================" -ForegroundColor Green
Write-Host "Project Root: $ProjectRoot" -ForegroundColor Cyan
Write-Host "Build Type: $BuildType" -ForegroundColor Cyan
Write-Host "Skip Tests: $SkipTests" -ForegroundColor Cyan

# Function to test if a command exists
function Test-Command($Command) {
    try {
        Get-Command $Command -ErrorAction Stop | Out-Null
        return $true
    } catch {
        return $false
    }
}

# Function to run command with error handling
function Invoke-BuildCommand($Command, $Arguments, $Description) {
    Write-Host "`n$Description..." -ForegroundColor Yellow
    
    if ($Verbose) {
        Write-Host "Executing: $Command $Arguments" -ForegroundColor Gray
    }
    
    try {
        $process = Start-Process -FilePath $Command -ArgumentList $Arguments -NoNewWindow -Wait -PassThru -RedirectStandardOutput "stdout.log" -RedirectStandardError "stderr.log"
        
        if ($process.ExitCode -eq 0) {
            Write-Host "✓ $Description completed successfully" -ForegroundColor Green
            return $true
        } else {
            Write-Host "✗ $Description failed (Exit Code: $($process.ExitCode))" -ForegroundColor Red
            if (Test-Path "stderr.log") {
                $errorContent = Get-Content "stderr.log" -Raw
                if ($errorContent -and $Verbose) {
                    Write-Host "Error details:" -ForegroundColor Red
                    Write-Host $errorContent -ForegroundColor Red
                }
            }
            return $false
        }
    } catch {
        Write-Host "✗ $Description failed with exception: $($_.Exception.Message)" -ForegroundColor Red
        return $false
    }
}

# Step 1: Verify prerequisites
Write-Host "`n1. Checking Prerequisites..." -ForegroundColor Cyan

$prerequisites = @{
    "CMake" = @("C:\Program Files\CMake\bin\cmake.exe", "cmake")
    "GDAL" = @("C:\Program Files\GDAL\gdalinfo.exe")
}

$missingPrereqs = @()

foreach ($prereq in $prerequisites.GetEnumerator()) {
    $found = $false
    foreach ($path in $prereq.Value) {
        if (Test-Path $path) {
            Write-Host "✓ $($prereq.Key): $path" -ForegroundColor Green
            $found = $true
            break
        } elseif (Test-Command $path) {
            Write-Host "✓ $($prereq.Key): Available in PATH" -ForegroundColor Green
            $found = $true
            break
        }
    }
    
    if (-not $found) {
        Write-Host "✗ $($prereq.Key): Not found" -ForegroundColor Red
        $missingPrereqs += $prereq.Key
    }
}

if ($missingPrereqs.Count -gt 0) {
    Write-Host "`nMissing prerequisites: $($missingPrereqs -join ', ')" -ForegroundColor Red
    Write-Host "Please run setup_build_environment.ps1 first" -ForegroundColor Yellow
    exit 1
}

# Step 2: Setup environment
Write-Host "`n2. Setting Up Environment..." -ForegroundColor Cyan

$env:GDAL_ROOT = "C:\Program Files\GDAL"
$env:GDAL_DATA = "C:\Program Files\GDAL\gdal-data"
$env:PATH = "C:\Program Files\CMake\bin;C:\Program Files\GDAL;$env:PATH"

Write-Host "✓ Environment variables configured" -ForegroundColor Green

# Step 3: Prepare build directory
Write-Host "`n3. Preparing Build Directory..." -ForegroundColor Cyan

if (Test-Path $BuildDir) {
    Write-Host "Cleaning existing build directory..."
    Remove-Item $BuildDir -Recurse -Force
}

New-Item -ItemType Directory -Path $BuildDir -Force | Out-Null
Set-Location $BuildDir

Write-Host "✓ Build directory prepared: $BuildDir" -ForegroundColor Green

# Step 4: Configure build system
Write-Host "`n4. Configuring Build System..." -ForegroundColor Cyan

$cmakePath = if (Test-Path "C:\Program Files\CMake\bin\cmake.exe") { "C:\Program Files\CMake\bin\cmake.exe" } else { "cmake" }

# Try different generators in order of preference
$generators = @(
    @{Name="Visual Studio 17 2022"; Args=@("-G", "Visual Studio 17 2022", "-A", "x64")},
    @{Name="MinGW Makefiles"; Args=@("-G", "MinGW Makefiles")},
    @{Name="Unix Makefiles"; Args=@("-G", "Unix Makefiles")},
    @{Name="Ninja"; Args=@("-G", "Ninja")}
)

$configSuccess = $false
$selectedGenerator = $null

foreach ($generator in $generators) {
    Write-Host "Trying generator: $($generator.Name)..."
    
    $cmakeArgs = $generator.Args + @(
        "-DCMAKE_BUILD_TYPE=$BuildType",
        "-DGDAL_ROOT=C:\Program Files\GDAL",
        "-DGDAL_INCLUDE_DIR=C:\Program Files\GDAL",
        "-DGDAL_LIBRARY=C:\Program Files\GDAL\gdal.dll",
        ".."
    )
    
    if (Invoke-BuildCommand $cmakePath $cmakeArgs "CMake configuration with $($generator.Name)") {
        $configSuccess = $true
        $selectedGenerator = $generator.Name
        Write-Host "✓ Configuration successful with $($generator.Name)" -ForegroundColor Green
        break
    }
}

if (-not $configSuccess) {
    Write-Host "✗ All CMake generators failed" -ForegroundColor Red
    Write-Host "Please check build environment setup" -ForegroundColor Yellow
    exit 1
}

# Step 5: Build the project
Write-Host "`n5. Building Project..." -ForegroundColor Cyan

$buildArgs = @("--build", ".", "--config", $BuildType)
if ($selectedGenerator -eq "MinGW Makefiles" -or $selectedGenerator -eq "Unix Makefiles" -or $selectedGenerator -eq "Ninja") {
    $buildArgs += @("--parallel", "4")
}

$buildSuccess = Invoke-BuildCommand $cmakePath $buildArgs "Project compilation"

if (-not $buildSuccess) {
    Write-Host "✗ Build failed" -ForegroundColor Red
    exit 1
}

# Step 6: Run tests (optional)
if (-not $SkipTests) {
    Write-Host "`n6. Running Tests..." -ForegroundColor Cyan
    
    # Run Python validation tests
    if (Test-Command "python") {
        Set-Location $ProjectRoot
        
        if (Test-Path "scripts\validation\compare_iso9613.py") {
            Write-Host "Running ISO 9613-2 validation..."
            $testSuccess = Invoke-BuildCommand "python" @("scripts\validation\compare_iso9613.py") "ISO 9613-2 validation"
        }
        
        Set-Location $BuildDir
    }
    
    # Run C++ unit tests if available
    $testExecutables = Get-ChildItem -Recurse -Name "*Test*.exe" -ErrorAction SilentlyContinue
    foreach ($testExe in $testExecutables) {
        Write-Host "Running C++ unit tests: $testExe"
        Invoke-BuildCommand $testExe @() "C++ unit tests"
    }
} else {
    Write-Host "`n6. Skipping Tests (--SkipTests specified)" -ForegroundColor Yellow
}

# Step 7: Summary
Write-Host "`n7. Build Summary" -ForegroundColor Cyan

$artifacts = @()

# Find built artifacts
$vstFiles = Get-ChildItem -Recurse -Name "*.vst3" -ErrorAction SilentlyContinue
$auFiles = Get-ChildItem -Recurse -Name "*.component" -ErrorAction SilentlyContinue  
$exeFiles = Get-ChildItem -Recurse -Name "LandscapeAcousticVST*.exe" -ErrorAction SilentlyContinue

if ($vstFiles) {
    $artifacts += "VST3 Plugin: $($vstFiles -join ', ')"
}
if ($auFiles) {
    $artifacts += "AU Plugin: $($auFiles -join ', ')"
}
if ($exeFiles) {
    $artifacts += "Standalone: $($exeFiles -join ', ')"
}

Write-Host "`nBuild Results:" -ForegroundColor Green
Write-Host "Generator Used: $selectedGenerator" -ForegroundColor White
Write-Host "Build Type: $BuildType" -ForegroundColor White
Write-Host "Build Directory: $BuildDir" -ForegroundColor White

if ($artifacts.Count -gt 0) {
    Write-Host "`nBuilt Artifacts:" -ForegroundColor Green
    foreach ($artifact in $artifacts) {
        Write-Host "  $artifact" -ForegroundColor White
    }
} else {
    Write-Host "`nNo plugin artifacts found - check build configuration" -ForegroundColor Yellow
}

Write-Host "`n✓ Build process completed!" -ForegroundColor Green
Write-Host "`nNext steps:" -ForegroundColor Cyan
Write-Host "1. Install the VST3 plugin to your DAW's plugin directory" -ForegroundColor White
Write-Host "2. Load terrain data (DEM files) into the plugin" -ForegroundColor White  
Write-Host "3. Test real-time acoustic simulation" -ForegroundColor White

# Return to original directory
Set-Location $ProjectRoot

exit 0