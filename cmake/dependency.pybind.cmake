# pybind11 dependency
# https://github.com/pybind/pybind11

# Output target: LibPybind
# PYBIND_INCLUDE_DIR - includes


include(${CMAKE_SOURCE_DIR}/cmake/external_project_common.cmake)


ExternalProject_Add(PYBIND
    PREFIX ${CMAKE_BINARY_DIR}/pybind
    URL "https://github.com/pybind/pybind11/archive/refs/tags/v2.12.0.zip"
    URL_MD5 "a09cb1982fc9ca4d6a4de27352cf29f2"
    DOWNLOAD_DIR "${CMAKE_SOURCE_DIR}/dependencies/pybind"
    SOURCE_DIR "${EXTERNALPROJECT_SOURCE_PREFIX}/dependencies/pybind/source"
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    EXCLUDE_FROM_ALL
)

ExternalProject_Get_Property(PYBIND SOURCE_DIR)

add_library(LibPybind INTERFACE)
add_dependencies(LibPybind PYBIND)
target_include_directories(LibPybind INTERFACE ${SOURCE_DIR}/include)
set_property(TARGET PYBIND PROPERTY FOLDER "Dependencies")

unset(SOURCE_DIR)

