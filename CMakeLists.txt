cmake_minimum_required(VERSION 3.30)
project(metal_cpp_test)

set(CMAKE_CXX_STANDARD 17)

add_executable(metal_cpp_test src/main.cpp)

# add dependencies
include(cmake/CPM.cmake)

CPMAddPackage("gh:glfw/glfw#3.4")

CPMAddPackage(
    NAME     metal-cpp
    URL      https://github.com/bkaradzic/metal-cpp/archive/refs/heads/metal-cpp_macOS15.2_iOS18.2.tar.gz
    URL_HASH SHA256=6b3c979339a74f4480e8c5ecfa427dfedcf2e9b3a66d0263c3e6de3f5b63bca8
)

target_include_directories(metal_cpp_test PRIVATE
    ${glfw_SOURCE_DIR}
    ${metal-cpp_SOURCE_DIR}
    src
    src/metal-cpp-extensions
)
target_link_libraries(metal_cpp_test
    glfw
    "-framework Foundation"
    "-framework QuartzCore"
    "-framework Metal"
)

