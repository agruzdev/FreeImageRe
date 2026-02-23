# kvazaar dependency, part of libheif
# https://github.com/ultravideo/kvazaar
#
# Output targets: LibKvazaar


include(${EXTERNALPROJECT_INCLUDE_DIR}/external_project_common.cmake)

ExternalProject_Add(KVAZAAR
    PREFIX ${EXTERNALPROJECT_BINARY_ROOT}/kvazaar
    URL "https://github.com/ultravideo/kvazaar/archive/refs/tags/v2.3.2.zip"
    URL_MD5 "ad540c7871d66d9ab8831aa96ee6c288"
    DOWNLOAD_DIR "${EXTERNALPROJECT_SOURCE_ROOT}/kvazaar"
    SOURCE_DIR "${EXTERNALPROJECT_SOURCE_PREFIX}/kvazaar/source"
    BINARY_DIR "${EXTERNALPROJECT_BINARY_ROOT}/kvazaar/build"
    INSTALL_DIR "${EXTERNALPROJECT_BINARY_ROOT}/kvazaar/install"
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    BUILD_COMMAND ${BUILD_COMMAND_FOR_TARGET} -t kvazaar
    INSTALL_COMMAND ${BUILD_COMMAND_FOR_TARGET} -t install
    CMAKE_ARGS ${CMAKE_TOOLCHAIN_FILE_ARG} ${CMAKE_BUILD_TYPE_ARG} "-DBUILD_SHARED_LIBS=OFF" "-DBUILD_TESTS=OFF" "-DUSE_CRYPTO=OFF"
        "-DCMAKE_C_FLAGS:STRING=${CMAKE_C_FLAGS} ${ZERO_WARNINGS_FLAG}" "-DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS} ${ZERO_WARNINGS_FLAG}"
        "-DCMAKE_INSTALL_PREFIX:PATH=${EXTERNALPROJECT_BINARY_ROOT}/kvazaar/install" "-DCMAKE_DEBUG_POSTFIX="
    EXCLUDE_FROM_ALL
)

ExternalProject_Get_Property(KVAZAAR INSTALL_DIR)

set_property(TARGET KVAZAAR PROPERTY FOLDER "Dependencies")

set(KVAZAAR_INCLUDE_DIRS ${INSTALL_DIR}/include)
set(KVAZAAR_LINK_DIRS ${INSTALL_DIR}/lib)
if(WIN32)
    set(KVAZAAR_LIBRARY libkvazaar)
else()
    set(KVAZAAR_LIBRARY kvazaar)
endif()

unset(INSTALL_DIR)


