# HIEF dependency
# https://github.com/strukturag/libheif
#
# Output target: LibHEIF


include(${CMAKE_SOURCE_DIR}/cmake/external_project_common.cmake)


ExternalProject_Add(HEIF
    PREFIX ${CMAKE_BINARY_DIR}/heif
    URL "https://github.com/strukturag/libheif/releases/download/v1.19.5/libheif-1.19.5.tar.gz"
    URL_MD5 "68a0b8924696b183e640fa03b73eca0c"
    DOWNLOAD_DIR "${CMAKE_SOURCE_DIR}/dependencies/heif"
    SOURCE_DIR "${EXTERNALPROJECT_SOURCE_PREFIX}/dependencies/heif/source"
    BINARY_DIR "${CMAKE_BINARY_DIR}/heif/build"
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    CMAKE_ARGS "--preset" "release-noplugins"
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    EXCLUDE_FROM_ALL
)

ExternalProject_Get_Property(HEIF SOURCE_DIR)
ExternalProject_Get_Property(HEIF BINARY_DIR)

add_library(LibHEIF INTERFACE)

target_include_directories(LibHEIF INTERFACE ${SOURCE_DIR}/libheif/api ${BINARY_DIR})
set_property(TARGET HEIF PROPERTY FOLDER "Dependencies")

unset(SOURCE_DIR)
unset(BINARY_DIR)
