# LandscapeAcousticVST Build Environment Setup Script
# Run this script as Administrator in PowerShell

Write-Host "LandscapeAcousticVST Build Environment Setup" -ForegroundColor Green
Write-Host "=============================================" -ForegroundColor Green

# Check if running as Administrator
if (-NOT ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole] "Administrator")) {
    Write-Host "ERROR: This script must be run as Administrator!" -ForegroundColor Red
    Write-Host "Right-click PowerShell and select 'Run as Administrator'" -ForegroundColor Yellow
    exit 1
}

Write-Host "`n1. Checking existing installations..." -ForegroundColor Cyan

# Check CMake
try {
    $cmakeVersion = & "C:\Program Files\CMake\bin\cmake.exe" --version 2>$null
    Write-Host "✓ CMake: $($cmakeVersion.Split("`n")[0])" -ForegroundColor Green
} catch {
    Write-Host "✗ CMake not found" -ForegroundColor Red
}

# Check GDAL
try {
    $gdalVersion = & "C:\Program Files\GDAL\gdalinfo.exe" --version 2>$null
    Write-Host "✓ GDAL: $gdalVersion" -ForegroundColor Green
} catch {
    Write-Host "✗ GDAL not found" -ForegroundColor Red
}

Write-Host "`n2. Installing Visual Studio Build Tools..." -ForegroundColor Cyan

# Method 1: Try winget installation with C++ workload
try {
    Write-Host "Attempting winget installation with C++ workload..."
    winget install Microsoft.VisualStudio.2022.BuildTools --override "--wait --add Microsoft.VisualStudio.Workload.VCTools --add Microsoft.VisualStudio.Component.VC.Tools.x86.x64 --add Microsoft.VisualStudio.Component.Windows11SDK.22000" --accept-source-agreements --accept-package-agreements
    Write-Host "✓ Visual Studio Build Tools installation completed" -ForegroundColor Green
} catch {
    Write-Host "⚠ Winget installation failed, trying alternative methods..." -ForegroundColor Yellow
    
    # Method 2: Try LLVM/Clang
    try {
        Write-Host "Installing LLVM/Clang as alternative compiler..."
        winget install LLVM.LLVM --accept-source-agreements --accept-package-agreements
        Write-Host "✓ LLVM/Clang installed" -ForegroundColor Green
    } catch {
        Write-Host "⚠ LLVM installation failed" -ForegroundColor Yellow
    }
    
    # Method 3: Try MinGW-w64
    try {
        Write-Host "Installing MSYS2 with MinGW-w64..."
        winget install MSYS2.MSYS2 --accept-source-agreements --accept-package-agreements
        
        # Install MinGW toolchain via MSYS2
        Write-Host "Installing MinGW development tools..."
        & C:\msys64\usr\bin\bash.exe -l -c "pacman -S --noconfirm mingw-w64-x86_64-toolchain mingw-w64-x86_64-cmake mingw-w64-x86_64-ninja mingw-w64-x86_64-gdal"
        Write-Host "✓ MinGW-w64 toolchain installed" -ForegroundColor Green
    } catch {
        Write-Host "✗ MinGW installation failed" -ForegroundColor Red
    }
}

Write-Host "`n3. Setting up environment variables..." -ForegroundColor Cyan

# Add tools to PATH
$pathAdditions = @(
    "C:\Program Files\CMake\bin",
    "C:\Program Files\GDAL",
    "C:\msys64\mingw64\bin",
    "C:\Program Files\LLVM\bin"
)

$currentPath = [Environment]::GetEnvironmentVariable("PATH", "Machine")
foreach ($addition in $pathAdditions) {
    if (Test-Path $addition) {
        if ($currentPath -notlike "*$addition*") {
            Write-Host "Adding to PATH: $addition"
            $currentPath += ";$addition"
        }
    }
}

# Set environment variables
[Environment]::SetEnvironmentVariable("PATH", $currentPath, "Machine")
[Environment]::SetEnvironmentVariable("GDAL_ROOT", "C:\Program Files\GDAL", "Machine")
[Environment]::SetEnvironmentVariable("GDAL_DATA", "C:\Program Files\GDAL\gdal-data", "Machine")

Write-Host "✓ Environment variables configured" -ForegroundColor Green

Write-Host "`n4. Testing build tools..." -ForegroundColor Cyan

# Test compilers
$compilerFound = $false

# Test MSVC
try {
    $vsPath = Get-ChildItem "C:\Program Files\Microsoft Visual Studio\2022" -Directory -ErrorAction SilentlyContinue
    if ($vsPath) {
        $vcvarsall = Get-ChildItem $vsPath.FullName -Recurse -Name "vcvarsall.bat" -ErrorAction SilentlyContinue | Select-Object -First 1
        if ($vcvarsall) {
            Write-Host "✓ Visual Studio C++ compiler available" -ForegroundColor Green
            $compilerFound = $true
        }
    }
} catch {}

# Test Clang
try {
    $clangVersion = & clang --version 2>$null
    if ($clangVersion) {
        Write-Host "✓ Clang compiler available" -ForegroundColor Green
        $compilerFound = $true
    }
} catch {}

# Test MinGW
try {
    $gccVersion = & C:\msys64\mingw64\bin\gcc.exe --version 2>$null
    if ($gccVersion) {
        Write-Host "✓ MinGW GCC compiler available" -ForegroundColor Green
        $compilerFound = $true
    }
} catch {}

if (-not $compilerFound) {
    Write-Host "⚠ No C++ compiler found. Manual installation may be required." -ForegroundColor Yellow
}

Write-Host "`n5. CMake Configuration Test..." -ForegroundColor Cyan

# Navigate to project directory
$projectDir = "C:\Users\marco\OneDrive\Skrivebord\Sound\SoundArch\LandscapeAcousticVST"
if (Test-Path $projectDir) {
    Set-Location $projectDir
    
    # Create/clean build directory
    if (Test-Path "build") {
        Remove-Item "build" -Recurse -Force
    }
    New-Item -ItemType Directory -Name "build" | Out-Null
    Set-Location "build"
    
    # Try different CMake generators
    $generators = @(
        "Visual Studio 17 2022",
        "MinGW Makefiles", 
        "Unix Makefiles",
        "Ninja"
    )
    
    $success = $false
    foreach ($generator in $generators) {
        try {
            Write-Host "Testing CMake with generator: $generator"
            & "C:\Program Files\CMake\bin\cmake.exe" -G $generator .. 2>$null
            if ($LASTEXITCODE -eq 0) {
                Write-Host "✓ CMake configuration successful with $generator" -ForegroundColor Green
                $success = $true
                break
            }
        } catch {
            Write-Host "✗ $generator failed" -ForegroundColor Red
        }
    }
    
    if (-not $success) {
        Write-Host "⚠ CMake configuration failed with all generators" -ForegroundColor Yellow
    }
    
    Set-Location $projectDir
} else {
    Write-Host "✗ Project directory not found: $projectDir" -ForegroundColor Red
}

Write-Host "`n6. Summary and Next Steps..." -ForegroundColor Cyan

Write-Host "`nSetup completed! Next steps:" -ForegroundColor Green
Write-Host "1. Restart PowerShell to pick up new environment variables" -ForegroundColor White
Write-Host "2. Navigate to project directory:" -ForegroundColor White
Write-Host "   cd '$projectDir'" -ForegroundColor Gray
Write-Host "3. Create and enter build directory:" -ForegroundColor White
Write-Host "   mkdir build; cd build" -ForegroundColor Gray
Write-Host "4. Run CMake configuration:" -ForegroundColor White
Write-Host "   cmake -G `"Visual Studio 17 2022`" .." -ForegroundColor Gray
Write-Host "   # OR for MinGW:" -ForegroundColor Gray
Write-Host "   cmake -G `"MinGW Makefiles`" .." -ForegroundColor Gray
Write-Host "5. Build the project:" -ForegroundColor White
Write-Host "   cmake --build . --config Release" -ForegroundColor Gray

if ($compilerFound) {
    Write-Host "`n✓ Build environment setup appears successful!" -ForegroundColor Green
} else {
    Write-Host "`n⚠ Build environment setup incomplete. Manual compiler installation may be needed." -ForegroundColor Yellow
    Write-Host "Consider installing Visual Studio Community 2022 with C++ Desktop Development workload." -ForegroundColor White
}

Write-Host "`nPress any key to continue..."
$null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")