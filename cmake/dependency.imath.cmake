# Imath dependency
#
# Output target: LibImath
#

if(NOT _IMATH_DEP_INCLUDE_GUARD_)
    set(_IMATH_DEP_INCLUDE_GUARD_ ON)


include(${CMAKE_SOURCE_DIR}/cmake/external_project_common.cmake)

ExternalProject_Add(IMATH
    PREFIX ${CMAKE_BINARY_DIR}/imath
    URL "https://github.com/AcademySoftwareFoundation/Imath/archive/refs/tags/v3.2.2.zip"
    URL_MD5 "d9c3aadc25a7d47a893b649787e59a44"
    DOWNLOAD_DIR "${CMAKE_SOURCE_DIR}/dependencies/imath"
    SOURCE_DIR "${EXTERNALPROJECT_SOURCE_PREFIX}/dependencies/imath/source"
    BINARY_DIR "${CMAKE_BINARY_DIR}/imath/build"
    INSTALL_DIR "${CMAKE_BINARY_DIR}/imath/install"
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
#    BUILD_COMMAND ${BUILD_COMMAND_FOR_TARGET}
    CMAKE_ARGS ${CMAKE_BUILD_TYPE_ARG} "-DBUILD_SHARED_LIBS=OFF" "-DBUILD_TESTING=OFF" "-DIMATH_LIB_SUFFIX="
        "-DCMAKE_C_FLAGS:STRING=${ZERO_WARNINGS_FLAG} -fPIC" "-DCMAKE_DEBUG_POSTFIX=d" "-DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_BINARY_DIR}/imath/install"
    EXCLUDE_FROM_ALL
)

ExternalProject_Get_Property(IMATH SOURCE_DIR)
ExternalProject_Get_Property(IMATH BINARY_DIR)
ExternalProject_Get_Property(IMATH INSTALL_DIR)

add_library(LibImath INTERFACE)
add_dependencies(LibImath IMATH)
if (MSVC)
    link_library_path2(LibImath ${INSTALL_DIR}/lib Imath.lib Imath-3_2d.lib)
else()
    target_link_libraries(LibImath INTERFACE ${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}Imath${CMAKE_STATIC_LIBRARY_SUFFIX})
endif()
target_include_directories(LibImath INTERFACE ${INSTALL_DIR}/include ${INSTALL_DIR}/include/Imath)
set_property(TARGET IMATH PROPERTY FOLDER "Dependencies")


set(IMATH_ROOT ${INSTALL_DIR})

unset(SOURCE_DIR)
unset(BINARY_DIR)
unset(INSTALL_DIR)

endif() #_IMATH_DEP_INCLUDE_GUARD_
