if(LUCARIA_BUILD_COMPILERS)

    set(base64_sources
        "upstream/base64/lib/lib.c"
        "upstream/base64/lib/codec_choose.c"
        "upstream/base64/lib/tables/tables.c"
        "upstream/base64/lib/arch/generic/codec.c")

    add_library(lucaria_external_base64 STATIC EXCLUDE_FROM_ALL ${base64_sources})
    set_target_properties(lucaria_external_base64 PROPERTIES
        C_STANDARD 99
        C_STANDARD_REQUIRED ON)

    target_include_directories(lucaria_external_base64 PUBLIC "upstream/base64/include")
	target_compile_definitions(lucaria_external_base64 PUBLIC BASE64_STATIC_DEFINE)
    target_include_directories(lucaria_external_base64 PRIVATE
    	"include"
        "upstream/base64/lib"
        "upstream/base64/lib/tables"
        "upstream/base64/lib/arch/generic"
        "upstream/base64/lib/arch/generic/32"
        "upstream/base64/lib/arch/generic/64")

    target_compile_definitions(lucaria_external_base64 PRIVATE
        HAVE_STDINT_H
        HAVE_STDDEF_H
        HAVE_STRING_H)

    if(MSVC)
        target_compile_options(lucaria_external_base64 PRIVATE
            /wd4244
            /wd4267
            /wd4996)
    endif()

	target_sources(lucaria_external_base64 PRIVATE "source/generic_aliases.c")

endif()