# CMake for LibRaw dependency


file(GLOB LibRaw_sources 
    ${RAW_FOUND_ROOT}/src/decoders/*
    ${RAW_FOUND_ROOT}/src/decompressors/*
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

add_library(LibRAW STATIC ${LibRaw_sources})

unset(LibRaw_sources)

target_include_directories(LibRAW 
    PUBLIC  ${RAW_FOUND_ROOT}
    PRIVATE ${RAW_FOUND_ROOT}/dcraw
            ${RAW_FOUND_ROOT}/internal
            ${RAW_FOUND_ROOT}/libraw
            ${RAW_FOUND_ROOT}/src
)


target_compile_definitions(LibRAW PUBLIC "-DLIBRAW_NODLL")

if (MSVC)
    target_compile_options(LibRAW PRIVATE "/w")
else()
    target_compile_options(LibRAW PRIVATE "-w")
endif()


