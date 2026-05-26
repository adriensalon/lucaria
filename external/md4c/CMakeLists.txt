if(LUCARIA_BUILD_COMPILERS)

    add_library(lucaria_external_md4c STATIC EXCLUDE_FROM_ALL "upstream/md4c/src/md4c.c")
    target_include_directories(lucaria_external_md4c PUBLIC "upstream/md4c/src")
    set_target_properties(lucaria_external_md4c PROPERTIES
        C_STANDARD 99
        C_STANDARD_REQUIRED ON)

    if(MSVC)
        target_compile_options(lucaria_external_md4c PRIVATE 
			/wd4267 
			/wd4244)
    endif()

endif()