cmake_minimum_required(VERSION 3.19)
project(lucaria)

set(LUCARIA_EXTERNAL_DIR "${CMAKE_CURRENT_LIST_DIR}/external" CACHE PATH "Lucaria external directory")
set(LUCARIA_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/include" CACHE PATH "Lucaria include directory")
set(LUCARIA_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/source" CACHE PATH "Lucaria source directory")
set(LUCARIA_CMAKE_DIR "${CMAKE_CURRENT_LIST_DIR}/cmake" CACHE PATH "Lucaria cmake directory")
set(LUCARIA_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}" CACHE PATH "Lucaria binary directory")

include("${LUCARIA_CMAKE_DIR}/xplatform_android.cmake")
include("${LUCARIA_CMAKE_DIR}/xplatform_psp.cmake")
include("${LUCARIA_CMAKE_DIR}/xplatform_web.cmake")

set(LUCARIA_BINC_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/tool/binc" CACHE INTERNAL "Lucaria asset compiler source directory" FORCE)
set(LUCARIA_BINC_BINARY_DIR "${LUCARIA_BINARY_DIR}/tool/binc" CACHE INTERNAL "Lucaria asset compiler binary directory" FORCE)
set(_lucaria_binc_runtime_dir "${LUCARIA_BINC_BINARY_DIR}/bin")
if(CMAKE_HOST_WIN32)
    set(_lucaria_binc_executable "${_lucaria_binc_runtime_dir}/lucaria_binc.exe")
else()
    set(_lucaria_binc_executable "${_lucaria_binc_runtime_dir}/lucaria_binc")
endif()

add_executable(lucaria_binc IMPORTED GLOBAL)
set_target_properties(lucaria_binc PROPERTIES
    IMPORTED_LOCATION "${_lucaria_binc_executable}")
# add_subdirectory("tool/tracyp")

function(_lucaria_make_game_path OUT_VAR GAME_DIR VALUE)
    if(IS_ABSOLUTE "${VALUE}")
        set(${OUT_VAR} "${VALUE}" PARENT_SCOPE)
    else()
        set(${OUT_VAR} "${GAME_DIR}/${VALUE}" PARENT_SCOPE)
    endif()
endfunction()

function(_lucaria_make_configure)
    set(options EXCLUDE_FROM_ALL)
    set(one_value_args
        PLATFORM
        GAME_DIR
        GAME_BINARY_DIR
        NAME
        INSTALL_DIR
        PACKAGE
        BACKEND
        PSP_PRX
        PSP_ENCRYPT)
    set(multi_value_args
        SOURCES
        INCLUDE_DIRS
        COMPILE_DEFINITIONS
        COMPILE_OPTIONS
        DEPENDS
        EXTRA_CMAKE_ARGS)
    cmake_parse_arguments(PARSE_ARGV 0 LCP "${options}" "${one_value_args}" "${multi_value_args}")
    if(NOT LCP_PLATFORM)
        message(FATAL_ERROR "lucaria_make_configure requires PLATFORM")
    endif()
    if(NOT LCP_GAME_DIR)
        message(FATAL_ERROR "lucaria_make_configure requires GAME_DIR")
    endif()
    if(NOT LCP_NAME)
        message(FATAL_ERROR "lucaria_make_configure requires NAME")
    endif()
    if(NOT LCP_INSTALL_DIR)
        message(FATAL_ERROR "lucaria_make_configure requires INSTALL_DIR")
    endif()
    if(NOT LCP_SOURCES)
        message(FATAL_ERROR "lucaria_make_configure requires SOURCES")
    endif()
    if(NOT LCP_GAME_BINARY_DIR)
        set(LCP_GAME_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/lucaria/${LCP_PLATFORM}")
    endif()
    get_filename_component(_game_dir_name "${LCP_GAME_DIR}" NAME)
    set(_superbuild_target "${_game_dir_name}_${LCP_PLATFORM}")
    file(MAKE_DIRECTORY "${LCP_GAME_BINARY_DIR}")
    set(_cmake_command "${CMAKE_COMMAND}")
    set(_generator "${CMAKE_GENERATOR}")
    set(_toolchain_args)
    set(_platform_args)
    if(LCP_PLATFORM STREQUAL "win32")
        if(NOT WIN32)
            message(STATUS "Skipping Lucaria Win32 build: host is not Win32")
            return()
        endif()
    elseif(LCP_PLATFORM STREQUAL "android")
        if(NOT lucaria_xplatform_android_toolchain)
            message(STATUS "Skipping Lucaria Android build: Android NDK not found")
            return()
        endif()
        set(_generator "Ninja")
        list(APPEND _toolchain_args
            "-DCMAKE_TOOLCHAIN_FILE=${lucaria_xplatform_android_toolchain}"
            "-DANDROID_ABI=arm64-v8a"
            "-DANDROID_PLATFORM=android-24")
    elseif(LCP_PLATFORM STREQUAL "web")
        if(NOT lucaria_xplatform_web_toolchain)
            message(STATUS "Skipping Lucaria Web build: Emscripten not found")
            return()
        endif()
        set(_generator "Ninja")
        list(APPEND _toolchain_args
            "-DCMAKE_TOOLCHAIN_FILE=${lucaria_xplatform_web_toolchain}")
    elseif(LCP_PLATFORM STREQUAL "psp")
        if(NOT lucaria_xplatform_psp_cmake_executable)
            message(STATUS "Skipping Lucaria PSP build: psp-cmake not found")
            return()
        endif()
        set(_cmake_command "${lucaria_xplatform_psp_cmake_executable}")
        set(_psp_encrypt_prx "${LCP_PSP_ENCRYPT}")
        set(_psp_build_prx "${LCP_PSP_PRX}")
        if(_psp_encrypt_prx AND NOT _psp_build_prx)
            message(FATAL_ERROR "PSP_ENCRYPT requires PSP_PRX to be true")
        endif()
		list(APPEND _platform_args
			"-DBUILD_PRX=${_psp_build_prx}"
			"-DENC_PRX=${_psp_encrypt_prx}")
    else()
        message(FATAL_ERROR "Unknown Lucaria platform: ${LCP_PLATFORM}")
    endif()
    if(LCP_BACKEND)
        list(APPEND _platform_args "-DLUCARIA_BACKEND=${LCP_BACKEND}")
    endif()
    set(_all_arg)
    if(NOT LCP_EXCLUDE_FROM_ALL)
        set(_all_arg ALL)
    endif()
    add_custom_target(${_superbuild_target}
        ${_all_arg}
        COMMAND "${_cmake_command}"
            -Wno-dev
            -S "${LUCARIA_SOURCE_DIR}"
            -B "${LCP_GAME_BINARY_DIR}"
            -G "${_generator}"
            ${_toolchain_args}
            "-DLUCARIA_SOURCE_DIR=${LUCARIA_SOURCE_DIR}"
            "-DLUCARIA_CMAKE_DIR=${LUCARIA_CMAKE_DIR}"
            "-DLUCARIA_EXTERNAL_DIR=${LUCARIA_EXTERNAL_DIR}"
            "-DLUCARIA_INCLUDE_DIR=${LUCARIA_INCLUDE_DIR}"
            "-DLUCARIA_BINARY_DIR=${LCP_GAME_BINARY_DIR}"

            "-DLUCARIA_GAME_DIR=${LCP_GAME_DIR}"
            "-DLUCARIA_DEBUG=$<IF:$<CONFIG:Debug>,ON,OFF>"
            "-DLUCARIA_TARGET=${LCP_NAME}"
            "-DLUCARIA_INSTALL_DIR=${LCP_INSTALL_DIR}"
            "-DLUCARIA_PACKAGE=${LCP_PACKAGE}"
            "-DLUCARIA_TARGET_SOURCE_PATTERNS=${LCP_SOURCES}"
            "-DLUCARIA_TARGET_INCLUDE_DIRS=${LCP_INCLUDE_DIRS}"
            "-DLUCARIA_TARGET_COMPILE_DEFINITIONS=${LCP_COMPILE_DEFINITIONS}"
            "-DLUCARIA_TARGET_COMPILE_OPTIONS=${LCP_COMPILE_OPTIONS}"

            ${_platform_args}
            ${LCP_EXTRA_CMAKE_ARGS}
        COMMAND "${CMAKE_COMMAND}"
            --build "${LCP_GAME_BINARY_DIR}"
            --config $<CONFIG>
        DEPENDS ${LCP_DEPENDS}
        WORKING_DIRECTORY "${LCP_GAME_DIR}"
        COMMENT "Building Lucaria game for ${LCP_PLATFORM}"
        VERBATIM
        USES_TERMINAL
		JOB_SERVER_AWARE TRUE)
endfunction()

# entry point
function(add_game)
    set(options)
    set(one_value_args
        NAME
        GAME_DIR
        BINARY_DIR
        INSTALL_DIR
        ASSETS_DIR
        PACKAGE
        WIN32_BACKEND
        PSP_PRX
        PSP_ENCRYPT)
    set(multi_value_args
        PLATFORMS
        SOURCES
        INCLUDE_DIRS
        COMPILE_DEFINITIONS
        COMPILE_OPTIONS
        EXTRA_CMAKE_ARGS)
    cmake_parse_arguments(PARSE_ARGV 0 LSB "${options}" "${one_value_args}" "${multi_value_args}")
    if(NOT LSB_NAME)
        message(FATAL_ERROR "add_game requires NAME")
    endif()
    if(NOT LSB_GAME_DIR)
        set(LSB_GAME_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
    endif()
    get_filename_component(LSB_GAME_DIR "${LSB_GAME_DIR}" ABSOLUTE)
    if(NOT LSB_INSTALL_DIR)
        message(FATAL_ERROR "add_game requires INSTALL_DIR")
    endif()
    _lucaria_make_game_path(LSB_INSTALL_DIR "${LSB_GAME_DIR}" "${LSB_INSTALL_DIR}")
    if(LSB_ASSETS_DIR)
        _lucaria_make_game_path(LSB_ASSETS_DIR "${LSB_GAME_DIR}" "${LSB_ASSETS_DIR}")
    endif()
    if(NOT LSB_SOURCES)
        message(FATAL_ERROR "add_game requires SOURCES")
    endif()
    if("${LSB_PACKAGE}" STREQUAL "")
        set(LSB_PACKAGE OFF)
    endif()
    if("${LSB_PSP_ENCRYPT}" STREQUAL "")
        set(LSB_PSP_ENCRYPT ON)
    endif()
    if("${LSB_PSP_PRX}" STREQUAL "")
        set(LSB_PSP_PRX "${LSB_PSP_ENCRYPT}")
    endif()
    if(NOT LSB_BINARY_DIR)
        set(LSB_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/lucaria")
    endif()
    set(_asset_inputs)
    if(LSB_ASSETS_DIR)
        file(GLOB_RECURSE _asset_inputs CONFIGURE_DEPENDS LIST_DIRECTORIES false "${LSB_ASSETS_DIR}/*")
        if(EXISTS "${LSB_ASSETS_DIR}/.assetignore")
            list(APPEND _asset_inputs "${LSB_ASSETS_DIR}/.assetignore")
        endif()
        set(_asset_inputs_file "${LSB_BINARY_DIR}/assets/lucaria_asset_inputs.txt")
        set(_asset_inputs_content)
        foreach(_asset_input IN LISTS _asset_inputs)
            string(APPEND _asset_inputs_content "${_asset_input}\n")
        endforeach()
        if(EXISTS "${_asset_inputs_file}")
            file(READ "${_asset_inputs_file}" _asset_inputs_existing)
        else()
            set(_asset_inputs_existing)
        endif()
        if(NOT _asset_inputs_existing STREQUAL _asset_inputs_content)
            file(MAKE_DIRECTORY "${LSB_BINARY_DIR}/assets")
            file(WRITE "${_asset_inputs_file}" "${_asset_inputs_content}")
        endif()
        list(APPEND _asset_inputs "${_asset_inputs_file}")
    endif()
    _lucaria_find_xplatform_android()
    _lucaria_find_xplatform_psp()
    _lucaria_find_xplatform_web()
    if(NOT LSB_PLATFORMS)
        set(LSB_PLATFORMS)
        if(WIN32)
            list(APPEND LSB_PLATFORMS win32)
        endif()
        if(lucaria_xplatform_android_toolchain)
            list(APPEND LSB_PLATFORMS android)
        endif()
        if(lucaria_xplatform_web_toolchain)
            list(APPEND LSB_PLATFORMS web)
        endif()
        if(lucaria_xplatform_psp_cmake_executable)
            list(APPEND LSB_PLATFORMS psp)
        endif()
    endif()
    foreach(platform IN LISTS LSB_PLATFORMS)
        set(_platform_install_dir "${LSB_INSTALL_DIR}/${platform}")
        set(_platform_assets_dir "${_platform_install_dir}/assets")
        set(_platform_depends)
        if(LSB_ASSETS_DIR)
            set(_assets_binary_dir "${LSB_BINARY_DIR}/${platform}/assets")
            set(_assets_stamp "${_assets_binary_dir}/lucaria_assets.stamp")
            set(_assets_manifest "${_assets_binary_dir}/lucaria_assets.json")
            set(_lucaria_binc_generator_args -G "${CMAKE_GENERATOR}")
            if(CMAKE_GENERATOR_PLATFORM)
                list(APPEND _lucaria_binc_generator_args -A "${CMAKE_GENERATOR_PLATFORM}")
            endif()
            if(CMAKE_GENERATOR_TOOLSET)
                list(APPEND _lucaria_binc_generator_args -T "${CMAKE_GENERATOR_TOOLSET}")
            endif()
            add_custom_command(
                OUTPUT "${_assets_stamp}"
                COMMAND ${CMAKE_COMMAND} -E make_directory "${_platform_assets_dir}"
                COMMAND ${CMAKE_COMMAND} -E make_directory "${_assets_binary_dir}"
                COMMAND "${CMAKE_COMMAND}"
                    -Wno-dev
                    -S "${LUCARIA_BINC_SOURCE_DIR}"
                    -B "${LUCARIA_BINC_BINARY_DIR}"
                    ${_lucaria_binc_generator_args}
                    "-DCMAKE_BUILD_TYPE=Release"
                    "-DLUCARIA_EXTERNAL_DIR=${LUCARIA_EXTERNAL_DIR}"
                    "-DLUCARIA_INCLUDE_DIR=${LUCARIA_INCLUDE_DIR}"
                    "-DLUCARIA_BINARY_DIR=${LUCARIA_BINC_BINARY_DIR}"
                COMMAND "${CMAKE_COMMAND}"
                    --build "${LUCARIA_BINC_BINARY_DIR}"
                    --config Release
                    --target lucaria_binc
                COMMAND $<TARGET_FILE:lucaria_binc>
                    -p "${platform}"
                    -i "${LSB_ASSETS_DIR}"
                    -o "${_platform_assets_dir}"
                    -m "${_assets_manifest}"
                COMMAND ${CMAKE_COMMAND} -E touch "${_assets_stamp}"
                DEPENDS ${_asset_inputs}
                BYPRODUCTS "${_assets_manifest}"
                WORKING_DIRECTORY "${LSB_GAME_DIR}"
                COMMENT "Compiling Lucaria assets for ${platform}"
                VERBATIM
                USES_TERMINAL
				JOB_SERVER_AWARE TRUE)
            list(APPEND _platform_depends "${_assets_stamp}")
        endif()
        set(_backend_args)
        if(platform STREQUAL "win32" AND LSB_WIN32_BACKEND)
            list(APPEND _backend_args BACKEND "${LSB_WIN32_BACKEND}")
        endif()
        _lucaria_make_configure(
            PLATFORM "${platform}"
            GAME_DIR "${LSB_GAME_DIR}"
            GAME_BINARY_DIR "${LSB_BINARY_DIR}/${platform}"
            NAME "${LSB_NAME}"
            INSTALL_DIR "${_platform_install_dir}"
            PACKAGE "${LSB_PACKAGE}"
            ${_backend_args}
            PSP_PRX "${LSB_PSP_PRX}"
            PSP_ENCRYPT "${LSB_PSP_ENCRYPT}"
            SOURCES ${LSB_SOURCES}
            INCLUDE_DIRS ${LSB_INCLUDE_DIRS}
            COMPILE_DEFINITIONS ${LSB_COMPILE_DEFINITIONS}
            COMPILE_OPTIONS ${LSB_COMPILE_OPTIONS}
            DEPENDS ${_platform_depends}
            EXTRA_CMAKE_ARGS ${LSB_EXTRA_CMAKE_ARGS})
    endforeach()
endfunction()
