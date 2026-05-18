cmake_minimum_required(VERSION 3.10)

if (NOT DEFINED CMAKE_TOOLCHAIN_FILE AND DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
endif()

project(Physics-Engine)

# Set C++ standard to 20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find packages (provided by vcpkg toolchain)
find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)

# Add executables
add_executable(main 
    src/main.cpp
    src/core/engine.cpp
    src/core/time_manager.cpp
    src/systems/input_system.cpp
    src/systems/render_system.cpp
    src/systems/window_system.cpp
    src/scene/entity.cpp
    src/scene/scene.cpp
    src/components/transform.cpp
)

# Link libraries
target_link_libraries(main PRIVATE 
    glfw
    glad::glad
    glm::glm
    assimp::assimp
)
 