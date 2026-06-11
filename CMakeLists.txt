cmake_minimum_required(VERSION 3.13)
project(lucaria)

set(LUCARIA_EXTERNAL_DIR "${CMAKE_CURRENT_LIST_DIR}/external" CACHE PATH "Lucaria external directory")
set(LUCARIA_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/include" CACHE PATH "Lucaria include directory")
set(LUCARIA_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/source" CACHE PATH "Lucaria source directory")
set(LUCARIA_CMAKE_DIR "${CMAKE_CURRENT_LIST_DIR}/cmake" CACHE PATH "Lucaria cmake directory")
set(LUCARIA_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}" CACHE PATH "Lucaria binary directory")

include("cmake/make_superbuild.cmake")

