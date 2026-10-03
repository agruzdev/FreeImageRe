# Imath dependency
#
# Output target: LibImath
#

if(NOT _IMATH_DEP_INCLUDE_GUARD_)
    set(_IMATH_DEP_INCLUDE_GUARD_ ON)


include(${EXTERNALPROJECT_INCLUDE_DIR}/external_project_common.cmake)

ExternalProject_Add(IMATH
    PREFIX ${EXTERNALPROJECT_BINARY_ROOT}/imath
    URL "https://github.com/AcademySoftwareFoundation/Imath/archive/refs/tags/v3.2.3.zip"
    URL_MD5 "aa7df0e279cbebe4e0f25e9e71a64733"
    DOWNLOAD_DIR "${EXTERNALPROJECT_SOURCE_ROOT}/imath"
    SOURCE_DIR "${EXTERNALPROJECT_SOURCE_PREFIX}/imath/source"
    BINARY_DIR "${EXTERNALPROJECT_BINARY_ROOT}/imath/build"
    INSTALL_DIR "${EXTERNALPROJECT_BINARY_ROOT}/imath/install"
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    BUILD_COMMAND ${BUILD_COMMAND_FOR_TARGET}
    INSTALL_COMMAND ${BUILD_COMMAND_FOR_TARGET} -t install
    CMAKE_ARGS ${EXTERNALPROJECT_CMAKE_ARGS} "-DBUILD_SHARED_LIBS=OFF" "-DBUILD_TESTING=OFF" "-DIMATH_LIB_SUFFIX="
        "-DCMAKE_C_FLAGS:STRING=${ZERO_WARNINGS_FLAG} ${FPIC_FLAG}" "-DCMAKE_DEBUG_POSTFIX=" "-DCMAKE_INSTALL_PREFIX:PATH=${EXTERNALPROJECT_BINARY_ROOT}/imath/install"
    EXCLUDE_FROM_ALL
)

ExternalProject_Get_Property(IMATH INSTALL_DIR)

add_library(LibImath INTERFACE)
add_dependencies(LibImath IMATH)
target_link_directories(LibImath INTERFACE ${INSTALL_DIR}/lib)
target_link_libraries(LibImath INTERFACE Imath)
target_include_directories(LibImath INTERFACE ${INSTALL_DIR}/include ${INSTALL_DIR}/include/Imath)
set_property(TARGET IMATH PROPERTY FOLDER "Dependencies")


set(IMATH_ROOT ${INSTALL_DIR})

unset(INSTALL_DIR)

endif() #_IMATH_DEP_INCLUDE_GUARD_
