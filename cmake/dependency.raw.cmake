# LibRaw dependency
# https://github.com/LibRaw/LibRaw
#
# Output target: LibRAW

if (USE_SYSTEM_LIBRAW OR USE_SYSTEM_LIBS)
    find_package(PkgConfig)
    pkg_check_modules(LIBRAW REQUIRED IMPORTED_TARGET libraw)
    add_library(LibRAW ALIAS PkgConfig::LIBRAW)
    return()
endif()

include(${EXTERNALPROJECT_INCLUDE_DIR}/external_project_common.cmake)


FetchContent_Declare(RAW
   URL "https://github.com/LibRaw/LibRaw/archive/refs/tags/0.22.1.zip"
   URL_MD5 "9817d4f903f5812be7e839b24c38934b"
   DOWNLOAD_DIR "${EXTERNALPROJECT_SOURCE_ROOT}/raw"
   SOURCE_DIR "${EXTERNALPROJECT_SOURCE_PREFIX}/raw/source"
   DOWNLOAD_EXTRACT_TIMESTAMP TRUE
   EXCLUDE_FROM_ALL
)

FetchContent_MakeAvailable(RAW)

set(RAW_FOUND_ROOT "${EXTERNALPROJECT_SOURCE_PREFIX}/raw/source" CACHE INTERNAL "")
add_subdirectory(${EXTERNALPROJECT_INCLUDE_DIR}/libraw ${EXTERNALPROJECT_BINARY_ROOT}/libraw/build)
set_property(TARGET LibRAW PROPERTY FOLDER "Dependencies")

unset(RAW_FOUND_ROOT)
