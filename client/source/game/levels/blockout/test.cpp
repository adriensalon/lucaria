
#include <entt/entt.hpp>

#include <ecs/component/model.hpp>
#include <ecs/component/collider.hpp>

void level_blockout_test(entt::registry& registry)
{
    const entt::entity _wall_entity = registry.create();

    registry.emplace<model_component<model_shader::blockout>>(_wall_entity)
        // .material(fetch_material({{ material_texture::color, "assets/room_color.bin" }}))
        .mesh(fetch_mesh("assets/blockout_test.bin"));

    registry.emplace<collider_component>(_wall_entity)
        .navmesh(fetch_shape("assets/blockout_test_wall.bin"))
        .wall();



    const entt::entity _ground_entity0 = registry.create();
    registry.emplace<collider_component>(_ground_entity0)
        .navmesh(fetch_shape("assets/blockout_test_ground0.bin"))
        .ground();
        
    const entt::entity _ground_entity1 = registry.create();
    registry.emplace<collider_component>(_ground_entity1)
        .navmesh(fetch_shape("assets/blockout_test_ground1.bin"))
        .ground();

    const entt::entity _ground_entity2 = registry.create();
    registry.emplace<collider_component>(_ground_entity2)
        .navmesh(fetch_shape("assets/blockout_test_ground2.bin"))
        .ground();

    const entt::entity _ground_entity3 = registry.create();
    registry.emplace<collider_component>(_ground_entity3)
        .navmesh(fetch_shape("assets/blockout_test_ground3.bin"))
        .ground();
        
}