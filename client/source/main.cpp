
#include <core/window.hpp>
#include <core/world.hpp>

#include <ecs/system/dynamics.hpp>
#include <ecs/system/interface.hpp>
#include <ecs/system/mixer.hpp>
#include <ecs/system/motion.hpp>
#include <ecs/system/player.hpp>
#include <ecs/system/rendering.hpp>

#include <game/actor/character_runner.hpp>
#include <game/scene/menu_splash.hpp>

int main()
{
    make_scene<menu_splash_scene>();

    run([]() {

        player_system::update();

        rendering_system::clear_debug_guizmos();
        rendering_system::clear_screen();
        rendering_system::compute_projection();

        interface_system::collect_gui_widgets();

        motion_system::advance_controllers();
        motion_system::apply_animations();
        motion_system::apply_motion_tracks();
        motion_system::collect_debug_guizmos();

        dynamics_system::step_simulation();
        dynamics_system::compute_kinematic_collisions();
        dynamics_system::collect_debug_guizmos();

        rendering_system::compute_view_projection();
        rendering_system::draw_skybox();
        rendering_system::draw_blockout_meshes();
        rendering_system::draw_unlit_meshes();
        rendering_system::draw_debug2();

        mixer_system::apply_speaker_transforms();
        mixer_system::apply_listener_transform();

        each_scene([](scene_data& scene) {
            scene.update_actors<character_runner_actor>();
        });

    });

    return 0;
}