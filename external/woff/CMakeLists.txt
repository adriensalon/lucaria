file(GLOB_RECURSE brotli_common_sources "upstream/woff/brotli/c/common/*.c")
file(GLOB_RECURSE brotli_decoder_sources "upstream/woff/brotli/c/dec/*.c")

set(woff2_common_sources
    "upstream/woff/src/table_tags.cc"
    "upstream/woff/src/variable_length.cc"
    "upstream/woff/src/woff2_common.cc")

set(woff2_decompress_sources
    "upstream/woff/src/woff2_dec.cc"
    "upstream/woff/src/woff2_out.cc")

set(woff2_sources
    ${brotli_common_sources}
    ${brotli_decoder_sources}
    ${woff2_common_sources}
    ${woff2_decompress_sources})

add_library(lucaria_external_woff STATIC ${woff2_sources})
target_include_directories(lucaria_external_woff PUBLIC "upstream/woff/brotli/c/include")
target_include_directories(lucaria_external_woff PUBLIC "upstream/woff/include")

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

file(GLOB_RECURSE brotli_encoder_sources "upstream/woff/brotli/c/enc/*.c")

set(woff2_compress_sources
    "upstream/woff/src/woff2_compress.cc"
    "upstream/woff/src/font.cc"
    "upstream/woff/src/glyph.cc"
    "upstream/woff/src/normalize.cc"
    "upstream/woff/src/transform.cc"
    "upstream/woff/src/woff2_enc.cc")

set(woff2compress_sources
    ${brotli_common_sources}
    ${brotli_encoder_sources}
    ${woff2_common_sources}
    ${woff2_compress_sources})

add_executable(lucaria_external_woff2compress ${woff2compress_sources})
target_include_directories(lucaria_external_woff2compress PRIVATE "upstream/woff/brotli/c/include")
target_include_directories(lucaria_external_woff2compress PUBLIC "upstream/woff/include")

if (MSVC)
    target_compile_options(lucaria_external_woff2compress PRIVATE /wd4018)
    target_compile_options(lucaria_external_woff2compress PRIVATE /wd4267)
    target_compile_options(lucaria_external_woff2compress PRIVATE /wd4477)
    target_compile_options(lucaria_external_woff2compress PRIVATE /wd4267)
    target_compile_options(lucaria_external_woff2compress PRIVATE /wd4244)
endif()
