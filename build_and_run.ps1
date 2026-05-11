# Build and run script for Physics-Engine
cd "c:\Users\rohan\Desktop\Physics-Engine"
cmake --build build --config Release --target main_opengl
if ($LASTEXITCODE -eq 0) {
    & ".\build\Release\main_opengl.exe"
} else {
    Write-Host "Build failed, not running exe."
}