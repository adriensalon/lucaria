
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
    rendering_system::use_clear_color({ 0.f, 0.f, 0.f, 1.f });
    scripting_system::use_controller_state<runner_controller_state>();

    const entt::entity _player_entity = registry.create();

    transform_component& _player_transform = registry.emplace<transform_component>(_player_entity)
        .position_warp(glm::vec3(-14.f, 6.2f, 3.f));
    
    registry.emplace<kinematic_rigidbody_component>(_player_entity)
        .glide_wall()
        .snap_ground()
        .collide_layer(kinematic_layer::layer_0)
        .capsule(0.5f, 1.83f);
    
    registry.emplace<model_component<model_shader::blockout>>(_player_entity)
        .mesh(fetch_mesh("assets/player.bin"));
    
    registry.emplace<animator_component>(_player_entity)
        .armature(fetch_armature("assets/lol_armature.bin"))
        .skeleton(fetch_skeleton("assets/lol_skeleton.bin"));
        // .moveset(fetch_moveset({
        //     { 33, "assets/lol_animation_AnimLol.bin" }
        // }));
    
    registry.emplace<controller_component<runner_controller_state>>(_player_entity)
        .state(runner_controller_state())
        .resolve([&] (runner_controller_state& state) {
            // _player_transform.position_relative({ 0.f, -0.01f, 0.f });
            _player_transform.position_relative({ 0.03f, 0.f, -0.005f });
        });

    // rendering_system::use_camera_transform(_player_transform);
}