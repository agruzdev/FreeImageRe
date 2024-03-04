# WEBP dependency
# https://chromium.googlesource.com/webm/libwebp

# Output variables:
# WEBP_INCLUDE_DIR - includes
# WEBP_LIBRARY_DIR - link directories
# WEBP_LIBRARY     - link targets

include(${CMAKE_SOURCE_DIR}/cmake/dependency.common.functions.cmake)

dependency_find_or_download(
    NAME WEBP
    VERBOSE_NAME "WEBP"
    URL "https://chromium.googlesource.com/webm/libwebp/+archive/ca332209cb5567c9b249c86788cb2dbf8847e760.tar.gz"   #v1.3.2
    HASH_MD5 OFF   # googlesource can't provde stable hash, so ignore hash check
    FILE_NAME "webp.tar.gz"
)


set(BUILD_SHARED_LIBS OFF)
set(WEBP_BUILD_ANIM_UTILS OFF)
set(WEBP_BUILD_CWEBP OFF)
set(WEBP_BUILD_DWEBP OFF)
set(WEBP_BUILD_GIF2WEBP OFF)
set(WEBP_BUILD_IMG2WEBP OFF)
set(WEBP_BUILD_VWEBP OFF)
set(WEBP_BUILD_WEBPINFO OFF)
set(WEBP_BUILD_LIBWEBPMUX ON)
set(WEBP_BUILD_WEBPMUX OFF)
set(WEBP_BUILD_EXTRAS OFF)
set(WEBP_UNICODE ON)

add_subdirectory(${WEBP_FOUND_ROOT} ${CMAKE_BINARY_DIR}/dependencies/webp EXCLUDE_FROM_ALL)
set_property(TARGET sharpyuv webp webpdecode webpencode webpdsp webputils libwebpmux PROPERTY FOLDER "Dependencies")

unset(BUILD_SHARED_LIBS)

if (MSVC)
    target_compile_options(tiff PRIVATE "/w")
endif()


set(WEBP_INCLUDE_DIR ${WEBP_FOUND_ROOT}/src CACHE PATH "")
set(WEBP_LIBRARY_DIR "" CACHE PATH "")
set(WEBP_LIBRARY webp libwebpmux CACHE STRING "")
