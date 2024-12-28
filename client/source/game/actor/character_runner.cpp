#include <core/window.hpp>
#include <ecs/component/model.hpp>
#include <ecs/component/rigidbody.hpp>
#include <ecs/component/speaker.hpp>
#include <ecs/system/rendering.hpp>
#include <game/actor/character_runner.hpp>

character_runner_actor::character_runner_actor(scene_data& scene)
{
    entt::entity _entity = scene.components.create();

    scene.components.emplace<kinematic_rigidbody_component>(_entity)
                      .collide_walls()
                      .collide_grounds()
                      .collide_layer(kinematic_layer::layer_0)
                      .capsule(0.5f, 1.75f);

    scene.components.emplace<model_component<model_shader::unlit>>(_entity)
                  .color(fetch_texture("assets/image/sophie_color.bin", "assets/image/sophie_color_etc.bin", "assets/image/sophie_color_s3tc.bin"))
                  .mesh(fetch_mesh("assets/character/testanim.bin"));

    scene.components.emplace<speaker_component>(_entity)
                    .sounds({ { 666, fetch_sound("assets/audio/audio_cFoJ.bin") } });

    _animator = scene.components.emplace<animator_component>(_entity)
                     .skeleton(fetch_skeleton("assets/character/testanim_skeleton.bin"))
                     .animation(444, fetch_animation("assets/character/testanim_animation_walk.bin"))
                     .motion_track(444, fetch_motion_track("assets/character/testanim_animation_walk_motion_track.bin"));

    _transform = scene.components.emplace<transform_component>(_entity)
        .rotation_relative(glm::vec3(0.f, glm::radians(90.f), 0.f));

    rendering_system::use_camera_transform(_transform.value().get());
    rendering_system::use_camera_bone(_animator.value().get(), "mixamorig9:Head");
}

void character_runner_actor::update()
{
    static bool _is_playing = false;
    if (get_keys()[keyboard_key::w] && !_is_playing) {
        _animator.value().get().get_controller(444).play();
        _is_playing = true;
    } else if (!get_keys()[keyboard_key::w] && _is_playing) {
        _animator.value().get().get_controller(444).pause();
        _is_playing = false;
    }

    const glm::mat4 _model(1.f);
    gui_mvp(rendering_system::get_projection() * rendering_system::get_view() * _model);
    get_gui_drawlist()->AddText({ 0, 0 }, ImGui::ColorConvertFloat4ToU32({ 0.f, 0.f, 0.f, 1.f }), "MON TEXTE 3D");
    gui_mvp(std::nullopt);
}

animator_component& character_runner_actor::get_animator()
{
    return _animator.value().get();
}

transform_component& character_runner_actor::get_transform()
{
    return _transform.value().get();
}
