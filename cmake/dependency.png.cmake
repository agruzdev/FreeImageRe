# LibPNG dependency
# https://github.com/madler/zlib

# Output variables:
# PNG_INCLUDE_DIR - includes
# PNG_LIBRARY_DIR - link directories
# PNG_LIBRARY     - link targets

include(${CMAKE_SOURCE_DIR}/cmake/dependency.common.functions.cmake)

dependency_find_or_download(
    NAME PNG
    VERBOSE_NAME "LibPNG"
    URL "https://sourceforge.net/projects/libpng/files/libpng16/1.6.37/lpng1637.zip/download?download="
    HASH_MD5 "883764a8ebb8352904679f2ec70eda69"
    FILE_NAME "lpng1637.zip"
    PREFIX "lpng1637"
)

if(NOT TARGET png_static)
    set(PNG_SHARED OFF)
    set(PNG_STATIC ON)
    set(PNG_TESTS OFF)
    set(PNG_FRAMEWORK OFF)
    set(PNG_DEBUG OFF)
    set(PNG_BUILD_ZLIB OFF)
    set(PNG_HARDWARE_OPTIMIZATIONS ON)
    set(SKIP_INSTALL_ALL ON)

    add_subdirectory(${PNG_FOUND_ROOT} ${CMAKE_BINARY_DIR}/dependencies/png EXCLUDE_FROM_ALL)
    set_property(TARGET png_static genfiles PROPERTY FOLDER "Dependencies")

    if (MSVC)
        target_compile_options(png_static PRIVATE "/w")
    endif()
endif()

set(PNG_INCLUDE_DIR ${PNG_FOUND_ROOT} ${CMAKE_BINARY_DIR}/dependencies/png CACHE PATH "")
set(PNG_LIBRARY_DIR "" CACHE PATH "")
set(PNG_LIBRARY png_static CACHE STRING "")
