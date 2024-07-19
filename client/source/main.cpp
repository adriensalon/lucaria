
#include <core/window.hpp>
#include <core/world.hpp>

#include <ecs/system/dynamics.hpp>
#include <ecs/system/motion.hpp>
#include <ecs/system/player.hpp>
#include <ecs/system/rendering.hpp>
#include <ecs/system/scripting.hpp>
#include <ecs/system/splash.hpp>

#include <game/gameplay/cop.hpp>
#include <game/gameplay/runner.hpp>
#include <game/levels/levels.hpp>

int main()
{
    register_level(levelID_persistent_player, level_persistent_player);
    register_level(levelID_blockout_test, level_blockout_test);

    add_level(levelID_persistent_player);
    add_level(levelID_blockout_test);

    run([]() {
        player_system::update(); //
        
        rendering_system::clear_screen();
        rendering_system::compute_projection();
        

        scripting_system::resolve_controller_states();


        motion_system::blend_animations();
        motion_system::apply_root_motion(); //

        dynamics_system::apply_transforms();
        dynamics_system::compute_wall_slide();
        dynamics_system::compute_ground_snap();
        dynamics_system::compute_dynamics();
        dynamics_system::compute_layers();
        dynamics_system::collect_debug_guizmos();
        
        motion_system::apply_foot_ik();
        motion_system::skin_meshes();

        rendering_system::compute_view_projection();
        rendering_system::draw_skybox();
        rendering_system::draw_blockout_meshes();
        rendering_system::draw_unlit_meshes();
        rendering_system::draw_guizmos();

        splash_system::update();
    });
    return 0;
}