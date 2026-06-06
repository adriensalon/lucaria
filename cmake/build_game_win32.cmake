# build_game_win32.cmake

function(add_lucaria_game_win32 TARGET)
    set(options)
    set(one_value_args INSTALL_DIR HIDE_CONSOLE)
    set(multi_value_args SOURCES INCLUDES DEFINES BUILD_ARGS)
    cmake_parse_arguments(PARSE_ARGV 0 LBG "${options}" "${one_value_args}" "${multi_value_args}")

    if(WIN32)
        lucaria_build_game(
            "win32"
            "${CMAKE_GENERATOR}"
            "" # toolchain
            "${TARGET}"
            "${LBG_SOURCES}"
            "${LBG_INSTALL_DIR}"
            "${LBG_INCLUDES}"
            "${LBG_DEFINES}"
            "${LBG_BUILD_ARGS}"
            "-DLUCARIA_PACKAGE=OFF;-DLUCARIA_HIDE_CONSOLE=${LBG_HIDE_CONSOLE}")

        if(LBG_INSTALL_DIR)
            add_custom_command(
                TARGET ${TARGET}_win32
                POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/external/lucaria/client/win32/$<CONFIG>/${TARGET}.exe ${LBG_INSTALL_DIR}/${TARGET}.exe

                COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/external/lucaria/client/win32/$<CONFIG>/${TARGET}.pdb ${LBG_INSTALL_DIR}/${TARGET}.pdb
                COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/external/lucaria/client/win32/$<CONFIG>/lucaria_runtime.dll ${LBG_INSTALL_DIR}/lucaria_runtime.dll
                COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/external/lucaria/client/win32/$<CONFIG>/lucaria_runtime.pdb ${LBG_INSTALL_DIR}/lucaria_runtime.pdb)
        endif()
    endif()

endfunction()