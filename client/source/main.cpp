
#include <core/window.hpp>
#include <core/world.hpp>

#include <ecs/system/dynamics.hpp>
#include <ecs/system/interface.hpp>
#include <ecs/system/motion.hpp>
#include <ecs/system/mixer.hpp>
#include <ecs/system/player.hpp>
#include <ecs/system/rendering.hpp>
#include <ecs/system/scripting.hpp>

#include <game/gameplay/runner.hpp>
#include <game/levels/levels.hpp>

int main()
{
    register_level(levelID_menu_splash, level_menu_splash);
    register_level(levelID_persistent_player, level_persistent_player);
    register_level(levelID_blockout_test, level_blockout_test);

    add_level(levelID_menu_splash);
    add_level(levelID_persistent_player);
    add_level(levelID_blockout_test);
    

    run([]() {
        player_system::update();
        
        rendering_system::clear_debug_guizmos();
        rendering_system::clear_screen();
        rendering_system::compute_projection();        
        
        scripting_system::resolve_controller_states();

        interface_system::collect_gui_widgets();
        
        motion_system::blend_animations();
        motion_system::apply_root_motion();
        motion_system::collect_debug_guizmos();

        dynamics_system::step_simulation();
        dynamics_system::compute_kinematic_collisions();
        dynamics_system::collect_debug_guizmos();
        
        motion_system::apply_foot_ik();
        motion_system::skin_meshes();

        rendering_system::compute_view_projection();
        rendering_system::draw_skybox();
        rendering_system::draw_blockout_meshes();
        rendering_system::draw_unlit_meshes();
        rendering_system::draw_debug_guizmos();
        
        mixer_system::apply_speaker_transforms();
        mixer_system::apply_listener_transform();

    });
    return 0;
}