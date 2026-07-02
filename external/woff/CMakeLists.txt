file(GLOB_RECURSE brotli_common_sources "upstream/woff/brotli/c/common/*.c")
file(GLOB_RECURSE brotli_decoder_sources "upstream/woff/brotli/c/dec/*.c")

set(woff2_common_sources
    "upstream/woff/src/table_tags.cc"
    "upstream/woff/src/variable_length.cc"
    "upstream/woff/src/woff2_common.cc")

if(PSP)
	set(woff2_decompress_sources
		"psp/woff2_dec.cc"
		"upstream/woff/src/woff2_out.cc")
else()
	set(woff2_decompress_sources
		"upstream/woff/src/woff2_dec.cc"
		"upstream/woff/src/woff2_out.cc")
endif()

set(woff2_sources
    ${brotli_common_sources}
    ${brotli_decoder_sources}
    ${woff2_common_sources}
    ${woff2_decompress_sources})

add_library(lucaria_external_woff STATIC ${woff2_sources})
target_include_directories(lucaria_external_woff PUBLIC "upstream/woff/brotli/c/include")
target_include_directories(lucaria_external_woff PUBLIC "upstream/woff/include")
if(PSP)
	target_include_directories(lucaria_external_woff PRIVATE "upstream/woff/src")
endif()

if (MSVC)
    target_compile_options(lucaria_external_woff PRIVATE /wd4018)
    target_compile_options(lucaria_external_woff PRIVATE /wd4267)
    target_compile_options(lucaria_external_woff PRIVATE /wd4477)
    target_compile_options(lucaria_external_woff PRIVATE /wd4267)
    target_compile_options(lucaria_external_woff PRIVATE /wd4244)
endif()

##
##
##

if(LUCARIA_BUILD_COMPILERS)

    file(GLOB_RECURSE brotli_encoder_sources "upstream/woff/brotli/c/enc/*.c")

    set(woff2_encoder_sources
        "upstream/woff/src/font.cc"
        "upstream/woff/src/glyph.cc"
        "upstream/woff/src/normalize.cc"
        "upstream/woff/src/transform.cc"
        "upstream/woff/src/woff2_enc.cc")

    set(woff2_encoder_library_sources
        ${brotli_common_sources}
        ${brotli_encoder_sources}
        ${woff2_common_sources}
        ${woff2_encoder_sources})

    add_library(lucaria_external_woff2enc STATIC ${woff2_encoder_library_sources})
    target_include_directories(lucaria_external_woff2enc PRIVATE "upstream/woff/brotli/c/include")
    target_include_directories(lucaria_external_woff2enc PUBLIC "upstream/woff/include")

    if (MSVC)
        target_compile_options(lucaria_external_woff2enc PRIVATE /wd4018)
        target_compile_options(lucaria_external_woff2enc PRIVATE /wd4267)
        target_compile_options(lucaria_external_woff2enc PRIVATE /wd4477)
        target_compile_options(lucaria_external_woff2enc PRIVATE /wd4267)
        target_compile_options(lucaria_external_woff2enc PRIVATE /wd4244)
    endif()

endif()
