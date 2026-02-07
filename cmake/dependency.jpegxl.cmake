# JpegXL dependency
#
# Output target: LibJpegXL
#

include(${CMAKE_SOURCE_DIR}/cmake/external_project_common.cmake)

include(${CMAKE_SOURCE_DIR}/cmake/dependency.zlib.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/dependency.highway.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/dependency.brotli.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/dependency.lcms2.cmake)

ExternalProject_Get_Property(ZLIB INSTALL_DIR)
set(ZLIB_INSTALL_DIR ${INSTALL_DIR})
unset(INSTALL_DIR)


# for patching
find_package(Git REQUIRED)
cmake_path(GET GIT_EXECUTABLE PARENT_PATH GIT_DIRECTORY)
find_program(PATCH_EXECUTABLE patch HINTS ${GIT_DIRECTORY} ${GIT_DIRECTORY}/../usr/bin REQUIRED)


ExternalProject_Add(JPEGXL
    PREFIX ${CMAKE_BINARY_DIR}/jpegxl
    URL "https://github.com/libjxl/libjxl/archive/refs/tags/v0.11.1.zip"
    URL_MD5 "87b3968e0878edf57b09bb8bfaf4618e"
    DOWNLOAD_DIR "${CMAKE_SOURCE_DIR}/dependencies/jpegxl"
    SOURCE_DIR "${EXTERNALPROJECT_SOURCE_PREFIX}/dependencies/jpegxl/source"
    BINARY_DIR "${CMAKE_BINARY_DIR}/jpegxl/build"
    INSTALL_DIR "${CMAKE_BINARY_DIR}/jpegxl/install"
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    BUILD_COMMAND ${BUILD_COMMAND_FOR_TARGET} -t jxl
    CMAKE_ARGS ${CMAKE_BUILD_TYPE_ARG} "-DBUILD_SHARED_LIBS=OFF" "-DBUILD_TESTING=OFF" "-DJPEGXL_ENABLE_BENCHMARK=OFF" "-DJPEGXL_ENABLE_EXAMPLES=OFF" "-DJPEGXL_ENABLE_JNI=OFF"
        "-DJPEGXL_ENABLE_JPEGLI=OFF" "-DJPEGXL_ENABLE_JPEGLI_LIBJPEG=OFF" "-DJPEGXL_ENABLE_MANPAGES=OFF" "-DJPEGXL_ENABLE_OPENEXR=OFF" "-DJPEGXL_ENABLE_SJPEG=OFF"
        "-DJPEGXL_ENABLE_TOOLS=OFF" "-DJPEGXL_ENABLE_TOOLS=ON" "-DJPEGXL_WARNINGS_AS_ERRORS=OFF" "-DJPEGXL_ENABLE_SKCMS=OFF"
        "-DJPEGXL_FORCE_SYSTEM_HWY=ON" "-DHWY_ROOT=${HWY_ROOT}"
        "-DJPEGXL_FORCE_SYSTEM_BROTLI=ON" "-DBrotli_ROOT=${Brotli_ROOT}"
        "-DJPEGXL_FORCE_SYSTEM_LCMS2=ON" "-DLCMS2_ROOT=${LCMS2_ROOT}"
        "-DZLIB_INCLUDE_DIR=${ZLIB_INSTALL_DIR}/include" "-DZLIB_LIBRARY_DEBUG=${ZLIB_INSTALL_DIR}/lib/zlibstaticd.lib" "-DZLIB_LIBRARY_RELEASE=${ZLIB_INSTALL_DIR}/lib/zlibstatic.lib"
        "-DCMAKE_C_FLAGS:STRING=${ZERO_WARNINGS_FLAG} -fPIC -DCMS_NO_REGISTER_KEYWORD=1" "-DCMAKE_CXX_FLAGS:STRING=${ZERO_WARNINGS_FLAG} ${EHSC_FLAG} -fPIC -DCMS_NO_REGISTER_KEYWORD=1 -DJXL_CMS_STATIC_DEFINE=1"
        "-DCMAKE_DEBUG_POSTFIX=d" "-DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_BINARY_DIR}/jpegxl/install"
    EXCLUDE_FROM_ALL
    DEPENDS ZLIB HIGHWAY BROTLI LCMS2
)

ExternalProject_Add_Step(JPEGXL git_patch
    DEPENDEES download
    DEPENDERS configure
    COMMAND echo "Applying git patch"
    COMMAND ${PATCH_EXECUTABLE} -N -p1 -i ${CMAKE_SOURCE_DIR}/cmake/libjpegxl/0001-Added-support-of-debug-libraries-for-hwy-brotli-and-.patch
    COMMAND echo " -- Done"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/dependencies/jpegxl/source
)

ExternalProject_Get_Property(JPEGXL SOURCE_DIR)
ExternalProject_Get_Property(JPEGXL BINARY_DIR)
ExternalProject_Get_Property(JPEGXL INSTALL_DIR)

add_library(LibJpegXL INTERFACE)
add_dependencies(LibJpegXL JPEGXL)
if (MSVC)
    link_library_path2(LibJpegXL ${INSTALL_DIR}/lib jxl.lib jxld.lib)
    link_library_path2(LibJpegXL ${INSTALL_DIR}/lib jxl_cms.lib jxl_cmsd.lib)
else()
    target_link_libraries(LibJpegXL INTERFACE ${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}jxl${CMAKE_STATIC_LIBRARY_SUFFIX})
    target_link_libraries(LibJpegXL INTERFACE ${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}jxl_cms${CMAKE_STATIC_LIBRARY_SUFFIX})
endif()
target_include_directories(LibJpegXL INTERFACE ${INSTALL_DIR}/include)
target_link_libraries(LibJpegXL INTERFACE LibHighway LibBrotli LibLCMS2)
set_property(TARGET JPEGXL PROPERTY FOLDER "Dependencies")


set(JPEGXL_ROOT ${INSTALL_DIR})

unset(SOURCE_DIR)
unset(BINARY_DIR)
unset(INSTALL_DIR)

