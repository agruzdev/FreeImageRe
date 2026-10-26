include_directories(${FREEIMAGE_INCLUDE_DIR})

add_executable(LoadSample LoadSample.cpp)

target_link_libraries(LoadSample FreeImage)
