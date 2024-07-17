
#include <ecs/system/async.hpp>
#include <ecs/system/dynamics.hpp>
#include <ecs/system/motion.hpp>
#include <ecs/system/player.hpp>
#include <ecs/system/rendering.hpp>
#include <ecs/system/splash.hpp>
#include <ecs/system/world.hpp>

#include <glue/window.hpp>
#include <levels/__lifetimelong.cpp>
#include <levels/room_001.cpp>

int main()
{
    world_system::register_level("lifetimelong", register_lifetimelong);
    // world_system::register_level("001_room", register_level_001_room);

    world_system::add_level("lifetimelong");
    // world_system::add_level("001_room");

    run([]() {
        
        rendering_system::clear_screen();
        rendering_system::compute_projection();

        async_system::update();
        
        
        player_system::update();


        motion_system::blend_animations();
        motion_system::apply_root_motion();

        dynamics_system::prevent_kinematic_wall_collisions();
        dynamics_system::snap_kinematic_grounds();
        
        motion_system::apply_foot_ik();
        motion_system::skin_meshes();

        rendering_system::draw_skybox();
        rendering_system::draw_meshes();

        splash_system::update();
    });
    return 0;
}