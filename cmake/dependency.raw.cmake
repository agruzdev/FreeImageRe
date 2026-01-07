# LibRaw dependency
# https://github.com/LibRaw/LibRaw
#
# Output target: LibRAW


include(${CMAKE_SOURCE_DIR}/cmake/external_project_common.cmake)


FetchContent_Declare(RAW
   URL "https://github.com/LibRaw/LibRaw/archive/refs/tags/0.21.5.zip"
   URL_MD5 "700bb865bf731d04e6e53d65b2e0e73e"
   DOWNLOAD_DIR "${CMAKE_SOURCE_DIR}/dependencies/raw"
   SOURCE_DIR "${EXTERNALPROJECT_SOURCE_PREFIX}/dependencies/raw/source"
   DOWNLOAD_EXTRACT_TIMESTAMP TRUE
   EXCLUDE_FROM_ALL
)

FetchContent_MakeAvailable(RAW)

set(RAW_FOUND_ROOT "${EXTERNALPROJECT_SOURCE_PREFIX}/dependencies/raw/source" CACHE INTERNAL "")
add_subdirectory(${CMAKE_SOURCE_DIR}/cmake/libraw ${CMAKE_BINARY_DIR}/dependencies/libraw)
set_property(TARGET LibRAW PROPERTY FOLDER "Dependencies")

unset(RAW_FOUND_ROOT)
