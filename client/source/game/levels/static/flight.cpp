
#include <entt/entt.hpp>

#include <ecs/component/collider.hpp>
#include <ecs/component/model.hpp>
#include <ecs/component/transform.hpp>
#include <ecs/system/rendering.hpp>

#define GAME_OBJECT_B2(uuid)                                          \
    const entt::entity go_##uuid = registry.create();                 \
    registry.emplace<model_component<model_shader::unlit>>(go_##uuid) \
        .color(fetch_texture("assets/image/image_" #uuid ".bin"))     \
        .mesh(fetch_mesh("assets/geometry/geometry_" #uuid ".bin"));  \
    registry.emplace<collider_component>(go_##uuid)                   \
        .shape(fetch_shape("assets/shape/geometry_" #uuid ".bin", shape_type::triangle_mesh))

void level_static_flight(entt::registry& registry)
{
    rendering_system::use_skybox_cubemap(fetch_cubemap({ 
        "assets/cubemap/cubemap_px_eLVJ.bin",
        "assets/cubemap/cubemap_py_eLVJ.bin",
        "assets/cubemap/cubemap_pz_eLVJ.bin",
        "assets/cubemap/cubemap_nx_eLVJ.bin",
        "assets/cubemap/cubemap_ny_eLVJ.bin",
        "assets/cubemap/cubemap_nz_eLVJ.bin" }));

    GAME_OBJECT_B2(9ETQ).wall();
    GAME_OBJECT_B2(8jHH).wall();
    GAME_OBJECT_B2(fXbl).wall();
    GAME_OBJECT_B2(8Ijp).wall();
    GAME_OBJECT_B2(UQZZ).ground();
}