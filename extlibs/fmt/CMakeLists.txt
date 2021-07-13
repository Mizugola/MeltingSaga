project(fmt)

file(GLOB FMT_HEADERS include/fmt/*.h)
file(GLOB FMT_SOURCES src/*.cc)

add_library(fmt ${FMT_HEADERS} ${FMT_SOURCES})
add_library(fmt::fmt ALIAS fmt)

target_include_directories(fmt
    PUBLIC 
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 14)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_EXTENSIONS OFF)