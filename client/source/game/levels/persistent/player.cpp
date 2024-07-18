
#include <entt/entt.hpp>

#include <ecs/component/animator.hpp>
#include <ecs/component/controller.hpp>
#include <ecs/component/model.hpp>
#include <ecs/component/transform.hpp>
#include <ecs/component/rigidbody.hpp>
#include <ecs/system/rendering.hpp>
#include <ecs/system/scripting.hpp>

#include <game/gameplay/runner.hpp>


void level_persistent_player(entt::registry& registry)
{    
    // scripting_system::use_controller_state<runner_controller_state>();

    const entt::entity _player_entity = registry.create();

    registry.emplace<transform_component>(_player_entity)
        .position_warp(glm::vec3(0.f, 0.f, 0.f));
    
    registry.emplace<rigidbody_component>(_player_entity)
        .capsule(0.5f, 1.83f);
        // .box({ 0.2f, 1.8f, 0.2f });
    
    registry.emplace<controller_component<runner_controller_state>>(_player_entity)
        .state(runner_controller_state())
        .resolve([&] (runner_controller_state& state) {
            
        });

    // rendering_system::use_camera_transform(_player_transform);
}