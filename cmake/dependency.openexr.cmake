# OpenEXR dependency
# https://github.com/AcademySoftwareFoundation/openexr

# Output variables:
# EXR_INCLUDE_DIR - includes
# EXR_LIBRARY_DIR - link directories
# EXR_LIBRARY     - link targets

include(${CMAKE_SOURCE_DIR}/cmake/dependency.common.functions.cmake)

dependency_find_or_download(
    NAME EXR
    VERBOSE_NAME "OpenEXR"
    URL "https://github.com/AcademySoftwareFoundation/openexr/archive/refs/tags/v3.2.2.zip"
    HASH_MD5 "0"
    FILE_NAME "v3.2.2.zip"
    PREFIX "openexr-3.2.2"
)

if(NOT TARGET OpenEXR)
    set(OPENEXR_IS_SUBPROJECT ON)
    set(OPENEXR_FORCE_INTERNAL_IMATH ON CACHE INTERNAL "")
    set(OPENEXR_FORCE_INTERNAL_ZLIB OFF CACHE INTERNAL "")

    set(OPENEXR_INSTALL OFF CACHE INTERNAL "")
    set(OPENEXR_INSTALL_TOOLS OFF CACHE INTERNAL "")
    set(OPENEXR_INSTALL_EXAMPLES OFF CACHE INTERNAL "")
    set(OPENEXR_INSTALL_PKG_CONFIG OFF CACHE INTERNAL "")

    set(BUILD_SHARED_LIBS OFF)

    add_subdirectory(${EXR_FOUND_ROOT} ${CMAKE_BINARY_DIR}/dependencies/exr EXCLUDE_FROM_ALL)
    set_property(TARGET OpenEXR OpenEXRCore Iex IlmThread Imath PROPERTY FOLDER "Dependencies")

    if (MSVC)
        target_compile_options(Iex PRIVATE "/w")
        target_compile_options(IlmThread PRIVATE "/w")
        target_compile_options(Imath PRIVATE "/w")
        target_compile_options(OpenEXR PRIVATE "/w")
    endif()

    target_link_libraries(OpenEXR PRIVATE zlibstatic)

    target_include_directories(OpenEXR INTERFACE ${EXR_FOUND_ROOT}/src/lib ${CMAKE_BINARY_DIR}/dependencies/exr)
    unset(BUILD_SHARED_LIBS)
endif()

set(EXR_INCLUDE_DIR ${EXR_FOUND_ROOT}/src/lib ${CMAKE_BINARY_DIR}/dependencies/exr CACHE PATH "")
set(EXR_LIBRARY_DIR "" CACHE PATH "")
set(EXR_LIBRARY OpenEXR CACHE STRING "")
