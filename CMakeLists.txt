cmake_minimum_required(VERSION 3.13)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project(FreeImageRe)

set(FREEIMAGERE_MAJOR_VERSION "0")
set(FREEIMAGERE_MINOR_VERSION "3")

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
include(${CMAKE_SOURCE_DIR}/cmake/dependency.jpeg.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/dependency.openjpeg.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/dependency.openexr.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/dependency.png.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/dependency.tiff.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/dependency.webp.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/dependency.raw.cmake)

# Internal dependencies
add_subdirectory(${CMAKE_SOURCE_DIR}/3rdParty/LibJXR ${CMAKE_BINARY_DIR}/LibJXR)


set(FREEIMAGE_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/Source)

add_definitions("-DFREEIMAGERE_MAJOR_VERSION=${FREEIMAGERE_MAJOR_VERSION}")
add_definitions("-DFREEIMAGERE_MINOR_VERSION=${FREEIMAGERE_MINOR_VERSION}")

add_subdirectory(Source)

option(FREEIMAGE_BUILD_TESTS "Build test executables" OFF)
if(FREEIMAGE_BUILD_TESTS)
    add_subdirectory(TestAPI)
endif()




