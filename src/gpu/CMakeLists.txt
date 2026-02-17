add_library(gpu_api STATIC
    gpu_api_stub.cpp
)

add_library(aa::gpu_api ALIAS gpu_api)

target_include_directories(gpu_api PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(gpu_api PUBLIC
    $<BUILD_INTERFACE:core>
    $<INSTALL_INTERFACE:aa::core>
)

set_target_properties(gpu_api PROPERTIES
    FOLDER "gpu_api"
    EXPORT_NAME gpu_api
)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

if(AA_ENABLE_MODULE_PACKAGE_EXPORTS)
    install(TARGETS gpu_api
        EXPORT gpuApiTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
else()
    install(TARGETS gpu_api
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()

install(DIRECTORY ${PROJECT_SOURCE_DIR}/src/gpu
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.hpp"
)

if(AA_ENABLE_MODULE_PACKAGE_EXPORTS)
    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/gpu_apiConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
    )

    configure_package_config_file(
        ${PROJECT_SOURCE_DIR}/cmake/gpu_apiConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/gpu_apiConfig.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/gpu_api
    )

    install(EXPORT gpuApiTargets
        FILE gpuApiTargets.cmake
        NAMESPACE aa::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/gpu_api
    )

    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/gpu_apiConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/gpu_apiConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/gpu_api
    )
endif()
