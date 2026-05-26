if(LUCARIA_BUILD_COMPILERS)

    set(tidy_sources
        "upstream/tidy/src/access.c"
        "upstream/tidy/src/alloc.c"
        "upstream/tidy/src/attrdict.c"
        "upstream/tidy/src/attrs.c"
        "upstream/tidy/src/buffio.c"
        "upstream/tidy/src/charsets.c"
        "upstream/tidy/src/clean.c"
        "upstream/tidy/src/config.c"
        "upstream/tidy/src/entities.c"
        "upstream/tidy/src/fileio.c"
        "upstream/tidy/src/gdoc.c"
        "upstream/tidy/src/istack.c"
        "upstream/tidy/src/language.c"
        "upstream/tidy/src/lexer.c"
        "upstream/tidy/src/mappedio.c"
        "upstream/tidy/src/message.c"
        "upstream/tidy/src/messageobj.c"
        "upstream/tidy/src/parser.c"
        "upstream/tidy/src/pprint.c"
        "upstream/tidy/src/sprtf.c"
        "upstream/tidy/src/streamio.c"
        "upstream/tidy/src/tagask.c"
        "upstream/tidy/src/tags.c"
        "upstream/tidy/src/tidylib.c"
        "upstream/tidy/src/tmbstr.c"
        "upstream/tidy/src/utf8.c")

    add_library(lucaria_external_tidy STATIC EXCLUDE_FROM_ALL ${tidy_sources})
    target_include_directories(lucaria_external_tidy PUBLIC "upstream/tidy/include")
    target_include_directories(lucaria_external_tidy PRIVATE "upstream/tidy/src")
    target_compile_definitions(lucaria_external_tidy PUBLIC TIDY_STATIC)
    set_target_properties(lucaria_external_tidy PROPERTIES
        C_STANDARD 99
        C_STANDARD_REQUIRED ON)

    if(MSVC)
        target_compile_options(lucaria_external_tidy PRIVATE
            /wd4018
            /wd4090
            /wd4244
            /wd4267
            /wd4996
            /wd5286)
    endif()

endif()