#include <fstream>

#include <lucaria/entity/game.hpp>

namespace lucaria {
namespace detail {

    namespace {

        template <typename ComponentType>
        [[nodiscard]] std::vector<scene_component_recipe<ComponentType>> _save_component_group(entt::registry* registry, const std::unordered_map<entt::entity, uint32>& entity_ids)
        {
            std::vector<scene_component_recipe<ComponentType>> _components = {};
            registry->view<ComponentType>().each(
                [&](entt::entity entity, ComponentType& component) {
                    scene_component_recipe<ComponentType>& _back = _components.emplace_back();
                    _back.entity = entity_ids.at(entity);
                    _back.component = &component;
                });
            return _components;
        }
    }

    game_context& engine_context()
    {
        static game_context _engine_context = {};
        return _engine_context;
    }

}

void game_context::save_snapshot(const std::filesystem::path& path)
{
    std::ofstream _ofstream(path, std::ios::binary);
    // cereal::PortableBinaryOutputArchive _archive(_ofstream);
    cereal::JSONOutputArchive _archive(_ofstream);

    // objects
    detail::implementation_save_database _save_database = {};
    detail::recipe_save_database _save_recipes = detail::engine_resources().make_all_recipes(_save_database);
    _archive(cereal::make_nvp("objects", _save_recipes));

	detail::scene_database& _scene_save_database = detail::engine_scene_database();
    _scene_save_database.save_map_scene_ids.clear();
    _scene_save_database.save_map_scene_entities.clear();
    uint32 _next_scene_id = 1;
    for (detail::scene_implementation& _scene : detail::engine_scenes()) {
        entt::registry* _registry = _scene.components.get();
        _scene_save_database.save_map_scene_ids.emplace(_registry, _next_scene_id++);
        std::unordered_map<entt::entity, uint32>& _entity_ids = _scene_save_database.save_map_scene_entities[_registry];
        uint32 _next_entity_id = 1;
        for (entt::entity _entity : _registry->storage<entt::entity>()) {
            _entity_ids.emplace(_entity, _next_entity_id++);
        }
    }

    // scenes
    detail::engine_scene_database().objects_save_database = &_save_database;
    std::vector<detail::scene_recipe> _scene_saves = {};
    std::unordered_map<std::string, detail::scene_type_implementation>& _scene_types = detail::engine_scene_types();
    for (detail::scene_implementation& _scene : detail::engine_scenes()) {
        const std::unordered_map<entt::entity, uint32>& _entity_ids = detail::engine_scene_database().save_map_scene_entities.at(_scene.components.get());
        detail::scene_recipe& _scene_save = _scene_saves.emplace_back();
        _scene_save.type_id = _scene.type_id;
        _scene_save.scene = &_scene;
        _scene_save.components.animators = detail::_save_component_group<animator_component>(_scene.components.get(), _entity_ids);
        _scene_save.components.screen_interfaces = detail::_save_component_group<screen_interface_component>(_scene.components.get(), _entity_ids);
        _scene_save.components.spatial_interfaces = detail::_save_component_group<spatial_interface_component>(_scene.components.get(), _entity_ids);
        _scene_save.components.blockout_models = detail::_save_component_group<blockout_model_component>(_scene.components.get(), _entity_ids);
        _scene_save.components.unlit_models = detail::_save_component_group<unlit_model_component>(_scene.components.get(), _entity_ids);
        _scene_save.components.passive_rigidbodies = detail::_save_component_group<passive_rigidbody_component>(_scene.components.get(), _entity_ids);
        _scene_save.components.kinematic_rigidbodies = detail::_save_component_group<kinematic_rigidbody_component>(_scene.components.get(), _entity_ids);
        _scene_save.components.dynamic_rigidbodies = detail::_save_component_group<dynamic_rigidbody_component>(_scene.components.get(), _entity_ids);
        _scene_save.components.speakers = detail::_save_component_group<speaker_component>(_scene.components.get(), _entity_ids);
        _scene_save.components.transforms = detail::_save_component_group<transform_component>(_scene.components.get(), _entity_ids);
    }
    _archive(cereal::make_nvp("scenes", _scene_saves));
    detail::engine_scene_database().objects_save_database = nullptr;
}

void game_context::load_snapshot(const std::filesystem::path& path)
{
    std::ifstream _ifstream(path, std::ios::binary);
    // cereal::PortableBinaryInputArchive _archive(_ifstream);
    cereal::JSONInputArchive _archive(_ifstream);

    // objects
    detail::recipe_save_database _save_recipes = {};
    _archive(cereal::make_nvp("objects", _save_recipes));
    detail::object_save_database _save_objects = detail::engine_resources().apply_recipes(std::move(_save_recipes));

    // scenes
    std::unordered_map<std::string, detail::scene_type_implementation>& _scene_types = detail::engine_scene_types();
    for (detail::scene_implementation& _scene : detail::engine_scenes()) {
        // components TODO LATER

        detail::scene_type_implementation& _scene_type = _scene_types.at(_scene.type_id);
        // _scene_type.binary_load(_scene, _archive);
        _scene_type.json_load(_scene, _archive);
    }
}

}