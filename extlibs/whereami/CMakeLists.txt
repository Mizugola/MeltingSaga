project(whereami)

file(GLOB WAI_HEADERS include/whereami/*.h)
file(GLOB WAI_SOURCES src/*.c)

add_library(whereami ${WAI_HEADERS} ${WAI_SOURCES})

target_include_directories(whereami
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/whereami>
)