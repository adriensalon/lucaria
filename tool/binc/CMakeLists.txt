cmake_minimum_required(VERSION 3.19)
project(lucaria_binc)

get_filename_component(_lucaria_root_dir "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
set(LUCARIA_EXTERNAL_DIR "${_lucaria_root_dir}/external" CACHE PATH "Lucaria external directory")
set(LUCARIA_INCLUDE_DIR "${_lucaria_root_dir}/include" CACHE PATH "Lucaria include directory")
set(LUCARIA_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}" CACHE PATH "Lucaria binary directory")

set(LUCARIA_BUILD_COMPILERS ON)
add_subdirectory(${LUCARIA_EXTERNAL_DIR}/assimp ${CMAKE_CURRENT_BINARY_DIR}/external/assimp)
add_subdirectory(${LUCARIA_EXTERNAL_DIR}/cereal ${CMAKE_CURRENT_BINARY_DIR}/external/cereal)
add_subdirectory(${LUCARIA_EXTERNAL_DIR}/etcpak ${CMAKE_CURRENT_BINARY_DIR}/external/etcpak)
add_subdirectory(${LUCARIA_EXTERNAL_DIR}/glm ${CMAKE_CURRENT_BINARY_DIR}/external/glm)
add_subdirectory(${LUCARIA_EXTERNAL_DIR}/glfw ${CMAKE_CURRENT_BINARY_DIR}/external/glfw)
add_subdirectory(${LUCARIA_EXTERNAL_DIR}/imgui ${CMAKE_CURRENT_BINARY_DIR}/external/imgui)
include(${LUCARIA_EXTERNAL_DIR}/meshoptimizer/CMakeLists.txt)
add_subdirectory(${LUCARIA_EXTERNAL_DIR}/ogg ${CMAKE_CURRENT_BINARY_DIR}/external/ogg)
add_subdirectory(${LUCARIA_EXTERNAL_DIR}/ozz ${CMAKE_CURRENT_BINARY_DIR}/external/ozz)
add_subdirectory(${LUCARIA_EXTERNAL_DIR}/stb ${CMAKE_CURRENT_BINARY_DIR}/external/stb)
add_subdirectory(${LUCARIA_EXTERNAL_DIR}/vorbis ${CMAKE_CURRENT_BINARY_DIR}/external/vorbis)
add_subdirectory(${LUCARIA_EXTERNAL_DIR}/vorbistools ${CMAKE_CURRENT_BINARY_DIR}/external/vorbistools)
add_subdirectory(${LUCARIA_EXTERNAL_DIR}/woff ${CMAKE_CURRENT_BINARY_DIR}/external/woff)

if(MSVC)
    target_compile_options(lucaria_external_assimp PRIVATE "$<$<CONFIG:Release>:/Od>")
endif()

file(GLOB_RECURSE lucaria_binc_source "*.cpp")
add_executable(lucaria_binc ${lucaria_binc_source})
target_include_directories(lucaria_binc PRIVATE "${LUCARIA_INCLUDE_DIR}")
target_include_directories(lucaria_binc PRIVATE "${CMAKE_CURRENT_LIST_DIR}")
set_target_properties(lucaria_binc PROPERTIES
    CXX_STANDARD 17
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_CURRENT_BINARY_DIR}/bin"
    INTERPROCEDURAL_OPTIMIZATION FALSE
    INTERPROCEDURAL_OPTIMIZATION_DEBUG FALSE
    INTERPROCEDURAL_OPTIMIZATION_RELEASE FALSE
    INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO FALSE
    INTERPROCEDURAL_OPTIMIZATION_MINSIZEREL FALSE)
target_link_options(lucaria_binc PRIVATE "$<$<CXX_COMPILER_ID:MSVC>:/INCREMENTAL:NO>")

target_link_libraries(lucaria_binc PRIVATE lucaria_external_assimp)
target_link_libraries(lucaria_binc PRIVATE lucaria_external_cereal)
target_link_libraries(lucaria_binc PRIVATE lucaria_external_etcpak)
target_link_libraries(lucaria_binc PRIVATE lucaria_external_glm)
target_link_libraries(lucaria_binc PRIVATE lucaria_external_meshoptimizer)
target_link_libraries(lucaria_binc PRIVATE lucaria_external_ozz)
target_link_libraries(lucaria_binc PRIVATE lucaria_external_gltf2ozz)
target_link_libraries(lucaria_binc PRIVATE lucaria_external_oggenc)
target_link_libraries(lucaria_binc PRIVATE lucaria_external_stb)
target_link_libraries(lucaria_binc PRIVATE lucaria_external_woff2enc)

target_include_directories(lucaria_binc BEFORE PRIVATE "${lucaria_patched_imgui_dir}")
