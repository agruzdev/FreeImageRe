# nanobind dependency
# https://github.com/wjakob/nanobind

include(${EXTERNALPROJECT_INCLUDE_DIR}/external_project_common.cmake)

find_package(Python 3.12 COMPONENTS Interpreter Development.Module Development.SABIModule REQUIRED)

FetchContent_Declare(NANOBIND
   GIT_REPOSITORY "https://github.com/wjakob/nanobind"
   GIT_TAG v3.1.0
   GIT_SHALLOW TRUE
   DOWNLOAD_DIR "${EXTERNALPROJECT_SOURCE_ROOT}/nanobind"
   SOURCE_DIR "${EXTERNALPROJECT_SOURCE_PREFIX}/nanobind/source"
   DOWNLOAD_EXTRACT_TIMESTAMP TRUE
   EXCLUDE_FROM_ALL
)

FetchContent_MakeAvailable(NANOBIND)

add_subdirectory(${EXTERNALPROJECT_SOURCE_PREFIX}/nanobind/source ${EXTERNALPROJECT_BINARY_ROOT}/nanobind/build)
#set_property(TARGET nanobind-static PROPERTY FOLDER "Dependencies")

