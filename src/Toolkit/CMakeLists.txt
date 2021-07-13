project(obey)

include(group_files)
include(setup_environment)

file(GLOB_RECURSE OBETOOLKIT_HEADERS "${ObEngine_SOURCE_DIR}/include/Toolkit/*.hpp")
file(GLOB_RECURSE OBETOOLKIT_SOURCES "${ObEngine_SOURCE_DIR}/src/Toolkit/*.cpp")


add_executable(obey ${OBETOOLKIT_HEADERS} ${OBETOOLKIT_SOURCES} ${OBENGINE_ICON})

target_include_directories(obey
    PRIVATE
        $<BUILD_INTERFACE:${ObEngine_SOURCE_DIR}/include/Player>
)

target_link_libraries(obey ObEngineCore)
target_link_libraries(obey styler)

group_files(
    "${OBETOOLKIT_HEADERS}" "${OBETOOLKIT_SOURCES}"
    "${ObEngine_SOURCE_DIR}/include/Toolkit"
    "${ObEngine_SOURCE_DIR}/src/Toolkit"
)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_EXTENSIONS OFF)

if (MSVC)
    if (NOT (MSVC_VERSION LESS 1910))
        target_compile_options(obey PRIVATE /permissive-)
    endif()
endif()

copy_required_dlls(obey)