project(sol2)

file(GLOB SOL_HEADERS include/sol/*.hpp)

add_library(sol2 INTERFACE)
add_library(sol2::sol2 ALIAS sol2)

target_link_libraries(sol2 INTERFACE lua)

target_include_directories(sol2
    INTERFACE
        $<BUILD_INTERFACE:${lua_SOURCE_DIR}/include/lua>
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_compile_definitions(sol2 INTERFACE SOL_ALL_SAFETIES_ON)