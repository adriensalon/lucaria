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

set(LUCARIA_BUILD_COMPILERS ON)
add_subdirectory("source/compiler")

function(_lucaria_read_json_platform_bool OUT_VAR PLATFORM FIELD DEFAULT_VALUE)
    string(JSON value_type ERROR_VARIABLE value_type_error
        TYPE "${LUCARIA_GAME_JSON}" platform ${PLATFORM} ${FIELD})
    if(value_type_error)
        set(${OUT_VAR} "${DEFAULT_VALUE}" PARENT_SCOPE)
        return()
    endif()
    if(NOT value_type STREQUAL "BOOLEAN")
        message(FATAL_ERROR "build.json field platform.${PLATFORM}.${FIELD} must be a boolean")
    endif()
    string(JSON value ERROR_VARIABLE value_error
        GET "${LUCARIA_GAME_JSON}" platform ${PLATFORM} ${FIELD})
    if(value_error)
        message(FATAL_ERROR
            "Could not read build.json field platform.${PLATFORM}.${FIELD}: ${value_error}")
    endif()
    set(${OUT_VAR} "${value}" PARENT_SCOPE)
endfunction()

function(_lucaria_make_configure)
    set(options EXCLUDE_FROM_ALL)
    set(one_value_args PLATFORM GAME_DIR GAME_BINARY_DIR)
    set(multi_value_args EXTRA_CMAKE_ARGS)
    cmake_parse_arguments(PARSE_ARGV 0 LCP "${options}" "${one_value_args}" "${multi_value_args}")
    if(NOT LCP_PLATFORM)
        message(FATAL_ERROR "lucaria_make_configure requires PLATFORM")
    endif()
    if(NOT LCP_GAME_DIR)
        message(FATAL_ERROR "lucaria_make_configure requires GAME_DIR")
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
        file(READ "${LCP_GAME_DIR}/build.json" LUCARIA_GAME_JSON)
        _lucaria_read_json_platform_bool(_psp_encrypt_prx psp encrypt ON)
        _lucaria_read_json_platform_bool(_psp_build_prx psp prx "${_psp_encrypt_prx}")
        if(_psp_encrypt_prx AND NOT _psp_build_prx)
            message(FATAL_ERROR "build.json field platform.psp.encrypt requires platform.psp.prx to be true")
        endif()
		list(APPEND _platform_args
			"-DBUILD_PRX=${_psp_build_prx}"
			"-DENC_PRX=${_psp_encrypt_prx}")
    else()
        message(FATAL_ERROR "Unknown Lucaria platform: ${LCP_PLATFORM}")
    endif()
    set(_all_arg)
    if(NOT LCP_EXCLUDE_FROM_ALL)
        set(_all_arg ALL)
    endif()
    add_custom_target(${_superbuild_target}
        ${_all_arg}
        COMMAND "${_cmake_command}"
            -Wno-dev
            -S "${LUCARIA_SOURCE_DIR}/game"
            -B "${LCP_GAME_BINARY_DIR}"
            -G "${_generator}"
            ${_toolchain_args}
            "-DLUCARIA_SOURCE_DIR=${LUCARIA_SOURCE_DIR}"
            "-DLUCARIA_CMAKE_DIR=${LUCARIA_CMAKE_DIR}"
            "-DLUCARIA_EXTERNAL_DIR=${LUCARIA_EXTERNAL_DIR}"
            "-DLUCARIA_INCLUDE_DIR=${LUCARIA_INCLUDE_DIR}"
            "-DLUCARIA_BINARY_DIR=${LCP_GAME_BINARY_DIR}"

            "-DLUCARIA_GAME_DIR=${LCP_GAME_DIR}"
            "-DLUCARIA_GAME_DEBUG=$<IF:$<CONFIG:Debug>,ON,OFF>"

            ${_platform_args}
            ${LCP_EXTRA_CMAKE_ARGS}
        COMMAND "${CMAKE_COMMAND}"
            --build "${LCP_GAME_BINARY_DIR}"
            --config $<CONFIG>
        WORKING_DIRECTORY "${LCP_GAME_DIR}"
        COMMENT "Building Lucaria game for ${LCP_PLATFORM}"
        VERBATIM
        USES_TERMINAL
		JOB_SERVER_AWARE TRUE)
endfunction()

# entry point
function(lucaria_superbuild GAME_DIR)
    get_filename_component(GAME_DIR "${GAME_DIR}" ABSOLUTE)
    set(options)
    set(one_value_args BINARY_DIR)
    set(multi_value_args PLATFORMS)
    cmake_parse_arguments(PARSE_ARGV 1 LSB "${options}" "${one_value_args}" "${multi_value_args}")
    if(NOT EXISTS "${GAME_DIR}/build.json")
        message(FATAL_ERROR "Missing Lucaria game config: ${GAME_DIR}/build.json")
    endif()
    if(NOT LSB_BINARY_DIR)
        set(LSB_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/lucaria")
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
        _lucaria_make_configure(
            PLATFORM "${platform}"
            GAME_DIR "${GAME_DIR}"
            GAME_BINARY_DIR "${LSB_BINARY_DIR}/${platform}")
    endforeach()
endfunction()
