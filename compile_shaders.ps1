Write-Host "Compiling Slang shaders to SPIR-V..."

# Compile both vertex and fragment shaders into a single SPIR-V file with multiple entry points
& "c:\Users\rohan\Desktop\Physics-Engine\vcpkg\packages\shader-slang_x64-windows\tools\shader-slang\slangc.exe" -target spirv -entry vertMain -entry fragMain -o "shaders\triangle.spv" "shaders slang\triangle.slang"

Write-Host "Compilation complete!"
Write-Host "Generated: shaders slang\triangle.spv (contains both vertex and fragment entry points)"