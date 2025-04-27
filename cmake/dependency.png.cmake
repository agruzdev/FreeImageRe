# LibPNG dependency
# https://sourceforge.net/projects/libpng
#
# Output target: LibPNG


include(${CMAKE_SOURCE_DIR}/cmake/external_project_common.cmake)

ExternalProject_Get_Property(ZLIB INSTALL_DIR)
set(ZLIB_INSTALL_DIR ${INSTALL_DIR})
unset(INSTALL_DIR)

ExternalProject_Add(PNG
    PREFIX ${CMAKE_BINARY_DIR}/png
    URL "http://prdownloads.sourceforge.net/libpng/lpng1647.zip?download="
    URL_MD5 "ad8a3a214ee411e4c1e1d0a64f9704a0"
    DOWNLOAD_DIR "${CMAKE_SOURCE_DIR}/dependencies/png"
    SOURCE_DIR "${EXTERNALPROJECT_SOURCE_PREFIX}/dependencies/png/source"
    BINARY_DIR "${CMAKE_BINARY_DIR}/png/build"
    INSTALL_DIR "${CMAKE_BINARY_DIR}/png/install"
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    BUILD_COMMAND ${BUILD_COMMAND_FOR_TARGET} -t png_static
    CMAKE_ARGS ${CMAKE_BUILD_TYPE_ARG} "-DZLIB_ROOT:PATH=${ZLIB_ROOT}" "-DPNG_SHARED=OFF" "-DPNG_STATIC=ON" "-DPNG_TESTS=OFF" "-DPNG_TOOLS=OFF" "-DPNG_FRAMEWORK=OFF" "-DPNG_DEBUG=OFF" 
        "-DPNG_BUILD_ZLIB=OFF" "-DPNG_HARDWARE_OPTIMIZATIONS=ON" "-DCMAKE_C_FLAGS:STRING=${ZERO_WARNINGS_FLAG} -fPIC"
        "-DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_BINARY_DIR}/png/install" "-DSKIP_INSTALL_EXECUTABLES=ON" "-DSKIP_INSTALL_PROGRAMS=ON"
    EXCLUDE_FROM_ALL
    DEPENDS ZLIB
)

ExternalProject_Get_Property(PNG INSTALL_DIR)

add_library(LibPNG INTERFACE)
add_dependencies(LibPNG PNG)
if(UNIX)
    link_library_path2(LibPNG ${INSTALL_DIR}/lib libpng16${CMAKE_STATIC_LIBRARY_SUFFIX} libpng16d${CMAKE_STATIC_LIBRARY_SUFFIX})
else()
    link_library_path2(LibPNG ${INSTALL_DIR}/lib libpng16_static${CMAKE_STATIC_LIBRARY_SUFFIX} libpng16_staticd${CMAKE_STATIC_LIBRARY_SUFFIX})
endif()
target_include_directories(LibPNG INTERFACE ${INSTALL_DIR}/include)
set_property(TARGET PNG PROPERTY FOLDER "Dependencies")

unset(INSTALL_DIR)

