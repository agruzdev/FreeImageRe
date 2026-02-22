#
# FreeImageRe library
#

if(NOT _EXTERNAL_PROJECT_INCLUDE_GUARD_)
set(_EXTERNAL_PROJECT_INCLUDE_GUARD_ ON)


cmake_minimum_required(VERSION 3.28)    # for FetchContent's EXCLUDE_FROM_ALL

include(FetchContent)
include(ExternalProject)


if(MSVC)
    if(MSVC_VERSION GREATER_EQUAL 1950)
        set(MSVC_NAME "vs2026")
    elseif(MSVC_VERSION GREATER_EQUAL 1930)
        set(MSVC_NAME "vs2022")
    elseif(MSVC_VERSION GREATER_EQUAL 1920)
        set(MSVC_NAME "vs2019")
    elseif(MSVC_VERSION GREATER_EQUAL 1910)
        set(MSVC_NAME "vs2017")
    elseif(MSVC_VERSION GREATER_EQUAL 1900)
        set(MSVC_NAME "vs2015")
    else()
        message("Unsupported MSVC version: ${MSVC_VERSION}")
    endif()
endif()



set(EXTERNALPROJECT_SOURCE_ROOT ${CMAKE_SOURCE_DIR}/dependencies CACHE PATH "Directory to download and unpack dependencies")
set(EXTERNALPROJECT_BINARY_ROOT ${CMAKE_BINARY_DIR}/dependencies CACHE PATH "Directory to build and install dependencies")

option(EXTERNALPROJECT_UNPACK_IN_BINARY_DIR "Unpack sources into binary dir to avoid possible permission issues" OFF)
if (EXTERNALPROJECT_UNPACK_IN_BINARY_DIR)
    set(EXTERNALPROJECT_SOURCE_PREFIX ${EXTERNALPROJECT_BINARY_ROOT})
else()
    set(EXTERNALPROJECT_SOURCE_PREFIX ${EXTERNALPROJECT_SOURCE_ROOT})
endif()

get_property(IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

#set(CMAKE_BUILD_TYPE_RELEASE "-DCMAKE_BUILD_TYPE=Release")

if (IS_MULTI_CONFIG)
    set(CMAKE_BUILD_TYPE_ARG "-DCMAKE_CONFIGURATION_TYPES=${CMAKE_CONFIGURATION_TYPES}")
    set(BUILD_COMMAND_FOR_TARGET ${CMAKE_COMMAND} --build . --config ${CMAKE_CONFIGURATION_TYPES})
    if (CMAKE_CONFIGURATION_TYPES STREQUAL "Debug")
        set(IS_DEBUG_CONFIG ON)
    endif()
else()
    set(CMAKE_BUILD_TYPE_ARG "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
    set(BUILD_COMMAND_FOR_TARGET ${CMAKE_COMMAND} --build . )
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(IS_DEBUG_CONFIG ON)
    endif()
endif()


#if (IS_MULTI_CONFIG)
#    list(APPEND BUILD_COMMAND_FOR_TARGET --config $<CONFIG>)
#    set(CMAKE_BUILD_TYPE_ARG "")
#    set(CMAKE_BUILD_TYPE_RELEASE "")
#    set(CMAKE_DEBUG_POSTFIX_MULTICONF "-DCMAKE_DEBUG_POSTFIX=d")
#endif()

if (MSVC)
    set(ZERO_WARNINGS_FLAG "/w")
    set(EHSC_FLAG "/EHsc")
    set(FPIC_FLAG "")
    set(DEF_FLAG "/D")
else()
    set(ZERO_WARNINGS_FLAG "-w")
    set(EHSC_FLAG "")
    set(FPIC_FLAG "-fPIC")
    set(DEF_FLAG "-D")
endif()

macro(link_library_path2 TARGET_ PREFIX_ LIBRARY_ LIBRARY_DEBUG_)
    if (IS_MULTI_CONFIG)
        target_link_libraries(${TARGET_} INTERFACE
            optimized "${PREFIX_}/${LIBRARY_}"
            debug "${PREFIX_}/${LIBRARY_DEBUG_}"
        )
    else ()
        target_link_libraries(${TARGET_} INTERFACE "${PREFIX_}/${LIBRARY_}")
    endif()
endmacro()

macro(link_config_aware_library_path2 TARGET_ PREFIX_ LIBRARY_ LIBRARY_DEBUG_)
    if (IS_MULTI_CONFIG)
        target_link_libraries(${TARGET_} INTERFACE
            optimized "${PREFIX_}/Release/${LIBRARY_}"
            debug "${PREFIX_}/Debug/${LIBRARY_DEBUG_}"
        )
    else ()
        target_link_libraries(${TARGET_} INTERFACE "${PREFIX_}/${LIBRARY_}")
    endif()
endmacro()

macro(link_config_aware_library_path TARGET_ PREFIX_ LIBRARY_)
    link_config_aware_library_path2(${TARGET_} ${PREFIX_} ${LIBRARY_} ${LIBRARY_})
endmacro()



function(meson_build_type_from_cmake RETVAR_)
    get_property(IS_MULTI_CONFIG_ GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
    if (IS_MULTI_CONFIG_)
        string(TOLOWER ${CMAKE_CONFIGURATION_TYPES} CMAKE_BUILD_TYPE_LOWER_)
    else()
        string(TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_LOWER_)
    endif()

    if(CMAKE_BUILD_TYPE_LOWER_ STREQUAL "debug")
        set(${RETVAR_} "debug" PARENT_SCOPE)
    elseif(CMAKE_BUILD_TYPE_LOWER_ STREQUAL "release")
        set(${RETVAR_} "release" PARENT_SCOPE)
    elseif(CMAKE_BUILD_TYPE_LOWER_ STREQUAL "relwithdebinfo")
        set(${RETVAR_} "debugoptimized" PARENT_SCOPE)
    elseif(CMAKE_BUILD_TYPE_LOWER_ STREQUAL "minsizerel")
        set(${RETVAR_} "minsize" PARENT_SCOPE)
    else()
        message(WARNING
           "Custom CMAKE_BUILD_TYPE '${CMAKE_BUILD_TYPE_LOWER_}' does not correspond to a Meson buildtype. Fallback to 'custom'."
        )
        set(${RETVAR_} "custom" PARENT_SCOPE)
    endif()
endfunction()


endif() #_EXTERNAL_PROJECT_INCLUDE_GUARD_
