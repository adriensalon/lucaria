set(vorbistools_oggencoder_sources
    "upstream/vorbistools/oggenc/audio.c"
    "upstream/vorbistools/oggenc/encode.c"
    "upstream/vorbistools/oggenc/resample.c"
    "upstream/vorbistools/oggenc/skeleton.c"
    "oggenc_timer.c")

add_library(lucaria_external_oggenc STATIC ${vorbistools_oggencoder_sources})
target_include_directories(lucaria_external_oggenc PUBLIC "upstream/vorbistools")
target_include_directories(lucaria_external_oggenc PUBLIC "upstream/vorbistools/include")
target_include_directories(lucaria_external_oggenc PUBLIC "upstream/vorbistools/oggenc")
target_link_libraries(lucaria_external_oggenc PUBLIC lucaria_external_vorbis)

if(MSVC)
    target_compile_options(lucaria_external_oggenc PRIVATE /wd4018)
    target_compile_options(lucaria_external_oggenc PRIVATE /wd4244)
    target_compile_options(lucaria_external_oggenc PRIVATE /wd4267)
    target_compile_options(lucaria_external_oggenc PRIVATE /wd4305)
    target_compile_options(lucaria_external_oggenc PRIVATE /wd4996)
endif()
