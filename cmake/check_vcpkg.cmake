function(obe_check_vcpkg ${OBE_USE_VCPKG})
    option(USE_VCPKG "Use vcpkg to handle some of the dependencies" OFF)
    if(USE_VCPKG AND DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
        set(OBE_USE_VCPKG ON)
        set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
            CACHE STRING "")
    else()
        set(CMAKE_TOOLCHAIN_FILE CACHE STRING "" FORCE)
    endif()

    if(${OBE_USE_VCPKG})
        if(NOT DEFINED ENV{VCPKG_FEATURE_FLAGS})
        message(FATAL_ERROR
            "Using vcpkg to build ObEngine without defining environment variable VCPKG_FEATURE_FLAGS to be \"manifests\"!! Aborting...") # manifests are required because of vcpkg.json
        endif()

        if(NOT DEFINED ENV{VCPKG_DEFAULT_TRIPLET})
            message(FATAL_ERROR
                "Using vcpkg to build ObEngine without defining environment variable VCPKG_DEFAULT_TRIPLET ! Aborting...")
        endif()
    endif()
endfunction()