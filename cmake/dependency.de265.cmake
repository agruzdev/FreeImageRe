# libde265 dependency, part of libheif
# https://github.com/strukturag/libde265
#
# Output targets: LibDE265


include(${CMAKE_SOURCE_DIR}/cmake/external_project_common.cmake)

ExternalProject_Add(DE265
    PREFIX ${CMAKE_BINARY_DIR}/libde265
    URL "https://github.com/strukturag/libde265/releases/download/v1.0.16/libde265-1.0.16.tar.gz"
    URL_MD5 "f3173ff6fa273e139de19e6e77bec9b6"
    DOWNLOAD_DIR "${CMAKE_SOURCE_DIR}/dependencies/libde265"
    SOURCE_DIR "${EXTERNALPROJECT_SOURCE_PREFIX}/dependencies/libde265/source"
    BINARY_DIR "${CMAKE_BINARY_DIR}/libde265/build"
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    BUILD_COMMAND ${BUILD_COMMAND_FOR_TARGET} -t de265
    INSTALL_COMMAND ${BUILD_COMMAND_FOR_TARGET} -t install
    CMAKE_ARGS ${CMAKE_BUILD_TYPE_ARG} "-DBUILD_SHARED_LIBS=OFF" "-DENABLE_DECODER=ON" "-DENABLE_ENCODER=OFF" "-DENABLE_SDL=OFF"
        "-DCMAKE_C_FLAGS:STRING=${CMAKE_C_FLAGS} ${ZERO_WARNINGS_FLAG}" "-DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS} ${ZERO_WARNINGS_FLAG}"
        "-DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_BINARY_DIR}/libde265/install" ${CMAKE_DEBUG_POSTFIX_MULTICONF}
    EXCLUDE_FROM_ALL
)

ExternalProject_Get_Property(DE265 SOURCE_DIR)
ExternalProject_Get_Property(DE265 BINARY_DIR)

set_property(TARGET DE265 PROPERTY FOLDER "Dependencies")

set(LIBDE265_INCLUDE_DIRS ${CMAKE_BINARY_DIR}/libde265/install/include)
set(LIBDE265_LINK_DIRS ${CMAKE_BINARY_DIR}/libde265/install/lib)
if (WIN32)
    set(LIBDE265_LIBRARY libde265)
    set(LIBDE265_DEBUG_LIBRARY libde265d)
else()
    set(LIBDE265_LIBRARY de265)
endif()

unset(SOURCE_DIR)
unset(BINARY_DIR)

