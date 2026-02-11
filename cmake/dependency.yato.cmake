# Yato dependency
# https://github.com/agruzdev/Yato

# Output target: LibYato
# YATO_INCLUDE_DIR - includes


include(${EXTERNALPROJECT_INCLUDE_DIR}/external_project_common.cmake)


ExternalProject_Add(YATO
    PREFIX ${EXTERNALPROJECT_BINARY_ROOT}/yato
    URL "https://github.com/agruzdev/Yato/archive/4920cfc8cf8fa2c4879f3b7679b2a1aa6092bae4.zip"
    URL_MD5 "02c227cb8199282f1f581289ba1e97b5"
    DOWNLOAD_DIR "${EXTERNALPROJECT_SOURCE_ROOT}/yato"
    SOURCE_DIR "${EXTERNALPROJECT_SOURCE_PREFIX}/yato/source"
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    EXCLUDE_FROM_ALL
)

ExternalProject_Get_Property(YATO SOURCE_DIR)

add_library(LibYato INTERFACE)
add_dependencies(LibYato YATO)
target_include_directories(LibYato INTERFACE ${SOURCE_DIR}/include)
set_property(TARGET YATO PROPERTY FOLDER "Dependencies")

unset(SOURCE_DIR)

