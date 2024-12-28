#include <game/actor/menu_splash.hpp>
#include <game/scene/user_ui.hpp>

user_ui_scene::user_ui_scene(scene_data& scene)
{
    scene.make_actor<menu_splash_actor>(scene);
}