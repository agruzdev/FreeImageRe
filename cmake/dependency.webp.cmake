# WEBP dependency
# https://chromium.googlesource.com/webm/libwebp
#
# Output target: LibWEBP


include(${CMAKE_SOURCE_DIR}/cmake/external_project_common.cmake)


ExternalProject_Add(WEBP
    PREFIX ${CMAKE_BINARY_DIR}/webp
    URL "https://chromium.googlesource.com/webm/libwebp/+archive/845d5476a866141ba35ac133f856fa62f0b7445f.tar.gz"   #v1.4.0
    # googlesource can't provide stable hash, so ignore hash check
    DOWNLOAD_DIR "${CMAKE_SOURCE_DIR}/dependencies/webp"
    SOURCE_DIR "${EXTERNALPROJECT_SOURCE_PREFIX}/dependencies/webp/source"
    BINARY_DIR "${CMAKE_BINARY_DIR}/webp/build"
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    BUILD_COMMAND ${BUILD_COMMAND_FOR_TARGET} -t webp libwebpmux sharpyuv
    INSTALL_COMMAND ""
    CMAKE_ARGS ${CMAKE_BUILD_TYPE_ARG} "-DWEBP_BUILD_ANIM_UTILS=OFF" "-DWEBP_BUILD_CWEBP=OFF" "-DWEBP_BUILD_DWEBP=OFF" "-DWEBP_BUILD_GIF2WEBP=OFF" "-DWEBP_BUILD_IMG2WEBP=OFF" 
        "-DWEBP_BUILD_VWEBP=OFF" "-DWEBP_BUILD_WEBPINFO=OFF" "-DWEBP_BUILD_LIBWEBPMUX=ON" "-DWEBP_BUILD_WEBPMUX=OFF" "-DWEBP_BUILD_EXTRAS=OFF" "-DWEBP_UNICODE=ON"
        "-DBUILD_SHARED_LIBS=OFF" "-DCMAKE_C_FLAGS:STRING=${ZERO_WARNINGS_FLAG}"
    EXCLUDE_FROM_ALL
)

ExternalProject_Get_Property(WEBP SOURCE_DIR)
ExternalProject_Get_Property(WEBP BINARY_DIR)

add_library(LibWEBP INTERFACE)
add_dependencies(LibWEBP WEBP)
link_config_aware_library_path(LibWEBP ${BINARY_DIR} libwebp${CMAKE_STATIC_LIBRARY_SUFFIX})
link_config_aware_library_path(LibWEBP ${BINARY_DIR} libwebpmux${CMAKE_STATIC_LIBRARY_SUFFIX})
link_config_aware_library_path(LibWEBP ${BINARY_DIR} libsharpyuv${CMAKE_STATIC_LIBRARY_SUFFIX})
target_include_directories(LibWEBP INTERFACE ${SOURCE_DIR} ${SOURCE_DIR}/src ${BINARY_DIR}/src)
set_property(TARGET WEBP PROPERTY FOLDER "Dependencies")

unset(SOURCE_DIR)
unset(BINARY_DIR)

