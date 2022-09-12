set(jpeg_sources
    ${JPEG_IJG_FOUND_ROOT}/jaricom.c
    ${JPEG_IJG_FOUND_ROOT}/jcapimin.c
    ${JPEG_IJG_FOUND_ROOT}/jcapistd.c 
    ${JPEG_IJG_FOUND_ROOT}/jcarith.c 
    ${JPEG_IJG_FOUND_ROOT}/jccoefct.c 
    ${JPEG_IJG_FOUND_ROOT}/jccolor.c 
    ${JPEG_IJG_FOUND_ROOT}/jcdctmgr.c 
    ${JPEG_IJG_FOUND_ROOT}/jchuff.c 
    ${JPEG_IJG_FOUND_ROOT}/jcinit.c 
    ${JPEG_IJG_FOUND_ROOT}/jcmainct.c 
    ${JPEG_IJG_FOUND_ROOT}/jcmarker.c 
    ${JPEG_IJG_FOUND_ROOT}/jcmaster.c 
    ${JPEG_IJG_FOUND_ROOT}/jcomapi.c 
    ${JPEG_IJG_FOUND_ROOT}/jcparam.c 
    ${JPEG_IJG_FOUND_ROOT}/jcprepct.c 
    ${JPEG_IJG_FOUND_ROOT}/jcsample.c 
    ${JPEG_IJG_FOUND_ROOT}/jctrans.c 
    ${JPEG_IJG_FOUND_ROOT}/jdapimin.c 
    ${JPEG_IJG_FOUND_ROOT}/jdapistd.c 
    ${JPEG_IJG_FOUND_ROOT}/jdarith.c 
    ${JPEG_IJG_FOUND_ROOT}/jdatadst.c 
    ${JPEG_IJG_FOUND_ROOT}/jdatasrc.c 
    ${JPEG_IJG_FOUND_ROOT}/jdcoefct.c 
    ${JPEG_IJG_FOUND_ROOT}/jdcolor.c 
    ${JPEG_IJG_FOUND_ROOT}/jddctmgr.c 
    ${JPEG_IJG_FOUND_ROOT}/jdhuff.c 
    ${JPEG_IJG_FOUND_ROOT}/jdinput.c 
    ${JPEG_IJG_FOUND_ROOT}/jdmainct.c 
    ${JPEG_IJG_FOUND_ROOT}/jdmarker.c 
    ${JPEG_IJG_FOUND_ROOT}/jdmaster.c 
    ${JPEG_IJG_FOUND_ROOT}/jdmerge.c 
    ${JPEG_IJG_FOUND_ROOT}/jdpostct.c 
    ${JPEG_IJG_FOUND_ROOT}/jdsample.c 
    ${JPEG_IJG_FOUND_ROOT}/jdtrans.c 
    ${JPEG_IJG_FOUND_ROOT}/jerror.c 
    ${JPEG_IJG_FOUND_ROOT}/jfdctflt.c 
    ${JPEG_IJG_FOUND_ROOT}/jfdctfst.c 
    ${JPEG_IJG_FOUND_ROOT}/jfdctint.c 
    ${JPEG_IJG_FOUND_ROOT}/jidctflt.c 
    ${JPEG_IJG_FOUND_ROOT}/jidctfst.c 
    ${JPEG_IJG_FOUND_ROOT}/jidctint.c 
    ${JPEG_IJG_FOUND_ROOT}/jmemmgr.c 
    ${JPEG_IJG_FOUND_ROOT}/jmemnobs.c 
    ${JPEG_IJG_FOUND_ROOT}/jquant1.c 
    ${JPEG_IJG_FOUND_ROOT}/jquant2.c 
    ${JPEG_IJG_FOUND_ROOT}/jutils.c 
    ${JPEG_IJG_FOUND_ROOT}/transupp.c 
)

add_library(LibJPEG STATIC ${jpeg_sources})

target_include_directories(LibJPEG PRIVATE ${JPEG_IJG_FOUND_ROOT})
target_include_directories(LibJPEG PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if (MSVC)
    target_compile_options(LibJPEG PRIVATE "/w")
endif()
