cmake_minimum_required(VERSION 3.10)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project(FreeImageRe)

set(FREEIMAGERE_MAJOR_VERSION "0")
set(FREEIMAGERE_MINOR_VERSION "1")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(${CMAKE_SOURCE_DIR}/cmake/dependency.zlib.cmake)


set(FREEIMAGE_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/Source)

add_subdirectory(Source)

option(FREEIMAGE_BUILD_TESTS "Build test executables" OFF)
if(FREEIMAGE_BUILD_TESTS)
    add_subdirectory(TestAPI)
endif()




