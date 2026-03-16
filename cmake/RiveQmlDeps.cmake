include(FetchContent)
include(ExternalProject)

if(LINUX)
    message(STATUS
        "RiveQml is configuring with the Linux Skia raster renderer."
    )
elseif(NOT APPLE)
    message(STATUS
        "RiveQml is configuring without a raster renderer on this platform. "
        "Non-Apple builds support document loading and API validation, but not raster output yet."
    )
endif()

set(RIVEQML_RIVE_RUNTIME_REVISION
    "1e9e2359126247da3434b73ed1ec49e139b779a2"
    CACHE STRING
    "Pinned rive-runtime revision."
)

set(RIVEQML_SKIA_REVISION
    "chrome/m99"
    CACHE STRING
    "Pinned Skia revision used for Linux rendering."
)

set(RIVEQML_RIVE_RUNTIME_DIR
    ""
    CACHE PATH
    "Path to a rive-runtime checkout."
)

set(_riveqml_root "${CMAKE_CURRENT_LIST_DIR}/..")
get_filename_component(_riveqml_root "${_riveqml_root}" ABSOLUTE)

if(NOT RIVEQML_RIVE_RUNTIME_DIR)
    set(_riveqml_default_runtime_dir "${_riveqml_root}/../_deps/rive-runtime")

    if(EXISTS "${_riveqml_default_runtime_dir}/premake5_v2.lua")
        set(RIVEQML_RIVE_RUNTIME_DIR
            "${_riveqml_default_runtime_dir}"
            CACHE PATH
            "Path to a rive-runtime checkout."
            FORCE
        )
    endif()
endif()

if(RIVEQML_FETCH_RIVE_RUNTIME AND NOT RIVEQML_RIVE_RUNTIME_DIR)
    FetchContent_Declare(
        rive_runtime
        GIT_REPOSITORY https://github.com/rive-app/rive-runtime.git
        GIT_TAG ${RIVEQML_RIVE_RUNTIME_REVISION}
        SOURCE_DIR "${CMAKE_BINARY_DIR}/_deps/rive-runtime"
    )
    FetchContent_Populate(rive_runtime)

    set(RIVEQML_RIVE_RUNTIME_DIR
        "${rive_runtime_SOURCE_DIR}"
        CACHE PATH
        "Path to a rive-runtime checkout."
        FORCE
    )
endif()

if(NOT RIVEQML_RIVE_RUNTIME_DIR OR NOT EXISTS "${RIVEQML_RIVE_RUNTIME_DIR}/premake5_v2.lua")
    message(FATAL_ERROR
        "RiveQml requires a rive-runtime checkout. Set RIVEQML_RIVE_RUNTIME_DIR "
        "or enable RIVEQML_FETCH_RIVE_RUNTIME."
    )
endif()

if(NOT EXISTS "${RIVEQML_RIVE_RUNTIME_DIR}/premake5.lua")
    file(WRITE "${RIVEQML_RIVE_RUNTIME_DIR}/premake5.lua" "dofile(\"premake5_v2.lua\")\n")
endif()

set(_rive_runtime_out_dir "${RIVEQML_RIVE_RUNTIME_DIR}/out/release")
set(_rive_runtime_libs
    "${_rive_runtime_out_dir}/librive.a"
    "${_rive_runtime_out_dir}/librive_harfbuzz.a"
    "${_rive_runtime_out_dir}/librive_sheenbidi.a"
    "${_rive_runtime_out_dir}/librive_yoga.a"
    "${_rive_runtime_out_dir}/libminiaudio.a"
    "${_rive_runtime_out_dir}/libluau_vm.a"
)

set(_rive_runtime_premake_args --with-pic)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    list(APPEND _rive_runtime_premake_args --no-lto)
endif()

string(JOIN " " _rive_runtime_premake_args_default ${_rive_runtime_premake_args})

set(RIVEQML_RUNTIME_PREMAKE_ARGS
    "${_rive_runtime_premake_args_default}"
    CACHE STRING
    "Arguments forwarded to the upstream rive-runtime premake invocation."
)

set(_rive_runtime_build_env
    "RIVE_PREMAKE_ARGS=${RIVEQML_RUNTIME_PREMAKE_ARGS}"
)

set(_rive_runtime_build_command
    ${CMAKE_COMMAND} -E chdir
    "${RIVEQML_RIVE_RUNTIME_DIR}"
    ${CMAKE_COMMAND} -E env
    ${_rive_runtime_build_env}
    "${RIVEQML_RIVE_RUNTIME_DIR}/build/build_rive.sh"
    release
)

set(_rive_runtime_ready TRUE)
foreach(_rive_runtime_lib IN LISTS _rive_runtime_libs)
    if(NOT EXISTS "${_rive_runtime_lib}")
        set(_rive_runtime_ready FALSE)
    endif()
endforeach()

if(_rive_runtime_ready)
    add_custom_target(RiveRuntimeBuild)
else()
    ExternalProject_Add(
        RiveRuntimeBuild
        SOURCE_DIR "${RIVEQML_RIVE_RUNTIME_DIR}"
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ${_rive_runtime_build_command}
        BUILD_BYPRODUCTS ${_rive_runtime_libs}
        INSTALL_COMMAND ""
        UPDATE_COMMAND ""
        LOG_BUILD ON
    )
endif()

add_library(RiveRuntime::Runtime INTERFACE IMPORTED GLOBAL)
add_dependencies(RiveRuntime::Runtime RiveRuntimeBuild)

set(_rive_runtime_include_directories
    "${RIVEQML_RIVE_RUNTIME_DIR}/include"
)

if(APPLE)
    list(APPEND _rive_runtime_include_directories
        "${RIVEQML_RIVE_RUNTIME_DIR}/cg_renderer/include"
    )
endif()

set_target_properties(RiveRuntime::Runtime PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES
        "${_rive_runtime_include_directories}"
    INTERFACE_LINK_LIBRARIES
        "${_rive_runtime_libs}"
)

if(LINUX)
    set(_rive_skia_root "${RIVEQML_RIVE_RUNTIME_DIR}/skia/dependencies/skia")
    if(NOT EXISTS "${_rive_skia_root}/include/core/SkCanvas.h")
        FetchContent_Declare(
            riveqml_skia
            GIT_REPOSITORY https://github.com/google/skia.git
            GIT_TAG ${RIVEQML_SKIA_REVISION}
            SOURCE_DIR "${_rive_skia_root}"
        )
        FetchContent_Populate(riveqml_skia)
    endif()

    set(_rive_skia_root "${RIVEQML_RIVE_RUNTIME_DIR}/skia/dependencies/skia")
    set(_rive_skia_out_dir "${_rive_skia_root}/out/riveqml-linux")
    set(_rive_skia_lib "${_rive_skia_out_dir}/libskia.a")
    set(_rive_skia_build_script "${CMAKE_CURRENT_LIST_DIR}/build_rive_skia_linux.sh")

    add_custom_command(
        OUTPUT "${_rive_skia_lib}"
        COMMAND "${_rive_skia_build_script}" "${RIVEQML_RIVE_RUNTIME_DIR}"
        DEPENDS
            RiveRuntimeBuild
            "${_rive_skia_build_script}"
        VERBATIM
    )

    add_custom_target(RiveRuntimeSkiaBuild DEPENDS "${_rive_skia_lib}")

    add_library(RiveRuntime::Skia INTERFACE IMPORTED GLOBAL)
    add_dependencies(RiveRuntime::Skia RiveRuntimeSkiaBuild)
    set_target_properties(RiveRuntime::Skia PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES
            "${RIVEQML_RIVE_RUNTIME_DIR}/skia/renderer/include;${_rive_skia_root}"
        INTERFACE_LINK_LIBRARIES
            "${_rive_skia_lib};m;dl"
    )
endif()
