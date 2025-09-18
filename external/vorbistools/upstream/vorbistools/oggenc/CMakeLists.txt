cmake_minimum_required(VERSION 3.12)
project(oggenc)

set(OGGENC_SOURCES 
    "oggenc.c"
    "audio.c"
    "encode.c"
    "platform.c"
    "resample.c"
    "skeleton.c")

add_executable(oggenc ${OGGENC_SOURCES} ${VORBISTOOLS_SOURCE})

target_include_directories(oggenc PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(oggenc PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../include)
target_link_libraries(oggenc PRIVATE ogg vorbis vorbisenc vorbisfile)