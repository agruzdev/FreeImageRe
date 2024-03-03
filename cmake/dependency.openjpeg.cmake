# OpenJPEG dependency (JPEG 2000)

# Output variables:
# OPENJPEG_INCLUDE_DIR - includes
# OPENJPEG_LIBRARY_DIR - link directories
# OPENJPEG_LIBRARY     - link targets

include(${CMAKE_SOURCE_DIR}/cmake/dependency.common.functions.cmake)

# https://github.com/uclouvain/openjpeg/releases
dependency_find_or_download(
    NAME OPENJPEG
    VERBOSE_NAME "OpenJPEG"
    URL "https://github.com/uclouvain/openjpeg/archive/refs/tags/v2.5.2.zip"
    HASH_MD5 "6295f52db915f7b4afd3241d3692cf00"
    FILE_NAME "v2.5.2.zip"
    PREFIX "openjpeg-2.5.2"
)

if(NOT TARGET openjp2)

    set(BUILD_STATIC_LIBS ON)
    set(BUILD_SHARED_LIBS OFF)
    set(BUILD_CODEC OFF)
    set(BUILD_JPIP OFF)
    set(BUILD_TESTING OFF)

    add_subdirectory(${OPENJPEG_FOUND_ROOT} ${CMAKE_BINARY_DIR}/dependencies/openjpeg)
    set_property(TARGET openjp2 PROPERTY FOLDER "Dependencies")

    unset(BUILD_STATIC_LIBS)
    unset(BUILD_SHARED_LIBS)
    unset(BUILD_CODEC)
    unset(BUILD_JPIP)
    unset(BUILD_TESTING)

endif()

set(OPENJPEG_INCLUDE_DIR ${OPENJPEG_FOUND_ROOT}/src/lib ${CMAKE_BINARY_DIR}/dependencies/openjpeg/src/lib/openjp2 CACHE PATH "")
set(OPENJPEG_LIBRARY_DIR "" CACHE PATH "")
set(OPENJPEG_LIBRARY openjp2 CACHE STRING "")




