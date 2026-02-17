cmake_minimum_required(VERSION 3.30)
# Required for EASTL/EABase CMake scripts when consumed by newer CMake versions.
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
project(metal_cpp_test VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(APPLE)
    enable_language(OBJCXX)
    set(CMAKE_OBJCXX_STANDARD 17)
endif()

option(AA_USE_STATIC_DEPS "Build dependencies as static libraries where supported" ON)
option(AA_FETCH_DEPS "Fetch dependencies with CPM when packages are missing" ON)
option(AA_STATIC_CRT "Use static MSVC runtime (/MT, /MTd)" OFF)
set(AA_IMGUI_SOURCE_DIR "" CACHE PATH "Path to a local imgui source checkout")
set(AA_OUTPUT_ROOT "${CMAKE_SOURCE_DIR}/build" CACHE PATH "Root directory for build outputs (Build/Intermediate)")

if(MSVC)
    if(AA_STATIC_CRT)
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    else()
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    endif()
endif()

if(AA_USE_STATIC_DEPS)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libs" FORCE)
endif()

if(CMAKE_CONFIGURATION_TYPES)
    set(AA_CONFIG_NAMES ${CMAKE_CONFIGURATION_TYPES})
else()
    if(CMAKE_BUILD_TYPE)
        set(AA_CONFIG_NAMES ${CMAKE_BUILD_TYPE})
    else()
        set(AA_CONFIG_NAMES Debug Release RelWithDebInfo MinSizeRel)
    endif()
endif()

foreach(AA_CONFIG_NAME ${AA_CONFIG_NAMES})
    string(TOUPPER "${AA_CONFIG_NAME}" AA_CONFIG_UPPER)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${AA_CONFIG_UPPER} "${AA_OUTPUT_ROOT}/Build/${AA_CONFIG_NAME}")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${AA_CONFIG_UPPER} "${AA_OUTPUT_ROOT}/Build/${AA_CONFIG_NAME}")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${AA_CONFIG_UPPER} "${AA_OUTPUT_ROOT}/Intermediate/${AA_CONFIG_NAME}/lib")
    set(CMAKE_PDB_OUTPUT_DIRECTORY_${AA_CONFIG_UPPER} "${AA_OUTPUT_ROOT}/Intermediate/${AA_CONFIG_NAME}/pdb")
    set(CMAKE_COMPILE_PDB_OUTPUT_DIRECTORY_${AA_CONFIG_UPPER} "${AA_OUTPUT_ROOT}/Intermediate/${AA_CONFIG_NAME}/pdb")
endforeach()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "_cmake")

include(cmake/aa_dependencies.cmake)

add_subdirectory(src/core)
add_subdirectory(src/gpu)
add_subdirectory(src/app)

# Ensure Visual Studio starts the actual app target, not ALL_BUILD.
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT metal_cpp_test)
