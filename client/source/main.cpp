
#include <core/window.hpp>
#include <core/world.hpp>

#include <ecs/system/async.hpp>
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
        
        rendering_system::clear_screen();
        rendering_system::compute_projection();

        async_system::update();
        

        player_system::update(); //
        // scripting_system::resolve_controller_states();


        motion_system::blend_animations();
        motion_system::apply_root_motion(); //

        dynamics_system::prevent_kinematic_wall_collisions(); //
        dynamics_system::snap_kinematic_grounds(); //
        
        motion_system::apply_foot_ik();
        motion_system::skin_meshes();

        // rendering_system::compute_view();
        rendering_system::draw_skybox();
        rendering_system::draw_meshes();

        splash_system::update();
    });
    return 0;
}