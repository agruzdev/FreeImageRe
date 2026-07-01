# ZLib dependency
# https://github.com/madler/zlib
#
# Output target: LibZLIB

if(NOT _ZLIB_DEP_INCLUDE_GUARD_)
set(_ZLIB_DEP_INCLUDE_GUARD_ ON)

if (USE_SYSTEM_ZLIB OR USE_SYSTEM_LIBS)
    find_package(ZLIB REQUIRED)
    add_library(LibZLIB ALIAS ZLIB::ZLIB)
    return()
endif()

include(${EXTERNALPROJECT_INCLUDE_DIR}/external_project_common.cmake)

if(WIN32)
    if (IS_DEBUG_CONFIG)
        set(ZLIB_NAME zsd${CMAKE_STATIC_LIBRARY_SUFFIX})
    else()
        set(ZLIB_NAME zs${CMAKE_STATIC_LIBRARY_SUFFIX})
    endif()
else()
    set(ZLIB_NAME libz${CMAKE_STATIC_LIBRARY_SUFFIX})
endif()

set(ZLIB_ROOT "${EXTERNALPROJECT_BINARY_ROOT}/zlib/install")

ExternalProject_Add(ZLIB
    PREFIX "${EXTERNALPROJECT_BINARY_ROOT}/zlib"
    URL "https://github.com/madler/zlib/archive/refs/tags/v1.3.2.zip"
    URL_MD5 "adbba6eef8960c3412818b2e241f46dc"
    DOWNLOAD_DIR "${EXTERNALPROJECT_SOURCE_ROOT}/zlib"
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    SOURCE_DIR "${EXTERNALPROJECT_SOURCE_PREFIX}/zlib/source"
    BINARY_DIR "${EXTERNALPROJECT_BINARY_ROOT}/zlib/build"
    INSTALL_DIR ${ZLIB_ROOT}
    UPDATE_COMMAND ""
    INSTALL_COMMAND ${BUILD_COMMAND_FOR_TARGET} -t install
            COMMAND ${CMAKE_COMMAND} -E copy "${EXTERNALPROJECT_SOURCE_PREFIX}/zlib/source/zutil.h" -t "${ZLIB_ROOT}/include"
    CMAKE_ARGS ${EXTERNALPROJECT_CMAKE_ARGS} "-DCMAKE_INSTALL_PREFIX:PATH=${ZLIB_ROOT}"
        "-DZLIB_BUILD_TESTING=OFF" "-DZLIB_BUILD_SHARED=OFF" "-DZLIB_BUILD_STATIC=ON"
        "-DCMAKE_C_FLAGS:STRING=${ZERO_WARNINGS_FLAG} ${FPIC_FLAG}"
)

add_library(LibZLIB INTERFACE)
add_dependencies(LibZLIB ZLIB)
target_include_directories(LibZLIB INTERFACE ${ZLIB_ROOT}/include)
target_link_directories(LibZLIB INTERFACE ${ZLIB_ROOT}/lib)
target_link_libraries(LibZLIB INTERFACE ${ZLIB_NAME})
set_property(TARGET ZLIB PROPERTY FOLDER "Dependencies")


endif() # _ZLIB_DEP_INCLUDE_GUARD_
