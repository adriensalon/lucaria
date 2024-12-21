#include <game/actor/environment_static.hpp>

environment_static_actor::environment_static_actor(scene_data& scene, const std::string& uuid)
{
    const entt::entity _entity = scene.components.create();
    
    scene.components.emplace<model_component<model_shader::unlit>>(_entity)
        .color(fetch_texture("assets/image/image_" + uuid + ".bin"))
        .mesh(fetch_mesh("assets/geometry/geometry_" + uuid + ".bin"));
    _collider = std::ref(scene.components.emplace<collider_component>(_entity));
    _collider.value().get().shape(fetch_shape("assets/shape/geometry_" + uuid + ".bin", shape_type::triangle_mesh));
}

collider_component& environment_static_actor::get_collider()
{
    return _collider.value().get();
}