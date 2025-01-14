project(FreeImage)

file(GLOB freeimage_sources FreeImage/*.cpp FreeImage/*.h)
file(GLOB freeimage_toolkit_sources FreeImageToolkit/*.cpp FreeImageToolkit/*.h)
file(GLOB metadata_sources Metadata/*.cpp Metadata/*.h)
file(GLOB freeimage_headers ./*.h ./*.hpp)
set(freeimage_plugins
    Plugins/PluginBMP.cpp
    Plugins/PluginCUT.cpp
    Plugins/PluginDDS.cpp
    Plugins/PluginGIF.cpp
    Plugins/PluginHDR.cpp
    Plugins/PluginICO.cpp
    Plugins/PluginIFF.cpp
    Plugins/PluginJNG.cpp
    Plugins/PluginKOALA.cpp
    Plugins/PluginMNG.cpp
    Plugins/PluginPCD.cpp
    Plugins/PluginPCX.cpp
    Plugins/PluginPFM.cpp
    Plugins/PluginPICT.cpp
    Plugins/PluginPNM.cpp
    Plugins/PluginPSD.cpp
    Plugins/PluginRAS.cpp
    Plugins/PluginSGI.cpp
    Plugins/PluginTARGA.cpp
    Plugins/PluginWBMP.cpp
    Plugins/PluginXBM.cpp
    Plugins/PluginXPM.cpp
    Plugins/PSDParser.cpp
    Plugins/PSDParser.h
)


include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Metadata
    ${CMAKE_CURRENT_SOURCE_DIR}/FreeImageToolkit
)

add_definitions(
    "-DFREEIMAGE_EXPORTS"
    "-DOPJ_STATIC"
)

if(WIN32)
    add_definitions("-DWINVER=0x0500")
    if(MSVC)
        add_definitions("-D_CRT_SECURE_NO_WARNINGS=1")
    endif()
endif()

add_library(FreeImage SHARED)

target_compile_definitions(FreeImage PUBLIC "-DFREEIMAGE_COLORORDER=1")
if (FREEIMAGE_WITH_PYTHON_BINDINGS)
    target_include_directories(FreeImage PRIVATE ${PYBIND_INCLUDE_DIR} ${Python3_INCLUDE_DIRS})
    target_compile_definitions(FreeImage PRIVATE "-DFREEIMAGE_WITH_PYTHON_BINDINGS=1")
    target_link_directories(FreeImage PRIVATE ${Python3_LIBRARY_DIRS})
    target_link_libraries(FreeImage PRIVATE LibPybind ${Python3_LIBRARIES})
endif()

target_include_directories(FreeImage PRIVATE ${CMAKE_SOURCE_DIR}/3rdParty/Yato/include)
target_link_libraries(FreeImage PRIVATE LibZLIB)

if (FREEIMAGE_WITH_LIBOPENEXR)
    target_compile_definitions(FreeImage PUBLIC "-DFREEIMAGE_WITH_LIBOPENEXR=1")
    target_link_libraries(FreeImage PRIVATE LibOpenEXR)
    list(APPEND freeimage_plugins Plugins/PluginEXR.cpp)
endif()

if (FREEIMAGE_WITH_LIBJPEG)
    target_compile_definitions(FreeImage PUBLIC "-DFREEIMAGE_WITH_LIBJPEG=1")
    target_link_libraries(FreeImage PRIVATE LibJPEG)
    list(APPEND freeimage_plugins Plugins/PluginJPEG.cpp)
endif()

if (FREEIMAGE_WITH_LIBOPENJPEG)
    target_compile_definitions(FreeImage PUBLIC "-DFREEIMAGE_WITH_LIBOPENJPEG=1")
    target_link_libraries(FreeImage PRIVATE LibOpenJPEG)
    list(APPEND freeimage_plugins Plugins/PluginJ2K.cpp Plugins/PluginJP2.cpp Plugins/J2KHelper.cpp Plugins/J2KHelper.h)
endif()

if (FREEIMAGE_WITH_LIBPNG)
    target_compile_definitions(FreeImage PUBLIC "-DFREEIMAGE_WITH_LIBPNG=1")
    target_link_libraries(FreeImage PRIVATE LibPNG)
    list(APPEND freeimage_plugins Plugins/PluginPNG.cpp)
endif()

if (FREEIMAGE_WITH_LIBTIFF)
    target_compile_definitions(FreeImage PUBLIC "-DFREEIMAGE_WITH_LIBTIFF=1")
    target_link_libraries(FreeImage PRIVATE LibTIFF)
    list(APPEND freeimage_plugins Plugins/PluginTIFF.cpp Plugins/PluginG3.cpp Plugins/TIFFLogLuv.cpp Plugins/XTIFF.cpp)
endif()

if (FREEIMAGE_WITH_LIBWEBP)
    target_compile_definitions(FreeImage PUBLIC "-DFREEIMAGE_WITH_LIBWEBP=1")
    target_link_libraries(FreeImage PRIVATE LibWEBP)
    list(APPEND freeimage_plugins Plugins/PluginWebP.cpp)
endif()

if (FREEIMAGE_WITH_LIBRAW)
    target_compile_definitions(FreeImage PUBLIC "-DFREEIMAGE_WITH_LIBRAW=1")
    target_link_libraries(FreeImage PRIVATE LibRAW)
    list(APPEND freeimage_plugins Plugins/PluginRAW.cpp)
endif()

if (FREEIMAGE_WITH_LIBHEIF)
    target_compile_definitions(FreeImage PUBLIC "-DFREEIMAGE_WITH_LIBHEIF=1")
    target_link_libraries(FreeImage PRIVATE LibHEIF)
    list(APPEND freeimage_plugins Plugins/PluginHEIF.cpp)
endif()

if (FREEIMAGE_WITH_LIBJXR)
    target_compile_definitions(FreeImage PUBLIC "-DFREEIMAGE_WITH_LIBJXR=1")
    target_link_libraries(FreeImage PRIVATE LibJXR)
    list(APPEND freeimage_plugins Plugins/PluginJXR.cpp)
endif()


list(APPEND freeimage_all_sources ${freeimage_sources} ${freeimage_toolkit_sources} ${metadata_sources} ${freeimage_headers} ${freeimage_plugins})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "Source files" FILES ${freeimage_all_sources})
target_sources(FreeImage PRIVATE ${freeimage_all_sources})
set_target_properties(FreeImage PROPERTIES PUBLIC_HEADER "FreeImage.h;FreeImage.hpp")

message(STATUS "CMAKE_INSTALL_BINDIR=${CMAKE_INSTALL_PREFIX}")
install(TARGETS FreeImage)
if (FREEIMAGE_WITH_PYTHON_BINDINGS)
    if (WIN32)
        install(CODE "message(STATUS \"Make a hardlink for Python: FreeImage.pyd\")")
        install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_hardlink \"./$<TARGET_FILE_NAME:FreeImage>\" FreeImage.pyd WORKING_DIRECTORY \"${CMAKE_INSTALL_PREFIX}/bin\" COMMAND_ERROR_IS_FATAL ANY)")
    else()
        install(CODE "message(STATUS \"Make a symlink for Python: FreeImage.so\")")
        install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink \"./$<TARGET_FILE_NAME:FreeImage>\" FreeImage.so WORKING_DIRECTORY \"${CMAKE_INSTALL_PREFIX}/lib\" COMMAND_ERROR_IS_FATAL ANY)")
    endif()
endif()
