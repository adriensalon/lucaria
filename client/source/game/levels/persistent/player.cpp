
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

    registry.emplace<model_component<model_shader::unlit>>(_player_entity)
        .mesh(fetch_mesh("assets/blockout_test.bin"))
        .material(fetch_material({{ material_texture::color, "assets/room_color.bin" }}));
    
    transform_component& _player_transform = registry.emplace<transform_component>(_player_entity)
        // .rotation_relative(glm::vec3(-1.57f, 0.f, 0.f))
        .position_warp(glm::vec3(3.f, 0.f, 0.f));
    
    registry.emplace<controller_component<runner_controller_state>>(_player_entity)
        .state(runner_controller_state())
        .resolve([&_player_transform] (runner_controller_state& state) {
            _player_transform.position_relative({ 0, 0, 0 });
        });

    // rendering_system::use_camera_transform(_player_transform);
}