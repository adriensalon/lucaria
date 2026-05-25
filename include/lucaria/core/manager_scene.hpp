#pragma once

#include <functional>
#include <stdexcept>
#include <typeindex>

#include <cereal/archives/json.hpp>
#include <cereal/archives/portable_binary.hpp>

#include <lucaria/bin/types_containers.hpp>
#include <lucaria/core/manager_object.hpp>
#include <lucaria/core/object_scene.hpp>

namespace lucaria {

struct context_game;

namespace detail {

    struct manager_game;

    // mappings

    struct mappings_manager_scene {
        std::unordered_map<const entt::registry*, uint32> save_map_scene_ids = {};
        std::unordered_map<const entt::registry*, std::unordered_map<entt::entity, uint32>> save_map_scene_entities = {};
        std::unordered_map<uint32, entt::registry*> load_map_scenes = {};
        std::unordered_map<uint32, std::unordered_map<uint32, entt::entity>> load_map_scene_entities = {};
    };

    struct mappings_manager_game_save {
        mappings_manager_object_save objects = {};
        mappings_manager_scene scenes = {};
    };

    // archives

    using archive_json_output_type_base = cereal::JSONOutputArchive;
    using archive_json_output = cereal::UserDataAdapter<mappings_manager_game_save, archive_json_output_type_base>;

    using archive_json_input_type_base = cereal::JSONInputArchive;
    using archive_json_input = cereal::UserDataAdapter<mappings_manager_game_save, archive_json_input_type_base>;

    using archive_binary_output_type_base = cereal::PortableBinaryOutputArchive;
    using archive_binary_output = cereal::UserDataAdapter<mappings_manager_game_save, archive_binary_output_type_base>;

    using archive_binary_input_type_base = cereal::PortableBinaryInputArchive;
    using archive_binary_input = cereal::UserDataAdapter<mappings_manager_game_save, archive_binary_input_type_base>;

    struct user_component_type_callbacks {
        std::function<void(object_scene&, const std::unordered_map<entt::entity, uint32>&, cereal::PortableBinaryOutputArchive&)> binary_save = nullptr;
        std::function<void(object_scene&, const std::unordered_map<entt::entity, uint32>&, cereal::JSONOutputArchive&)> json_save = nullptr;
    };

    struct object_scene_type_callbacks {
        std::function<void(object_scene&)> construct = nullptr;
        std::function<void(context_game&, object_scene&)> start = nullptr;
        std::function<void(context_game&, object_scene&)> update = nullptr;
        std::function<void(context_game&, object_scene&)> stop = nullptr;
        std::function<void(object_scene&, cereal::PortableBinaryOutputArchive&)> binary_save = nullptr;
        std::function<void(object_scene&, cereal::PortableBinaryInputArchive&)> binary_load = nullptr;
        std::function<void(object_scene&, cereal::JSONOutputArchive&)> json_save = nullptr;
        std::function<void(object_scene&, cereal::JSONInputArchive&)> json_load = nullptr;
    };

    struct manager_scene {
        std::vector<std::unique_ptr<object_scene>> scenes = {};
        std::unordered_map<std::string, object_scene_type_callbacks> scene_types = {};
        std::unordered_map<std::type_index, std::string> scene_type_ids = {};
        std::unordered_map<std::string, user_component_type_callbacks> user_component_types = {};
        std::unordered_map<std::type_index, std::string> user_component_type_ids = {};
        object_scene* current_scene = nullptr;

        template <typename... ComponentTypes, typename Callback>
        void each_view(Callback&& callback)
        {
            for (std::unique_ptr<object_scene>& _scene : scenes) {
                _scene->components.view<ComponentTypes...>().each(std::forward<Callback>(callback));
            }
        }

        template <typename... ComponentTypes, typename... ExcludeComponentTypes, typename Callback>
        void each_view(entt::exclude_t<ExcludeComponentTypes...> exclude, Callback&& callback)
        {
            for (std::unique_ptr<object_scene>& _scene : scenes) {
                _scene->components.view<ComponentTypes...>(exclude).each(std::forward<Callback>(callback));
            }
        }

        template <typename SceneType>
        void register_scene(std::string type_id)
        {
            object_scene_type_callbacks _scene_type = {};

            _scene_type.construct = [](object_scene& scene) {
                scene.user_data.emplace<SceneType>();
            };

            if constexpr (has_start_v<SceneType>) {
                _scene_type.start = [](context_game& game, object_scene& scene) {
                    SceneType& _typed_scene = std::any_cast<SceneType&>(scene.user_data);
                    _typed_scene.start(game);
                };
            }
            if constexpr (has_update_v<SceneType>) {
                _scene_type.update = [](context_game& game, object_scene& scene) {
                    SceneType& _typed_scene = std::any_cast<SceneType&>(scene.user_data);
                    _typed_scene.update(game);
                };
            }
            if constexpr (has_stop_v<SceneType>) {
                _scene_type.stop = [](context_game& game, object_scene& scene) {
                    SceneType& _typed_scene = std::any_cast<SceneType&>(scene.user_data);
                    _typed_scene.stop(game);
                };
            }

            _scene_type.binary_save = [](object_scene& scene, cereal::PortableBinaryOutputArchive& archive) {
                SceneType& _typed_scene = std::any_cast<SceneType&>(scene.user_data);
                archive(cereal::make_nvp("user_scene", _typed_scene));
            };

            _scene_type.binary_load = [](object_scene& scene, cereal::PortableBinaryInputArchive& archive) {
                SceneType& _typed_scene = std::any_cast<SceneType&>(scene.user_data);
                archive(cereal::make_nvp("user_scene", _typed_scene));
            };

            _scene_type.json_save = [](object_scene& scene, cereal::JSONOutputArchive& archive) {
                SceneType& _typed_scene = std::any_cast<SceneType&>(scene.user_data);
                archive(cereal::make_nvp("user_scene", _typed_scene));
            };

            _scene_type.json_load = [](object_scene& scene, cereal::JSONInputArchive& archive) {
                SceneType& _typed_scene = std::any_cast<SceneType&>(scene.user_data);
                archive(cereal::make_nvp("user_scene", _typed_scene));
            };

            scene_type_ids[std::type_index(typeid(SceneType))] = type_id;
            scene_types[std::move(type_id)] = std::move(_scene_type);
        }

        template <typename ComponentType>
        void register_component_user(std::string type_id)
        {
            user_component_type_callbacks _component_type = {};

            _component_type.binary_save = [](object_scene& scene, const std::unordered_map<entt::entity, uint32>& entity_ids, cereal::PortableBinaryOutputArchive& archive) {
                std::vector<recipe_object_scene_component<ComponentType>> _components = {};
                scene.components.view<ComponentType>().each(
                    [&](entt::entity entity, ComponentType& component) {
                        recipe_object_scene_component<ComponentType>& _back = _components.emplace_back();
                        _back.component_save_id = entity_ids.at(entity);
                        _back.component = &component;
                    });
                archive(cereal::make_nvp("components", _components));
            };

            _component_type.json_save = [](object_scene& scene, const std::unordered_map<entt::entity, uint32>& entity_ids, cereal::JSONOutputArchive& archive) {
                std::vector<recipe_object_scene_component<ComponentType>> _components = {};
                scene.components.view<ComponentType>().each(
                    [&](entt::entity entity, ComponentType& component) {
                        recipe_object_scene_component<ComponentType>& _back = _components.emplace_back();
                        _back.component_save_id = entity_ids.at(entity);
                        _back.component = &component;
                    });
                archive(cereal::make_nvp("components", _components));
            };

            user_component_type_ids[std::type_index(typeid(ComponentType))] = type_id;
            user_component_types[std::move(type_id)] = std::move(_component_type);
        }

        template <typename SceneType>
        [[nodiscard]] std::pair<SceneType&, object_scene&> construct_scene()
        {
            std::unique_ptr<object_scene>& _scene = scenes.emplace_back();
            _scene = std::make_unique<object_scene>();
            _scene->type_id = scene_type_ids.at(std::type_index(typeid(SceneType)));
            SceneType& _typed_scene = _scene->user_data.emplace<SceneType>();
            return { _typed_scene, *_scene.get() };
        }

        void start_scene(context_game& game, object_scene& scene);
        void update_callbacks(context_game& game);
        void update_systems(manager_game& game);
    };

    // recipes

    struct recipe_object_scene_user_component_group {
        std::string type_id = {};
        object_scene* scene = nullptr;
        user_component_type_callbacks* component_type_callbacks = nullptr;
        const std::unordered_map<entt::entity, uint32>* entity_ids = nullptr;

        template <typename ArchiveType>
        void save(ArchiveType& archive) const
        {
            archive(cereal::make_nvp("type_id", type_id));
            if constexpr (std::is_base_of_v<cereal::JSONOutputArchive, ArchiveType>) {
                component_type_callbacks->json_save(*scene, *entity_ids, archive);
            } else if constexpr (std::is_base_of_v<cereal::PortableBinaryOutputArchive, ArchiveType>) {
                component_type_callbacks->binary_save(*scene, *entity_ids, archive);
            }
        }

        template <typename ArchiveType>
        void load(ArchiveType& archive)
        {
            archive(cereal::make_nvp("type_id", type_id));
            // User-component loading is intentionally left for the scene load pipeline.
        }
    };

    struct recipe_object_scene {
        std::string type_id = {};
        recipe_object_scene_registry components = {};
        std::vector<recipe_object_scene_user_component_group> user_components = {};
        object_scene* scene = nullptr;
        object_scene_type_callbacks* scene_type_callbacks = nullptr;

        template <typename ArchiveType>
        void save(ArchiveType& archive) const
        {
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
            archive(cereal::make_nvp("type_id", type_id));
            archive(cereal::make_nvp("components", components));
            archive(cereal::make_nvp("user_components", user_components));
            // if constexpr (std::is_same_v<ArchiveType, archive_json_input>) {
            //     scene_type_callbacks->json_load(*scene, archive);
            // } else if constexpr (std::is_same_v<ArchiveType, archive_binary_input>) {
            //     scene_type_callbacks->binary_load(*scene, archive);
            // }
        }
    };

    [[nodiscard]] recipe_object_scene make_recipe(const context_game& game, object_scene& scene, const object_scene_type_callbacks& callbacks);

    struct recipe_manager_scene {
        std::vector<recipe_object_scene> scenes = {};

        template <typename ArchiveType>
        void serialize(ArchiveType& archive)
        {
            archive(cereal::make_nvp("scenes", scenes));
        }
    };

    [[nodiscard]] recipe_manager_scene make_recipe(manager_scene& manager, mappings_manager_scene& mappings);

}
}
