cmake_minimum_required(VERSION 3.10)
project(Physics-Engine)

# Set C++ standard to 20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set CMAKE_PREFIX_PATH to vcpkg installed directory
set(CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/installed/x64-windows" ${CMAKE_PREFIX_PATH})

# Find packages
find_package(glfw3 CONFIG REQUIRED)
find_package(Vulkan REQUIRED)
find_package(glm CONFIG REQUIRED)

# Include directories globally
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/glad/include")

# Add executables
add_executable(main src/main.cpp glad/src/glad.c)

# Include directories for header-only libraries
target_include_directories(main PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/installed/x64-windows/include"
)

# Link libraries
target_link_libraries(main PRIVATE 
    glfw 
    Vulkan::Vulkan 
    glm::glm
)
 