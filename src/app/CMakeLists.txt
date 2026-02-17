add_executable(metal_cpp_test
    ${PROJECT_SOURCE_DIR}/src/main.cpp
    ${PROJECT_SOURCE_DIR}/src/render/renderer_factory.cpp
)

if(APPLE)
    target_sources(metal_cpp_test PRIVATE
        ${PROJECT_SOURCE_DIR}/src/render/renderer_metal.cpp
    )
else()
    target_sources(metal_cpp_test PRIVATE
        ${PROJECT_SOURCE_DIR}/src/render/renderer_opengl.cpp
    )
endif()

target_include_directories(metal_cpp_test PRIVATE
    ${PROJECT_SOURCE_DIR}/src
)

if(APPLE)
    target_include_directories(metal_cpp_test PRIVATE
        ${AA_METAL_CPP_INCLUDE_DIR}
        ${PROJECT_SOURCE_DIR}/src/metal-cpp-extensions
    )
endif()

target_link_libraries(metal_cpp_test PRIVATE
    gpu_api
    core
    dear_imgui
)

set_target_properties(metal_cpp_test PROPERTIES
    FOLDER "metal_cpp_test"
)
