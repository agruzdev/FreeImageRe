# OpenJPH dependency (JPEG 2000)
#
# Output target: LibOpenJPH
#

include(${EXTERNALPROJECT_INCLUDE_DIR}/external_project_common.cmake)

ExternalProject_Add(OPENJPH
    PREFIX ${EXTERNALPROJECT_BINARY_ROOT}/openjph
    URL "https://github.com/aous72/OpenJPH/archive/refs/tags/0.27.0.zip"
    URL_MD5 "c073bd13517c3b26bf5c17863fd94c1b"
    DOWNLOAD_DIR "${EXTERNALPROJECT_SOURCE_ROOT}/openjph"
    SOURCE_DIR "${EXTERNALPROJECT_SOURCE_PREFIX}/openjph/source"
    BINARY_DIR "${EXTERNALPROJECT_BINARY_ROOT}/openjph/build"
    INSTALL_DIR "${EXTERNALPROJECT_BINARY_ROOT}/openjph/install"
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    BUILD_COMMAND ${BUILD_COMMAND_FOR_TARGET} -t openjph
    CMAKE_ARGS ${EXTERNALPROJECT_CMAKE_ARGS} "-DBUILD_SHARED_LIBS=OFF" "-DOJPH_BUILD_TESTS=OFF" "-DOJPH_ENABLE_TIFF_SUPPORT=OFF" "-DOJPH_BUILD_EXECUTABLES=OFF"
        "-DCMAKE_C_FLAGS:STRING=${ZERO_WARNINGS_FLAG} ${FPIC_FLAG}" "-DCMAKE_DEBUG_POSTFIX=" "-DCMAKE_INSTALL_PREFIX:PATH=${EXTERNALPROJECT_BINARY_ROOT}/openjph/install"
    EXCLUDE_FROM_ALL
)

ExternalProject_Get_Property(OPENJPH INSTALL_DIR)

add_library(LibOpenJPH INTERFACE)
add_dependencies(LibOpenJPH OPENJPH)
target_link_directories(LibOpenJPH INTERFACE ${INSTALL_DIR}/lib)
if (MSVC)
    target_link_libraries(LibOpenJPH INTERFACE openjph.0.27)
else()
    target_link_libraries(LibOpenJPH INTERFACE openjph)
endif()
target_include_directories(LibOpenJPH INTERFACE ${INSTALL_DIR}/include)
set_property(TARGET OPENJPH PROPERTY FOLDER "Dependencies")


set(OPENJPH_ROOT ${INSTALL_DIR})

unset(INSTALL_DIR)

