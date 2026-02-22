# JPEG dependency
#
# Output target: LibJPEG

include(${EXTERNALPROJECT_INCLUDE_DIR}/external_project_common.cmake)


set(JPEG_REPOSITORY "JPEG-turbo" CACHE STRING "Repository of LibJPEG to use")
set_property(CACHE JPEG_REPOSITORY PROPERTY STRINGS "IJG" "JPEG-turbo")

if (JPEG_REPOSITORY STREQUAL "IJG")
    ExternalProject_Add(JPEG
        URL "https://www.ijg.org/files/jpegsr9f.zip"
        URL_MD5 "8bd2706c80ac696856a1334430a6ffd1"
        DOWNLOAD_DIR "${EXTERNALPROJECT_SOURCE_ROOT}/jpeg"
        SOURCE_DIR "${EXTERNALPROJECT_SOURCE_PREFIX}/jpeg/source"
        BINARY_DIR "${EXTERNALPROJECT_BINARY_ROOT}/jpeg/build"
        INSTALL_DIR "${EXTERNALPROJECT_BINARY_ROOT}/jpeg/install"
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        PATCH_COMMAND ${CMAKE_COMMAND} -E copy "${EXTERNALPROJECT_INCLUDE_DIR}/libjpeg/CMakeLists.txt"
                                               "${EXTERNALPROJECT_INCLUDE_DIR}/libjpeg/jconfig.h"
                -t "${EXTERNALPROJECT_SOURCE_PREFIX}/jpeg/source"
        BUILD_COMMAND ${BUILD_COMMAND_FOR_TARGET}
        INSTALL_COMMAND ${BUILD_COMMAND_FOR_TARGET} -t install
        CMAKE_ARGS ${CMAKE_BUILD_TYPE_ARG} "-DCMAKE_INSTALL_PREFIX:PATH=${EXTERNALPROJECT_BINARY_ROOT}/jpeg/install"
        EXCLUDE_FROM_ALL
    )

    ExternalProject_Get_Property(JPEG INSTALL_DIR)

    add_library(LibJPEG INTERFACE)
    add_dependencies(LibJPEG JPEG)
    target_link_libraries(LibJPEG INTERFACE ${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}jpeg${CMAKE_STATIC_LIBRARY_SUFFIX})
    target_include_directories(LibJPEG INTERFACE ${INSTALL_DIR}/include)
    set_property(TARGET JPEG PROPERTY FOLDER "Dependencies")

    unset(INSTALL_DIR)

elseif(JPEG_REPOSITORY STREQUAL "JPEG-turbo")
    # https://github.com/libjpeg-turbo/libjpeg-turbo
    ExternalProject_Add(TURBOJPEG
        PREFIX ${EXTERNALPROJECT_BINARY_ROOT}/turbojpeg
        URL "https://github.com/libjpeg-turbo/libjpeg-turbo/archive/refs/tags/3.1.3.zip"
        URL_MD5 "4c3ebabaabd2fa2e9f29cc97ac7e3946"
        DOWNLOAD_DIR "${EXTERNALPROJECT_SOURCE_ROOT}/turbojpeg"
        SOURCE_DIR "${EXTERNALPROJECT_SOURCE_PREFIX}/turbojpeg/source"
        BINARY_DIR "${EXTERNALPROJECT_BINARY_ROOT}/turbojpeg/build"
        INSTALL_DIR "${EXTERNALPROJECT_BINARY_ROOT}/turbojpeg/install"
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        UPDATE_COMMAND ""
        PATCH_COMMAND ""
        BUILD_COMMAND ${BUILD_COMMAND_FOR_TARGET} -t turbojpeg-static
        INSTALL_COMMAND ${BUILD_COMMAND_FOR_TARGET} -t install
                COMMAND ${CMAKE_COMMAND} -E copy "${EXTERNALPROJECT_SOURCE_PREFIX}/turbojpeg/source/src/jinclude.h"
                                                 "${EXTERNALPROJECT_SOURCE_PREFIX}/turbojpeg/source/src/transupp.h"
                                                 "${EXTERNALPROJECT_BINARY_ROOT}/turbojpeg/build/jversion.h"
                                                 "${EXTERNALPROJECT_BINARY_ROOT}/turbojpeg/build/jconfigint.h"
                    -t "${EXTERNALPROJECT_BINARY_ROOT}/turbojpeg/install/include"
        CMAKE_ARGS ${CMAKE_BUILD_TYPE_ARG} "-DENABLE_SHARED=OFF" "-DENABLE_STATIC=ON" "-DWITH_JPEG7=ON" "-DWITH_CRT_DLL=ON" "-DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON"
            "-DWITH_TOOLS=OFF" "-DCMAKE_INSTALL_PREFIX:PATH=${EXTERNALPROJECT_BINARY_ROOT}/turbojpeg/install"
        EXCLUDE_FROM_ALL
    )

    ExternalProject_Get_Property(TURBOJPEG INSTALL_DIR)

    add_library(LibJPEG INTERFACE)
    add_dependencies(LibJPEG TURBOJPEG)
    if (MSVC)
        target_link_libraries(LibJPEG INTERFACE ${INSTALL_DIR}/lib/turbojpeg-static${CMAKE_STATIC_LIBRARY_SUFFIX})
    else()
        target_link_libraries(LibJPEG INTERFACE ${INSTALL_DIR}/lib/libturbojpeg${CMAKE_STATIC_LIBRARY_SUFFIX})
    endif()
    target_compile_options(LibJPEG INTERFACE "-DJPEG_HAS_READ_ICC_PROFILE=1")
    target_include_directories(LibJPEG INTERFACE ${INSTALL_DIR}/include)
    set_property(TARGET TURBOJPEG PROPERTY FOLDER "Dependencies")

    unset(INSTALL_DIR)

else()
    message(FATAL "Invalid JPEG repository = ${JPEG_REPOSITORY}")

endif()

