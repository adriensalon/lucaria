# build_game_android.cmake


function(add_lucaria_game_android TARGET)
    set(options)
    set(one_value_args INSTALL_DIR ASSETS_DIR ANDROID_ABI ANDROID_PLATFORM)
    set(multi_value_args SOURCES INCLUDES DEFINES BUILD_ARGS)
    cmake_parse_arguments(PARSE_ARGV 0 LBG "${options}" "${one_value_args}" "${multi_value_args}")

    # find NDK
    set(ANDROID_NDK_CANDIDATES)
    if(DEFINED ENV{ANDROID_NDK_ROOT})
        list(APPEND ANDROID_NDK_CANDIDATES "$ENV{ANDROID_NDK_ROOT}")
    endif()
    if(DEFINED ENV{ANDROID_NDK_HOME})
        list(APPEND ANDROID_NDK_CANDIDATES "$ENV{ANDROID_NDK_HOME}")
    endif()
    if(DEFINED ENV{ANDROID_NDK})
        list(APPEND ANDROID_NDK_CANDIDATES "$ENV{ANDROID_NDK}")
    endif()
    if(DEFINED ENV{ANDROID_SDK_ROOT})
        file(GLOB _ndk_dirs "$ENV{ANDROID_SDK_ROOT}/ndk/*")
        list(APPEND ANDROID_NDK_CANDIDATES ${_ndk_dirs})
    endif()
    set(ANDROID_NDK_FOUND FALSE)
    foreach(cand IN LISTS ANDROID_NDK_CANDIDATES)
        if(EXISTS "${cand}/build/cmake/android.toolchain.cmake")
            set(ANDROID_NDK "${cand}")
            set(ANDROID_NDK_FOUND TRUE)
            break()
        endif()
    endforeach()
    if(NOT ANDROID_NDK_FOUND)
        find_program(ANDROID_NDK_BUILD_EXECUTABLE ndk-build)
        if(ANDROID_NDK_BUILD_EXECUTABLE)
            get_filename_component(_ndk_dir "${ANDROID_NDK_BUILD_EXECUTABLE}" DIRECTORY)
            get_filename_component(_ndk_root "${_ndk_dir}/.." ABSOLUTE)
            if(EXISTS "${_ndk_root}/build/cmake/android.toolchain.cmake")
                set(ANDROID_NDK "${_ndk_root}")
                set(ANDROID_NDK_FOUND TRUE)
            endif()
        endif()
    endif()
    if(NOT ANDROID_NDK_FOUND)
        message(WARNING "Android NDK not found. lucaria_android will not be built")
        return()
    endif()
    set(ANDROID_TOOLCHAIN "${ANDROID_NDK}/build/cmake/android.toolchain.cmake")
    message(STATUS "Found Android NDK at ${ANDROID_NDK}")
    message(STATUS "Found Android toolchain file at ${ANDROID_TOOLCHAIN}")

    # select gradlew
    if(WIN32)
        set(ANDROID_GRADLEW "gradlew.bat")
    else()
        set(ANDROID_GRADLEW "gradlew")
    endif()

    # default parameters
    if(NOT LBG_ASSETS_DIR)
        set(ANDROID_ASSETS_DIR_ARG "")
        message(STATUS "No assets provided for Android APK package")
    else()        
        set(ANDROID_ASSETS_DIR_ARG "-PassetsDir=${LBG_ASSETS_DIR}")
    endif()
    if(NOT LBG_ANDROID_ABI)
        set(LBG_ANDROID_ABI "arm64-v8a")
    endif()
    if(NOT LBG_ANDROID_PLATFORM)
        set(LBG_ANDROID_PLATFORM "android-24")
    endif()
    
    set(GAME_ANDROID_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}/android")
    set(GAME_ANDROID_SO "${GAME_ANDROID_OUTPUT_DIR}/lib${TARGET}.so")
    set(ANDROID_JNILIBS_DIR "${LUCARIA_SOURCE_DIR}/game/android/src/main/jniLibs/${LBG_ANDROID_ABI}")
    set(ANDROID_JNILIBS_SO "${ANDROID_JNILIBS_DIR}/lib${TARGET}.so")
    set(ANDROID_APK "${TARGET}-debug.apk")
    set(ANDROID_APK_DIR "${LUCARIA_SOURCE_DIR}/game/android/build/outputs/apk/debug")

    lucaria_build_game(
        "android"
        "Ninja"
        "${ANDROID_TOOLCHAIN}"
        "${TARGET}"
        "${LBG_SOURCES}"
        "${LBG_INSTALL_DIR}"
        "${LBG_INCLUDES}"
        "${LBG_DEFINES}"
        "${LBG_BUILD_ARGS}"
        "-DANDROID_ABI=${LBG_ANDROID_ABI};-DANDROID_PLATFORM=${LBG_ANDROID_PLATFORM}")

    add_custom_command(
        TARGET ${TARGET}_android
        POST_BUILD

        # copy built shared library
        COMMAND ${CMAKE_COMMAND} -E make_directory ${ANDROID_JNILIBS_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy ${GAME_ANDROID_SO} ${ANDROID_JNILIBS_SO}

        # pack with gradle
        COMMAND ${CMAKE_COMMAND} -E chdir ${LUCARIA_SOURCE_DIR}/game/android ${ANDROID_GRADLEW} assembleDebug ${ANDROID_ASSETS_DIR_ARG} -PtargetName=${TARGET}

        # copy to build directory
        COMMAND ${CMAKE_COMMAND} -E copy ${ANDROID_APK_DIR}/${ANDROID_APK} ${GAME_ANDROID_OUTPUT_DIR}/${ANDROID_APK}
        USES_TERMINAL)
    
    if(LBG_INSTALL_DIR)
        add_custom_command(
            TARGET ${TARGET}_android
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${GAME_ANDROID_OUTPUT_DIR}/${ANDROID_APK} ${LBG_INSTALL_DIR}/${ANDROID_APK})
    endif()

endfunction()