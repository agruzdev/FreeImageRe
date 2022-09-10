# JPEG dependency

# Output variables:
# JPEG_INCLUDE_DIR - includes
# JPEG_LIBRARY_DIR - link directories
# JPEG_LIBRARIES   - link targets

include(${CMAKE_SOURCE_DIR}/cmake/dependency.common.functions.cmake)

# Original LibJPEG https://www.ijg.org
dependency_find_or_download(
    NAME JPEG
    VERBOSE_NAME "JPEG"
    URL "https://www.ijg.org/files/jpegsr9e.zip"
    HASH_MD5 "e580b368b85f380abfdde816d37cf855"
    FILE_NAME "jpegsr9e.zip"
    PREFIX "jpeg-9e"
)

if(NOT TARGET LibJPEG)
    add_subdirectory(${CMAKE_SOURCE_DIR}/cmake/libjpeg ${CMAKE_BINARY_DIR}/dependencies/libjpeg)
    set_property(TARGET LibJPEG PROPERTY FOLDER "Dependencies")
endif()

set(JPEG_INCLUDE_DIR ${JPEG_FOUND_ROOT} ${CMAKE_SOURCE_DIR}/cmake/libjpeg CACHE PATH "")
set(JPEG_LIBRARY_DIR "" CACHE PATH "")
set(JPEG_LIBRARIES LibJPEG CACHE STRING "")
