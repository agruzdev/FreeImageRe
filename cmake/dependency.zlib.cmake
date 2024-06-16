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
    URL "https://github.com/madler/zlib/archive/refs/tags/v1.3.1.zip"
    HASH_MD5 "127b8a71a3fb8bebe89df1080f15fdf6"
    FILE_NAME "v1.3.1.zip"
    PREFIX "zlib-1.3.1"
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
set(ZLIB_INCLUDE_DIRS ${ZLIB_INCLUDE_DIR} CACHE PATH "") # For libpng
set(ZLIB_LIBRARY_DIR "" CACHE PATH "")
set(ZLIB_LIBRARY zlibstatic CACHE STRING "")
