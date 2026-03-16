cmake_minimum_required(VERSION 3.24)

project(
    RiveQml
    VERSION 0.1.0
    DESCRIPTION "Qt 6.8+ QML integration layer for Rive animations"
    LANGUAGES CXX
)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(RIVEQML_FETCH_RIVE_RUNTIME "Fetch and build the upstream Rive runtime during configure." ON)
option(RIVEQML_BUILD_TESTS "Build RiveQml test targets." ON)

find_package(Qt6 6.8 REQUIRED COMPONENTS Core Gui Qml Quick)

if(COMMAND qt_standard_project_setup)
    qt_standard_project_setup(REQUIRES 6.8)
endif()

include(cmake/RiveQmlDeps.cmake)

add_subdirectory(src)

if(RIVEQML_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/RiveQmlConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/RiveQmlConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/RiveQmlConfig.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/RiveQml"
)

export(
    EXPORT RiveQmlTargets
    NAMESPACE RiveQml::
    FILE "${CMAKE_CURRENT_BINARY_DIR}/RiveQmlTargets.cmake"
)

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/RiveQmlConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/RiveQmlConfigVersion.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/RiveQml"
)
