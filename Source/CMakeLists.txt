project(FreeImage)

file(GLOB freeimage_sources FreeImage/*.cpp FreeImage/*.h)
file(GLOB freeimage_toolkit_sources FreeImageToolkit/*.cpp FreeImageToolkit/*.h)
file(GLOB metadata_sources Metadata/*.cpp Metadata/*.h)
file(GLOB freeimage_headers ./*.h ./*.hpp)
list(APPEND freeimage_all_sources ${freeimage_sources} ${freeimage_toolkit_sources} ${metadata_sources} ${freeimage_headers})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "Source files" FILES ${freeimage_all_sources})

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Metadata
    ${CMAKE_CURRENT_SOURCE_DIR}/FreeImageToolkit
)

add_definitions(
    "-DFREEIMAGE_EXPORTS"
    "-DOPJ_STATIC"
)

if(WIN32)
    add_definitions("-DWINVER=0x0500")
    if(MSVC)
        add_definitions("-D_CRT_SECURE_NO_WARNINGS=1")
    endif()
endif()

add_library(FreeImage SHARED ${freeimage_all_sources})

target_compile_definitions(FreeImage PUBLIC "-DFREEIMAGE_COLORORDER=1")
if (FREEIMAGE_WITH_PYTHON_BINDINGS)
    target_include_directories(FreeImage PRIVATE ${PYBIND_INCLUDE_DIR} ${Python3_INCLUDE_DIRS})
    target_compile_definitions(FreeImage PRIVATE "-DFREEIMAGE_WITH_PYTHON_BINDINGS=1")
    target_link_directories(FreeImage PRIVATE ${Python3_LIBRARY_DIRS})
endif()

target_link_libraries(FreeImage PRIVATE
    ${ZLIB_LIBRARY}
    ${LIBJPEG_LIBRARY}
    ${EXR_LIBRARY}
    ${OPENJPEG_LIBRARY}
    ${PNG_LIBRARY}
    ${TIFF_LIBRARY}
    ${WEBP_LIBRARY}
    ${RAW_LIBRARY}
    LibJXR
)

target_include_directories(FreeImage PRIVATE
    ${LIBJPEG_INCLUDE_DIR}
    ${OPENJPEG_INCLUDE_DIR}
    ${PNG_INCLUDE_DIR}
    ${TIFF_INCLUDE_DIR}
    ${WEBP_INCLUDE_DIR}
    ${RAW_INCLUDE_DIR}
)


if (FREEIMAGE_WITH_PYTHON_BINDINGS)
    if (WIN32)
        add_custom_command(TARGET FreeImage POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E create_hardlink ".\\$<TARGET_FILE_NAME:FreeImage>" "FreeImage.pyd"
            WORKING_DIRECTORY $<TARGET_FILE_DIR:FreeImage>
        )
    else()
        add_custom_command(TARGET FreeImage POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E create_symlink "./$<TARGET_FILE_NAME:FreeImage>" "FreeImage.so"
            WORKING_DIRECTORY $<TARGET_FILE_DIR:FreeImage>
        )
    endif()
endif()
