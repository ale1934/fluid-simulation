cmake_minimum_required(VERSION 3.20)

project(SPH-simulation
    VERSION 1.0
    DESCRIPTION "2D Smoothed Particle Hydrodynamics Simulation"
    LANGUAGES CXX
)

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/rlImGui/imgui)
set(RLIMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/rlImGui)

# Executable source files
add_executable(sph-simulation
    src/main.cpp
	src/fluidsimulation.cpp
	src/particlerenderer.cpp
)

target_sources(sph-simulation PRIVATE
    ${RLIMGUI_DIR}/rlImGui.cpp

    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
)

# Include project headers
target_include_directories(sph-simulation PRIVATE
	${RLIMGUI_DIR}
    ${IMGUI_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Find raylib installed on the system
find_package(raylib CONFIG REQUIRED)
find_package(OpenMP REQUIRED)

target_link_libraries(sph-simulation PRIVATE OpenMP::OpenMP_CXX)
target_link_libraries(sph-simulation PRIVATE raylib)

# Compiler warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(sph-simulation PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()

