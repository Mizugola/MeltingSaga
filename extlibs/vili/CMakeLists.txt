cmake_minimum_required(VERSION 3.6)

project(vili)

add_subdirectory(extlibs/fifo_map)
add_subdirectory(extlibs/pegtl)

set(VILI_HEADERS
    include/vili/config.hpp
    include/vili/exceptions.hpp
    include/vili/node.hpp
    include/vili/parser.hpp
    include/vili/types.hpp
    include/vili/utils.hpp
    include/vili/writer.hpp
    include/vili/parser/actions.hpp
    include/vili/parser/grammar.hpp
    include/vili/parser/grammar_errors.hpp
    include/vili/parser/parser_state.hpp
)
set(VILI_SOURCES
    src/node.cpp
    src/parser.cpp
    src/types.cpp
    src/utils.cpp
    src/writer.cpp
    src/parser/parser_state.cpp
)

add_library(vili ${VILI_HEADERS} ${VILI_SOURCES})

target_link_libraries(vili fifo_map)

if(OBE_USE_VCPKG)
    find_package(fmt CONFIG REQUIRED)
    target_link_libraries(vili fmt::fmt)
else()
    target_link_libraries(vili fmt)
endif()

target_link_libraries(vili pegtl)

target_include_directories(vili
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_EXTENSIONS OFF)