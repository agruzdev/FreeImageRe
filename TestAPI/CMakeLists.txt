
file(GLOB all_test_sources ./*.cpp ./*.h)

include(GoogleTest)

add_executable(TestAPI ${all_test_sources})

target_include_directories(TestAPI PRIVATE
	${FREEIMAGE_INCLUDE_DIR}
	${CMAKE_SOURCE_DIR}/3rdParty/Yato/include
)
target_link_libraries(TestAPI PRIVATE FreeImage LibGTestMain)

if(BUILD_TESTING)
	gtest_discover_tests(TestAPI WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()
