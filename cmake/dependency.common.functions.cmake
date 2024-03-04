#
# YATO library
#
# Apache License, Version 2.0
# Copyright 2016-2020 Alexey Gruzdev
#

# Check that directory exists and is not empty
function(dependency_folder_not_empty PATH_ RESULT_)
    set(res_ FALSE)
    if(EXISTS ${PATH_})
        file(GLOB files_ "${PATH_}/*")
        if(files_)
            set(res_ TRUE)
        endif()
        unset(files_)
    endif()
    set(${RESULT_} ${res_} PARENT_SCOPE)
endfunction()

# Check that file exists and has valid hash
function(file_exists STATUS_ FILE_ HASH_MD5_)
    set(${STATUS_} FALSE PARENT_SCOPE)
    if(EXISTS ${FILE_})
        if(NOT HASH_MD5_)
            set(${STATUS_} TRUE PARENT_SCOPE)
            return()
        endif()
        file(MD5 ${FILE_} hash_)
        if(${hash_} STREQUAL ${HASH_MD5_})
            set(${STATUS_} TRUE PARENT_SCOPE)
        endif()
    endif()
endfunction()


function(dependency_download_and_unzip URL_ HASH_MD5_ FILE_ UNZIPPED_DIR_ ONLY_CHECK_ RET_CODE_)
    get_filename_component(workdir_ ${FILE_} DIRECTORY)
    set(unzipped_directory ${workdir_}/unzipped)

    dependency_folder_not_empty(${unzipped_directory} unzip_exists_)
    if(NOT ${unzip_exists_})
        file_exists(archive_is_valid_ ${FILE_} ${HASH_MD5_})
        if(NOT ${archive_is_valid_})
            if(ONLY_CHECK_)
                set(${RET_CODE_} FALSE PARENT_SCOPE)
                return()
            endif()

            message(STATUS "Downloading: ${URL_}")
            if (NOT HASH_MD5_)
                file(DOWNLOAD ${URL_} ${FILE_}
                    SHOW_PROGRESS
                    STATUS ret_
                )
            else()
                file(DOWNLOAD ${URL_} ${FILE_}
                    SHOW_PROGRESS
                    EXPECTED_MD5 ${HASH_MD5_}
                    STATUS ret_
                )
            endif()
            if(NOT ret_)
                message(SEND_ERROR "Failed to download: ${URL_}")
                set(${RET_CODE_} FALSE PARENT_SCOPE)
                return()
            endif()
        endif()

        message(STATUS "Unzipping: ${FILE_}")
        file(MAKE_DIRECTORY ${unzipped_directory})
        execute_process(COMMAND "${CMAKE_COMMAND}" -E tar -xzf ${FILE_}
            WORKING_DIRECTORY ${unzipped_directory}
            RESULT_VARIABLE ret_
        )
        if(ret_)
            message(SEND_ERROR "Failed to extract: ${FILE_}")
            set(${RET_CODE_} FALSE PARENT_SCOPE)
            return()
        endif()
    endif()

    set(${UNZIPPED_DIR_} ${unzipped_directory} PARENT_SCOPE)
    set(${RET_CODE_} TRUE PARENT_SCOPE)
endfunction()




# Defines ${DEPENDENCY_NAME}_FOUND_ROOT on success
# Use global flag DOWNLOAD_ALL to enable downloading by default
function(dependency_find_or_download)
    set(oneValueArgs
        NAME            # Short dependency name used for cache variables
        VERBOSE_NAME    # Verbose name for messages and tooltips
        URL             # Url for downloading.
        HASH_MD5        # Hash of downloaded file
        FILE_NAME       # [Optional] specify downloaded file name
        PREFIX          # [Optional] Additonal path prefix after extracting archive
    )
    cmake_parse_arguments(DEPENDENCY "" "${oneValueArgs}" "" ${ARGN})

    if(DEPENDENCY_FILE_NAME)
        set(file_name_ ${DEPENDENCY_FILE_NAME})
    else()
        set(file_name_ "package.zip")
    endif()
    
    string(TOLOWER ${DEPENDENCY_NAME} folder_name_)
    set(dependency_file_ "${CMAKE_SOURCE_DIR}/dependencies/${folder_name_}/${file_name_}")

    if(NOT DEFINED DOWNLOAD_ALL)
        set(DOWNLOAD_ALL FALSE)
    endif()

    # 1. check locally
    dependency_download_and_unzip(${DEPENDENCY_URL} ${DEPENDENCY_HASH_MD5} ${dependency_file_} unzipped_location_ TRUE dependency_is_found_)
    if(${dependency_is_found_})
        set(root_ ${unzipped_location_})
        if(DEPENDENCY_PREFIX)
            set(root_ ${root_}/${DEPENDENCY_PREFIX})
        endif()
    endif()

    # 2. check environment
    if(NOT EXISTS ${root_})
        if(${DEPENDENCY_NAME}_ROOT)
            set(root_ ${${DEPENDENCY_NAME}_ROOT})
        else()
            set(root_ $ENV{${DEPENDENCY_NAME}_ROOT})
        endif()
    endif()

    # 3. download
    option(${DEPENDENCY_NAME}_DOWNLOAD "Download ${DEPENDENCY_VERBOSE_NAME} sources" OFF)
    if((NOT EXISTS ${root_}) OR ${DOWNLOAD_ALL})
        if(${${DEPENDENCY_NAME}_DOWNLOAD} OR ${DOWNLOAD_ALL})
            dependency_download_and_unzip(${DEPENDENCY_URL} ${DEPENDENCY_HASH_MD5} ${dependency_file_} unzipped_location_ FALSE dependency_is_downloaded_)
            if(${dependency_is_downloaded_})
                set(root_ ${unzipped_location_})
                if(DEPENDENCY_PREFIX)
                    set(root_ ${root_}/${DEPENDENCY_PREFIX})
                endif()
            endif()
        endif()
    endif()

    if(NOT EXISTS ${root_})
        message(FATAL_ERROR "${DEPENDENCY_VERBOSE_NAME} is not found! Please set ${DEPENDENCY_NAME}_ROOT variable to source directory or enable flag ${DEPENDENCY_NAME}_DOWNLOAD for automatic downloading of the dependency.")
    endif()

    set(${DEPENDENCY_NAME}_FOUND_ROOT ${root_} CACHE PATH "Found ${DEPENDENCY_VERBOSE_NAME} root directory. Read-only variable. Set ${DEPENDENCY_NAME}_ROOT to change the root directory." FORCE)
endfunction()

