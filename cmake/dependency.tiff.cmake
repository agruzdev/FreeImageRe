# LibTIFF dependency
# http://www.libtiff.org/
#
# Output targets: LibTIFF


include(${EXTERNALPROJECT_INCLUDE_DIR}/external_project_common.cmake)
include(${EXTERNALPROJECT_INCLUDE_DIR}/dependency.imath.cmake) # Only for half.h

ExternalProject_Add(TIFF
    PREFIX ${EXTERNALPROJECT_BINARY_ROOT}/tiff
    URL "http://download.osgeo.org/libtiff/tiff-4.7.1.zip"
    URL_MD5 "a45fdf2169dae98635c7c16729a85471"
    DOWNLOAD_DIR "${EXTERNALPROJECT_SOURCE_ROOT}/tiff"
    SOURCE_DIR "${EXTERNALPROJECT_SOURCE_PREFIX}/tiff/source"
    BINARY_DIR "${EXTERNALPROJECT_BINARY_ROOT}/tiff/build"
    INSTALL_DIR "${EXTERNALPROJECT_BINARY_ROOT}/tiff/install"
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    BUILD_COMMAND ${BUILD_COMMAND_FOR_TARGET} -t tiff
    INSTALL_COMMAND ${BUILD_COMMAND_FOR_TARGET} -t install
            COMMAND ${CMAKE_COMMAND} -E copy "${EXTERNALPROJECT_SOURCE_PREFIX}/tiff/source/libtiff/tiffiop.h"
                                             "${EXTERNALPROJECT_SOURCE_PREFIX}/tiff/source/libtiff/tif_hash_set.h"
                                             "${EXTERNALPROJECT_SOURCE_PREFIX}/tiff/source/libtiff/tif_dir.h"
                                             "${EXTERNALPROJECT_BINARY_ROOT}/tiff/build/libtiff/tif_config.h"
                -t "${EXTERNALPROJECT_BINARY_ROOT}/tiff/install/include"
    CMAKE_ARGS ${CMAKE_TOOLCHAIN_FILE_ARG} ${CMAKE_BUILD_TYPE_ARG} "-DZLIB_ROOT:PATH=${ZLIB_ROOT}" 
        "-Dzlib=ON" "-Dlibdeflate=OFF" "-Djpeg=OFF" "-Dold-jpeg=OFF" "-Djpeg12=OFF" "-Djbig=OFF" "-Dwebp=OFF" "-Dzstd=OFF" "-Dlzma=OFF"
        "-Dtiff-tools=OFF" "-Dtiff-tests=OFF" "-Dtiff-docs=OFF" "-Dtiff-install=ON" "-Dwin32-io=OFF"
        "-DBUILD_SHARED_LIBS=OFF" "-DCMAKE_C_FLAGS:STRING=${ZERO_WARNINGS_FLAG} ${FPIC_FLAG}" "-DCMAKE_INSTALL_PREFIX:PATH=${EXTERNALPROJECT_BINARY_ROOT}/tiff/install"
    EXCLUDE_FROM_ALL
    DEPENDS ZLIB IMATH
)

ExternalProject_Get_Property(TIFF INSTALL_DIR)

add_library(LibTIFF INTERFACE)
add_dependencies(LibTIFF TIFF)
if (MSVC AND IS_DEBUG_CONFIG)
    target_link_libraries(LibTIFF INTERFACE ${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}tiffd${CMAKE_STATIC_LIBRARY_SUFFIX})
else()
    target_link_libraries(LibTIFF INTERFACE ${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}tiff${CMAKE_STATIC_LIBRARY_SUFFIX})
endif()
target_include_directories(LibTIFF INTERFACE ${INSTALL_DIR}/include)
set_property(TARGET TIFF PROPERTY FOLDER "Dependencies")
target_link_libraries(LibTIFF INTERFACE LibImath)

unset(INSTALL_DIR)

