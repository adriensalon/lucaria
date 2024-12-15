#include <game/gameplay/runner.hpp>

runner_actor::runner_actor(entt::registry& registry)
{
    _entity = registry.create();

    _transform = &registry.emplace<transform_component>(_entity)
        .rotation_relative(glm::vec3(0.f, glm::radians(90.f), 0.f));

    _rigidbody = &registry.emplace<kinematic_rigidbody_component>(_entity)
                      .collide_walls()
                      .collide_grounds()
                      .collide_layer(kinematic_layer::layer_0)
                      .capsule(0.5f, 1.75f);

    _model = &registry.emplace<model_component<model_shader::unlit>>(_entity)
                  .color(fetch_texture("assets/image/sophie_color.bin", "assets/image/sophie_color_etc.bin", "assets/image/sophie_color_s3tc.bin"))
                  .mesh(fetch_mesh("assets/character/sophie.bin"));

    _speaker = &registry.emplace<speaker_component>(_entity)
                    .sounds({ { 666, fetch_sound("assets/audio/audio_Fcs8.bin") } });

    _animator = &registry.emplace<animator_component>(_entity)
                     .skeleton(fetch_skeleton("assets/character/sophie_skeleton.bin"))
                     .animation(444, fetch_animation("assets/character/sophie_animation_AnimOk.bin"))
                     .motion_track(444, fetch_motion_track("assets/character/sophie_animation_AnimOk_motion_track.bin"));

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

animator_component& runner_actor::get_animator()
{
    return *_animator;
}

transform_component& runner_actor::get_transform()
{
    return *_transform;
}
