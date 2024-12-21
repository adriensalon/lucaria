#include <game/actor/environment_static.hpp>
#include <game/scene/zone_flight.hpp>

zone_flight_scene::zone_flight_scene(scene_data& scene)
{
    scene.make_actor<environment_static_actor>(scene, "6kqP").get_collider().wall();
    scene.make_actor<environment_static_actor>(scene, "8Rr6").get_collider().wall();
    scene.make_actor<environment_static_actor>(scene, "udJv").get_collider().wall();
    scene.make_actor<environment_static_actor>(scene, "9ETQ").get_collider().wall();
    scene.make_actor<environment_static_actor>(scene, "D6q9").get_collider().wall();
    scene.make_actor<environment_static_actor>(scene, "8jHH").get_collider().wall();
    scene.make_actor<environment_static_actor>(scene, "fXbl").get_collider().wall();
    scene.make_actor<environment_static_actor>(scene, "8Ijp").get_collider().wall();
    scene.make_actor<environment_static_actor>(scene, "UQZZ").get_collider().ground();
}