project(FreeImage)

file(GLOB freeimage_sources FreeImage/*.cpp FreeImage/*.h)
file(GLOB freeimage_toolkit_sources FreeImageToolkit/*.cpp FreeImageToolkit/*.h)
file(GLOB metadata_sources Metadata/*.cpp Metadata/*.h)
file(GLOB freeimage_headers ./*.h ./*.hpp)

# The MNG chunks parser is shared by the JNG and the MNG plugins only
if (NOT FREEIMAGE_WITH_PLUGIN_JNG AND NOT FREEIMAGE_WITH_PLUGIN_MNG)
    list(REMOVE_ITEM freeimage_sources ${CMAKE_CURRENT_SOURCE_DIR}/FreeImage/MNGHelper.cpp)
endif()
set(freeimage_plugins)


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


if (FREEIMAGE_WITH_PYTHON_BINDINGS)
    target_include_directories(FreeImage PRIVATE ${PYBIND_INCLUDE_DIR} ${Python3_INCLUDE_DIRS})
    target_compile_definitions(FreeImage PRIVATE "-DFREEIMAGE_WITH_PYTHON_BINDINGS=1")
    target_link_directories(FreeImage PRIVATE ${Python3_LIBRARY_DIRS})
    target_link_libraries(FreeImage PRIVATE LibPybind ${Python3_LIBRARIES})
endif()


target_link_libraries(FreeImage PRIVATE LibYato)
target_link_libraries(FreeImage PRIVATE LibZLIB)

# The header with the FREEIMAGE_WITH_PLUGIN_<NAME> and FREEIMAGE_WITH_LIB<NAME> switches (1 or 0) is generated into the build tree
set(FREEIMAGE_OPTION_DEFINES "")
foreach(plugin_name IN LISTS FREEIMAGE_BUILTIN_PLUGINS)
    if (FREEIMAGE_WITH_PLUGIN_${plugin_name})
        string(APPEND FREEIMAGE_OPTION_DEFINES "#define FREEIMAGE_WITH_PLUGIN_${plugin_name} 1
")
        list(APPEND freeimage_plugins Plugins/Plugin${plugin_name}.cpp)
    else()
        string(APPEND FREEIMAGE_OPTION_DEFINES "#define FREEIMAGE_WITH_PLUGIN_${plugin_name} 0
")
    endif()
endforeach()
foreach(lib_name OPENEXR JPEG OPENJPEG PNG TIFF WEBP RAW HEIF JPEGXL JXR)
    if (FREEIMAGE_WITH_LIB${lib_name})
        string(APPEND FREEIMAGE_OPTION_DEFINES "#define FREEIMAGE_WITH_LIB${lib_name} 1
")
    else()
        string(APPEND FREEIMAGE_OPTION_DEFINES "#define FREEIMAGE_WITH_LIB${lib_name} 0
")
    endif()
endforeach()
if (FREEIMAGE_WITH_LIBJPEG AND FREEIMAGE_WITH_LIBJPEG_TRANSFORMS)
    string(APPEND FREEIMAGE_OPTION_DEFINES "#define FREEIMAGE_WITH_LIBJPEG_TRANSFORMS 1
")
else()
    string(APPEND FREEIMAGE_OPTION_DEFINES "#define FREEIMAGE_WITH_LIBJPEG_TRANSFORMS 0
")
endif()
configure_file(FreeImage/FreeImageOptions.h.in ${CMAKE_CURRENT_BINARY_DIR}/gen/FreeImageOptions.h @ONLY)
target_include_directories(FreeImage PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/gen)

# PSDParser is used by the PSD and the TIFF plugins
if (FREEIMAGE_WITH_PLUGIN_PSD OR FREEIMAGE_WITH_LIBTIFF)
    list(APPEND freeimage_plugins Plugins/PSDParser.cpp Plugins/PSDParser.h)
endif()


if (FREEIMAGE_WITH_LIBOPENEXR)
    target_link_libraries(FreeImage PRIVATE LibOpenEXR)
    list(APPEND freeimage_plugins Plugins/PluginEXR.cpp)
endif()

if (FREEIMAGE_WITH_LIBJPEG)
    target_link_libraries(FreeImage PRIVATE LibJPEG)
    list(APPEND freeimage_plugins Plugins/PluginJPEG.cpp)
endif()

if (FREEIMAGE_WITH_LIBOPENJPEG)
    target_link_libraries(FreeImage PRIVATE LibOpenJPEG)
    list(APPEND freeimage_plugins Plugins/PluginJ2K.cpp Plugins/PluginJP2.cpp Plugins/J2KHelper.cpp Plugins/J2KHelper.h)
endif()

if (FREEIMAGE_WITH_LIBPNG)
    target_link_libraries(FreeImage PRIVATE LibPNG)
    list(APPEND freeimage_plugins Plugins/PluginPNG.cpp)
endif()

if (FREEIMAGE_WITH_LIBTIFF)
    target_link_libraries(FreeImage PRIVATE LibTIFF)
    list(APPEND freeimage_plugins Plugins/PluginTIFF.cpp Plugins/PluginG3.cpp Plugins/TIFFLogLuv.cpp Plugins/XTIFF.cpp)
endif()

if (FREEIMAGE_WITH_LIBWEBP)
    target_link_libraries(FreeImage PRIVATE LibWEBP)
    list(APPEND freeimage_plugins Plugins/PluginWebP.cpp)
endif()

if (FREEIMAGE_WITH_LIBRAW)
    target_link_libraries(FreeImage PRIVATE LibRAW)
    list(APPEND freeimage_plugins Plugins/PluginRAW.cpp)
endif()

if (FREEIMAGE_WITH_LIBHEIF)
    target_link_libraries(FreeImage PRIVATE LibHEIF)
    list(APPEND freeimage_plugins Plugins/PluginHEIF.cpp)
endif()

if (FREEIMAGE_WITH_LIBJXR)
    target_link_libraries(FreeImage PRIVATE LibJXR)
    list(APPEND freeimage_plugins Plugins/PluginJXR.cpp)
endif()

if (FREEIMAGE_WITH_LIBJPEGXL)
    target_link_libraries(FreeImage PRIVATE LibJpegXL)
    list(APPEND freeimage_plugins Plugins/PluginJPEGXL.cpp)
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
if (FREEIMAGE_WITH_LIBHEIF)
    install(FILES ${LibHEIF_INSTALL_LIBRARY} TYPE ${LibHEIF_INSTALL_TYPE} RENAME ${LibHEIF_INSTALL_NAME})
endif()
