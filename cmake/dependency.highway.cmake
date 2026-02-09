# Highway dependency
#
# Output target: LibHighway
#

include(${CMAKE_SOURCE_DIR}/cmake/external_project_common.cmake)

ExternalProject_Add(HIGHWAY
    PREFIX ${CMAKE_BINARY_DIR}/highway
    URL "https://github.com/google/highway/archive/refs/tags/1.3.0.zip"
    URL_MD5 "e91017527ce45fad36e2e8803ecda565"
    DOWNLOAD_DIR "${CMAKE_SOURCE_DIR}/dependencies/highway"
    SOURCE_DIR "${EXTERNALPROJECT_SOURCE_PREFIX}/dependencies/highway/source"
    BINARY_DIR "${CMAKE_BINARY_DIR}/highway/build"
    INSTALL_DIR "${CMAKE_BINARY_DIR}/highway/install"
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    #BUILD_COMMAND ${BUILD_COMMAND_FOR_TARGET} -t hwy
    CMAKE_ARGS ${CMAKE_BUILD_TYPE_ARG} "-DBUILD_SHARED_LIBS=OFF" "-DBUILD_TESTING=OFF" "-DHWY_ENABLE_TESTS=OFF" "-DHWY_ENABLE_EXAMPLES=OFF"
        "-DHWY_FORCE_STATIC_LIBS=ON" "-DHWY_ENABLE_CONTRIB=OFF" "-DHWY_TEST_STANDALONE=OFF" "-DHWY_WARNINGS_ARE_ERRORS=OFF" "-DHWY_CMAKE_HEADER_ONLY=OFF"
        "-DCMAKE_C_FLAGS:STRING=${ZERO_WARNINGS_FLAG} -fPIC" "-DCMAKE_DEBUG_POSTFIX=d" "-DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_BINARY_DIR}/highway/install"
    EXCLUDE_FROM_ALL
)

if (MSVC)
    ExternalProject_Add_Step(HIGHWAY build_debug
        DEPENDEES build
        DEPENDERS install
        COMMAND echo "Build Debug"
        COMMAND ${BUILD_COMMAND_FOR_TARGET} -t install --config Debug
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/highway/build
    )
    ExternalProject_Add_Step(HIGHWAY build_release
        DEPENDEES build_debug
        DEPENDERS install
        COMMAND echo "Build Release"
        COMMAND ${BUILD_COMMAND_FOR_TARGET} -t install --config Release
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/highway/build
    )
else()
    ExternalProject_Add_Step(HIGHWAY build_default
        DEPENDEES build
        DEPENDERS install
        COMMAND echo "Build"
        COMMAND ${BUILD_COMMAND_FOR_TARGET} -t install
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/highway/build
    )
endif()

ExternalProject_Get_Property(HIGHWAY INSTALL_DIR)

add_library(LibHighway INTERFACE)
add_dependencies(LibHighway HIGHWAY)
if (MSVC)
    link_library_path2(LibHighway ${INSTALL_DIR}/lib hwy.lib hwyd.lib)
else()
    target_link_libraries(LibHighway INTERFACE ${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}hwy${CMAKE_STATIC_LIBRARY_SUFFIX})
endif()
target_include_directories(LibHighway INTERFACE ${INSTALL_DIR}/include)
set_property(TARGET HIGHWAY PROPERTY FOLDER "Dependencies")

set(HWY_ROOT ${INSTALL_DIR})

unset(INSTALL_DIR)

