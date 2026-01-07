# JPEG dependency
#
# Output target: LibJPEG

include(${CMAKE_SOURCE_DIR}/cmake/external_project_common.cmake)


set(JPEG_REPOSITORY "JPEG-turbo" CACHE STRING "Repository of LibJPEG to use")
set_property(CACHE JPEG_REPOSITORY PROPERTY STRINGS "IJG" "JPEG-turbo")

if (JPEG_REPOSITORY STREQUAL "IJG")
    FetchContent_Declare(JPEG
        URL "https://www.ijg.org/files/jpegsr9f.zip"
        URL_MD5 "8bd2706c80ac696856a1334430a6ffd1"
        DOWNLOAD_DIR "${CMAKE_SOURCE_DIR}/dependencies/jpeg"
        SOURCE_DIR "${EXTERNALPROJECT_SOURCE_PREFIX}/dependencies/jpeg/source"
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        EXCLUDE_FROM_ALL
    )

    FetchContent_MakeAvailable(JPEG)

    set(JPEG_IJG_FOUND_ROOT "${EXTERNALPROJECT_SOURCE_PREFIX}/dependencies/jpeg/source" CACHE INTERNAL "")
    add_subdirectory(${CMAKE_SOURCE_DIR}/cmake/libjpeg ${CMAKE_BINARY_DIR}/dependencies/libjpeg)
    set_property(TARGET LibJPEG PROPERTY FOLDER "Dependencies")
    target_include_directories(LibJPEG INTERFACE ${JPEG_IJG_FOUND_ROOT} ${CMAKE_SOURCE_DIR}/cmake/libjpeg)

    unset(JPEG_IJG_FOUND_ROOT)

elseif(JPEG_REPOSITORY STREQUAL "JPEG-turbo")
    # https://github.com/libjpeg-turbo/libjpeg-turbo
    ExternalProject_Add(TURBOJPEG
        PREFIX ${CMAKE_BINARY_DIR}/turbojpeg
        URL "https://github.com/libjpeg-turbo/libjpeg-turbo/archive/refs/tags/3.1.3.zip"
        URL_MD5 "4c3ebabaabd2fa2e9f29cc97ac7e3946"
        DOWNLOAD_DIR "${CMAKE_SOURCE_DIR}/dependencies/jpeg-turbo"
        SOURCE_DIR "${EXTERNALPROJECT_SOURCE_PREFIX}/dependencies/jpeg-turbo/source"
        BINARY_DIR "${CMAKE_BINARY_DIR}/turbojpeg/build"
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        UPDATE_COMMAND ""
        PATCH_COMMAND ""
        BUILD_COMMAND ${BUILD_COMMAND_FOR_TARGET} -t turbojpeg-static
        INSTALL_COMMAND ""
        CMAKE_ARGS ${CMAKE_BUILD_TYPE_ARG} "-DENABLE_SHARED=OFF" "-DENABLE_STATIC=ON" "-DWITH_JPEG7=ON" "-DWITH_CRT_DLL=ON" "-DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON"
        EXCLUDE_FROM_ALL
    )

    ExternalProject_Get_Property(TURBOJPEG SOURCE_DIR)
    ExternalProject_Get_Property(TURBOJPEG BINARY_DIR)

    add_library(LibJPEG INTERFACE)
    add_dependencies(LibJPEG TURBOJPEG)
    if (MSVC)
        link_config_aware_library_path(LibJPEG ${BINARY_DIR} turbojpeg-static${CMAKE_STATIC_LIBRARY_SUFFIX})
    else()
        link_config_aware_library_path(LibJPEG ${BINARY_DIR} libturbojpeg${CMAKE_STATIC_LIBRARY_SUFFIX})
    endif()
    target_compile_options(LibJPEG INTERFACE "-DJPEG_HAS_READ_ICC_PROFILE=1")
    target_include_directories(LibJPEG INTERFACE ${SOURCE_DIR}/src ${BINARY_DIR})
    set_property(TARGET TURBOJPEG PROPERTY FOLDER "Dependencies")

    unset(SOURCE_DIR)
    unset(BINARY_DIR)
else()
    message(FATAL "Invalid JPEG repository = ${JPEG_REPOSITORY}")

endif()

