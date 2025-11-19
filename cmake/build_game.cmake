# build_game.cmake

function(lucaria_build_game
        PLATFORM 
        GENERATOR 
        TOOLCHAIN 
        TARGET 
        SOURCES 
        INSTALL_DIR 
        INCLUDES 
        DEFINES 
        BUILD_ARGS
        ARGS)

    set(game_source_dir "${LUCARIA_SOURCE_DIR}/game")
    set(game_output_dir "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}/${PLATFORM}")
    
    file(MAKE_DIRECTORY "${game_output_dir}")

    set(game_forward
            "-DLUCARIA_EXTERNAL_DIR:PATH=${LUCARIA_EXTERNAL_DIR}"
            "-DLUCARIA_INCLUDE_DIR:PATH=${LUCARIA_INCLUDE_DIR}"
            "-DLUCARIA_SOURCE_DIR:PATH=${LUCARIA_SOURCE_DIR}"
            "-DLUCARIA_BINARY_DIR=${LUCARIA_BINARY_DIR}"

            "-DLUCARIA_TARGET=${TARGET}"
            "-DLUCARIA_TARGET_PLATFORM=${PLATFORM}"
            "-DLUCARIA_TARGET_SOURCES=${SOURCES}"
            "-DLUCARIA_TARGET_INCLUDES=${INCLUDES}"
            "-DLUCARIA_TARGET_DEFINES=${DEFINES}"
            "-DLUCARIA_TARGET_BUILD_ARGS=${BUILD_ARGS}")
    
    set(game_toolchain)
    if(TOOLCHAIN)
        list(APPEND game_toolchain "-DCMAKE_TOOLCHAIN_FILE:FILEPATH=${TOOLCHAIN}")
    endif()

    add_custom_target(${TARGET}_${PLATFORM}
        ALL
        COMMAND "${CMAKE_COMMAND}" -S "${game_source_dir}" -B "${game_output_dir}" -G "${GENERATOR}" ${game_toolchain} ${game_forward} ${ARGS} -DLUCARIA_CONFIG=$<CONFIG> -DLUCARIA_DEBUG=$<IF:$<CONFIG:Debug>,ON,OFF>
        COMMAND "${CMAKE_COMMAND}" --build "${game_output_dir}" --config $<CONFIG> 
        WORKING_DIRECTORY "${game_output_dir}"
        COMMENT "Building ${TARGET} for ${PLATFORM}..."
        VERBATIM
        USES_TERMINAL)

endfunction()
