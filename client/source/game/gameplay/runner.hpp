#pragma once 

#include <entt/entt.hpp>

#include <ecs/component/animator.hpp>
#include <ecs/component/controller.hpp>
#include <ecs/component/model.hpp>
#include <ecs/component/transform.hpp>
#include <ecs/component/rigidbody.hpp>
#include <ecs/component/speaker.hpp>
#include <ecs/system/mixer.hpp>
#include <ecs/system/rendering.hpp>
#include <ecs/system/scripting.hpp>

struct runner_controller_state {

    glm::float32 forward_movement = 0.f;
    glm::float32 lateral_movement = 0.f;
    glm::vec2 look_angle = { 0.f, 0.f };

    inline void update()
    {

    }


};

struct runner {

    runner(entt::registry& registry)
    {
        _entity = registry.create();

        _transform = &registry.emplace<transform_component>(_entity);
        
        _rigidbody = &registry.emplace<kinematic_rigidbody_component>(_entity)
            .collide_walls()
            .collide_grounds()
            .collide_layer(kinematic_layer::layer_0)
            .capsule(0.5f, 1.83f);
        
        _model = &registry.emplace<model_component<model_shader::unlit>>(_entity)
            .color(fetch_texture("assets/room_color.bin", "assets/room_color_etc.bin", "assets/room_color_s3tc.bin"))
            .mesh(fetch_mesh("assets/player.bin"));
        
        _speaker = &registry.emplace<speaker_component>(_entity)
            .sounds({{ 666, fetch_sound("assets/anorlondo.bin") }});
        
        _animator = &registry.emplace<animator_component>(_entity)
            .motion_bone_index(4)
            .armature(fetch_armature("assets/player_armature.bin"))
            .skeleton(fetch_skeleton("assets/player_skeleton.bin"))
            .animations({ 
                { 444, fetch_animation("assets/player_animation_AnimLol.bin") },
            });
        
        _controller = &registry.emplace<controller_component<runner_controller_state>>(_entity);

        
        scripting_system::use_controller_state<runner_controller_state>();
    }

    inline transform_component& get_transform()
    {
        return *_transform;
    }

    inline void add_script(const std::function<void(runner_controller_state&)>& script)
    {
        _controller->add_script(script);
    }

private:
    entt::entity _entity = {};
    transform_component* _transform = nullptr;
    animator_component* _animator = nullptr;
    rigidbody_component<rigidbody_kind::kinematic>* _rigidbody = nullptr;
    model_component<model_shader::unlit>* _model = nullptr;
    speaker_component* _speaker = nullptr;
    controller_component<runner_controller_state>* _controller = nullptr;
};