# pybind11 dependency
# https://github.com/pybind/pybind11

# Output variables:
# PYBIND_INCLUDE_DIR - includes

include(${CMAKE_SOURCE_DIR}/cmake/dependency.common.functions.cmake)

dependency_find_or_download(
    NAME PYBIND
    VERBOSE_NAME "pybind11"
    URL "https://github.com/pybind/pybind11/archive/refs/tags/v2.12.0.zip"
    HASH_MD5 "a09cb1982fc9ca4d6a4de27352cf29f2"
    FILE_NAME "pybind11-2.12.0.zip"
    PREFIX "pybind11-2.12.0"
)

set(PYBIND_INCLUDE_DIR ${PYBIND_FOUND_ROOT}/include CACHE PATH "")

