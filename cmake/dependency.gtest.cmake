# GoogleTest dependency
# https://github.com/google/googletest/releases/tag/v1.18.0
#
# Output targets: LibGTest, LibGTestMain

if(NOT _GTEST_DEP_INCLUDE_GUARD_)
set(_GTEST_DEP_INCLUDE_GUARD_ ON)

include(${EXTERNALPROJECT_INCLUDE_DIR}/external_project_common.cmake)

set(GTEST_ROOT "${EXTERNALPROJECT_BINARY_ROOT}/googletest/install")
set(GTEST_SOURCE_ROOT "${EXTERNALPROJECT_SOURCE_PREFIX}/googletest/source")

ExternalProject_Add(GTEST
	PREFIX "${EXTERNALPROJECT_BINARY_ROOT}/googletest"
	URL "https://github.com/google/googletest/archive/refs/tags/v1.18.0.zip"
	URL_HASH "SHA256=63b9c77751a5b8f492486005f67533fdc58682b67476fcb91650be5958d5195a"
	DOWNLOAD_DIR "${EXTERNALPROJECT_SOURCE_ROOT}/googletest"
	DOWNLOAD_EXTRACT_TIMESTAMP TRUE
	SOURCE_DIR "${GTEST_SOURCE_ROOT}"
	BINARY_DIR "${EXTERNALPROJECT_BINARY_ROOT}/googletest/build"
	INSTALL_DIR "${GTEST_ROOT}"
	UPDATE_COMMAND ""
	CMAKE_ARGS ${EXTERNALPROJECT_CMAKE_ARGS}
		"-DCMAKE_INSTALL_PREFIX:PATH=${GTEST_ROOT}"
		"-DBUILD_GMOCK=OFF"
		"-DINSTALL_GTEST=ON"
		"-Dgtest_build_tests=OFF"
		"-Dgtest_build_samples=OFF"
)

add_library(LibGTest INTERFACE)
add_dependencies(LibGTest GTEST)
target_include_directories(LibGTest INTERFACE "${GTEST_SOURCE_ROOT}/googletest/include")
target_link_directories(LibGTest INTERFACE "${GTEST_ROOT}/lib" "${GTEST_ROOT}/lib/Debug" "${GTEST_ROOT}/lib/Release")
if(MSVC)
	target_link_libraries(LibGTest INTERFACE gtest)
else()
	target_link_libraries(LibGTest INTERFACE gtest)
endif()

add_library(LibGTestMain INTERFACE)
add_dependencies(LibGTestMain GTEST)
target_link_libraries(LibGTestMain INTERFACE LibGTest)
if(MSVC)
	target_link_libraries(LibGTestMain INTERFACE gtest_main)
else()
	target_link_libraries(LibGTestMain INTERFACE gtest_main)
endif()

set_property(TARGET GTEST PROPERTY FOLDER "Dependencies")

endif() # _GTEST_DEP_INCLUDE_GUARD_
