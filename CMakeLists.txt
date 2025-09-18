cmake_minimum_required(VERSION 3.13)
project(lucaria)

set(LUCARIA_EXTERNAL_DIR "${CMAKE_CURRENT_LIST_DIR}/external" CACHE PATH "Lucaria external directory")
set(LUCARIA_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/include" CACHE PATH "Lucaria include directory")
set(LUCARIA_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/source" CACHE PATH "Lucaria source directory")
set(LUCARIA_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}" CACHE PATH "Lucaria binary directory")

include("cmake/build_assets.cmake")
include("cmake/build_game.cmake")
include("cmake/build_game_web.cmake")
include("cmake/build_game_win32.cmake")
