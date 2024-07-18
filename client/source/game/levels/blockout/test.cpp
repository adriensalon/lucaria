
#include <entt/entt.hpp>

#include <ecs/component/model.hpp>
#include <ecs/component/collider.hpp>

void level_blockout_test(entt::registry& registry)
{
    const entt::entity _test_entity = registry.create();

    registry.emplace<model_component<model_shader::blockout>>(_test_entity)
        .mesh(fetch_mesh("assets/blockout_test.bin"));

    registry.emplace<collider_component<collider_algorithm::ground>>(_test_entity)
        .navmesh(fetch_navmesh("assets/blockout_test_ground.bin"));
}