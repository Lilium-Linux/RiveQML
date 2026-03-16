find_package(Qt6 6.8 REQUIRED COMPONENTS Test)

qt_add_executable(RiveQmlRiveFileSmokeTest
    unit/rivefile_smoke_test.cpp
)

target_compile_definitions(RiveQmlRiveFileSmokeTest
    PRIVATE
        RIVEQML_TEST_ASSET_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data"
)

target_link_libraries(RiveQmlRiveFileSmokeTest
    PRIVATE
        Qt6::Core
        Qt6::Test
        RiveQml::RiveQml
)

add_test(
    NAME RiveQmlRiveFileSmokeTest
    COMMAND RiveQmlRiveFileSmokeTest
)

if(APPLE)
    qt_add_executable(RiveQmlRiveItemRenderTest
        unit/riveitem_render_test.cpp
    )

    target_compile_definitions(RiveQmlRiveItemRenderTest
        PRIVATE
            RIVEQML_TEST_ASSET_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data"
    )

    target_link_libraries(RiveQmlRiveItemRenderTest
        PRIVATE
            Qt6::Core
            Qt6::Gui
            Qt6::Test
            Qt6::Quick
            RiveQml::RiveQml
    )

    target_compile_options(RiveQmlRiveItemRenderTest
        PRIVATE
            -fno-rtti
    )

    add_test(
        NAME RiveQmlRiveItemRenderTest
        COMMAND RiveQmlRiveItemRenderTest
    )
endif()
