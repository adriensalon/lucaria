#pragma once

#include <lucaria/core/serialize_containers.hpp>
#include <lucaria/core/serialize_mappings.hpp>
#include <lucaria/core/user_scene.hpp>
#include <lucaria/public/component_animator.hpp>
#include <lucaria/public/component_interface.hpp>
#include <lucaria/public/component_model.hpp>
#include <lucaria/public/component_rigidbody.hpp>
#include <lucaria/public/component_speaker.hpp>
#include <lucaria/public/component_transform.hpp>

namespace lucaria {
namespace detail {

    template <>
    struct entt_emplace_factory<component_rigidbody_passive> {
        template <typename ArchiveType>
        static component_rigidbody_passive& emplace(
            ArchiveType& archive,
            entt::registry& registry,
            entt::entity entity)
        {
            const auto& mappings = cereal::get_user_data<mappings_manager_game_load>(archive);

            if (mappings.dynamics == nullptr) {
                LUCARIA_DEBUG_ERROR("Missing dynamics while loading component_rigidbody_passive");
                throw std::runtime_error("Missing dynamics while loading component_rigidbody_passive");
            }

            return registry.emplace_or_replace<component_rigidbody_passive>(
                entity,
                *mappings.dynamics);
        }
    };

    template <>
    struct entt_emplace_factory<component_rigidbody_kinematic> {
        template <typename ArchiveType>
        static component_rigidbody_kinematic& emplace(
            ArchiveType& archive,
            entt::registry& registry,
            entt::entity entity)
        {
            const auto& mappings = cereal::get_user_data<mappings_manager_game_load>(archive);

            if (mappings.dynamics == nullptr) {
                LUCARIA_DEBUG_ERROR("Missing dynamics while loading component_rigidbody_kinematic");
                throw std::runtime_error("Missing dynamics while loading component_rigidbody_kinematic");
            }

            return registry.emplace_or_replace<component_rigidbody_kinematic>(
                entity,
                *mappings.dynamics);
        }
    };

    template <>
    struct entt_emplace_factory<component_rigidbody_dynamic> {
        template <typename ArchiveType>
        static component_rigidbody_dynamic& emplace(
            ArchiveType& archive,
            entt::registry& registry,
            entt::entity entity)
        {
            const auto& mappings = cereal::get_user_data<mappings_manager_game_load>(archive);

            if (mappings.dynamics == nullptr) {
                LUCARIA_DEBUG_ERROR("Missing dynamics while loading component_rigidbody_dynamic");
                throw std::runtime_error("Missing dynamics while loading component_rigidbody_dynamic");
            }

            return registry.emplace_or_replace<component_rigidbody_dynamic>(
                entity,
                *mappings.dynamics);
        }
    };

    //

    template <typename ComponentType>
    struct recipe_object_scene_component {
        uint32 component_save_id = 0;
        ComponentType* component_save = nullptr;

        template <typename ArchiveType>
        void save(ArchiveType& archive) const
        {
            archive(cereal::make_nvp("component_save_id", component_save_id));

            if (component_save == nullptr) {
                LUCARIA_DEBUG_ERROR("Missing component while saving scene component recipe");
                return;
            }

            archive(cereal::make_nvp("component", *component_save));
        }

        template <typename ArchiveType>
        void load(ArchiveType& archive)
        {
            archive(cereal::make_nvp("component_save_id", component_save_id));

            auto& game_mappings = cereal::get_user_data<mappings_manager_game_load>(archive);

            if (game_mappings.loading_scene == nullptr) {
                LUCARIA_DEBUG_ERROR("Missing scene while loading component");
                return;
            }

            entt::entity entity = game_mappings.scenes.load_map_scene_entities
                                      .at(game_mappings.loading_scene_save_id)
                                      .at(component_save_id);

            ComponentType& component = entt_emplace_factory<ComponentType>::emplace(
                archive,
                game_mappings.loading_scene->components,
                entity);

            archive(cereal::make_nvp("component", component));
        }
    };

    struct recipe_object_scene_registry {
        std::vector<recipe_object_scene_component<component_animator>> animators = {};
        std::vector<recipe_object_scene_component<component_interface_screen>> screen_interfaces = {};
        std::vector<recipe_object_scene_component<component_interface_spatial>> spatial_interfaces = {};
        std::vector<recipe_object_scene_component<component_model_blockout>> blockout_models = {};
        std::vector<recipe_object_scene_component<component_model_unlit>> unlit_models = {};
        std::vector<recipe_object_scene_component<component_rigidbody_passive>> passive_rigidbodies = {};
        std::vector<recipe_object_scene_component<component_rigidbody_kinematic>> kinematic_rigidbodies = {};
        std::vector<recipe_object_scene_component<component_rigidbody_dynamic>> dynamic_rigidbodies = {};
        std::vector<recipe_object_scene_component<component_speaker_spatial>> speakers = {};
        std::vector<recipe_object_scene_component<component_transform>> transforms = {};
        std::vector<uint32> entity_save_ids = {};

        template <typename ArchiveType>
        void save(ArchiveType& archive) const
        {
            archive(cereal::make_nvp("entity_save_ids", entity_save_ids));
            archive(cereal::make_nvp("animators", animators));
            archive(cereal::make_nvp("screen_interfaces", screen_interfaces));
            archive(cereal::make_nvp("spatial_interfaces", spatial_interfaces));
            archive(cereal::make_nvp("blockout_models", blockout_models));
            archive(cereal::make_nvp("unlit_models", unlit_models));
            archive(cereal::make_nvp("passive_rigidbodies", passive_rigidbodies));
            archive(cereal::make_nvp("kinematic_rigidbodies", kinematic_rigidbodies));
            archive(cereal::make_nvp("dynamic_rigidbodies", dynamic_rigidbodies));
            archive(cereal::make_nvp("speakers", speakers));
            archive(cereal::make_nvp("transforms", transforms));
        }

        template <typename ArchiveType>
        void load(ArchiveType& archive)
        {
            archive(cereal::make_nvp("entity_save_ids", entity_save_ids));

            auto& game_mappings = cereal::get_user_data<mappings_manager_game_load>(archive);

            if (game_mappings.loading_scene == nullptr) {
                LUCARIA_DEBUG_ERROR("Missing scene while loading scene registry");
                return;
            }

            auto& entity_map = game_mappings.scenes.load_map_scene_entities
                                   [game_mappings.loading_scene_save_id];

            for (uint32 entity_save_id : entity_save_ids) {
                entt::entity entity = game_mappings.loading_scene->components.create();
                entity_map[entity_save_id] = entity;
            }

            archive(cereal::make_nvp("animators", animators));
            archive(cereal::make_nvp("screen_interfaces", screen_interfaces));
            archive(cereal::make_nvp("spatial_interfaces", spatial_interfaces));
            archive(cereal::make_nvp("blockout_models", blockout_models));
            archive(cereal::make_nvp("unlit_models", unlit_models));
            archive(cereal::make_nvp("passive_rigidbodies", passive_rigidbodies));
            archive(cereal::make_nvp("kinematic_rigidbodies", kinematic_rigidbodies));
            archive(cereal::make_nvp("dynamic_rigidbodies", dynamic_rigidbodies));
            archive(cereal::make_nvp("speakers", speakers));
            archive(cereal::make_nvp("transforms", transforms));
        }
    };

    struct recipe_object_scene_user_component_group {
        std::string type_id = {};
        object_user_scene* scene = nullptr;
        user_component_type_callbacks* component_type_callbacks = nullptr;

        template <typename ArchiveType>
        void save(ArchiveType& archive) const
        {
            archive(cereal::make_nvp("type_id", type_id));
            if constexpr (std::is_base_of_v<cereal::JSONOutputArchive, ArchiveType>) {
                component_type_callbacks->json_save(*scene, archive);
            } else if constexpr (std::is_base_of_v<cereal::PortableBinaryOutputArchive, ArchiveType>) {
                component_type_callbacks->binary_save(*scene, archive);
            }
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

            if constexpr (std::is_base_of_v<cereal::JSONInputArchive, ArchiveType>) {
                if (it->second.json_load) {
                    it->second.json_load(*game_mappings.loading_scene, archive);
                }
            } else if constexpr (std::is_base_of_v<cereal::PortableBinaryInputArchive, ArchiveType>) {
                if (it->second.binary_load) {
                    it->second.binary_load(*game_mappings.loading_scene, archive);
                }
            }
        }
    };

    struct recipe_object_scene {
        uint32 scene_save_id = 0;
        std::string type_id = {};
        recipe_object_scene_registry components = {};
        std::vector<recipe_object_scene_user_component_group> user_components = {};
        object_user_scene* scene = nullptr;
        user_scene_type_callbacks* scene_type_callbacks = nullptr;

        template <typename ArchiveType>
        void save(ArchiveType& archive) const
        {
            archive(cereal::make_nvp("scene_save_id", scene_save_id));
            archive(cereal::make_nvp("type_id", type_id));
            archive(cereal::make_nvp("components", components));
            archive(cereal::make_nvp("user_components", user_components));
            if constexpr (std::is_base_of_v<cereal::JSONOutputArchive, ArchiveType>) {
                scene_type_callbacks->json_save(*scene, archive);
            } else if constexpr (std::is_base_of_v<cereal::PortableBinaryOutputArchive, ArchiveType>) {
                scene_type_callbacks->binary_save(*scene, archive);
            }
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

            std::unique_ptr<object_user_scene>& scene_ptr = manager.scenes.emplace_back();
            scene_ptr = std::make_unique<object_user_scene>();

            object_user_scene& loaded_scene = *scene_ptr;
            loaded_scene.type_id = type_id;

            if (it->second.construct) {
                it->second.construct(loaded_scene);
            }

            game_mappings.scenes.load_map_scenes[scene_save_id] = &loaded_scene.components;

            object_user_scene* previous_scene = game_mappings.loading_scene;
            uint32 previous_scene_id = game_mappings.loading_scene_save_id;

            game_mappings.loading_scene = &loaded_scene;
            game_mappings.loading_scene_save_id = scene_save_id;

            archive(cereal::make_nvp("components", components));
            archive(cereal::make_nvp("user_components", user_components));

            if constexpr (std::is_base_of_v<cereal::JSONInputArchive, ArchiveType>) {
                if (it->second.json_load) {
                    it->second.json_load(loaded_scene, archive);
                }
            } else if constexpr (std::is_base_of_v<cereal::PortableBinaryInputArchive, ArchiveType>) {
                if (it->second.binary_load) {
                    it->second.binary_load(loaded_scene, archive);
                }
            }

            game_mappings.loading_scene = previous_scene;
            game_mappings.loading_scene_save_id = previous_scene_id;
        }
    };

    struct recipe_manager_scene {
        std::vector<recipe_object_scene> scenes = {};

        template <typename ArchiveType>
        void serialize(ArchiveType& archive)
        {
            archive(cereal::make_nvp("scenes", scenes));
        }
    };

    //

    [[nodiscard]] recipe_object_scene make_recipe(const context_game& game, object_user_scene& scene, const user_scene_type_callbacks& callbacks);
    [[nodiscard]] recipe_manager_scene make_recipe(manager_scenes& manager, mappings_manager_scene_save& mappings);

    //

    // declared in manager_scenes.hpp
    template <typename ComponentType>
    void manager_scenes::register_user_component(std::string type_id)
    {
        user_component_type_callbacks _component_type = {};

        _component_type.binary_save = [](object_user_scene& scene, cereal::PortableBinaryOutputArchive& archive) {
            const mappings_manager_game_save& _mappings = cereal::get_user_data<mappings_manager_game_save>(archive);
            std::vector<recipe_object_scene_component<ComponentType>> _components = {};
            scene.components.view<ComponentType>().each(
                [&](entt::entity entity, ComponentType& component) {
                    recipe_object_scene_component<ComponentType>& _back = _components.emplace_back();
                    _back.component_save_id = _mappings.scenes.save_map_scene_entities.at(&scene.components).at(entity);
                    _back.component_save = &component;
                });
            archive(cereal::make_nvp("components", _components));
        };

        _component_type.binary_load = [](object_user_scene& scene, cereal::PortableBinaryInputArchive& archive) {
            std::vector<recipe_object_scene_component<ComponentType>> components = {};
            archive(cereal::make_nvp("components", components));
        };

        _component_type.json_save = [](object_user_scene& scene, cereal::JSONOutputArchive& archive) {
            const mappings_manager_game_save& _mappings = cereal::get_user_data<mappings_manager_game_save>(archive);
            std::vector<recipe_object_scene_component<ComponentType>> _components = {};
            scene.components.view<ComponentType>().each(
                [&](entt::entity entity, ComponentType& component) {
                    recipe_object_scene_component<ComponentType>& _back = _components.emplace_back();
                    _back.component_save_id = _mappings.scenes.save_map_scene_entities.at(&scene.components).at(entity);
                    _back.component_save = &component;
                });
            archive(cereal::make_nvp("components", _components));
        };

        _component_type.json_load = [](object_user_scene& scene, cereal::JSONInputArchive& archive) {
            std::vector<recipe_object_scene_component<ComponentType>> components = {};
            archive(cereal::make_nvp("components", components));
        };

        user_component_type_ids[std::type_index(typeid(ComponentType))] = type_id;
        user_component_types[std::move(type_id)] = std::move(_component_type);
    }
}
}
