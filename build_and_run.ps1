# Build and run script for Physics-Engine
$ErrorActionPreference = "Stop"
$projectRoot = "c:\Users\rohan\Desktop\Physics-Engine"
cd $projectRoot

# Set VCPKG_ROOT for vcpkg integration
$env:VCPKG_ROOT = "$projectRoot\vcpkg"

# Build the project
Write-Host "Building Physics-Engine..." -ForegroundColor Green
cmake --build build --config Release --target main

if ($LASTEXITCODE -eq 0) {
    Write-Host "Build successful! Running executable..." -ForegroundColor Green
    # Run from project root so relative paths to assets work
    & "$projectRoot\build\Release\main.exe"
} else {
    Write-Host "Build failed, not running exe." -ForegroundColor Red
    exit 1
}