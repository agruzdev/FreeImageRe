
file(GLOB all_test_sources ./*.cpp ./*.h)

include(GoogleTest)

add_executable(TestAPI ${all_test_sources})

target_include_directories(TestAPI PRIVATE
	${FREEIMAGE_INCLUDE_DIR}
	${CMAKE_SOURCE_DIR}/3rdParty/Yato/include
)
target_link_libraries(TestAPI PRIVATE FreeImage LibGTestMain)

set(TESTAPI_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/TestAPI)
set(TESTAPI_RUNTIME_PATH ${EXTERNALPROJECT_BINARY_ROOT}/heif/install/bin)
set_target_properties(TestAPI PROPERTIES
	VS_DEBUGGER_WORKING_DIRECTORY ${TESTAPI_WORKING_DIRECTORY}
	VS_DEBUGGER_ENVIRONMENT "PATH=%PATH%;${TESTAPI_RUNTIME_PATH}"
)

if(BUILD_TESTING)
	gtest_discover_tests(TestAPI WORKING_DIRECTORY ${TESTAPI_WORKING_DIRECTORY} PROPERTIES ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:${TESTAPI_RUNTIME_PATH}")
endif()
