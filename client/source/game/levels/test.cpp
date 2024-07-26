
#include <entt/entt.hpp>

#include <ecs/component/model.hpp>
#include <ecs/component/collider.hpp>
#include <ecs/system/rendering.hpp>
#include <game/gameplay/runner.hpp>

fetch_container<texture_ref> _okokok;

extern std::unique_ptr<runner_actor> faith;

void level_blockout_test(entt::registry& registry)
{
    faith->get_transform().position_relative({ -5.f, 1.f, 0.f });

    const entt::entity _wall_entity = registry.create();

    // _okokok.emplace(fetch_texture("room_color_compressed.bin"));

    rendering_system::use_skybox_cubemap(fetch_cubemap({
        "assets/px.bin",
        "assets/py.bin",
        "assets/pz.bin",
        "assets/nx.bin",
        "assets/ny.bin",
        "assets/nz.bin"
    }
    ));
    // std::array<std::filesystem::path, 6> {
    //     "assets/skyboxes/test/px_etc.bin",
    //     "assets/skyboxes/test/py_etc.bin",
    //     "assets/skyboxes/test/pz_etc.bin",
    //     "assets/skyboxes/test/nx_etc.bin",
    //     "assets/skyboxes/test/ny_etc.bin",
    //     "assets/skyboxes/test/nz_etc.bin"
    // }, std::array<std::filesystem::path, 6> {
    //     "assets/skyboxes/test/px_s3tc.bin",
    //     "assets/skyboxes/test/py_s3tc.bin",
    //     "assets/skyboxes/test/pz_s3tc.bin",
    //     "assets/skyboxes/test/nx_s3tc.bin",
    //     "assets/skyboxes/test/ny_s3tc.bin",
    //     "assets/skyboxes/test/nz_s3tc.bin"
    // }));


    registry.emplace<model_component<model_shader::blockout>>(_wall_entity)
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