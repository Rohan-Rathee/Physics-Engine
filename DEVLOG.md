# Physics Engine Development Log

## Project Overview
A C++ physics engine project using OpenGL, GLFW, and GLM for 3D graphics rendering. Built with CMake and vcpkg for dependency management.

## Development Timeline

### Initial Setup (May 11, 2026)
- **Project Structure**: Created basic CMake project with OpenGL support
- **Dependencies**: Integrated GLFW, GLAD, GLM via vcpkg
- **Build System**: Set up CMakeLists.txt with proper include paths and linking
- **Shaders**: Basic vertex and fragment shaders for triangle rendering

### Repo Cleanup (May 16, 2026)
- **Submodule Removal**: Removed the vcpkg submodule to keep the repo lightweight
- **Ignore Rules**: Added ignores for generated/output folders (build, glad, .vscode, vcpkg)

### Code Development
- **Main Application**: `main.cpp` - OpenGL window creation, shader loading, triangle rendering
- **Shader Class**: `shader.h` - Utility class for loading, compiling, and using GLSL shaders
- **Error Handling**: Added comprehensive error checking for shader compilation and linking

### Build Fixes
- **Compilation Errors**: Fixed syntax issues in `shader.h` (missing semicolon after constructor)
- **Linker Issues**: Resolved OpenGL function loading with GLAD
- **Build Configuration**: Ensured proper CMake configuration for Release builds

### Version Control & Distribution
- **Git Setup**: Initialized repository with proper `.gitignore`
- **vcpkg Integration**: Converted to manifest mode with `vcpkg.json`
- **GitHub Integration**: Pushed to https://github.com/Rohan-Rathee/Physics-Engine
- **Automated Releases**: Set up GitHub Actions workflow for automatic builds and releases
- **Release v1.0.0**: Created first release with Windows executable

### Current Features
- ✅ OpenGL 3.3 Core Profile rendering
- ✅ GLFW window management
- ✅ Custom shader support
- ✅ Colored triangle rendering
- ✅ Cross-platform build system
- ✅ Automated CI/CD pipeline

### Technical Stack
- **Language**: C++20
- **Graphics**: OpenGL 3.3, GLAD
- **Windowing**: GLFW
- **Math**: GLM
- **Build**: CMake
- **Dependencies**: vcpkg
- **Version Control**: Git + GitHub
- **CI/CD**: GitHub Actions

### Next Steps
- [ ] Add physics simulation (collision detection, rigid bodies)
- [ ] Implement more complex 3D models
- [ ] Add texture loading and mapping
- [ ] Create Vulkan renderer alternative
- [ ] Add input handling for interactive physics
- [ ] Implement camera controls
- [ ] Add UI overlay for physics parameters

### Build Instructions
```bash
# Clone repository
git clone https://github.com/Rohan-Rathee/Physics-Engine.git
cd Physics-Engine

# Install dependencies
vcpkg install

# Configure and build
cmake -S . -B build
cmake --build build --config Release

# Run
./build/Release/main.exe
```

### Release Notes
- **v1.0.1**: Automated release pipeline setup
- **v1.0.0**: Initial release with basic triangle rendering

---
*Development log maintained as of May 11, 2026*