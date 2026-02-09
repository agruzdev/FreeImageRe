# Brotli dependency
#
# Output target: LibBrotli
#

include(${CMAKE_SOURCE_DIR}/cmake/external_project_common.cmake)

ExternalProject_Add(BROTLI
    PREFIX ${CMAKE_BINARY_DIR}/brotli
    URL "https://github.com/google/brotli/archive/refs/tags/v1.2.0.zip"
    URL_MD5 "5b7c422f6c3d7c2362df4d54b6ab36f9"
    DOWNLOAD_DIR "${CMAKE_SOURCE_DIR}/dependencies/brotli"
    SOURCE_DIR "${EXTERNALPROJECT_SOURCE_PREFIX}/dependencies/brotli/source"
    BINARY_DIR "${CMAKE_BINARY_DIR}/brotli/build"
    INSTALL_DIR "${CMAKE_BINARY_DIR}/brotli/install"
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    CMAKE_ARGS ${CMAKE_BUILD_TYPE_ARG} "-DBUILD_SHARED_LIBS=OFF" "-DBUILD_TESTING=OFF" "-DBROTLI_BUILD_FOR_PACKAGE=OFF" "-DBROTLI_BUILD_TOOLS=OFF"
        "-DCMAKE_C_FLAGS:STRING=${ZERO_WARNINGS_FLAG} -fPIC" "-DCMAKE_DEBUG_POSTFIX=d" "-DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_BINARY_DIR}/brotli/install"
    EXCLUDE_FROM_ALL
)

if (MSVC)
    ExternalProject_Add_Step(BROTLI build_debug
        DEPENDEES build
        DEPENDERS install
        COMMAND echo "Build Debug"
        COMMAND ${BUILD_COMMAND_FOR_TARGET} -t install --config Debug
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/brotli/build
    )
    ExternalProject_Add_Step(BROTLI build_release
        DEPENDEES build_debug
        DEPENDERS install
        COMMAND echo "Build Release"
        COMMAND ${BUILD_COMMAND_FOR_TARGET} -t install --config Release
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/brotli/build
    )
else()
    ExternalProject_Add_Step(BROTLI build_default
        DEPENDEES build
        DEPENDERS install
        COMMAND echo "Build"
        COMMAND ${BUILD_COMMAND_FOR_TARGET} -t install
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/brotli/build
    )
endif()

ExternalProject_Get_Property(BROTLI INSTALL_DIR)

add_library(LibBrotli INTERFACE)
add_dependencies(LibBrotli BROTLI)
if (MSVC)
    link_library_path2(LibBrotli ${INSTALL_DIR}/lib brotlidec.lib brotlidecd.lib)
    link_library_path2(LibBrotli ${INSTALL_DIR}/lib brotlienc.lib brotliencd.lib)
    link_library_path2(LibBrotli ${INSTALL_DIR}/lib brotlicommon.lib brotlicommond.lib)
else()
    target_link_libraries(LibBrotli INTERFACE
        ${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}brotlidec${CMAKE_STATIC_LIBRARY_SUFFIX}
        ${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}brotlienc${CMAKE_STATIC_LIBRARY_SUFFIX}
        ${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}brotlicommon${CMAKE_STATIC_LIBRARY_SUFFIX}
    )
endif()
target_include_directories(LibBrotli INTERFACE ${INSTALL_DIR}/include)
set_property(TARGET BROTLI PROPERTY FOLDER "Dependencies")

set(Brotli_ROOT ${INSTALL_DIR})

unset(INSTALL_DIR)

