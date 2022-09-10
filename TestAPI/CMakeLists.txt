
file(GLOB all_test_sources ./*.cpp ./*.h)

include_directories(${FREEIMAGE_INCLUDE_DIR})

add_executable(TestAPI ${all_test_sources})

target_link_libraries(TestAPI FreeImage)
