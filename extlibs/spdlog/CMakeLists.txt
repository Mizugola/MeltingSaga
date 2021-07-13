project(spdlog)

file(GLOB SPDLOG_HEADERS
    include/spdlog/*.h
    include/spdlog/details/*.h
    include/spdlog/fmt/*.h
    include/spdlog/sinks/*.h
)
file(GLOB SPDLOG_SOURCES src/*.cpp)

add_library(spdlog ${SPDLOG_HEADERS} ${SPDLOG_SOURCES})
add_library(spdlog::spdlog ALIAS spdlog)

target_compile_definitions(spdlog PUBLIC SPDLOG_FMT_EXTERNAL)
target_compile_definitions(spdlog PUBLIC SPDLOG_COMPILED_LIB)
target_link_libraries(spdlog PUBLIC fmt)

target_include_directories(spdlog
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/spdlog
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 11)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_EXTENSIONS OFF)