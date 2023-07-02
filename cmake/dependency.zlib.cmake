# ZLib dependency
# https://github.com/madler/zlib

# Output variables:
# ZLIB_INCLUDE_DIR - includes
# ZLIB_LIBRARY_DIR - link directories
# ZLIB_LIBRARY     - link targets

include(${CMAKE_SOURCE_DIR}/cmake/dependency.common.functions.cmake)

dependency_find_or_download(
    NAME ZLIB
    VERBOSE_NAME "ZLib"
    URL "https://github.com/madler/zlib/archive/refs/tags/v1.2.13.zip"
    HASH_MD5 "fdedf0c8972a04a7c153dd73492d2d91"
    FILE_NAME "v1.2.13.zip"
    PREFIX "zlib-1.2.13"
)

if(NOT TARGET zlibstatic)
    set(SKIP_INSTALL_ALL ON)

    add_subdirectory(${ZLIB_FOUND_ROOT} ${CMAKE_BINARY_DIR}/dependencies/zlib EXCLUDE_FROM_ALL)
    set_property(TARGET zlibstatic PROPERTY FOLDER "Dependencies")

    if (MSVC)
        target_compile_options(zlibstatic PRIVATE "/w")
    endif()

    unset(SKIP_INSTALL_ALL)

    add_library(ZLIB::ZLIB ALIAS zlibstatic)

    target_include_directories(zlibstatic INTERFACE ${ZLIB_FOUND_ROOT} ${CMAKE_BINARY_DIR}/dependencies/zlib)
endif()
export(TARGETS zlibstatic FILE "${CMAKE_BINARY_DIR}/dependencies/zlib/zlibstaticTargets.cmake")

set(ZLIB_INCLUDE_DIR ${ZLIB_FOUND_ROOT} ${CMAKE_BINARY_DIR}/dependencies/zlib CACHE PATH "")
set(ZLIB_LIBRARY_DIR "" CACHE PATH "")
set(ZLIB_LIBRARY zlibstatic CACHE STRING "")
