include(${EXTERNALPROJECT_INCLUDE_DIR}/dependency.nanobind.cmake)


# ===========================
# Bindings module

nanobind_add_module(FreeImagePy STABLE_ABI FreeImagePy.cpp)

set_target_properties(FreeImagePy PROPERTIES OUTPUT_NAME "_freeimage")
target_link_libraries(FreeImagePy PRIVATE FreeImage)

if (TARGET nanobind-static-abi3)
    set_property(TARGET nanobind-static-abi3 PROPERTY FOLDER "Dependencies")
endif()

if(UNIX AND NOT APPLE)
    set_target_properties(FreeImagePy PROPERTIES INSTALL_RPATH "\$ORIGIN")
endif()

if (WIN32)
    set(FREEIMAGE_INSTALL_DIR bin)
else()
    set(FREEIMAGE_INSTALL_DIR lib)
endif()

set(FREEIMAGERE_BUILD_METADATA [=[
{
  "version": "@FREEIMAGERE_MAJOR_VERSION@.@FREEIMAGERE_MINOR_VERSION@.@FREEIMAGERE_PATCH_VERSION@",
  "version_major": @FREEIMAGERE_MAJOR_VERSION@,
  "version_minor": @FREEIMAGERE_MINOR_VERSION@,
  "version_patch": @FREEIMAGERE_PATCH_VERSION@,
  "targets": {
    "freeimage": {
      "file": "$<TARGET_FILE_NAME:FreeImage>",
      "install_path": "@CMAKE_INSTALL_PREFIX@/@FREEIMAGE_INSTALL_DIR@/$<TARGET_FILE_NAME:FreeImage>"
    },
    "python": {
      "file": "$<TARGET_FILE_NAME:FreeImagePy>",
      "install_path": "@CMAKE_INSTALL_PREFIX@/python/$<TARGET_FILE_NAME:FreeImagePy>"
    }
  }
}
]=])
string(CONFIGURE "${FREEIMAGERE_BUILD_METADATA}" FREEIMAGERE_BUILD_METADATA @ONLY)

file(GENERATE
    OUTPUT "${CMAKE_BINARY_DIR}/freeimagere-build.json"
    CONTENT "${FREEIMAGERE_BUILD_METADATA}"
)

install(TARGETS FreeImagePy
    LIBRARY DESTINATION python
    RUNTIME DESTINATION python
    COMPONENT python
)

# ========================
# Stubs

nanobind_add_stub(FreeImagePyStubs
  MODULE _freeimage
  OUTPUT "${CMAKE_BINARY_DIR}/_freeimage.pyi"
  PYTHON_PATH $<TARGET_FILE_DIR:FreeImagePy>
  DEPENDS FreeImagePy
)

install(FILES "${CMAKE_BINARY_DIR}/_freeimage.pyi"
    DESTINATION python
    COMPONENT python
)
