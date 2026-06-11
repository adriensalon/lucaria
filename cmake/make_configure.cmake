include_guard(GLOBAL)

function(lucaria_make_configure)
    set(options)
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
        list(APPEND _platform_args
            "-DBUILD_PRX=1"
            "-DENC_PRX=1")
    else()
        message(FATAL_ERROR "Unknown Lucaria platform: ${LCP_PLATFORM}")
    endif()
    add_custom_target(${_superbuild_target}
        ALL
        COMMAND "${_cmake_command}"
            -Wno-dev
            -S "${LUCARIA_SOURCE_DIR}/game"
            -B "${LCP_GAME_BINARY_DIR}"
            -G "${_generator}"
            ${_toolchain_args}
            "-DLUCARIA_GAME_DIR=${LCP_GAME_DIR}"
            "-DLUCARIA_TARGET_PLATFORM=${LCP_PLATFORM}"
            "-DLUCARIA_SOURCE_DIR=${LUCARIA_SOURCE_DIR}"
            "-DLUCARIA_CMAKE_DIR=${LUCARIA_CMAKE_DIR}"
            "-DLUCARIA_EXTERNAL_DIR=${LUCARIA_EXTERNAL_DIR}"
            "-DLUCARIA_INCLUDE_DIR=${LUCARIA_INCLUDE_DIR}"
            "-DLUCARIA_BINARY_DIR=${LCP_GAME_BINARY_DIR}"
            "-DLUCARIA_DEBUG=$<IF:$<CONFIG:Debug>,ON,OFF>"
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