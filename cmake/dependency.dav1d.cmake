# dav1d dependency, part of libheif
# https://code.videolan.org/videolan/dav1d
#
# Output targets: LibDav1d


include(${EXTERNALPROJECT_INCLUDE_DIR}/external_project_common.cmake)

# For running meson
find_package(Python3 COMPONENTS Interpreter REQUIRED)

find_program(MESON_EXECUTABLE meson
    PATHS ${Python_LIBRARY_DIRS} ${Python_RUNTIME_LIBRARY_DIRS} ${Python_ROOT_DIR}
    REQUIRED
)


if (MSVC)
    set(MESON_BACKEND ${MSVC_NAME})
else()
    set(MESON_BACKEND ninja)
endif()

meson_build_type_from_cmake(MESON_BUILD_TYPE)

ExternalProject_Add(DAVID
    PREFIX ${EXTERNALPROJECT_BINARY_ROOT}/dav1d
    URL "https://code.videolan.org/videolan/dav1d/-/archive/1.5.3/dav1d-1.5.3.zip"
    URL_MD5 "1d7d9f14e106ed10d376bac434e113b7"
    DOWNLOAD_DIR "${EXTERNALPROJECT_SOURCE_ROOT}/dav1d"
    SOURCE_DIR "${EXTERNALPROJECT_SOURCE_PREFIX}/dav1d/source"
    BINARY_DIR "${EXTERNALPROJECT_BINARY_ROOT}/dav1d/build"
    INSTALL_DIR "${EXTERNALPROJECT_BINARY_ROOT}/dav1d/install"
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    CONFIGURE_COMMAND ${MESON_EXECUTABLE} setup --backend ${MESON_BACKEND} --buildtype ${MESON_BUILD_TYPE}
            --default-library static -Denable_tools=false -Denable_tests=false --prefix ${EXTERNALPROJECT_BINARY_ROOT}/dav1d/install --libdir=lib
            ${EXTERNALPROJECT_BINARY_ROOT}/dav1d/build ${EXTERNALPROJECT_SOURCE_PREFIX}/dav1d/source
    BUILD_COMMAND ${MESON_EXECUTABLE} compile -C ${EXTERNALPROJECT_BINARY_ROOT}/dav1d/build
    INSTALL_COMMAND ${MESON_EXECUTABLE} install -C ${EXTERNALPROJECT_BINARY_ROOT}/dav1d/build
    EXCLUDE_FROM_ALL
)

ExternalProject_Get_Property(DAVID INSTALL_DIR)

if (WIN32)
    ExternalProject_Add_Step(DAVID rename_stupid_meson
        DEPENDEES install
        COMMAND ${CMAKE_COMMAND} -E rename ${INSTALL_DIR}/lib/libdav1d.a ${INSTALL_DIR}/lib/dav1d.lib
    )
endif()



set(DAVID_INCLUDE_DIRS ${INSTALL_DIR}/include)
set(DAVID_LINK_DIRS ${INSTALL_DIR}/lib)
set(DAVID_LIBRARY dav1d)

set_property(TARGET DAVID PROPERTY FOLDER "Dependencies")

unset(INSTALL_DIR)
unset(MESON_BACKEND)
unset(MESON_BUILD_TYPE)

