set(RIVEQML_PUBLIC_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/../include/RiveQml/riveqmlglobal.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../include/RiveQml/riveassetprovider.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../include/RiveQml/rivefile.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../include/RiveQml/riveinputmap.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../include/RiveQml/riveitem.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../include/RiveQml/riveviewmodeladapter.h"
)

set(RIVEQML_PRIVATE_SOURCES
    core/riveassetprovider.cpp
    core/rivefile.cpp
    core/riveinputmap.cpp
    core/riveviewmodeladapter.cpp
    core/runtimebridge.cpp
    qml/riveitem.cpp
    render/qt/qtgraphicsapi.cpp
)

if(APPLE)
    list(APPEND RIVEQML_PRIVATE_SOURCES
        "${RIVEQML_RIVE_RUNTIME_DIR}/cg_renderer/src/cg_factory.cpp"
    )
elseif(LINUX)
    list(APPEND RIVEQML_PRIVATE_SOURCES
        "${RIVEQML_RIVE_RUNTIME_DIR}/skia/renderer/src/skia_factory.cpp"
    )
else()
    list(APPEND RIVEQML_PRIVATE_SOURCES
        "${RIVEQML_RIVE_RUNTIME_DIR}/utils/no_op_factory.cpp"
    )
endif()

set(RIVEQML_PRIVATE_HEADERS
    core/runtimebridge.h
    render/qt/qtgraphicsapi.h
)

qt_add_library(RiveQml SHARED)
add_library(RiveQml::RiveQml ALIAS RiveQml)

set(RIVEQML_QML_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/qml/RiveQml")

qt_add_qml_module(RiveQml
    URI RiveQml
    VERSION 1.0
    CLASS_NAME RiveQmlPlugin
    PLUGIN_TARGET RiveQmlPlugin
    OUTPUT_DIRECTORY "${RIVEQML_QML_OUTPUT_DIRECTORY}"
    SOURCES
        ${RIVEQML_PUBLIC_HEADERS}
        ${RIVEQML_PRIVATE_HEADERS}
        ${RIVEQML_PRIVATE_SOURCES}
)

target_compile_definitions(RiveQml PRIVATE RIVEQML_LIBRARY)
target_compile_options(RiveQml PRIVATE -fno-rtti)

if(APPLE)
    target_compile_definitions(RiveQml PRIVATE RIVEQML_USE_CG_RENDERER)
elseif(LINUX)
    target_compile_definitions(RiveQml PRIVATE RIVEQML_USE_SKIA_RENDERER)
endif()

target_include_directories(RiveQml
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${CMAKE_CURRENT_SOURCE_DIR}/../include/RiveQml"
)

target_link_libraries(RiveQml
    PUBLIC
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
    PRIVATE
        RiveRuntime::Runtime
)

if(APPLE)
    target_link_libraries(RiveQml PRIVATE "-framework ApplicationServices")
elseif(LINUX)
    target_link_libraries(RiveQml PRIVATE RiveRuntime::Skia)
endif()

set_target_properties(RiveQml PROPERTIES
    EXPORT_NAME RiveQml
)

install(
    TARGETS RiveQml
    EXPORT RiveQmlTargets
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
)

install(
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../include/RiveQml"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

install(
    DIRECTORY "${RIVEQML_QML_OUTPUT_DIRECTORY}"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/qml"
)

install(
    EXPORT RiveQmlTargets
    NAMESPACE RiveQml::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/RiveQml"
)
