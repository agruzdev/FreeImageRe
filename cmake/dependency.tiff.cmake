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
    URL "http://download.osgeo.org/libtiff/tiff-4.4.0.zip"
    HASH_MD5 "81b88f590c77dbfb9eb8e3b2c1843e77"
    FILE_NAME "tiff-4.4.0.zip"
    PREFIX "tiff-4.4.0"
)

if(NOT TARGET tiff)
#    set(SKIP_INSTALL_ALL ON)

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

    macro(install)
    endmacro()

    add_subdirectory(${TIFF_FOUND_ROOT} ${CMAKE_BINARY_DIR}/dependencies/tiff EXCLUDE_FROM_ALL)
    set_property(TARGET tiff PROPERTY FOLDER "Dependencies")

    if (MSVC)
        target_compile_options(tiff PRIVATE "/w")
        target_compile_options(tiff PRIVATE "/FI stdlib.h")
    endif()
endif()

set(TIFF_INCLUDE_DIR ${TIFF_FOUND_ROOT} ${CMAKE_BINARY_DIR}/dependencies/tiff CACHE PATH "")
set(TIFF_LIBRARY_DIR "" CACHE PATH "")
set(TIFF_LIBRARY tiff CACHE STRING "")
