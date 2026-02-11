# Little CMS dependency
#
# Output target: LibLCMS2
#

include(${EXTERNALPROJECT_INCLUDE_DIR}/external_project_common.cmake)

ExternalProject_Add(LCMS2
    PREFIX ${EXTERNALPROJECT_BINARY_ROOT}/lcms2
    URL "https://github.com/mm2/Little-CMS/archive/6ae7e97cc1b0a44f1996d51160de9fbab97bb7b8.zip"    # use release tag as soon as CMakeLists.txt released
    URL_MD5 "59e25c1eb15d3de85f4b691e3fea96fd"
    DOWNLOAD_DIR "${EXTERNALPROJECT_SOURCE_ROOT}/lcms2"
    SOURCE_DIR "${EXTERNALPROJECT_SOURCE_PREFIX}/lcms2/source"
    BINARY_DIR "${EXTERNALPROJECT_BINARY_ROOT}/lcms2/build"
    INSTALL_DIR "${EXTERNALPROJECT_BINARY_ROOT}/lcms2/install"
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    CMAKE_ARGS ${CMAKE_BUILD_TYPE_ARG} "-DLCMS2_BUILD_SHARED=OFF" "-DLCMS2_BUILD_STATIC=ON" "-DLCMS2_BUILD_TESTS=OFF" "-DLCMS2_BUILD_TIFICC=OFF"
        "-DLCMS2_BUILD_JPGICC=OFF" "-DLCMS2_BUILD_TOOLS=OFF" "-DLCMS2_WITH_JPEG=OFF" "-DLCMS2_WITH_TIFF=OFF" "-DLCMS2_WITH_ZLIB=OFF"
        "-DCMAKE_C_FLAGS:STRING=${ZERO_WARNINGS_FLAG} -fPIC -DCMS_NO_REGISTER_KEYWORD=1" "-DCMAKE_DEBUG_POSTFIX=d" "-DCMAKE_INSTALL_PREFIX:PATH=${EXTERNALPROJECT_BINARY_ROOT}/lcms2/install"
    EXCLUDE_FROM_ALL
)

if (MSVC)
    ExternalProject_Add_Step(LCMS2 build_debug
        DEPENDEES build
        DEPENDERS install
        COMMAND echo "Build Debug"
        COMMAND ${BUILD_COMMAND_FOR_TARGET} -t install --config Debug
        WORKING_DIRECTORY ${EXTERNALPROJECT_BINARY_ROOT}/lcms2/build
    )
    ExternalProject_Add_Step(LCMS2 build_release
        DEPENDEES build_debug
        DEPENDERS install
        COMMAND echo "Build Release"
        COMMAND ${BUILD_COMMAND_FOR_TARGET} -t install --config Release
        WORKING_DIRECTORY ${EXTERNALPROJECT_BINARY_ROOT}/lcms2/build
    )
else()
    ExternalProject_Add_Step(LCMS2 build_default
        DEPENDEES build
        DEPENDERS install
        COMMAND echo "Build"
        COMMAND ${BUILD_COMMAND_FOR_TARGET} -t install
        WORKING_DIRECTORY ${EXTERNALPROJECT_BINARY_ROOT}/lcms2/build
    )
endif()

ExternalProject_Get_Property(LCMS2 INSTALL_DIR)

add_library(LibLCMS2 INTERFACE)
add_dependencies(LibLCMS2 LCMS2)
if (MSVC)
    link_library_path2(LibLCMS2 ${INSTALL_DIR}/lib lcms2.lib lcms2d.lib)
else()
    target_link_libraries(LibLCMS2 INTERFACE ${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}lcms2${CMAKE_STATIC_LIBRARY_SUFFIX})
endif()
target_include_directories(LibLCMS2 INTERFACE ${INSTALL_DIR}/include)
set_property(TARGET LCMS2 PROPERTY FOLDER "Dependencies")

set(LCMS2_ROOT ${INSTALL_DIR})

unset(INSTALL_DIR)

