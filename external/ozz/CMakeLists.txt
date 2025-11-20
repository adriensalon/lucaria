file(GLOB_RECURSE ozz_base_sources "upstream/ozz/src/base/*.cc")
file(GLOB_RECURSE ozz_animation_sources "upstream/ozz/src/animation/runtime/*.cc")
file(GLOB_RECURSE ozz_geometry_sources "upstream/ozz/src/animation/runtime*.cc")

set(ozz_runtime_sources
    ${ozz_base_sources}
    ${ozz_animation_sources}
    ${ozz_geometry_sources})

set(lucaria_external_ozz_sources
    ${ozz_runtime_sources})

add_library(lucaria_external_ozz STATIC EXCLUDE_FROM_ALL ${lucaria_external_ozz_sources})
set_target_properties(lucaria_external_ozz PROPERTIES CXX_STANDARD 17)
target_include_directories(lucaria_external_ozz PRIVATE "upstream/ozz/src")
target_include_directories(lucaria_external_ozz PUBLIC "upstream/ozz/include")

# simd config
if(EMSCRIPTEN)
    target_compile_options(lucaria_external_ozz PUBLIC -msimd128 -msse2)
else()
    if(MSVC)
        target_compile_definitions(lucaria_external_ozz PUBLIC __SSE4_1__=1)
    elseif(NOT ANDROID)
        target_compile_options(lucaria_external_ozz PUBLIC -msse4.1)
    endif()
endif()

if(MSVC)
    target_compile_options(lucaria_external_ozz PRIVATE /wd4996)
endif()

##
##
##

if(LUCARIA_BUILD_COMPILERS)

    set(ozz_json_sources
        "upstream/ozz/extern/jsoncpp/dist/jsoncpp.cpp")

    set(ozz_options_sources
        "upstream/ozz/src/options/options.cc")

    set(ozz_offline_sources 
        "upstream/ozz/src/animation/offline/additive_animation_builder.cc"
        "upstream/ozz/src/animation/offline/animation_builder.cc"
        "upstream/ozz/src/animation/offline/animation_optimizer.cc"
        "upstream/ozz/src/animation/offline/motion_extractor.cc"
        "upstream/ozz/src/animation/offline/raw_animation_archive.cc"
        "upstream/ozz/src/animation/offline/raw_animation_utils.cc"
        "upstream/ozz/src/animation/offline/raw_animation.cc"
        "upstream/ozz/src/animation/offline/raw_skeleton_archive.cc"
        "upstream/ozz/src/animation/offline/raw_skeleton.cc"
        "upstream/ozz/src/animation/offline/raw_track_utils.cc"
        "upstream/ozz/src/animation/offline/raw_track.cc"
        "upstream/ozz/src/animation/offline/skeleton_builder.cc"
        "upstream/ozz/src/animation/offline/track_builder.cc"
        "upstream/ozz/src/animation/offline/track_optimizer.cc")

    set(ozz_tools_sources
        "upstream/ozz/src/animation/offline/tools/import2ozz_anim.cc"
        "upstream/ozz/src/animation/offline/tools/import2ozz_config.cc"
        "upstream/ozz/src/animation/offline/tools/import2ozz_skel.cc"
        "upstream/ozz/src/animation/offline/tools/import2ozz_track.cc"
        "upstream/ozz/src/animation/offline/tools/import2ozz.cc")

    set(ozz_gltf_sources
        "upstream/ozz/src/animation/offline/gltf/gltf2ozz.cc")

    set(gltf2ozz_sources
        ${ozz_json_sources}
        ${ozz_options_sources}
        ${ozz_offline_sources}
        ${ozz_tools_sources}
        ${ozz_gltf_sources})

    add_executable(lucaria_external_gltf2ozz ${gltf2ozz_sources})
    target_include_directories(lucaria_external_gltf2ozz PRIVATE "upstream/ozz/extern/jsoncpp/dist")
    target_include_directories(lucaria_external_gltf2ozz PRIVATE "upstream/ozz/include")
    target_include_directories(lucaria_external_gltf2ozz PRIVATE "upstream/ozz/src")
    set_target_properties(lucaria_external_gltf2ozz PROPERTIES CXX_STANDARD 17)
    target_link_libraries(lucaria_external_gltf2ozz PRIVATE lucaria_external_ozz)

    if(MSVC)
        target_compile_options(lucaria_external_gltf2ozz PRIVATE /wd4996)
    endif()

endif()
