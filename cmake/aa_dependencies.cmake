function(aaSetFolderIfTarget targetName folderName)
    if(TARGET ${targetName})
        set_target_properties(${targetName} PROPERTIES FOLDER ${folderName})
    endif()
endfunction()

if(AA_FETCH_DEPS)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/CPM.cmake)
endif()

find_package(EASTL CONFIG QUIET)
if(NOT TARGET EASTL)
    if(AA_FETCH_DEPS)
        CPMAddPackage(
            NAME eastl
            GITHUB_REPOSITORY electronicarts/EASTL
            GIT_TAG 3.27.00
        )
    else()
        message(FATAL_ERROR "EASTL package not found. Install EASTL and set CMAKE_PREFIX_PATH, or set AA_FETCH_DEPS=ON.")
    endif()
endif()
aaSetFolderIfTarget(EASTL "dependencies/eastl")
aaSetFolderIfTarget(EABase "dependencies/eastl")

get_target_property(AA_EASTL_IS_IMPORTED EASTL IMPORTED)
set(AA_ENABLE_MODULE_PACKAGE_EXPORTS ON)
if(NOT AA_EASTL_IS_IMPORTED)
    set(AA_ENABLE_MODULE_PACKAGE_EXPORTS OFF)
    message(WARNING "Module package exports are disabled because EASTL is built in-tree. Install EASTL as a package and configure with AA_FETCH_DEPS=OFF to enable exported core/gpu_api packages.")
endif()

find_package(glfw3 CONFIG QUIET)
if(NOT TARGET glfw)
    if(AA_FETCH_DEPS)
        CPMAddPackage("gh:glfw/glfw#3.4")
    else()
        message(FATAL_ERROR "glfw package not found. Install glfw and set CMAKE_PREFIX_PATH, or set AA_FETCH_DEPS=ON.")
    endif()
endif()
aaSetFolderIfTarget(glfw "dependencies/glfw")
aaSetFolderIfTarget(update_mappings "dependencies/glfw")
aaSetFolderIfTarget(uninstall "dependencies/glfw")

if(NOT AA_IMGUI_SOURCE_DIR)
    if(AA_FETCH_DEPS)
        CPMAddPackage(
            NAME imgui
            GITHUB_REPOSITORY ocornut/imgui
            GIT_TAG v1.92.5-docking
        )
        set(AA_IMGUI_SOURCE_DIR ${imgui_SOURCE_DIR})
    else()
        message(FATAL_ERROR "imgui source not found. Set AA_IMGUI_SOURCE_DIR to a local checkout, or set AA_FETCH_DEPS=ON.")
    endif()
endif()

set(AA_METAL_CPP_INCLUDE_DIR "")
if(APPLE)
    find_path(AA_METAL_CPP_INCLUDE_DIR
        NAMES Metal/Metal.hpp
        PATH_SUFFIXES metal-cpp
    )

    if(NOT AA_METAL_CPP_INCLUDE_DIR)
        if(AA_FETCH_DEPS)
            CPMAddPackage(
                NAME metal-cpp
                URL https://github.com/bkaradzic/metal-cpp/archive/refs/heads/metal-cpp_macOS15.2_iOS18.2.tar.gz
                URL_HASH SHA256=6b3c979339a74f4480e8c5ecfa427dfedcf2e9b3a66d0263c3e6de3f5b63bca8
            )
            set(AA_METAL_CPP_INCLUDE_DIR ${metal-cpp_SOURCE_DIR}/metal-cpp)
        else()
            message(FATAL_ERROR "metal-cpp headers not found. Install metal-cpp and set CMAKE_PREFIX_PATH, or set AA_FETCH_DEPS=ON.")
        endif()
    endif()
endif()

add_library(dear_imgui STATIC
    ${AA_IMGUI_SOURCE_DIR}/imgui.cpp
    ${AA_IMGUI_SOURCE_DIR}/imgui_draw.cpp
    ${AA_IMGUI_SOURCE_DIR}/imgui_tables.cpp
    ${AA_IMGUI_SOURCE_DIR}/imgui_widgets.cpp
    ${AA_IMGUI_SOURCE_DIR}/imgui_demo.cpp
    ${AA_IMGUI_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
)
add_library(aa::dear_imgui ALIAS dear_imgui)

target_include_directories(dear_imgui PUBLIC
    ${AA_IMGUI_SOURCE_DIR}
    ${AA_IMGUI_SOURCE_DIR}/backends
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(dear_imgui PUBLIC glfw)

if(APPLE)
    target_sources(dear_imgui PRIVATE
        ${AA_IMGUI_SOURCE_DIR}/backends/imgui_impl_metal.mm
    )
    target_compile_definitions(dear_imgui PUBLIC IMGUI_IMPL_METAL_CPP)
    target_include_directories(dear_imgui PUBLIC
        ${AA_METAL_CPP_INCLUDE_DIR}
    )
    target_link_libraries(dear_imgui PUBLIC
        "-framework Foundation"
        "-framework QuartzCore"
        "-framework Metal"
    )
else()
    find_package(OpenGL REQUIRED)
    target_sources(dear_imgui PRIVATE
        ${AA_IMGUI_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    )
    target_link_libraries(dear_imgui PUBLIC OpenGL::GL)
endif()

aaSetFolderIfTarget(dear_imgui "dependencies/dear_imgui")
