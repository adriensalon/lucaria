#include <ecs/system/computer.hpp>
#include <ecs/system/mixer.hpp>
#include <ecs/system/player.hpp>
#include <ecs/system/rendering.hpp>
#include <ecs/system/splash.hpp>
#include <ecs/system/world.hpp>

#include <glue/window.hpp>
#include <glue/fetch.hpp>

#include <levels/levels.hpp>

int main()
{
    splash_system::splash_texture(std::move(fetch_texture("assets/splash_texture.bin")));
    splash_system::trigger_splash(true);

    player_system::player_position(glm::vec3(0.f, 1.8f, 3.f));
    player_system::player_direction(glm::vec3(0.f, 0.f, -1.f));
    player_system::player_height(1.83f);
    player_system::player_radius(0.2f);

    // rendering_system::cubemap_skybox(std::move(fetch_cubemap(
    //     "",
    //     "",
    //     "")));

    world_system::register_level("001_room", register_level_001_room);
    world_system::add_level("001_room");

    run(update);
    return 0;
}