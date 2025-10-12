# LibTIFF dependency
# http://www.libtiff.org/
#
# Output targets: LibTIFF


include(${CMAKE_SOURCE_DIR}/cmake/external_project_common.cmake)

ExternalProject_Add(TIFF
    PREFIX ${CMAKE_BINARY_DIR}/tiff
    URL "http://download.osgeo.org/libtiff/tiff-4.7.1.zip"
    URL_MD5 "a45fdf2169dae98635c7c16729a85471"
    DOWNLOAD_DIR "${CMAKE_SOURCE_DIR}/dependencies/tiff"
    SOURCE_DIR "${EXTERNALPROJECT_SOURCE_PREFIX}/dependencies/tiff/source"
    BINARY_DIR "${CMAKE_BINARY_DIR}/tiff/build"
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    BUILD_COMMAND ${BUILD_COMMAND_FOR_TARGET} -t tiff
    INSTALL_COMMAND ""
    CMAKE_ARGS ${CMAKE_BUILD_TYPE_ARG} "-DZLIB_ROOT:PATH=${ZLIB_ROOT}" "-Dzlib=ON" "-Dlibdeflate=OFF" "-Djpeg=OFF" "-Dold-jpeg=OFF" "-Djpeg12=OFF" "-Djbig=OFF" "-Dwebp=OFF"
        "-Dtiff-tools=OFF" "-Dtiff-tests=OFF" "-Dtiff-docs=OFF" "-Dtiff-install=OFF" "-Dwin32-io=OFF" "-Dcxx=OFF"
        "-DBUILD_SHARED_LIBS=OFF" "-DCMAKE_C_FLAGS:STRING=${ZERO_WARNINGS_FLAG} -fPIC"
    EXCLUDE_FROM_ALL
    DEPENDS ZLIB
)

ExternalProject_Get_Property(TIFF SOURCE_DIR)
ExternalProject_Get_Property(TIFF BINARY_DIR)

add_library(LibTIFF INTERFACE)
add_dependencies(LibTIFF TIFF)
link_config_aware_library_path2(LibTIFF ${BINARY_DIR}/libtiff ${CMAKE_STATIC_LIBRARY_PREFIX}tiff${CMAKE_STATIC_LIBRARY_SUFFIX} ${CMAKE_STATIC_LIBRARY_PREFIX}tiffd${CMAKE_STATIC_LIBRARY_SUFFIX})
target_include_directories(LibTIFF INTERFACE ${SOURCE_DIR}/libtiff ${BINARY_DIR}/libtiff)
set_property(TARGET TIFF PROPERTY FOLDER "Dependencies")

unset(SOURCE_DIR)
unset(BINARY_DIR)

