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
find_package(Vulkan REQUIRED)
find_package(glm CONFIG REQUIRED)

# Add executables
add_executable(main src/main.cpp)

# Link libraries
target_link_libraries(main PRIVATE 
    glfw
    glad::glad
    Vulkan::Vulkan
    glm::glm
)
 