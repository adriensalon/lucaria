#include <game/gameplay/runner.hpp>

runner_actor::runner_actor(entt::registry& registry)
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
                    .sounds({ { 666, fetch_sound("assets/anorlondo.bin") } });

    _animator = &registry.emplace<animator_component>(_entity)
                     .motion_bone_index(4)
                     .skeleton(fetch_skeleton("assets/player_skeleton.bin"))
                     .animations({
                         { 444, fetch_animation("assets/player_animation_AnimLol.bin") },
                     });

    _controller = &registry.emplace<controller_component<runner_actor>>(_entity)
                       .state(this);
}

void runner_actor::add_script(const std::function<void(runner_actor&)>& script)
{
    _scripts.emplace_back(script);
}

void runner_actor::update()
{
    for (std::function<void(runner_actor&)>& _script : _scripts) {
        _script(*this);
    }
}

transform_component& runner_actor::get_transform()
{
    return *_transform;
}
