project(styler)

file(GLOB STYLER_HEADERS include/styler/*.hpp)

add_library(styler INTERFACE)

target_include_directories(styler
    INTERFACE
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)