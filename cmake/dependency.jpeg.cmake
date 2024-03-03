# JPEG dependency

# Output variables:
# JPEG_INCLUDE_DIR - includes
# JPEG_LIBRARY_DIR - link directories
# JPEG_LIBRARY     - link targets

include(${CMAKE_SOURCE_DIR}/cmake/dependency.common.functions.cmake)

set(JPEG_REPOSITORY "IJG" CACHE STRING "Repository of LibJPEG to use")
set_property(CACHE JPEG_REPOSITORY PROPERTY STRINGS "IJG" "JPEG-turbo")

if (JPEG_REPOSITORY STREQUAL "IJG")
    # Original LibJPEG https://www.ijg.org
    dependency_find_or_download(
        NAME JPEG_IJG
        VERBOSE_NAME "JPEG"
        URL "https://www.ijg.org/files/jpegsr9f.zip"
        HASH_MD5 "8bd2706c80ac696856a1334430a6ffd1"
        FILE_NAME "jpegsr9f.zip"
        PREFIX "jpeg-9f"
    )

    if(NOT TARGET LibJPEG)
        add_subdirectory(${CMAKE_SOURCE_DIR}/cmake/libjpeg ${CMAKE_BINARY_DIR}/dependencies/libjpeg)
        set_property(TARGET LibJPEG PROPERTY FOLDER "Dependencies")
    endif()

    set(JPEG_IJG_INCLUDE_DIR ${JPEG_IJG_FOUND_ROOT} ${CMAKE_SOURCE_DIR}/cmake/libjpeg CACHE PATH "")
    set(JPEG_IJG_LIBRARY_DIR "" CACHE PATH "")
    set(JPEG_IJG_LIBRARY LibJPEG CACHE STRING "")

    set(LIBJPEG_INCLUDE_DIR ${JPEG_IJG_INCLUDE_DIR} CACHE INTERNAL "" FORCE)
    set(LIBJPEG_LIBRARY_DIR ${JPEG_IJG_LIBRARY_DIR} CACHE INTERNAL "" FORCE)
    set(LIBJPEG_LIBRARY ${JPEG_IJG_LIBRARY} CACHE INTERNAL "" FORCE)

elseif(JPEG_REPOSITORY STREQUAL "JPEG-turbo")
    # https://github.com/libjpeg-turbo/libjpeg-turbo
    dependency_find_or_download(
        NAME JPEG_TURBO
        VERBOSE_NAME "JPEG-turbo"
        URL "https://github.com/libjpeg-turbo/libjpeg-turbo/archive/refs/tags/2.1.4.zip"
        HASH_MD5 "ef4c3147b67629958a4c4a7b5af898eb"
        FILE_NAME "2.1.4.zip"
        PREFIX "libjpeg-turbo-2.1.4"
    )

    if(NOT TARGET turbojpeg-static)
        set(ENABLE_SHARED OFF)
        set(ENABLE_STATIC ON)
        set(WITH_12BIT OFF)
        set(WITH_JPEG7 ON)
        set(WITH_CRT_DLL ON)

        add_subdirectory(${JPEG_TURBO_FOUND_ROOT} ${CMAKE_BINARY_DIR}/dependencies/libjpeg_turbo EXCLUDE_FROM_ALL)

        unset(ENABLE_SHARED)
        unset(ENABLE_STATIC)
        unset(WITH_12BIT)
        unset(WITH_JPEG7)
        unset(WITH_CRT_DLL)

        set_property(TARGET turbojpeg-static simd PROPERTY FOLDER "Dependencies")
        # cannot use INTERFACE due to install()
        #target_include_directories(turbojpeg-static INTERFACE ${JPEG_TURBO_FOUND_ROOT} ${CMAKE_BINARY_DIR}/dependencies/libjpeg_turbo)

        target_compile_options(turbojpeg-static INTERFACE "-DJPEG_HAS_READ_ICC_PROFILE=1")
    endif()

    set(JPEG_TURBO_INCLUDE_DIR ${JPEG_TURBO_FOUND_ROOT} ${CMAKE_BINARY_DIR}/dependencies/libjpeg_turbo CACHE PATH "")
    set(JPEG_TURBO_LIBRARY_DIR "" CACHE PATH "")
    set(JPEG_TURBO_LIBRARY turbojpeg-static CACHE STRING "")

    set(LIBJPEG_INCLUDE_DIR ${JPEG_TURBO_INCLUDE_DIR} CACHE INTERNAL "" FORCE)
    set(LIBJPEG_LIBRARY_DIR ${JPEG_TURBO_LIBRARY_DIR} CACHE INTERNAL "" FORCE)
    set(LIBJPEG_LIBRARY ${JPEG_TURBO_LIBRARY} CACHE INTERNAL "" FORCE)
else()
    message(FATAL "Invalid JPEG repository = ${JPEG_REPOSITORY}")
endif()


