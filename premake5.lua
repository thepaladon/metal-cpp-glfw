workspace "metalCppGlfw"
    architecture "x86_64"
    configurations { "Debug", "Release" }
    location "build/ProjectFiles/%{_ACTION}"
    startproject "metalCppTest"

    filter "configurations:Debug"
        symbols "On"
        runtime "Debug"
    filter "configurations:Release"
        optimize "Speed"
        runtime "Release"
    filter {}

local thirdPartyPath = "thirdParty"
local glfwPath = thirdPartyPath .. "/glfw"
local imguiPath = thirdPartyPath .. "/imgui"
local eastlPath = thirdPartyPath .. "/EASTL"
local eabasePath = thirdPartyPath .. "/EABase"
local eabaseIncludePath = eabasePath .. "/include/Common"
local metalCppPath = thirdPartyPath .. "/metal-cpp"

project "glfw"
    kind "StaticLib"
    language "C"
    warnings "Off"
    targetdir "build/Intermediate/%{cfg.buildcfg}"
    objdir "build/Intermediate/%{cfg.buildcfg}/%{prj.name}"

    includedirs {
        glfwPath .. "/include",
        glfwPath .. "/src"
    }

    files {
        glfwPath .. "/include/GLFW/glfw3.h",
        glfwPath .. "/include/GLFW/glfw3native.h",
        glfwPath .. "/src/context.c",
        glfwPath .. "/src/egl_context.c",
        glfwPath .. "/src/init.c",
        glfwPath .. "/src/input.c",
        glfwPath .. "/src/monitor.c",
        glfwPath .. "/src/null_init.c",
        glfwPath .. "/src/null_joystick.c",
        glfwPath .. "/src/null_monitor.c",
        glfwPath .. "/src/null_window.c",
        glfwPath .. "/src/osmesa_context.c",
        glfwPath .. "/src/platform.c",
        glfwPath .. "/src/vulkan.c",
        glfwPath .. "/src/window.c"
    }

    filter "system:windows"
        defines { "_GLFW_WIN32" }
        files {
            glfwPath .. "/src/win32_init.c",
            glfwPath .. "/src/win32_joystick.c",
            glfwPath .. "/src/win32_monitor.c",
            glfwPath .. "/src/win32_time.c",
            glfwPath .. "/src/win32_thread.c",
            glfwPath .. "/src/win32_module.c",
            glfwPath .. "/src/win32_window.c",
            glfwPath .. "/src/wgl_context.c"
        }
        links {
            "gdi32",
            "user32",
            "shell32"
        }

    filter "system:macosx"
        defines { "_GLFW_COCOA" }
        files {
            glfwPath .. "/src/cocoa_init.m",
            glfwPath .. "/src/cocoa_joystick.m",
            glfwPath .. "/src/cocoa_monitor.m",
            glfwPath .. "/src/cocoa_time.c",
            glfwPath .. "/src/cocoa_window.m",
            glfwPath .. "/src/nsgl_context.m",
            glfwPath .. "/src/posix_module.c",
            glfwPath .. "/src/posix_thread.c"
        }
        links {
            "Cocoa.framework",
            "IOKit.framework",
            "CoreFoundation.framework"
        }

    filter "system:linux"
        defines { "_GLFW_X11" }
        files {
            glfwPath .. "/src/glx_context.c",
            glfwPath .. "/src/linux_joystick.c",
            glfwPath .. "/src/posix_module.c",
            glfwPath .. "/src/posix_poll.c",
            glfwPath .. "/src/posix_thread.c",
            glfwPath .. "/src/posix_time.c",
            glfwPath .. "/src/x11_init.c",
            glfwPath .. "/src/x11_monitor.c",
            glfwPath .. "/src/x11_window.c",
            glfwPath .. "/src/xkb_unicode.c"
        }
        links {
            "X11",
            "Xrandr",
            "Xi",
            "Xxf86vm",
            "Xcursor",
            "pthread",
            "dl"
        }
    filter {}

project "eastl"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    targetdir "build/Intermediate/%{cfg.buildcfg}"
    objdir "build/Intermediate/%{cfg.buildcfg}/%{prj.name}"

    defines {
        "_CHAR16T",
        "_CRT_SECURE_NO_WARNINGS",
        "_SCL_SECURE_NO_WARNINGS",
        "EASTL_OPENSOURCE=1"
    }

    externalincludedirs {
        eastlPath .. "/include",
        eabaseIncludePath
    }

    files {
        eastlPath .. "/include/**.h",
        eastlPath .. "/source/**.cpp"
    }

project "gpuApi"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    targetdir "build/Intermediate/%{cfg.buildcfg}"
    objdir "build/Intermediate/%{cfg.buildcfg}/%{prj.name}"

    includedirs {
        "src"
    }

    files {
        "src/gpu/gpu_api_stub.cpp",
        "src/gpu/gpu_api.hpp"
    }

project "dearImgui"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    targetdir "build/Intermediate/%{cfg.buildcfg}"
    objdir "build/Intermediate/%{cfg.buildcfg}/%{prj.name}"

    includedirs {
        imguiPath,
        imguiPath .. "/backends"
    }

    externalincludedirs {
        glfwPath .. "/include"
    }

    files {
        imguiPath .. "/imgui.cpp",
        imguiPath .. "/imgui_demo.cpp",
        imguiPath .. "/imgui_draw.cpp",
        imguiPath .. "/imgui_tables.cpp",
        imguiPath .. "/imgui_widgets.cpp",
        imguiPath .. "/backends/imgui_impl_glfw.cpp"
    }

    filter "system:windows"
        files {
            imguiPath .. "/backends/imgui_impl_opengl3.cpp"
        }
    filter "system:linux"
        files {
            imguiPath .. "/backends/imgui_impl_opengl3.cpp"
        }
    filter "system:macosx"
        files {
            imguiPath .. "/backends/imgui_impl_metal.mm"
        }
        defines { "IMGUI_IMPL_METAL_CPP" }
        externalincludedirs {
            metalCppPath,
            metalCppPath .. "/metal-cpp"
        }
        links {
            "Foundation.framework",
            "QuartzCore.framework",
            "Metal.framework"
        }
    filter {}

project "metalCppTest"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    targetdir "build/Build/%{cfg.buildcfg}"
    objdir "build/Intermediate/%{cfg.buildcfg}/%{prj.name}"

    dependson {
        "glfw",
        "eastl",
        "gpuApi",
        "dearImgui"
    }

    includedirs {
        "src",
        imguiPath,
        imguiPath .. "/backends",
    }

    externalincludedirs {
        glfwPath .. "/include",
        eastlPath .. "/include",
        eabaseIncludePath
    }

    files {
        "src/main.cpp",
        "src/core/aa_build_config.hpp",
        "src/core/aa_containers.hpp",
        "src/core/aa_eastl_new.cpp",
        "src/core/aa_file.hpp",
        "src/core/aa_file_common.cpp",
        "src/core/aa_global_new.cpp",
        "src/core/aa_memory.hpp",
        "src/core/aa_memory_tracker.cpp",
        "src/core/aa_memory_tracker.hpp",
        "src/core/aa_path.cpp",
        "src/core/aa_path.hpp",
        "src/core/aa_platform.cpp",
        "src/core/aa_platform.hpp",
        "src/core/aa_types.hpp",
        "src/gpu/gpu_api.hpp",
        "src/render/renderer.hpp",
        "src/render/renderer_factory.cpp"
    }

    links {
        "glfw",
        "eastl",
        "gpuApi",
        "dearImgui"
    }

    filter "system:windows"
        files {
            "src/core/aa_file_windows.cpp",
            "src/render/renderer_opengl.cpp",
            "src/render/renderer_opengl.hpp"
        }
    filter "system:linux"
        files {
            "src/core/aa_file_macos.cpp",
            "src/render/renderer_opengl.cpp",
            "src/render/renderer_opengl.hpp"
        }

    filter "system:windows"
        links {
            "opengl32"
        }

    filter "system:linux"
        links {
            "GL"
        }

    filter "system:macosx"
        files {
            "src/core/aa_file_macos.cpp",
            "src/render/renderer_metal.cpp",
            "src/render/renderer_metal.hpp",
            "src/metal-cpp-extensions/**.hpp"
        }
        defines {
            "IMGUI_IMPL_METAL_CPP"
        }
        externalincludedirs {
            metalCppPath,
            "src/metal-cpp-extensions"
        }
        links {
            "AppKit.framework",
            "Cocoa.framework",
            "Foundation.framework",
            "IOKit.framework",
            "Metal.framework",
            "QuartzCore.framework",
            "CoreFoundation.framework"
        }
    filter {}
