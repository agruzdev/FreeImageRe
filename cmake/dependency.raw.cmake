# LibRaw dependency
# https://github.com/LibRaw/LibRaw/releases/tag/0.20.0

# Output variables:
# RAW_INCLUDE_DIR - includes
# RAW_LIBRARY_DIR - link directories
# RAW_LIBRARY     - link targets

include(${CMAKE_SOURCE_DIR}/cmake/dependency.common.functions.cmake)

dependency_find_or_download(
    NAME RAW
    VERBOSE_NAME "LibRaw"
    URL "https://github.com/LibRaw/LibRaw/archive/refs/tags/0.20.0.zip"
    HASH_MD5 "b1de41f0cab4817e72e5e38aab3eb912"
    FILE_NAME "rawlite.0.20.0.zip"
    PREFIX "LibRaw-0.20.0"
)

add_library(LibRaw STATIC
    ${RAW_FOUND_ROOT}/internal/dcraw_common.cpp 
    ${RAW_FOUND_ROOT}/internal/dcraw_fileio.cpp 
    ${RAW_FOUND_ROOT}/internal/demosaic_packs.cpp 
    ${RAW_FOUND_ROOT}/src/libraw_c_api.cpp 
    ${RAW_FOUND_ROOT}/src/libraw_cxx.cpp 
    ${RAW_FOUND_ROOT}/src/libraw_datastream.cpp
)

target_include_directories(LibRaw 
    PUBLIC  ${RAW_FOUND_ROOT}
    PRIVATE ${RAW_FOUND_ROOT}/dcraw
            ${RAW_FOUND_ROOT}/internal
            ${RAW_FOUND_ROOT}/libraw
            ${RAW_FOUND_ROOT}/src
)

target_compile_definitions(LibRaw PUBLIC "-DLIBRAW_NODLL")

set_property(TARGET LibRaw PROPERTY FOLDER "Dependencies")

if (MSVC)
    target_compile_options(LibRaw PRIVATE "/w")
endif()


set(RAW_INCLUDE_DIR ${RAWLITE_FOUND_ROOT} CACHE PATH "")
set(RAW_LIBRARY_DIR "" CACHE PATH "")
set(RAW_LIBRARY LibRaw CACHE STRING "")
