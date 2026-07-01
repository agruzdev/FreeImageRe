# WEBP dependency
# https://chromium.googlesource.com/webm/libwebp
#
# Output target: LibWEBP

if (USE_SYSTEM_LIBWEBP OR USE_SYSTEM_LIBS)
    find_package(PkgConfig)
    pkg_check_modules(LIBWEBP REQUIRED IMPORTED_TARGET libwebp)
    add_library(LibWEBP ALIAS PkgConfig::LIBWEBP)
    return()
endif()

include(${EXTERNALPROJECT_INCLUDE_DIR}/external_project_common.cmake)


ExternalProject_Add(WEBP
    PREFIX ${EXTERNALPROJECT_BINARY_ROOT}/webp
    URL "https://chromium.googlesource.com/webm/libwebp/+archive/4fa21912338357f89e4fd51cf2368325b59e9bd9.tar.gz"   #v1.6.0
    # googlesource can't provide stable hash, so ignore hash check
    DOWNLOAD_DIR "${EXTERNALPROJECT_SOURCE_ROOT}/webp"
    SOURCE_DIR "${EXTERNALPROJECT_SOURCE_PREFIX}/webp/source"
    BINARY_DIR "${EXTERNALPROJECT_BINARY_ROOT}/webp/build"
    INSTALL_DIR "${EXTERNALPROJECT_BINARY_ROOT}/webp/install"
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    BUILD_COMMAND ${BUILD_COMMAND_FOR_TARGET} -t webp libwebpmux sharpyuv
    CMAKE_ARGS ${EXTERNALPROJECT_CMAKE_ARGS} "-DWEBP_BUILD_ANIM_UTILS=OFF" "-DWEBP_BUILD_CWEBP=OFF" "-DWEBP_BUILD_DWEBP=OFF" "-DWEBP_BUILD_GIF2WEBP=OFF" "-DWEBP_BUILD_IMG2WEBP=OFF" 
        "-DWEBP_BUILD_VWEBP=OFF" "-DWEBP_BUILD_WEBPINFO=OFF" "-DWEBP_BUILD_LIBWEBPMUX=ON" "-DWEBP_BUILD_WEBPMUX=OFF" "-DWEBP_BUILD_EXTRAS=OFF" "-DWEBP_UNICODE=ON"
        "-DBUILD_SHARED_LIBS=OFF" "-DCMAKE_C_FLAGS:STRING=${ZERO_WARNINGS_FLAG}" "-DCMAKE_INSTALL_PREFIX:PATH=${EXTERNALPROJECT_BINARY_ROOT}/webp/install"
    EXCLUDE_FROM_ALL
)

ExternalProject_Get_Property(WEBP INSTALL_DIR)

add_library(LibWEBP INTERFACE)
add_dependencies(LibWEBP WEBP)
target_link_directories(LibWEBP INTERFACE ${INSTALL_DIR}/lib)
target_link_libraries(LibWEBP INTERFACE libwebp${CMAKE_STATIC_LIBRARY_SUFFIX} libwebpmux${CMAKE_STATIC_LIBRARY_SUFFIX} libsharpyuv${CMAKE_STATIC_LIBRARY_SUFFIX})
target_include_directories(LibWEBP INTERFACE ${INSTALL_DIR}/include)
set_property(TARGET WEBP PROPERTY FOLDER "Dependencies")

unset(INSTALL_DIR)

