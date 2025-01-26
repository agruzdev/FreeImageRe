# dav1d dependency, part of libheif
# https://code.videolan.org/videolan/dav1d
#
# Output targets: LibDav1d


include(${CMAKE_SOURCE_DIR}/cmake/external_project_common.cmake)

# For running meson
find_package(Python3 COMPONENTS Interpreter REQUIRED)

find_program(MESON_EXECUTABLE meson
    PATHS ${Python_LIBRARY_DIRS} ${Python_RUNTIME_LIBRARY_DIRS} ${Python_ROOT_DIR}
    REQUIRED
)

ExternalProject_Add(DAVID
    PREFIX ${CMAKE_BINARY_DIR}/dav1d
    URL "https://code.videolan.org/videolan/dav1d/-/archive/1.5.1/dav1d-1.5.1.zip"
    URL_MD5 "1437156f02211c4e9aebfb0ca6a748d7"
    DOWNLOAD_DIR "${CMAKE_SOURCE_DIR}/dependencies/dav1d"
    SOURCE_DIR "${EXTERNALPROJECT_SOURCE_PREFIX}/dependencies/dav1d/source"
    BINARY_DIR "${CMAKE_BINARY_DIR}/dav1d/build"
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    EXCLUDE_FROM_ALL
)

ExternalProject_Get_Property(DAVID SOURCE_DIR)

set(DAVID_INCLUDE_DIRS ${CMAKE_BINARY_DIR}/dav1d/install/include)
set(DAVID_LINK_DIRS ${CMAKE_BINARY_DIR}/dav1d/install/lib)

if (MSVC)
    ExternalProject_Add_Step(DAVID build_debug
        DEPENDEES configure
        DEPENDERS build
        COMMAND echo "Build Debug"
        COMMAND ${MESON_EXECUTABLE} setup ${CMAKE_BINARY_DIR}/dav1d/build_debug --backend vs2022 --buildtype debug
            --default-library static -Denable_tools=false -Denable_tests=false --prefix ${CMAKE_BINARY_DIR}/dav1d/install
        COMMAND ${CMAKE_VS_MSBUILD_COMMAND} ${CMAKE_BINARY_DIR}/dav1d/build_debug/dav1d.sln
        COMMAND ${CMAKE_VS_MSBUILD_COMMAND} ${CMAKE_BINARY_DIR}/dav1d/build_debug/RUN_INSTALL.vcxproj
        COMMAND ${CMAKE_COMMAND} -E rename ${DAVID_LINK_DIRS}/libdav1d.a ${DAVID_LINK_DIRS}/libdav1d_deb.lib
        COMMAND echo " -- Done"
        WORKING_DIRECTORY ${SOURCE_DIR}
    )

    ExternalProject_Add_Step(DAVID build_release
        DEPENDEES build_debug
        DEPENDERS build
        COMMAND echo "Build Release"
        COMMAND ${MESON_EXECUTABLE} setup ${CMAKE_BINARY_DIR}/dav1d/build_release --backend vs2022 --buildtype release
            --default-library static -Denable_tools=false -Denable_tests=false --prefix ${CMAKE_BINARY_DIR}/dav1d/install
        COMMAND ${CMAKE_VS_MSBUILD_COMMAND} ${CMAKE_BINARY_DIR}/dav1d/build_release/dav1d.sln
        COMMAND ${CMAKE_VS_MSBUILD_COMMAND} ${CMAKE_BINARY_DIR}/dav1d/build_release/RUN_INSTALL.vcxproj
        COMMAND ${CMAKE_COMMAND} -E rename ${DAVID_LINK_DIRS}/libdav1d.a ${DAVID_LINK_DIRS}/libdav1d_rel.lib
        COMMAND echo " -- Done"
        WORKING_DIRECTORY ${SOURCE_DIR}
    )

    set(DAVID_LIBRARY libdav1d_rel)
    set(DAVID_DEBUG_LIBRARY libdav1d_deb)

elseif(UNIX)
     string(TOLOWER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_FOR_STUPID_MESON)

     ExternalProject_Add_Step(DAVID build_unix
        DEPENDEES configure
        DEPENDERS build
        COMMAND echo "Build Unix"
        COMMAND ${MESON_EXECUTABLE} setup ${CMAKE_BINARY_DIR}/dav1d/build_unix --backend ninja --buildtype ${CMAKE_BUILD_TYPE_FOR_STUPID_MESON}
            --default-library static -Denable_tools=false -Denable_tests=false --prefix ${CMAKE_BINARY_DIR}/dav1d/install --libdir=lib
        COMMAND ${MESON_EXECUTABLE} compile -C ${CMAKE_BINARY_DIR}/dav1d/build_unix
        COMMAND ${MESON_EXECUTABLE} install -C ${CMAKE_BINARY_DIR}/dav1d/build_unix
        COMMAND echo " -- Done"
        WORKING_DIRECTORY ${SOURCE_DIR}
    )

    set(DAVID_LIBRARY dav1d)

else()
    message(FATAL_ERROR "Unknown platform")

endif()


set_property(TARGET DAVID PROPERTY FOLDER "Dependencies")

unset(SOURCE_DIR)
unset(BINARY_DIR)

