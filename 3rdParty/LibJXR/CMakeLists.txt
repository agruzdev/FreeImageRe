
set(jxr_sources
    ./image/decode/decode.c 
    ./image/decode/JXRTranscode.c 
    ./image/decode/postprocess.c 
    ./image/decode/segdec.c 
    ./image/decode/strdec.c 
    ./image/decode/strdec_x86.c 
    ./image/decode/strInvTransform.c 
    ./image/decode/strPredQuantDec.c 
    ./image/encode/encode.c 
    ./image/encode/segenc.c 
    ./image/encode/strenc.c 
    ./image/encode/strenc_x86.c 
    ./image/encode/strFwdTransform.c 
    ./image/encode/strPredQuantEnc.c 
    ./image/sys/adapthuff.c 
    ./image/sys/image.c 
    ./image/sys/strcodec.c 
    ./image/sys/strPredQuant.c 
    ./image/sys/strTransform.c 
    ./jxrgluelib/JXRGlue.c 
    ./jxrgluelib/JXRGlueJxr.c 
    ./jxrgluelib/JXRGluePFC.c 
    ./jxrgluelib/JXRMeta.c 
)

add_library(LibJXR STATIC ${jxr_sources})

target_compile_definitions(LibJXR PUBLIC -DDISABLE_PERF_MEASUREMENT)

target_include_directories(LibJXR PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(LibJXR PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/jxrgluelib)
target_include_directories(LibJXR PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/image/sys)
target_include_directories(LibJXR PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/common/include)
target_include_directories(LibJXR PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/image)
target_include_directories(LibJXR PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/image/decode)
target_include_directories(LibJXR PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/image/encode)
target_include_directories(LibJXR PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/image/x86)

set_property(TARGET LibJXR PROPERTY FOLDER "Dependencies")

if (MSVC)
    target_compile_options(LibJXR PRIVATE "/w")
else() # UNIX like
    target_compile_options(LibJXR PRIVATE "-D__ANSI__=1")
endif()

