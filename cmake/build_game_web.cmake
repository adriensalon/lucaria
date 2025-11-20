# build_game_web.cmake

function(add_lucaria_game_web TARGET)
    set(options)
    set(one_value_args INSTALL_DIR ASSETS_DIR HTML_SHELL)
    set(multi_value_args SOURCES INCLUDES DEFINES BUILD_ARGS)
    cmake_parse_arguments(PARSE_ARGV 0 LBG "${options}" "${one_value_args}" "${multi_value_args}")
    
    # find SDK
    if(DEFINED ENV{EMSDK})
        set(EMSDK $ENV{EMSDK})
    elseif(DEFINED ENV{EMSCRIPTEN})
        set(EMSDK $ENV{EMSCRIPTEN})
    endif()
    if(EMSDK AND EXISTS "${EMSDK}")
        message(STATUS "Found Emsdk at ${EMSDK}")
        set(EMSDK_FOUND TRUE)
    else()
        find_program(EMCC_EXECUTABLE NAMES emcc PATHS ENV PATH)
        if(EMCC_EXECUTABLE)
            get_filename_component(EMSDK_DIR "${EMCC_EXECUTABLE}" DIRECTORY)
            get_filename_component(EMSDK "${EMSDK_DIR}/../../" ABSOLUTE)
            if(EXISTS "${EMSDK}")
                message(STATUS "Found Emscripten SDK at ${EMSDK}")
                set(EMSDK_FOUND TRUE)
            endif()
        endif()
    endif()
    if(EMSDK_FOUND)
        set(EMSDK_TOOLCHAIN "${EMSDK}/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake" CACHE PATH "EMSDK toolchain file")
        message(STATUS "Found Emscripten toolchain file at ${EMSDK_TOOLCHAIN}")
    else()
        message(STATUS "Emscripten SDK could not be located. lucaria_web will not be built")
        return()
    endif()

    # default parameters
    if(NOT LBG_ASSETS_DIR)
        set(LUCARIA_ASSETS_PACKAGE OFF)
        message(STATUS "Emscripten will rely on same origin fetch for assets")
    else()        
        set(LUCARIA_ASSETS_PACKAGE ON)
        message(STATUS "Found Emscripten assets at ${LBG_ASSETS_DIR}")
    endif()

    lucaria_build_game(
        "web"
        "Ninja"
        "${EMSDK_TOOLCHAIN}"
        "${TARGET}"
        "${LBG_SOURCES}"
        "${LBG_INSTALL_DIR}"
        "${LBG_INCLUDES}"
        "${LBG_DEFINES}"
        "${LBG_BUILD_ARGS}"
        "-DLUCARIA_ASSETS_DIR=${LBG_ASSETS_DIR};-DLUCARIA_ASSETS_PACKAGE=${LUCARIA_ASSETS_PACKAGE};-DLUCARIA_HTML_SHELL=${LBG_HTML_SHELL}")
    
    if(LBG_INSTALL_DIR)
        add_custom_command(
            TARGET ${TARGET}_web
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}/web/${TARGET}.html ${LBG_INSTALL_DIR}/index.html
            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}/web/${TARGET}.js ${LBG_INSTALL_DIR}/${TARGET}.js
            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}/web/${TARGET}.wasm ${LBG_INSTALL_DIR}/${TARGET}.wasm)

        if(LBG_ASSETS_DIR)
            add_custom_command(
                TARGET ${TARGET}_web
                POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}/web/${TARGET}.data ${LBG_INSTALL_DIR}/${TARGET}.data)
        endif()
    endif()

endfunction()