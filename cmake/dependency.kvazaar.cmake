# kvazaar dependency, part of libheif
# https://github.com/ultravideo/kvazaar
#
# Output targets: LibKvazaar


include(${CMAKE_SOURCE_DIR}/cmake/external_project_common.cmake)

ExternalProject_Add(KVAZAAR
    PREFIX ${CMAKE_BINARY_DIR}/kvazaar
    URL "https://github.com/ultravideo/kvazaar/archive/refs/tags/v2.3.1.zip"
    URL_MD5 "18197b9467e2346f29de7c0bc437ba9e"
    DOWNLOAD_DIR "${CMAKE_SOURCE_DIR}/dependencies/kvazaar"
    SOURCE_DIR "${EXTERNALPROJECT_SOURCE_PREFIX}/dependencies/kvazaar/source"
    BINARY_DIR "${CMAKE_BINARY_DIR}/kvazaar/build"
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    BUILD_COMMAND ${BUILD_COMMAND_FOR_TARGET} -t kvazaar
    INSTALL_COMMAND ${BUILD_COMMAND_FOR_TARGET} -t install
    CMAKE_ARGS ${CMAKE_BUILD_TYPE_ARG} "-DBUILD_SHARED_LIBS=OFF" "-DBUILD_TESTS=OFF" "-DUSE_CRYPTO=OFF"
        "-DCMAKE_C_FLAGS:STRING=${CMAKE_C_FLAGS} ${ZERO_WARNINGS_FLAG}" "-DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS} ${ZERO_WARNINGS_FLAG}"
        "-DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_BINARY_DIR}/kvazaar/install" ${CMAKE_DEBUG_POSTFIX_MULTICONF}
    EXCLUDE_FROM_ALL
)

ExternalProject_Get_Property(KVAZAAR SOURCE_DIR)
ExternalProject_Get_Property(KVAZAAR BINARY_DIR)

set_property(TARGET KVAZAAR PROPERTY FOLDER "Dependencies")

set(KVAZAAR_INCLUDE_DIRS ${CMAKE_BINARY_DIR}/kvazaar/install/include)
set(KVAZAAR_LINK_DIRS ${CMAKE_BINARY_DIR}/kvazaar/install/lib)
if(WIN32)
    set(KVAZAAR_LIBRARY libkvazaar)
    set(KVAZAAR_DEBUG_LIBRARY libkvazaard)
else()
    set(KVAZAAR_LIBRARY kvazaar)
endif()

unset(SOURCE_DIR)
unset(BINARY_DIR)

