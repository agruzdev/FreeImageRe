# ZLib dependency
# https://github.com/madler/zlib
#
# Output target: LibZLIB

if(NOT _ZLIB_DEP_INCLUDE_GUARD_)
set(_ZLIB_DEP_INCLUDE_GUARD_ ON)


include(${EXTERNALPROJECT_INCLUDE_DIR}/external_project_common.cmake)


ExternalProject_Add(ZLIB
    PREFIX "${EXTERNALPROJECT_BINARY_ROOT}/zlib"
    URL "https://github.com/madler/zlib/archive/refs/tags/v1.3.2.zip"
    URL_MD5 "adbba6eef8960c3412818b2e241f46dc"
    DOWNLOAD_DIR "${EXTERNALPROJECT_SOURCE_ROOT}/zlib"
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    SOURCE_DIR "${EXTERNALPROJECT_SOURCE_PREFIX}/zlib/source"
    BINARY_DIR "${EXTERNALPROJECT_BINARY_ROOT}/zlib/build"
    INSTALL_DIR "${EXTERNALPROJECT_BINARY_ROOT}/zlib/install"
    UPDATE_COMMAND ""
    INSTALL_COMMAND ${BUILD_COMMAND_FOR_TARGET} -t install
            COMMAND ${CMAKE_COMMAND} -E copy "${EXTERNALPROJECT_SOURCE_PREFIX}/zlib/source/zutil.h" -t "${EXTERNALPROJECT_BINARY_ROOT}/zlib/install/include"
    CMAKE_ARGS ${CMAKE_TOOLCHAIN_FILE_ARG} ${CMAKE_BUILD_TYPE_ARG} "-DCMAKE_INSTALL_PREFIX:PATH=${EXTERNALPROJECT_BINARY_ROOT}/zlib/install"
        "-DZLIB_BUILD_TESTING=OFF" "-DZLIB_BUILD_SHARED=OFF" "-DZLIB_BUILD_STATIC=ON"
        "-DCMAKE_C_FLAGS:STRING=${ZERO_WARNINGS_FLAG} ${FPIC_FLAG}"
)

# For configuring other dependencies
ExternalProject_Get_Property(ZLIB INSTALL_DIR)
set(ZLIB_ROOT ${INSTALL_DIR})
unset(INSTALL_DIR)

add_library(LibZLIB INTERFACE)
add_dependencies(LibZLIB ZLIB)
target_include_directories(LibZLIB INTERFACE ${ZLIB_ROOT}/include)
if(WIN32)
    if (IS_DEBUG_CONFIG)
        target_link_libraries(LibZLIB INTERFACE ${ZLIB_ROOT}/lib/zsd${CMAKE_STATIC_LIBRARY_SUFFIX})
    else()
        target_link_libraries(LibZLIB INTERFACE ${ZLIB_ROOT}/lib/zs${CMAKE_STATIC_LIBRARY_SUFFIX})
    endif()
else()
    target_link_libraries(LibZLIB INTERFACE ${ZLIB_ROOT}/lib/libz${CMAKE_STATIC_LIBRARY_SUFFIX})
endif()
set_property(TARGET ZLIB PROPERTY FOLDER "Dependencies")


endif() # _ZLIB_DEP_INCLUDE_GUARD_
