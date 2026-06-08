# OpenJPEG dependency (JPEG 2000)
#
# Output target: LibOpenJPEG
#

include(${EXTERNALPROJECT_INCLUDE_DIR}/external_project_common.cmake)

ExternalProject_Add(OPENJPEG
    PREFIX ${EXTERNALPROJECT_BINARY_ROOT}/openjpeg
    URL "https://github.com/uclouvain/openjpeg/archive/refs/tags/v2.5.4.zip"
    URL_MD5 "c8dbac9e49662217d782c1b9078dbfa7"
    DOWNLOAD_DIR "${EXTERNALPROJECT_SOURCE_ROOT}/openjpeg"
    SOURCE_DIR "${EXTERNALPROJECT_SOURCE_PREFIX}/openjpeg/source"
    BINARY_DIR "${EXTERNALPROJECT_BINARY_ROOT}/openjpeg/build"
    INSTALL_DIR "${EXTERNALPROJECT_BINARY_ROOT}/openjpeg/install"
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    BUILD_COMMAND ${BUILD_COMMAND_FOR_TARGET} -t openjp2
    CMAKE_ARGS ${EXTERNALPROJECT_CMAKE_ARGS} "-DBUILD_STATIC_LIBS=ON" "-DBUILD_SHARED_LIBS=OFF" "-DBUILD_CODEC=OFF" "-DBUILD_JPIP=OFF" "-DBUILD_TESTING=OFF"
        "-DCMAKE_C_FLAGS:STRING=${ZERO_WARNINGS_FLAG} ${FPIC_FLAG}"
        "-DCMAKE_INSTALL_PREFIX:PATH=${EXTERNALPROJECT_BINARY_ROOT}/openjpeg/install"
    EXCLUDE_FROM_ALL
)

ExternalProject_Get_Property(OPENJPEG INSTALL_DIR)

add_library(LibOpenJPEG INTERFACE)
add_dependencies(LibOpenJPEG OPENJPEG)
target_link_directories(LibOpenJPEG INTERFACE ${INSTALL_DIR}/lib)
target_link_libraries(LibOpenJPEG INTERFACE openjp2)
target_include_directories(LibOpenJPEG INTERFACE ${INSTALL_DIR}/include/openjpeg-2.5)
set_property(TARGET OPENJPEG PROPERTY FOLDER "Dependencies")

unset(INSTALL_DIR)

