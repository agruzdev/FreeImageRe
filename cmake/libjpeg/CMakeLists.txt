cmake_minimum_required(VERSION 3.28)
project(LibJPEG)

set(jpeg_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/jaricom.c
    ${CMAKE_CURRENT_SOURCE_DIR}/jcapimin.c
    ${CMAKE_CURRENT_SOURCE_DIR}/jcapistd.c 
    ${CMAKE_CURRENT_SOURCE_DIR}/jcarith.c 
    ${CMAKE_CURRENT_SOURCE_DIR}/jccoefct.c 
    ${CMAKE_CURRENT_SOURCE_DIR}/jccolor.c 
    ${CMAKE_CURRENT_SOURCE_DIR}/jcdctmgr.c 
    ${CMAKE_CURRENT_SOURCE_DIR}/jchuff.c 
    ${CMAKE_CURRENT_SOURCE_DIR}/jcinit.c 
    ${CMAKE_CURRENT_SOURCE_DIR}/jcmainct.c 
    ${CMAKE_CURRENT_SOURCE_DIR}/jcmarker.c 
    ${CMAKE_CURRENT_SOURCE_DIR}/jcmaster.c 
    ${CMAKE_CURRENT_SOURCE_DIR}/jcomapi.c 
    ${CMAKE_CURRENT_SOURCE_DIR}/jcparam.c 
    ${CMAKE_CURRENT_SOURCE_DIR}/jcprepct.c 
    ${CMAKE_CURRENT_SOURCE_DIR}/jcsample.c 
    ${CMAKE_CURRENT_SOURCE_DIR}/jctrans.c 
    ${CMAKE_CURRENT_SOURCE_DIR}/jdapimin.c 
    ${CMAKE_CURRENT_SOURCE_DIR}/jdapistd.c 
    ${CMAKE_CURRENT_SOURCE_DIR}/jdarith.c 
    ${CMAKE_CURRENT_SOURCE_DIR}/jdatadst.c 
    ${CMAKE_CURRENT_SOURCE_DIR}/jdatasrc.c 
    ${CMAKE_CURRENT_SOURCE_DIR}/jdcoefct.c 
    ${CMAKE_CURRENT_SOURCE_DIR}/jdcolor.c 
    ${CMAKE_CURRENT_SOURCE_DIR}/jddctmgr.c 
    ${CMAKE_CURRENT_SOURCE_DIR}/jdhuff.c 
    ${CMAKE_CURRENT_SOURCE_DIR}/jdinput.c 
    ${CMAKE_CURRENT_SOURCE_DIR}/jdmainct.c 
    ${CMAKE_CURRENT_SOURCE_DIR}/jdmarker.c 
    ${CMAKE_CURRENT_SOURCE_DIR}/jdmaster.c 
    ${CMAKE_CURRENT_SOURCE_DIR}/jdmerge.c 
    ${CMAKE_CURRENT_SOURCE_DIR}/jdpostct.c 
    ${CMAKE_CURRENT_SOURCE_DIR}/jdsample.c 
    ${CMAKE_CURRENT_SOURCE_DIR}/jdtrans.c 
    ${CMAKE_CURRENT_SOURCE_DIR}/jerror.c 
    ${CMAKE_CURRENT_SOURCE_DIR}/jfdctflt.c 
    ${CMAKE_CURRENT_SOURCE_DIR}/jfdctfst.c 
    ${CMAKE_CURRENT_SOURCE_DIR}/jfdctint.c 
    ${CMAKE_CURRENT_SOURCE_DIR}/jidctflt.c 
    ${CMAKE_CURRENT_SOURCE_DIR}/jidctfst.c 
    ${CMAKE_CURRENT_SOURCE_DIR}/jidctint.c 
    ${CMAKE_CURRENT_SOURCE_DIR}/jmemmgr.c 
    ${CMAKE_CURRENT_SOURCE_DIR}/jmemnobs.c 
    ${CMAKE_CURRENT_SOURCE_DIR}/jquant1.c 
    ${CMAKE_CURRENT_SOURCE_DIR}/jquant2.c 
    ${CMAKE_CURRENT_SOURCE_DIR}/jutils.c 
    ${CMAKE_CURRENT_SOURCE_DIR}/transupp.c 
)

add_library(LibJPEG STATIC ${jpeg_sources})
set_target_properties(LibJPEG PROPERTIES OUTPUT_NAME jpeg)

target_include_directories(LibJPEG PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if (MSVC)
    target_compile_options(LibJPEG PRIVATE "/w")
else()
    target_compile_options(LibJPEG PRIVATE "-w")
endif()

if (NOT WIN32)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
endif()


install(TARGETS LibJPEG DESTINATION lib)
install(FILES jconfig.h jerror.h jinclude.h jmorecfg.h jpeglib.h transupp.h jversion.h
        DESTINATION include)

