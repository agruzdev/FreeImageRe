# svt-av1 dependency, part of libheif
# https://gitlab.com/AOMediaCodec/SVT-AV1
#
# Output targets: LibSvtAv1


include(${CMAKE_SOURCE_DIR}/cmake/external_project_common.cmake)

# svtav1 can't compile CPUINFO for MSVC, so disable it for now
# prebuild it manually if necessary

ExternalProject_Add(SVTAV1
    PREFIX ${CMAKE_BINARY_DIR}/svtav1
    URL "https://gitlab.com/AOMediaCodec/SVT-AV1/-/archive/v3.1.2/SVT-AV1-v3.1.2.zip"
    URL_MD5 "6bf4dabc7d1f84ecf784c5d80e381113"
    DOWNLOAD_DIR "${CMAKE_SOURCE_DIR}/dependencies/svtav1"
    SOURCE_DIR "${EXTERNALPROJECT_SOURCE_PREFIX}/dependencies/svtav1/source"
    BINARY_DIR "${CMAKE_BINARY_DIR}/svtav1/build"
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    BUILD_COMMAND ${BUILD_COMMAND_FOR_TARGET}
    INSTALL_COMMAND ${BUILD_COMMAND_FOR_TARGET} -t install
    CMAKE_ARGS ${CMAKE_BUILD_TYPE_ARG} "-DBUILD_SHARED_LIBS=OFF" "-DBUILD_APPS=OFF" "-DBUILD_TESTING=OFF" "-DSVT_AV1_LTO=ON" "-DUSE_CPUINFO=OFF" #"-DMINIMAL_BUILD=ON"
        "-DCMAKE_C_FLAGS:STRING=${CMAKE_C_FLAGS} ${ZERO_WARNINGS_FLAG}" "-DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS} ${ZERO_WARNINGS_FLAG}"
        "-DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_BINARY_DIR}/svtav1/install" ${CMAKE_DEBUG_POSTFIX_MULTICONF}
    EXCLUDE_FROM_ALL
)

ExternalProject_Get_Property(SVTAV1 SOURCE_DIR)
ExternalProject_Get_Property(SVTAV1 BINARY_DIR)

set_property(TARGET SVTAV1 PROPERTY FOLDER "Dependencies")

set(SVTAV1_INCLUDE_DIRS ${CMAKE_BINARY_DIR}/svtav1/install/include)
set(SVTAV1_LINK_DIRS ${CMAKE_BINARY_DIR}/svtav1/install/lib)
set(SVTAV1_LIBRARY SvtAv1Enc)
if (IS_MULTI_CONFIG)
    set(SVTAV1_DEBUG_LIBRARY SvtAv1Encd)
endif()

unset(SOURCE_DIR)
unset(BINARY_DIR)

