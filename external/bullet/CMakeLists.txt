file(GLOB_RECURSE bullet_inverse_dynamics_sources "upstream/bullet/src/BulletInverseDynamics/*.cpp")
file(GLOB_RECURSE bullet_soft_body_sources "upstream/bullet/src/BulletSoftBody/*.cpp")
file(GLOB_RECURSE bullet_collision_sources "upstream/bullet/src/BulletCollision/*.cpp")
file(GLOB_RECURSE bullet_dynamics_sources "upstream/bullet/src/BulletDynamics/*.cpp")
file(GLOB_RECURSE bullet_linear_math_sources "upstream/bullet/src/LinearMath/*.cpp")

set(bullet_sources
    ${bullet_inverse_dynamics_sources}    
    ${bullet_soft_body_sources}    
    ${bullet_collision_sources}    
    ${bullet_dynamics_sources}    
    ${bullet_linear_math_sources})

add_library(lucaria_external_bullet STATIC EXCLUDE_FROM_ALL ${bullet_sources})
set_target_properties(lucaria_external_bullet PROPERTIES CXX_STANDARD 17)
target_include_directories(lucaria_external_bullet PUBLIC "added/bullet/src")
target_include_directories(lucaria_external_bullet PUBLIC "added/bullet/src/LinearMath")
target_include_directories(lucaria_external_bullet PUBLIC "upstream/bullet/src")

# simd config
if(EMSCRIPTEN)
    target_compile_options(lucaria_external_bullet PUBLIC -msimd128 -msse2)
else()
    if(MSVC)
        target_compile_definitions(lucaria_external_bullet PUBLIC __SSE4_1__=1)
    elseif(NOT ANDROID)
        target_compile_options(lucaria_external_bullet PUBLIC -msse4.1)
    endif()
endif()

if(MSVC)
    target_compile_options(lucaria_external_bullet PRIVATE /wd4018)
    target_compile_options(lucaria_external_bullet PRIVATE /wd4056)
    target_compile_options(lucaria_external_bullet PRIVATE /wd4244)
    target_compile_options(lucaria_external_bullet PRIVATE /wd4267)
    target_compile_options(lucaria_external_bullet PRIVATE /wd4305)
endif()
