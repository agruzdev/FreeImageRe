cmake_minimum_required(VERSION 3.28)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project(FreeImageRe)

set(FREEIMAGERE_MAJOR_VERSION "0")
set(FREEIMAGERE_MINOR_VERSION "4")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_INSTALL_DOCDIR doc)

set(CMAKE_POLICY_DEFAULT_CMP0077 "NEW")
cmake_policy(SET CMP0079 NEW) # -> target_link_libraries() allows use with targets in other directories.

if (UNIX)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# External dependencies
include(${CMAKE_SOURCE_DIR}/cmake/dependency.zlib.cmake)

option(FREEIMAGE_WITH_LIBJPEG "Compile with the LibJPEG backend" ON)
if (FREEIMAGE_WITH_LIBJPEG)
    include(${CMAKE_SOURCE_DIR}/cmake/dependency.jpeg.cmake)
endif()

option(FREEIMAGE_WITH_LIBOPENJPEG "Compile with the LibOpenJPEG backend" ON)
if (FREEIMAGE_WITH_LIBOPENJPEG)
    include(${CMAKE_SOURCE_DIR}/cmake/dependency.openjpeg.cmake)
endif()

option(FREEIMAGE_WITH_LIBOPENEXR "Compile with the LibOpenEXR backend" ON)
if (FREEIMAGE_WITH_LIBOPENEXR)
    include(${CMAKE_SOURCE_DIR}/cmake/dependency.openexr.cmake)
endif()

option(FREEIMAGE_WITH_LIBPNG "Compile with the LibPNG backend" ON)
if (FREEIMAGE_WITH_LIBPNG)
    include(${CMAKE_SOURCE_DIR}/cmake/dependency.png.cmake)
endif()

option(FREEIMAGE_WITH_LIBTIFF "Compile with the LibTIFF backend" ON)
if (FREEIMAGE_WITH_LIBTIFF)
    include(${CMAKE_SOURCE_DIR}/cmake/dependency.tiff.cmake)
endif()

option(FREEIMAGE_WITH_LIBWEBP "Compile with the LibWEBP backend" ON)
if (FREEIMAGE_WITH_LIBWEBP)
    include(${CMAKE_SOURCE_DIR}/cmake/dependency.webp.cmake)
endif()

option(FREEIMAGE_WITH_LIBRAW "Compile with the LibRAW backend" ON)
if (FREEIMAGE_WITH_LIBRAW)
    include(${CMAKE_SOURCE_DIR}/cmake/dependency.raw.cmake)
endif()

option(FREEIMAGE_WITH_LIBHEIF "Compile with the LibHEIF backend" ON)
if (FREEIMAGE_WITH_LIBHEIF)
    include(${CMAKE_SOURCE_DIR}/cmake/dependency.heif.cmake)
endif()

option(FREEIMAGE_WITH_PYTHON_BINDINGS "Build pythin bindings" OFF)
if (FREEIMAGE_WITH_PYTHON_BINDINGS)
    find_package(Python3 COMPONENTS Development REQUIRED)
    include(${CMAKE_SOURCE_DIR}/cmake/dependency.pybind.cmake)
endif()

# Internal dependencies
option(FREEIMAGE_WITH_LIBJXR "Compile with the LibJXR backend" ON)
if (FREEIMAGE_WITH_LIBJXR)
    add_subdirectory(${CMAKE_SOURCE_DIR}/3rdParty/LibJXR ${CMAKE_BINARY_DIR}/LibJXR)
endif()

set(FREEIMAGE_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/Source)

add_definitions("-DFREEIMAGERE_MAJOR_VERSION=${FREEIMAGERE_MAJOR_VERSION}")
add_definitions("-DFREEIMAGERE_MINOR_VERSION=${FREEIMAGERE_MINOR_VERSION}")

add_subdirectory(Source)

option(FREEIMAGE_BUILD_TESTS "Build test executables" OFF)
if(FREEIMAGE_BUILD_TESTS)
    add_subdirectory(TestAPI)
endif()




