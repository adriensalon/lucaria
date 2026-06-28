#pragma once

#include <cassert>
#include <limits>

#include <lucaria/core/manager_scenes.hpp>
#include <lucaria/core/serialize_containers.hpp>
#include <lucaria/core/serialize_mappings.hpp>
#include <lucaria/core/user_components.hpp>
#include <lucaria/core/user_scenes.hpp>
#include <lucaria/engine/component_animator.hpp>
#include <lucaria/engine/component_interface.hpp>
#include <lucaria/engine/component_model.hpp>
#include <lucaria/engine/component_rigidbody.hpp>
#include <lucaria/engine/component_speaker.hpp>
#include <lucaria/engine/component_transform.hpp>

namespace lucaria {
namespace detail {

    template <>
    struct entt_emplace_factory<component_rigidbody_passive> {

        template <typename ArchiveType>
        static component_rigidbody_passive& emplace(ArchiveType& archive, storage_registry& registry, object_entity entity)
        {
            const mappings_manager_game_load& _mappings = cereal::get_user_data<mappings_manager_game_load>(archive);
            LUCARIA_DEBUG_ASSERT(_mappings.dynamics != nullptr, "Missing dynamics while loading component_rigidbody_passive");
            return registry.emplace_or_replace<component_rigidbody_passive>(entity, *_mappings.dynamics);
        }
    };

    template <>
    struct entt_emplace_factory<component_rigidbody_kinematic> {

        template <typename ArchiveType>
        static component_rigidbody_kinematic& emplace(ArchiveType& archive, storage_registry& registry, object_entity entity)
        {
            const mappings_manager_game_load& _mappings = cereal::get_user_data<mappings_manager_game_load>(archive);
            LUCARIA_DEBUG_ASSERT(_mappings.dynamics != nullptr, "Missing dynamics while loading component_rigidbody_kinematic");
            return registry.emplace_or_replace<component_rigidbody_kinematic>(entity, *_mappings.dynamics);
        }
    };

    template <>
    struct entt_emplace_factory<component_rigidbody_dynamic> {

        template <typename ArchiveType>
        static component_rigidbody_dynamic& emplace(ArchiveType& archive, storage_registry& registry, object_entity entity)
        {
            const mappings_manager_game_load& _mappings = cereal::get_user_data<mappings_manager_game_load>(archive);
            LUCARIA_DEBUG_ASSERT(_mappings.dynamics != nullptr, "Missing dynamics while loading component_rigidbody_dynamic");
            return registry.emplace_or_replace<component_rigidbody_dynamic>(entity, *_mappings.dynamics);
        }
    };

    //

    template <typename ComponentType>
    struct snapshot_object_scene_component {
        uint32 component_save_id = 0;
        ComponentType* component_save = nullptr;

        template <typename ArchiveType>
        void save(ArchiveType& archive) const
        {
            archive(cereal::make_nvp("component_save_id", component_save_id));
            LUCARIA_DEBUG_ASSERT(component_save != nullptr, "Missing component while saving scene component snapshot");
            if constexpr (has_user_component_save_game_v<ComponentType>) {
                mappings_manager_game_save& _mappings = cereal::get_user_data<mappings_manager_game_save>(archive);
                LUCARIA_DEBUG_ASSERT(_mappings.saving_objects != nullptr && _mappings.scenes.saving_scene_manager != nullptr, "Missing game context while saving component");
                context_save_game _context { archive, *_mappings.saving_objects, *_mappings.scenes.saving_scene_manager, _mappings };
                _context.field("component", *component_save);
            } else {
                archive(cereal::make_nvp("component", *component_save));
            }
        }

        template <typename ArchiveType>
        void load(ArchiveType& archive)
        {
            archive(cereal::make_nvp("component_save_id", component_save_id));
            mappings_manager_game_load& _mappings = cereal::get_user_data<mappings_manager_game_load>(archive);
            LUCARIA_DEBUG_ASSERT(_mappings.loading_scene != nullptr && _mappings.loading_scene_manager != nullptr, "Missing scene while loading component");
            object_entity _entity = _mappings.scenes.load_map_scene_entities.at(_mappings.loading_scene_save_id).at(component_save_id);
            ComponentType& _component = entt_emplace_factory<ComponentType>::emplace(archive, _mappings.loading_scene_manager->registry, _entity);
            if constexpr (has_user_component_load_game_v<ComponentType>) {
                LUCARIA_DEBUG_ASSERT(_mappings.loading_objects != nullptr, "Missing manager_assets while loading component");
                context_load_game _context { archive, *_mappings.loading_objects, *_mappings.loading_scene_manager, _mappings.dynamics, _mappings };
                _context.field("component", _component);
            } else {
                archive(cereal::make_nvp("component", _component));
            }
        }
    };

    struct snapshot_object_scene_registry {
        std::vector<snapshot_object_scene_component<component_animator>> animators = {};
        std::vector<snapshot_object_scene_component<component_interface_screen>> screen_interfaces = {};
        std::vector<snapshot_object_scene_component<component_interface_spatial>> spatial_interfaces = {};
        std::vector<snapshot_object_scene_component<component_model_blockout>> blockout_models = {};
        std::vector<snapshot_object_scene_component<component_model_unlit>> unlit_models = {};
        std::vector<snapshot_object_scene_component<component_rigidbody_passive>> passive_rigidbodies = {};
        std::vector<snapshot_object_scene_component<component_rigidbody_kinematic>> kinematic_rigidbodies = {};
        std::vector<snapshot_object_scene_component<component_rigidbody_dynamic>> dynamic_rigidbodies = {};
        std::vector<snapshot_object_scene_component<component_speaker_spatial>> speakers = {};
        std::vector<snapshot_object_scene_component<component_transform>> transforms = {};
        std::vector<uint32> entity_save_ids = {};

        template <typename ArchiveType, typename RegistryType>
        static void serialize_components(ArchiveType& archive, RegistryType& registry)
        {
            archive(cereal::make_nvp("animators", registry.animators));
            archive(cereal::make_nvp("screen_interfaces", registry.screen_interfaces));
            archive(cereal::make_nvp("spatial_interfaces", registry.spatial_interfaces));
            archive(cereal::make_nvp("blockout_models", registry.blockout_models));
            archive(cereal::make_nvp("unlit_models", registry.unlit_models));
            archive(cereal::make_nvp("passive_rigidbodies", registry.passive_rigidbodies));
            archive(cereal::make_nvp("kinematic_rigidbodies", registry.kinematic_rigidbodies));
            archive(cereal::make_nvp("dynamic_rigidbodies", registry.dynamic_rigidbodies));
            archive(cereal::make_nvp("speakers", registry.speakers));
            archive(cereal::make_nvp("transforms", registry.transforms));
        }

        template <typename ArchiveType>
        void save(ArchiveType& archive) const
        {
            archive(cereal::make_nvp("entity_save_ids", entity_save_ids));
            serialize_components(archive, *this);
        }

        template <typename ArchiveType>
        void load(ArchiveType& archive)
        {
            archive(cereal::make_nvp("entity_save_ids", entity_save_ids));
            mappings_manager_game_load& _mappings = cereal::get_user_data<mappings_manager_game_load>(archive);
			LUCARIA_DEBUG_ASSERT(_mappings.loading_scene != nullptr && _mappings.loading_scene_manager != nullptr, "Missing scene while loading scene registry");
            std::unordered_map<uint32, object_entity>& _entity_map = _mappings.scenes.load_map_scene_entities[_mappings.loading_scene_save_id];
            for (uint32 _entity_save_id : entity_save_ids) {
                object_entity _entity = _mappings.loading_scene_manager->registry.create(_mappings.loading_scene->index_for_context);
                _entity_map[_entity_save_id] = _entity;
            }
            serialize_components(archive, *this);
        }
    };

    struct snapshot_object_scene_user_component_group {
        std::string type_id = {};
        object_user_scene* scene = nullptr;
        user_component_type_callbacks* component_type_callbacks = nullptr;

        template <typename ArchiveType>
        void save(ArchiveType& archive) const
        {
            archive(cereal::make_nvp("type_id", type_id));
            save_registered_object(archive, *component_type_callbacks, *scene);
        }

        template <typename ArchiveType>
        void load(ArchiveType& archive)
        {
            archive(cereal::make_nvp("type_id", type_id));

            auto& game_mappings = cereal::get_user_data<mappings_manager_game_load>(archive);

            if (game_mappings.loading_scene_manager == nullptr || game_mappings.loading_scene == nullptr) {
                LUCARIA_DEBUG_ERROR("Missing scene context while loading user component group");
                return;
            }

            auto it = game_mappings.loading_scene_manager->user_component_types.find(type_id);
            if (it == game_mappings.loading_scene_manager->user_component_types.end()) {
                LUCARIA_DEBUG_ERROR("Unknown user component type while loading snapshot");
                return;
            }

            load_registered_object(archive, it->second, *game_mappings.loading_scene, "user_component_group", type_id);
        }
    };

    struct snapshot_object_scene {
        uint32 scene_save_id = 0;
        std::string type_id = {};
        snapshot_object_scene_registry components = {};
        std::vector<snapshot_object_scene_user_component_group> user_components = {};
        object_user_scene* scene = nullptr;
        user_scene_type_callbacks* scene_type_callbacks = nullptr;

        template <typename ArchiveType>
        void save(ArchiveType& archive) const
        {
            archive(cereal::make_nvp("scene_save_id", scene_save_id));
            archive(cereal::make_nvp("type_id", type_id));
            archive(cereal::make_nvp("components", components));
            archive(cereal::make_nvp("user_components", user_components));
            save_registered_object(archive, *scene_type_callbacks, *scene);
        }

        template <typename ArchiveType>
        void load(ArchiveType& archive)
        {
            archive(cereal::make_nvp("scene_save_id", scene_save_id));
            archive(cereal::make_nvp("type_id", type_id));

            auto& game_mappings = cereal::get_user_data<mappings_manager_game_load>(archive);

            if (game_mappings.loading_scene_manager == nullptr) {
                LUCARIA_DEBUG_ERROR("Missing manager_scene while loading scene");
                return;
            }

            manager_scenes& manager = *game_mappings.loading_scene_manager;

            auto it = manager.scene_types.find(type_id);
            if (it == manager.scene_types.end()) {
                LUCARIA_DEBUG_ERROR("Unknown scene type while loading snapshot");
                return;
            }

            const auto scene_index = static_cast<object_entity_scene_index>(manager.scenes.size());
            assert(manager.scenes.size() <= std::numeric_limits<object_entity_scene_index>::max());

            object_user_scene& loaded_scene = manager.scenes.emplace_back();
            loaded_scene.type_id = type_id;
            loaded_scene.index_for_context = scene_index;

            if (it->second.construct) {
                it->second.construct(loaded_scene);
            }

            game_mappings.scenes.load_map_scenes[scene_save_id] = loaded_scene.index_for_context;

            object_user_scene* previous_scene = game_mappings.loading_scene;
            uint32 previous_scene_id = game_mappings.loading_scene_save_id;

            game_mappings.loading_scene = &loaded_scene;
            game_mappings.loading_scene_save_id = scene_save_id;

            archive(cereal::make_nvp("components", components));
            archive(cereal::make_nvp("user_components", user_components));

            load_registered_object(archive, it->second, loaded_scene, "user_scene", type_id);

            game_mappings.loading_scene = previous_scene;
            game_mappings.loading_scene_save_id = previous_scene_id;
        }
    };

    struct snapshot_manager_scene {
        std::vector<snapshot_object_scene> scenes = {};

        template <typename ArchiveType>
        void serialize(ArchiveType& archive)
        {
            archive(cereal::make_nvp("scenes", scenes));
        }
    };

    //

    [[nodiscard]] snapshot_object_scene make_snapshot(const context_game& game, object_user_scene& scene, const user_scene_type_callbacks& callbacks);
    [[nodiscard]] snapshot_manager_scene make_snapshot(manager_scenes& manager, mappings_manager_scene_save& mappings);

    struct snapshot_scenes {
        manager_scenes& scenes;

        template <typename ArchiveType>
        void save(ArchiveType& archive) const
        {
            mappings_manager_game_save& mappings = cereal::get_user_data<mappings_manager_game_save>(archive);

            if (mappings.saving_objects == nullptr) {
                LUCARIA_DEBUG_ERROR("Missing manager_assets while saving snapshot scenes");
                return;
            }

            context_save_game context { archive, *mappings.saving_objects, scenes, mappings };
            scenes.save(context);
        }

        template <typename ArchiveType>
        void load(ArchiveType& archive)
        {
            mappings_manager_game_load& mappings = cereal::get_user_data<mappings_manager_game_load>(archive);

            if (mappings.loading_objects == nullptr || mappings.loading_scene_manager == nullptr) {
                LUCARIA_DEBUG_ERROR("Missing game managers while loading snapshot scenes");
                return;
            }

            context_load_game context { archive, *mappings.loading_objects, scenes, mappings.dynamics, mappings };
            scenes.load(context);
        }
    };

    //

    // declared in manager_scenes.hpp
    template <typename ComponentType>
    void manager_scenes::register_user_component(std::string type_id)
    {
        user_component_type_callbacks _component_type = {};

        if constexpr (!traits::component_compute_enable_v<ComponentType>) { // TODO implement this one is fun ^^

            _component_type.binary_save = [](object_user_scene& scene, cereal::PortableBinaryOutputArchive& archive) {
                const mappings_manager_game_save& _mappings = cereal::get_user_data<mappings_manager_game_save>(archive);
                std::vector<snapshot_object_scene_component<ComponentType>> _components = {};
                assert(_mappings.scenes.saving_scene_manager != nullptr);
                _mappings.scenes.saving_scene_manager->registry.view<ComponentType>(scene.index_for_context, exclude<>).each([&](object_entity entity, ComponentType& component) {
                    snapshot_object_scene_component<ComponentType>& _back = _components.emplace_back();
                    _back.component_save_id = _mappings.scenes.save_map_scene_entities.at(scene.index_for_context).at(entity);
                    _back.component_save = &component;
                });
                archive(cereal::make_nvp("components", _components));
            };

            _component_type.binary_load = [](object_user_scene& scene, cereal::PortableBinaryInputArchive& archive) {
                std::vector<snapshot_object_scene_component<ComponentType>> components = {};
                archive(cereal::make_nvp("components", components));
            };

            _component_type.json_save = [](object_user_scene& scene, cereal::JSONOutputArchive& archive) {
                const mappings_manager_game_save& _mappings = cereal::get_user_data<mappings_manager_game_save>(archive);
                std::vector<snapshot_object_scene_component<ComponentType>> _components = {};
                assert(_mappings.scenes.saving_scene_manager != nullptr);
                _mappings.scenes.saving_scene_manager->registry.view<ComponentType>(scene.index_for_context, exclude<>).each([&](object_entity entity, ComponentType& component) {
                    snapshot_object_scene_component<ComponentType>& _back = _components.emplace_back();
                    _back.component_save_id = _mappings.scenes.save_map_scene_entities.at(scene.index_for_context).at(entity);
                    _back.component_save = &component;
                });
                archive(cereal::make_nvp("components", _components));
            };

            _component_type.json_load = [](object_user_scene& scene, cereal::JSONInputArchive& archive) {
                std::vector<snapshot_object_scene_component<ComponentType>> components = {};
                archive(cereal::make_nvp("components", components));
            };
        }

        user_component_type_ids[std::type_index(typeid(ComponentType))] = type_id;
        user_component_types[std::move(type_id)] = std::move(_component_type);
    }
}
}
