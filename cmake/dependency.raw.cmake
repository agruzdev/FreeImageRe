# LibRaw dependency
# https://github.com/LibRaw/LibRaw

# Output variables:
# RAW_INCLUDE_DIR - includes
# RAW_LIBRARY_DIR - link directories
# RAW_LIBRARY     - link targets

include(${CMAKE_SOURCE_DIR}/cmake/dependency.common.functions.cmake)

dependency_find_or_download(
    NAME RAW
    VERBOSE_NAME "LibRaw"
    URL "https://github.com/LibRaw/LibRaw/archive/refs/tags/0.21.2.zip"
    HASH_MD5 "37a020696bda1f071c99ff22a5130e83"
    FILE_NAME "rawlite.0.21.2.zip"
    PREFIX "LibRaw-0.21.2"
)

file(GLOB LibRaw_sources 
    ${RAW_FOUND_ROOT}/src/decoders/*
    ${RAW_FOUND_ROOT}/src/demosaic/*
    ${RAW_FOUND_ROOT}/src/integration/*
    ${RAW_FOUND_ROOT}/src/metadata/*
    ${RAW_FOUND_ROOT}/src/tables/*
    ${RAW_FOUND_ROOT}/src/utils/*
    ${RAW_FOUND_ROOT}/src/x3f/*
    ${RAW_FOUND_ROOT}/src/postprocessing/aspect_ratio.cpp
    ${RAW_FOUND_ROOT}/src/postprocessing/dcraw_process.cpp
    ${RAW_FOUND_ROOT}/src/postprocessing/mem_image.cpp
    ${RAW_FOUND_ROOT}/src/postprocessing/postprocessing_aux.cpp
    ${RAW_FOUND_ROOT}/src/postprocessing/postprocessing_utils.cpp
    ${RAW_FOUND_ROOT}/src/postprocessing/postprocessing_utils_dcrdefs.cpp
    ${RAW_FOUND_ROOT}/src/preprocessing/ext_preprocess.cpp
    ${RAW_FOUND_ROOT}/src/preprocessing/raw2image.cpp
    ${RAW_FOUND_ROOT}/src/preprocessing/subtract_black.cpp
    ${RAW_FOUND_ROOT}/src/write/apply_profile.cpp
    ${RAW_FOUND_ROOT}/src/write/file_write.cpp
    ${RAW_FOUND_ROOT}/src/write/tiff_writer.cpp
    ${RAW_FOUND_ROOT}/src/libraw_c_api.cpp
    ${RAW_FOUND_ROOT}/src/libraw_datastream.cpp
)

add_library(LibRaw STATIC ${LibRaw_sources})

unset(LibRaw_sources)

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
