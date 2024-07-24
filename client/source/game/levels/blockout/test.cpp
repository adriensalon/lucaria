
#include <entt/entt.hpp>

#include <ecs/component/model.hpp>
#include <ecs/component/collider.hpp>

fetch_container<texture_ref> _okokok;

void level_blockout_test(entt::registry& registry)
{
    const entt::entity _wall_entity = registry.create();

    // _okokok.emplace(fetch_texture("room_color_compressed.bin"));



    registry.emplace<model_component<model_shader::unlit>>(_wall_entity)
        .color(fetch_texture("assets/room_color_s3tc.bin"))
        // .color(fetch_texture("assets/room_color.bin", "assets/room_color_etc.bin", "assets/room_color_s3tc.bin"))
        .mesh(fetch_mesh("assets/blockout_test.bin"));

    registry.emplace<collider_component>(_wall_entity)
        .shape(fetch_shape("assets/blockout_test_wall.bin", shape_type::convex_hull))
        .wall();



    const entt::entity _ground_entity0 = registry.create();
    registry.emplace<collider_component>(_ground_entity0)
        .shape(fetch_shape("assets/blockout_test_ground0.bin", shape_type::convex_hull))
        .ground();
        
    const entt::entity _ground_entity1 = registry.create();
    registry.emplace<collider_component>(_ground_entity1)
        .shape(fetch_shape("assets/blockout_test_ground1.bin", shape_type::convex_hull))
        .ground();

    const entt::entity _ground_entity2 = registry.create();
    registry.emplace<collider_component>(_ground_entity2)
        .shape(fetch_shape("assets/blockout_test_ground2.bin", shape_type::convex_hull))
        .ground();

    const entt::entity _ground_entity3 = registry.create();
    registry.emplace<collider_component>(_ground_entity3)
        .shape(fetch_shape("assets/blockout_test_ground3.bin", shape_type::convex_hull))
        .ground();
        
}