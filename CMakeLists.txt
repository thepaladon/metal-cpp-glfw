cmake_minimum_required(VERSION 3.30)
project(metal_cpp_test LANGUAGES CXX OBJCXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_OBJCXX_STANDARD 17)

add_executable(metal_cpp_test
    src/main.cpp
)

# add dependencies
include(cmake/CPM.cmake)

CPMAddPackage("gh:glfw/glfw#3.4")

CPMAddPackage(
    NAME imgui
    GITHUB_REPOSITORY ocornut/imgui
    GIT_TAG v1.92.5
)

CPMAddPackage(
    NAME     metal-cpp
    URL      https://github.com/bkaradzic/metal-cpp/archive/refs/heads/metal-cpp_macOS15.2_iOS18.2.tar.gz
    URL_HASH SHA256=6b3c979339a74f4480e8c5ecfa427dfedcf2e9b3a66d0263c3e6de3f5b63bca8
)

add_library(dear_imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp

    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_metal.mm
)

target_include_directories(dear_imgui PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends

    # So <Metal/Metal.hpp> resolves (comes from metal-cpp, not Metal.framework)
    ${metal-cpp_SOURCE_DIR}
    ${metal-cpp_SOURCE_DIR}/metal-cpp
)

target_compile_definitions(dear_imgui PUBLIC IMGUI_IMPL_METAL_CPP)

target_link_libraries(dear_imgui PUBLIC
    glfw
    "-framework Foundation"
    "-framework QuartzCore"
    "-framework Metal"
)

target_include_directories(metal_cpp_test PRIVATE
    ${glfw_SOURCE_DIR}/include
    ${metal-cpp_SOURCE_DIR}
    src
    src/metal-cpp-extensions
)

target_link_libraries(metal_cpp_test PRIVATE
    dear_imgui
)
