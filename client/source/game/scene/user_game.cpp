#include <ecs/system/rendering.hpp>
#include <ecs/system/mixer.hpp>
#include <game/actor/character_runner.hpp>
#include <game/actor/menu_splash.hpp>
#include <game/scene/user_game.hpp>

user_game_scene::user_game_scene(scene_data& scene)
{
    rendering_system::use_skybox_cubemap(fetch_cubemap({ 
        "assets/cubemap/cubemap_px_eLVJ.bin",
        "assets/cubemap/cubemap_py_eLVJ.bin",
        "assets/cubemap/cubemap_pz_eLVJ.bin",
        "assets/cubemap/cubemap_nx_eLVJ.bin",
        "assets/cubemap/cubemap_ny_eLVJ.bin",
        "assets/cubemap/cubemap_nz_eLVJ.bin" }));
        
    scene.make_actor<character_runner_actor>(scene);

    // mixer_system::use_listener_transform(faith->get_transform());


    

}