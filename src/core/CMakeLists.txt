add_library(core STATIC
    aa_global_new.cpp
    aa_eastl_new.cpp
    aa_memory_tracker.cpp
    aa_path.cpp
    aa_platform.cpp
    aa_file_common.cpp
)

if(WIN32)
    target_sources(core PRIVATE
        aa_file_windows.cpp
    )
else()
    target_sources(core PRIVATE
        aa_file_macos.cpp
    )
endif()

add_library(aa::core ALIAS core)

target_include_directories(core PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(core PUBLIC
    $<BUILD_INTERFACE:EASTL>
    $<INSTALL_INTERFACE:EASTL>
)

set_target_properties(core PROPERTIES
    FOLDER "core"
    EXPORT_NAME core
)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

if(AA_ENABLE_MODULE_PACKAGE_EXPORTS)
    install(TARGETS core
        EXPORT coreTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
else()
    install(TARGETS core
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()

install(DIRECTORY ${PROJECT_SOURCE_DIR}/src/core
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.hpp"
)

if(AA_ENABLE_MODULE_PACKAGE_EXPORTS)
    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/coreConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
    )

    configure_package_config_file(
        ${PROJECT_SOURCE_DIR}/cmake/coreConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/coreConfig.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/core
    )

    install(EXPORT coreTargets
        FILE coreTargets.cmake
        NAMESPACE aa::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/core
    )

    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/coreConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/coreConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/core
    )
endif()
