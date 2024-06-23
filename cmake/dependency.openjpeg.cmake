# OpenJPEG dependency (JPEG 2000)
#
# Output target: LibOpenJPEG
#

include(${CMAKE_SOURCE_DIR}/cmake/external_project_common.cmake)

ExternalProject_Add(OPENJPEG
    PREFIX ${CMAKE_BINARY_DIR}/openjpeg
    URL "https://github.com/uclouvain/openjpeg/archive/refs/tags/v2.5.2.zip"
    URL_MD5 "6295f52db915f7b4afd3241d3692cf00"
    DOWNLOAD_DIR "${CMAKE_SOURCE_DIR}/dependencies/openjpeg"
    SOURCE_DIR "${EXTERNALPROJECT_SOURCE_PREFIX}/dependencies/openjpeg/source"
    BINARY_DIR "${CMAKE_BINARY_DIR}/openjpeg/build"
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    BUILD_COMMAND ${BUILD_COMMAND_FOR_TARGET} -t openjp2
    INSTALL_COMMAND ""
    CMAKE_ARGS ${CMAKE_BUILD_TYPE_ARG} "-DBUILD_STATIC_LIBS=ON" "-DBUILD_SHARED_LIBS=OFF" "-DBUILD_CODEC=OFF" "-DBUILD_JPIP=OFF" "-DBUILD_TESTING=OFF"
        "-DCMAKE_C_FLAGS:STRING=${ZERO_WARNINGS_FLAG} -fPIC"
    EXCLUDE_FROM_ALL
)

ExternalProject_Get_Property(OPENJPEG SOURCE_DIR)
ExternalProject_Get_Property(OPENJPEG BINARY_DIR)

add_library(LibOpenJPEG INTERFACE)
add_dependencies(LibOpenJPEG OPENJPEG)
link_config_aware_library_path(LibOpenJPEG ${BINARY_DIR}/bin ${CMAKE_STATIC_LIBRARY_PREFIX}openjp2${CMAKE_STATIC_LIBRARY_SUFFIX})
target_include_directories(LibOpenJPEG INTERFACE ${SOURCE_DIR}/src/lib ${BINARY_DIR}/src/lib/openjp2)
set_property(TARGET OPENJPEG PROPERTY FOLDER "Dependencies")

unset(SOURCE_DIR)
unset(BINARY_DIR)

