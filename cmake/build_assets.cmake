# build_assets.cmake

function(add_lucaria_assets TARGET ASSETS_INPUT_DIR ASSETS_OUTPUT_DIR)

    set(LUCARIA_BUILD_COMPILERS ON)
    add_subdirectory("${LUCARIA_SOURCE_DIR}/compiler" "${LUCARIA_BINARY_DIR}/compiler")

    file(GLOB_RECURSE assets_inputs CONFIGURE_DEPENDS "${ASSETS_INPUT_DIR}/*")

    file(MAKE_DIRECTORY "${ASSETS_OUTPUT_DIR}")
    set(assets_stamp "${ASSETS_OUTPUT_DIR}/.built.stamp")

    add_custom_command(
        OUTPUT "${assets_stamp}"
        COMMAND $<TARGET_FILE:lucaria_compiler>
            -i "${ASSETS_INPUT_DIR}"
            -o "${ASSETS_OUTPUT_DIR}"
            -etcpak $<TARGET_FILE:lucaria_external_etcpak>
            -gltf2ozz $<TARGET_FILE:lucaria_external_gltf2ozz>
            -oggenc $<TARGET_FILE:lucaria_external_oggenc>
            -woff2compress $<TARGET_FILE:lucaria_external_woff2compress>
        COMMAND "${CMAKE_COMMAND}" -E touch "${assets_stamp}"
        DEPENDS "${assets_inputs}"
        COMMENT "Compiling assets..."
        USES_TERMINAL)

    add_custom_target(${TARGET} 
        ALL
        DEPENDS "${assets_stamp}")
    
    add_dependencies(${TARGET} lucaria_compiler)

endfunction()