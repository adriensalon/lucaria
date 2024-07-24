add_subdirectory(upstream/brotli)

set(BROTLIDEC_INCLUDE_DIRS ${BROTLI_INCLUDE_DIRS})
set(BROTLIENC_INCLUDE_DIRS ${BROTLI_INCLUDE_DIRS})
set(BROTLIDEC_LIBRARIES brotlidec brotlicommon)
set(BROTLIENC_LIBRARIES brotlienc brotlicommon)

add_subdirectory(upstream)

if (MSVC)
    target_compile_options(woff2dec PRIVATE /wd4267)
    target_compile_options(woff2dec PRIVATE /wd4477)
    target_compile_options(woff2enc PRIVATE /wd4267)
    target_compile_options(woff2enc PRIVATE /wd4018)
    target_compile_options(woff2enc PRIVATE /wd4244)
    target_compile_options(woff2enc PRIVATE /wd4018)
endif()