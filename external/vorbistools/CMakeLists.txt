set(vorbistools_share_sources 
    "upstream/vorbistools/share/base64.c"
    "upstream/vorbistools/share/getopt.c"
    "upstream/vorbistools/share/getopt1.c"
    "upstream/vorbistools/share/utf8.c")

set(vorbistools_oggenc_sources 
    "upstream/vorbistools/oggenc/oggenc.c"
    "upstream/vorbistools/oggenc/audio.c"
    "upstream/vorbistools/oggenc/encode.c"
    "upstream/vorbistools/oggenc/platform.c"
    "upstream/vorbistools/oggenc/resample.c"
    "upstream/vorbistools/oggenc/skeleton.c")

set(oggenc_sources
    ${vorbistools_share_sources}
    ${vorbistools_oggenc_sources})

add_executable(lucaria_external_oggenc ${oggenc_sources})
target_include_directories(lucaria_external_oggenc PRIVATE "upstream/vorbistools")
target_include_directories(lucaria_external_oggenc PRIVATE "upstream/vorbistools/include")
target_link_libraries(lucaria_external_oggenc PRIVATE lucaria_external_vorbis)

if(MSVC)
    target_compile_options(lucaria_external_oggenc PRIVATE /wd4018)
    target_compile_options(lucaria_external_oggenc PRIVATE /wd4244)
    target_compile_options(lucaria_external_oggenc PRIVATE /wd4267)
    target_compile_options(lucaria_external_oggenc PRIVATE /wd4305)
    target_compile_options(lucaria_external_oggenc PRIVATE /wd4996)
endif()