# LibTIFF dependency
# http://www.libtiff.org/

# Output variables:
# TIFF_INCLUDE_DIR - includes
# TIFF_LIBRARY_DIR - link directories
# TIFF_LIBRARY     - link targets

include(${CMAKE_SOURCE_DIR}/cmake/dependency.common.functions.cmake)

dependency_find_or_download(
    NAME TIFF
    VERBOSE_NAME "LibTIFF"
    URL "http://download.osgeo.org/libtiff/tiff-4.6.0.zip"
    HASH_MD5 "fe7cf2bda082f644b2aec2eddc3f5153"
    FILE_NAME "tiff-4.6.0.zip"
    PREFIX "tiff-4.6.0"
)

if(NOT TARGET tiff)

    set(zlib ON)
    set(libdeflate OFF)
    set(jpeg OFF)
    set(old-jpeg OFF)
    set(jpeg12 OFF)
    set(jbig OFF)
    set(webp OFF)
    set(win32-io OFF)
    set(BUILD_SHARED_LIBS OFF)
    set(cxx OFF)

    add_subdirectory(${TIFF_FOUND_ROOT} ${CMAKE_BINARY_DIR}/dependencies/tiff EXCLUDE_FROM_ALL)
    set_property(TARGET tiff PROPERTY FOLDER "Dependencies")

    if (MSVC)
        target_compile_options(tiff PRIVATE "/w")
    endif()
endif()

set(TIFF_INCLUDE_DIR ${TIFF_FOUND_ROOT} ${CMAKE_BINARY_DIR}/dependencies/tiff CACHE PATH "")
set(TIFF_LIBRARY_DIR "" CACHE PATH "")
set(TIFF_LIBRARY tiff CACHE STRING "")
