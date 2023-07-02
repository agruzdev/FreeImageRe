# LibPNG dependency
# https://sourceforge.net/projects/libpng

# Output variables:
# PNG_INCLUDE_DIR - includes
# PNG_LIBRARY_DIR - link directories
# PNG_LIBRARY     - link targets

include(${CMAKE_SOURCE_DIR}/cmake/dependency.common.functions.cmake)

dependency_find_or_download(
    NAME PNG
    VERBOSE_NAME "LibPNG"
    URL "https://sourceforge.net/projects/libpng/files/libpng16/1.6.37/libpng-1.6.37.tar.gz/download?download="
    HASH_MD5 "6c7519f6c75939efa0ed3053197abd54"
    FILE_NAME "libpng-1.6.37.tar.gz"
    PREFIX "libpng-1.6.37"
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

    add_dependencies(png_static zlibstatic)
endif()

set(PNG_INCLUDE_DIR ${PNG_FOUND_ROOT} ${CMAKE_BINARY_DIR}/dependencies/png CACHE PATH "")
set(PNG_LIBRARY_DIR "" CACHE PATH "")
set(PNG_LIBRARY png_static CACHE STRING "")
