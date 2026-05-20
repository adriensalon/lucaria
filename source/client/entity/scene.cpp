#include <lucaria/entity/scene.hpp>
#include <lucaria/entity/game.hpp>

namespace lucaria {
namespace detail {

    std::vector<scene_implementation>& engine_scenes()
    {
        static std::vector<scene_implementation> _engine_scenes;
        return _engine_scenes;
    }

    std::unordered_map<std::string, scene_type_implementation>& engine_scene_types()
    {
        static std::unordered_map<std::string, scene_type_implementation> _engine_scene_types;
        return _engine_scene_types;
    }

    std::unordered_map<std::type_index, std::string>& engine_scene_type_ids()
    {
        static std::unordered_map<std::type_index, std::string> _engine_scene_type_ids;
        return _engine_scene_type_ids;
    }

    void update_callbacks()
    {
        for (scene_implementation& _scene : engine_scenes()) {
            engine_context().scene._self_scene = &_scene;
            scene_type_implementation& _scene_type = engine_scene_types().at(_scene.type_id);
			if (_scene_type.update) {
				_scene_type.update(_scene);
			}
        }
    }

}

scene_entity scene_context::emplace_entity()
{
    return _self_scene->components.create();
}

void scene_context::mark_erase_entity(const scene_entity entity)
{
    _self_scene->entities_marked_erase.emplace_back(entity);
}

void scene_context::mark_erase_self()
{
    _self_scene->is_marked_erase = true;
}

animator_component& scene_context::emplace_animator(const scene_entity entity)
{
    return _self_scene->components.emplace<animator_component>(entity);
}

screen_interface_component& scene_context::emplace_screen_interface(const scene_entity entity)
{
    return _self_scene->components.emplace<screen_interface_component>(entity);
}

spatial_interface_component& scene_context::emplace_spatial_interface(const scene_entity entity)
{
    return _self_scene->components.emplace<spatial_interface_component>(entity);
}

blockout_model_component& scene_context::emplace_blockout_model(const scene_entity entity)
{
    return _self_scene->components.emplace<blockout_model_component>(entity);
}

unlit_model_component& scene_context::emplace_unlit_model(const scene_entity entity)
{
    return _self_scene->components.emplace<unlit_model_component>(entity);
}

passive_rigidbody_component& scene_context::emplace_passive_rigidbody(const scene_entity entity)
{
    return _self_scene->components.emplace<passive_rigidbody_component>(entity);
}

kinematic_rigidbody_component& scene_context::emplace_kinematic_rigidbody(const scene_entity entity)
{
    return _self_scene->components.emplace<kinematic_rigidbody_component>(entity);
}

dynamic_rigidbody_component& scene_context::emplace_dynamic_rigidbody(const scene_entity entity)
{
    return _self_scene->components.emplace<dynamic_rigidbody_component>(entity);
}

speaker_component& scene_context::emplace_speaker(const scene_entity entity)
{
    return _self_scene->components.emplace<speaker_component>(entity);
}

transform_component& scene_context::emplace_transform(const scene_entity entity)
{
    return _self_scene->components.emplace<transform_component>(entity);
}

}