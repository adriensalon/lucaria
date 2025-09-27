# build_assets.cmake

function(add_lucaria_assets TARGET ASSETS_INPUT_DIR ASSETS_OUTPUT_DIR)

    set(LUCARIA_BUILD_COMPILERS ON)
    add_subdirectory("${LUCARIA_SOURCE_DIR}/compiler" "${LUCARIA_BINARY_DIR}/compiler")

    file(MAKE_DIRECTORY "${ASSETS_OUTPUT_DIR}")

    add_custom_target(${TARGET} ALL
        COMMAND $<TARGET_FILE:lucaria_compiler>
            -i "${ASSETS_INPUT_DIR}"
            -o "${ASSETS_OUTPUT_DIR}"
            -etcpak     $<TARGET_FILE:lucaria_external_etcpak>
            -gltf2ozz   $<TARGET_FILE:lucaria_external_gltf2ozz>
            -oggenc     $<TARGET_FILE:lucaria_external_oggenc>
            -woff2compress $<TARGET_FILE:lucaria_external_woff2compress>
        WORKING_DIRECTORY "${ASSETS_OUTPUT_DIR}"
        COMMENT "Compiling assets..."
        VERBATIM
        USES_TERMINAL)

    
    add_dependencies(${TARGET} lucaria_compiler)

endfunction()