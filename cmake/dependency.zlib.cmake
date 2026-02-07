# ZLib dependency
# https://github.com/madler/zlib
#
# Output target: LibZLIB

if(NOT _ZLIB_DEP_INCLUDE_GUARD_)
set(_ZLIB_DEP_INCLUDE_GUARD_ ON)


include(${CMAKE_SOURCE_DIR}/cmake/external_project_common.cmake)


ExternalProject_Add(ZLIB
    PREFIX "${CMAKE_BINARY_DIR}/zlib"
    URL "https://github.com/madler/zlib/archive/refs/tags/v1.3.1.zip"
    URL_MD5 "127b8a71a3fb8bebe89df1080f15fdf6"
    DOWNLOAD_DIR "${CMAKE_SOURCE_DIR}/dependencies/zlib"
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    SOURCE_DIR "${EXTERNALPROJECT_SOURCE_PREFIX}/dependencies/zlib/source"
    BINARY_DIR "${CMAKE_BINARY_DIR}/zlib/build"
    INSTALL_DIR "${CMAKE_BINARY_DIR}/zlib/install"
    UPDATE_COMMAND ""
    BUILD_COMMAND ${BUILD_COMMAND_FOR_TARGET} -t zlibstatic
    INSTALL_COMMAND ${BUILD_COMMAND_FOR_TARGET} -t install
            COMMAND ${CMAKE_COMMAND} -E copy "${EXTERNALPROJECT_SOURCE_PREFIX}/dependencies/zlib/source/zutil.h" -t "${CMAKE_BINARY_DIR}/zlib/install/include"
    CMAKE_ARGS ${CMAKE_BUILD_TYPE_RELEASE} "-DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_BINARY_DIR}/zlib/install" "-DZLIB_BUILD_EXAMPLES=OFF"
        "-DCMAKE_C_FLAGS:STRING=${ZERO_WARNINGS_FLAG} -fPIC"
)

# For configuring other dependencies
ExternalProject_Get_Property(ZLIB INSTALL_DIR)
set(ZLIB_ROOT ${INSTALL_DIR})
unset(INSTALL_DIR)

add_library(LibZLIB INTERFACE)
add_dependencies(LibZLIB ZLIB)
target_include_directories(LibZLIB INTERFACE ${ZLIB_ROOT}/include)
if(MSVC)
    link_library_path2(LibZLIB ${ZLIB_ROOT}/lib zlibstatic${CMAKE_STATIC_LIBRARY_SUFFIX} zlibstaticd${CMAKE_STATIC_LIBRARY_SUFFIX})
else()
    target_link_libraries(LibZLIB INTERFACE "${ZLIB_ROOT}/lib/libz${CMAKE_STATIC_LIBRARY_SUFFIX}")
endif()
set_property(TARGET ZLIB PROPERTY FOLDER "Dependencies")


endif() # _ZLIB_DEP_INCLUDE_GUARD_
