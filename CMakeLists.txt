cmake_minimum_required(VERSION 3.30)
project(metal_cpp_test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(APPLE)
    enable_language(OBJCXX)
    set(CMAKE_OBJCXX_STANDARD 17)
endif()

add_executable(metal_cpp_test
    src/main.cpp
    src/render/renderer_factory.cpp
)

add_library(gpu_api STATIC
    src/gpu/gpu_api_stub.cpp
)

target_include_directories(gpu_api PUBLIC
    src
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
)

target_include_directories(dear_imgui PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)
target_link_libraries(dear_imgui PUBLIC
    glfw
)

if(APPLE)
    target_sources(metal_cpp_test PRIVATE
        src/render/renderer_metal.cpp
    )
    target_sources(dear_imgui PRIVATE
        ${imgui_SOURCE_DIR}/backends/imgui_impl_metal.mm
    )
    target_compile_definitions(dear_imgui PUBLIC IMGUI_IMPL_METAL_CPP)
    target_include_directories(dear_imgui PUBLIC
        # So <Metal/Metal.hpp> resolves (comes from metal-cpp, not Metal.framework)
        ${metal-cpp_SOURCE_DIR}
        ${metal-cpp_SOURCE_DIR}/metal-cpp
    )
    target_link_libraries(dear_imgui PUBLIC
        "-framework Foundation"
        "-framework QuartzCore"
        "-framework Metal"
    )
else()
    target_sources(metal_cpp_test PRIVATE
        src/render/renderer_opengl.cpp
    )
    find_package(OpenGL REQUIRED)
    target_sources(dear_imgui PRIVATE
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    )
    target_link_libraries(dear_imgui PUBLIC OpenGL::GL)
endif()

target_include_directories(metal_cpp_test PRIVATE
    ${glfw_SOURCE_DIR}/include
    src
)
if(APPLE)
    target_include_directories(metal_cpp_test PRIVATE
        ${metal-cpp_SOURCE_DIR}
        src/metal-cpp-extensions
    )
endif()

target_link_libraries(metal_cpp_test PRIVATE
    gpu_api
    dear_imgui
)
