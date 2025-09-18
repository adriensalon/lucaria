cmake_minimum_required(VERSION 3.12)
project(vorbistools)

set(VORBISTOOLS_SOURCE
    "${CMAKE_CURRENT_LIST_DIR}/share/base64.c"
    "${CMAKE_CURRENT_LIST_DIR}/share/getopt.c"
    "${CMAKE_CURRENT_LIST_DIR}/share/getopt1.c"
    "${CMAKE_CURRENT_LIST_DIR}/share/utf8.c")

add_subdirectory(oggenc)