# OpenEXR dependency
# https://github.com/AcademySoftwareFoundation/openexr
#
# Output target: LibOpenEXR
#


include(${EXTERNALPROJECT_INCLUDE_DIR}/external_project_common.cmake)
include(${EXTERNALPROJECT_INCLUDE_DIR}/dependency.openjph.cmake)
include(${EXTERNALPROJECT_INCLUDE_DIR}/dependency.imath.cmake)


find_package(Git REQUIRED) # needed by OpenEXR

ExternalProject_Add(EXR
    PREFIX ${EXTERNALPROJECT_BINARY_ROOT}/openexr
    URL "https://github.com/AcademySoftwareFoundation/openexr/archive/refs/tags/v3.4.10.zip"
    URL_MD5 "7c485810b90620589997a3212b69ed8b"
    DOWNLOAD_DIR "${EXTERNALPROJECT_SOURCE_ROOT}/openexr"
    SOURCE_DIR "${EXTERNALPROJECT_SOURCE_PREFIX}/openexr/source"
    BINARY_DIR "${EXTERNALPROJECT_BINARY_ROOT}/openexr/build"
    INSTALL_DIR "${EXTERNALPROJECT_BINARY_ROOT}/openexr/install"
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    BUILD_COMMAND ${BUILD_COMMAND_FOR_TARGET} -t OpenEXR
    CMAKE_ARGS ${CMAKE_TOOLCHAIN_FILE_ARG} ${CMAKE_BUILD_TYPE_ARG} "-DOPENEXR_IS_SUBPROJECT=ON" "-DOPENEXR_FORCE_INTERNAL_IMATH=OFF" "-DOPENEXR_FORCE_INTERNAL_DEFLATE=ON" "-DOPENEXR_INSTALL=ON"
        "-DOPENEXR_INSTALL_TOOLS=OFF" "-DOPENEXR_BUILD_EXAMPLES=OFF" "-DOPENEXR_INSTALL_PKG_CONFIG=OFF" "-DOPENEXR_BUILD_TOOLS=OFF" "-DBUILD_SHARED_LIBS=OFF" "-DBUILD_TESTING=OFF"
        "-DOPENEXR_LIB_SUFFIX=" "-DCMAKE_C_FLAGS:STRING=${ZERO_WARNINGS_FLAG}" "-DCMAKE_CXX_FLAGS:STRING=${ZERO_WARNINGS_FLAG}" "-DCMAKE_DEBUG_POSTFIX="
        "-DImath_ROOT=${IMATH_ROOT}" "-Dopenjph_ROOT=${OPENJPH_ROOT}" "-DCMAKE_INSTALL_PREFIX:PATH=${EXTERNALPROJECT_BINARY_ROOT}/openexr/install"
    EXCLUDE_FROM_ALL
    DEPENDS OPENJPH IMATH
)

ExternalProject_Get_Property(EXR INSTALL_DIR)

add_library(LibOpenEXR INTERFACE)
add_dependencies(LibOpenEXR EXR)
target_link_directories(LibOpenEXR INTERFACE ${INSTALL_DIR}/lib)
target_link_libraries(LibOpenEXR INTERFACE
    OpenEXR OpenEXRCore Iex IlmThread
    LibOpenJPH LibImath
)
target_include_directories(LibOpenEXR INTERFACE ${INSTALL_DIR}/include ${INSTALL_DIR}/include/Imath ${INSTALL_DIR}/include/OpenEXR)
set_property(TARGET EXR PROPERTY FOLDER "Dependencies")

unset(INSTALL_DIR)
