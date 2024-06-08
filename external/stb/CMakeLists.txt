cmake_minimum_required(VERSION 3.6...3.15)

project(stb_image LANGUAGES CXX VERSION 1.3.2)

add_library(stb_image EXCLUDE_FROM_ALL "stb_image.cpp")
target_include_directories(stb_image PUBLIC ${CMAKE_CURRENT_LIST_DIR})