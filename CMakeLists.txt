cmake_minimum_required(VERSION 3.15)
project(flashgui VERSION 1.0.0 LANGUAGES CXX)

# Library target
add_library(flashgui STATIC
    flashgui/renderer.cpp
    flashgui/fonts.cpp
    flashgui/dxgicontext.cpp
    flashgui/procmanager.cpp
    flashgui/flashgui.cpp
    flashgui/pch.cpp
    # Add other source files
)

target_include_directories(flashgui
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/flashgui/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/flashgui
)

target_compile_features(flashgui PUBLIC cxx_std_17)

# Find dependencies
find_package(directx-headers CONFIG REQUIRED)
find_package(directxtk12 CONFIG REQUIRED)

target_link_libraries(flashgui
    PUBLIC
        Microsoft::DirectX-Headers
        Microsoft::DirectXTK12
        d3d12.lib
        dxgi.lib
        dwrite.lib
)

# Installation
include(GNUInstallDirs)

install(TARGETS flashgui
    EXPORT flashgui-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY flashgui/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT flashgui-targets
    FILE flashgui-targets.cmake
    NAMESPACE flashgui::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/flashgui
)

# Create config file
include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/flashgui-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/flashgui-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/flashgui
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/flashgui-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/flashgui-config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/flashgui-config-version.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/flashgui
)