
#include <ecs/system/async.hpp>
#include <ecs/system/sampling.hpp>
#include <ecs/system/skinning.hpp>
#include <ecs/system/motion.hpp>
#include <ecs/system/dynamics.hpp>
#include <ecs/system/collision.hpp>
#include <ecs/system/player.hpp>
#include <ecs/system/rendering.hpp>
#include <ecs/system/splash.hpp>
#include <ecs/system/mixer.hpp>
#include <ecs/system/world.hpp>

#include <glue/window.hpp>
#include <levels/__lifetimelong.cpp>
#include <levels/room_001.cpp>

int main()
{
    world_system::register_level("lifetimelong", register_lifetimelong);
    world_system::register_level("001_room", register_level_001_room);

    world_system::add_level("lifetimelong");
    world_system::add_level("001_room");

    run([] () {

        async_system::update();
        // sampling_system::update();
        // skinning_system::update();
        motion_system::update();
        // dynamics_system::update();
        // collision_system::update();
        player_system::update();
        rendering_system::update();
        splash_system::update();
        mixer_system::update();

    });
    return 0;
}