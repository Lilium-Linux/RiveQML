include(FetchContent)
include(ExternalProject)

if(NOT APPLE)
    message(FATAL_ERROR "RiveQml currently ships a macOS renderer backend only.")
endif()

set(RIVEQML_RIVE_RUNTIME_REVISION
    "1e9e2359126247da3434b73ed1ec49e139b779a2"
    CACHE STRING
    "Pinned rive-runtime revision."
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
set(_rive_runtime_build_command
    ${CMAKE_COMMAND} -E chdir
    "${RIVEQML_RIVE_RUNTIME_DIR}"
    ${CMAKE_COMMAND} -E env
    RIVE_PREMAKE_ARGS=--with-pic
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

set_target_properties(RiveRuntime::Runtime PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES
        "${RIVEQML_RIVE_RUNTIME_DIR}/include;${RIVEQML_RIVE_RUNTIME_DIR}/cg_renderer/include"
    INTERFACE_LINK_LIBRARIES
        "${_rive_runtime_libs}"
)
