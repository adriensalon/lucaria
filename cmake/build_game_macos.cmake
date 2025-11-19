# build_game_macos.cmake

function(add_lucaria_game_macos TARGET)
    set(options)
    set(one_value_args INSTALL_DIR)
    set(multi_value_args SOURCES INCLUDES DEFINES BUILD_ARGS)
    cmake_parse_arguments(PARSE_ARGV 0 LBG "${options}" "${one_value_args}" "${multi_value_args}")

    if(APPLE AND NOT IOS)
        lucaria_build_game(
            "macos"
            "${CMAKE_GENERATOR}"
            "" # toolchain
            "${TARGET}"
            "${LBG_SOURCES}"
            "${LBG_INSTALL_DIR}"
            "${LBG_INCLUDES}"
            "${LBG_DEFINES}"
            "${LBG_BUILD_ARGS}"
            "")

        if(LBG_INSTALL_DIR)
            add_custom_command(
                TARGET ${TARGET}_macos
                POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}/macos/$<CONFIG>/${TARGET} ${LBG_INSTALL_DIR}/${TARGET})
        endif()
    endif()

endfunction()