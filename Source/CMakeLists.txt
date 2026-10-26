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

if(MSVC)
    add_definitions("-D_CRT_SECURE_NO_WARNINGS=1")
endif()


add_library(FreeImage SHARED)


set(FREEIMAGE_COLORORDER "RGB" CACHE STRING "Color order used for FIR_BITMAP by default (RGB, BGR). Use BGR only for backward compatibility.")
set_property(CACHE FREEIMAGE_COLORORDER PROPERTY STRINGS RGB BGR)
if (${FREEIMAGE_COLORORDER} STREQUAL "BGR")
    target_compile_definitions(FreeImage PUBLIC "-DFREEIMAGE_COLORORDER=0")
elseif(${FREEIMAGE_COLORORDER} STREQUAL "RGB")
    target_compile_definitions(FreeImage PUBLIC "-DFREEIMAGE_COLORORDER=1")
else()
    message(FATAL_ERROR "Invalid value of the FREEIMAGE_COLORORDER option. Must be RGB or BGR")
endif()


target_link_libraries(FreeImage PRIVATE LibYato)
target_link_libraries(FreeImage PRIVATE LibZLIB)


if (FREEIMAGE_WITH_LIBOPENEXR)
    target_compile_definitions(FreeImage PUBLIC "-DFREEIMAGE_WITH_LIBOPENEXR=1")
    target_link_libraries(FreeImage PRIVATE LibOpenEXR)
    list(APPEND freeimage_plugins Plugins/PluginEXR.cpp)
endif()

if (FREEIMAGE_WITH_LIBJPEG)
    target_compile_definitions(FreeImage PUBLIC "-DFREEIMAGE_WITH_LIBJPEG=1")
    if (FREEIMAGE_WITH_LIBJPEG_TRANSFORMS)
        target_compile_definitions(FreeImage PUBLIC "-DFREEIMAGE_WITH_LIBJPEG_TRANSFORMS=1")
    endif()
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

if (FREEIMAGE_WITH_LIBJPEGXL)
    target_compile_definitions(FreeImage PUBLIC "-DFREEIMAGE_WITH_LIBJPEGXL=1")
    target_link_libraries(FreeImage PRIVATE LibJpegXL)
    list(APPEND freeimage_plugins Plugins/PluginJPEGXL.cpp)
endif()


list(APPEND freeimage_all_sources ${freeimage_sources} ${freeimage_toolkit_sources} ${metadata_sources} ${freeimage_headers} ${freeimage_plugins})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "Source files" FILES ${freeimage_all_sources})
target_sources(FreeImage PRIVATE ${freeimage_all_sources})
set_target_properties(FreeImage PROPERTIES PUBLIC_HEADER "FreeImage.h;FreeImage.hpp")

# Version resource of the DLL
if (WIN32)
    # The fourth number of the file version. Use it to distinguish rebuilds of the same FreeImageRe version.
    set(FREEIMAGE_VERSION_TWEAK "0" CACHE STRING "Fourth component of the version number in the Windows version resource")
    # If it is not empty, the file is marked as a special build and the text is shown in the file properties.
    set(FREEIMAGE_SPECIAL_BUILD "" CACHE STRING "Description of a special build in the Windows version resource")

    if (NOT FREEIMAGE_VERSION_TWEAK MATCHES "^(0|[1-9][0-9]*)$" OR FREEIMAGE_VERSION_TWEAK GREATER 65535)
        message(FATAL_ERROR "Invalid value of the FREEIMAGE_VERSION_TWEAK option. Must be an integer in range 0..65535 without leading zeros")
    endif()

    if (NOT "${FREEIMAGE_SPECIAL_BUILD}" STREQUAL "")
        set(FREEIMAGE_RC_SPECIAL_BUILD_FLAG "VS_FF_SPECIALBUILD")
        # escape the string for the resource compiler
        string(REPLACE "\\" "\\\\" FREEIMAGE_SPECIAL_BUILD_ESCAPED "${FREEIMAGE_SPECIAL_BUILD}")
        string(REPLACE "\"" "\"\"" FREEIMAGE_SPECIAL_BUILD_ESCAPED "${FREEIMAGE_SPECIAL_BUILD_ESCAPED}")
        set(FREEIMAGE_RC_SPECIAL_BUILD_VALUE "            VALUE \"SpecialBuild\", \"${FREEIMAGE_SPECIAL_BUILD_ESCAPED}\\0\"")
    else()
        set(FREEIMAGE_RC_SPECIAL_BUILD_FLAG "0")
        set(FREEIMAGE_RC_SPECIAL_BUILD_VALUE "")
    endif()

    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/FreeImage.rc.in ${CMAKE_CURRENT_BINARY_DIR}/gen/FreeImage.rc @ONLY)
    target_sources(FreeImage PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/gen/FreeImage.rc)
endif()

message(STATUS "CMAKE_INSTALL_BINDIR=${CMAKE_INSTALL_PREFIX}")
install(TARGETS FreeImage COMPONENT dist)
if (FREEIMAGE_WITH_LIBHEIF)
    install(FILES ${LibHEIF_INSTALL_LIBRARY} TYPE ${LibHEIF_INSTALL_TYPE} RENAME ${LibHEIF_INSTALL_NAME} COMPONENT dist)
endif()


if (FREEIMAGE_WITH_PYTHON_BINDINGS)
    add_subdirectory(Python)
endif()
