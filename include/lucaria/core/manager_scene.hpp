#pragma once

#include <functional>
#include <stdexcept>
#include <typeindex>

#include <lucaria/bin/types_containers.hpp>
#include <lucaria/core/manager_object.hpp>
#include <lucaria/core/object_scene.hpp>

namespace lucaria {

struct context_game;
struct context_dynamics;

namespace detail {

    struct object_font;
    struct manager_window;
    struct manager_game;
    struct manager_scene;

    struct mappings_manager_scene_save {
        std::unordered_map<const entt::registry*, uint32> save_map_scene_ids = {};
        std::unordered_map<const entt::registry*, std::unordered_map<entt::entity, uint32>> save_map_scene_entities = {};
    };

    struct mappings_manager_scene_load {
        std::unordered_map<uint32, entt::registry*> load_map_scenes = {};
        std::unordered_map<uint32, std::unordered_map<uint32, entt::entity>> load_map_scene_entities = {};
    };

    struct mappings_manager_game_save {
        mappings_manager_object_save objects = {};
        mappings_manager_scene_save scenes = {};
    };

    struct mappings_manager_game_load {
        mappings_manager_object_load objects = {};
        mappings_manager_scene_load scenes = {};
        manager_object* loading_objects = nullptr;
        context_dynamics* dynamics = nullptr;
        manager_scene* loading_scene_manager = nullptr;
        object_scene* loading_scene = nullptr;
        uint32 loading_scene_save_id = 0;
    };

    using archive_json_output_type_base = cereal::JSONOutputArchive;
    using archive_json_output = cereal::UserDataAdapter<mappings_manager_game_save, archive_json_output_type_base>;

    using archive_json_input_type_base = cereal::JSONInputArchive;
    using archive_json_input = cereal::UserDataAdapter<mappings_manager_game_load, archive_json_input_type_base>;

    using archive_binary_output_type_base = cereal::PortableBinaryOutputArchive;
    using archive_binary_output = cereal::UserDataAdapter<mappings_manager_game_save, archive_binary_output_type_base>;

    using archive_binary_input_type_base = cereal::PortableBinaryInputArchive;
    using archive_binary_input = cereal::UserDataAdapter<mappings_manager_game_load, archive_binary_input_type_base>;

    struct user_component_type_callbacks {
        std::function<void(object_scene&, cereal::PortableBinaryOutputArchive&)> binary_save = nullptr;
        std::function<void(object_scene&, cereal::PortableBinaryInputArchive&)> binary_load = nullptr;
        std::function<void(object_scene&, cereal::JSONOutputArchive&)> json_save = nullptr;
        std::function<void(object_scene&, cereal::JSONInputArchive&)> json_load = nullptr;
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

            ComponentType& component = component_emplace_factory<ComponentType>::emplace(
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

    template <typename ArchiveType>
    void recipe_object_user_asset_group::load(ArchiveType& archive)
    {
        archive(cereal::make_nvp("type_id", type_id));

        auto& mappings = cereal::get_user_data<mappings_manager_game_load>(archive);

        if (mappings.loading_objects == nullptr) {
            LUCARIA_DEBUG_ERROR("Missing manager_object while loading user asset group");
            return;
        }

        auto it = mappings.loading_objects->user_asset_types.find(type_id);
        if (it == mappings.loading_objects->user_asset_types.end()) {
            LUCARIA_DEBUG_ERROR("Unknown user asset type while loading snapshot");
            return;
        }

        if constexpr (std::is_base_of_v<cereal::JSONInputArchive, ArchiveType>) {
            it->second.json_load(*mappings.loading_objects, archive);
        } else if constexpr (std::is_base_of_v<cereal::PortableBinaryInputArchive, ArchiveType>) {
            it->second.binary_load(*mappings.loading_objects, archive);
        }
    }

    template <typename ObjectType, typename RecipeType>
    void make_recipes_for(std::vector<recipe_object_entry<RecipeType>>& recipes, const container_cache_vector<ObjectType>& cached_vector, mappings_container_cache_vector_save<ObjectType>& ids)
    {
        for (const std::unique_ptr<container_cache<ObjectType>>& _cached_ptr : cached_vector.cells) {
            const container_cache<ObjectType>* _cached = _cached_ptr.get();
            recipes.push_back(recipe_object_entry<RecipeType> { ids.get_or_create(_cached), make_recipe(*_cached) });
        }
    }

    template <typename ObjectType, typename RecipeType>
    void apply_recipes_for(manager_object& objects, container_cache_vector<ObjectType>& cached_vector, mappings_container_cache_vector_load<ObjectType>& mappings, std::vector<recipe_object_entry<RecipeType>>& recipes)
    {
        for (auto& entry : recipes) {
            container_cache<ObjectType>* cell = apply_recipe(objects, cached_vector, entry.recipe);
            if (cell == nullptr) {
                LUCARIA_DEBUG_ERROR("Failed to apply object recipe");
                continue;
            }
            mappings.set(entry.save_id, cell);
        }
    }

    void apply_recipes_for(manager_window& window, manager_object& objects, container_cache_vector<object_font>& cached_vector, mappings_container_cache_vector_load<object_font>& mappings, std::vector<recipe_object_entry<recipe_object_font>>& recipes);

    template <typename AssetType, typename ArchiveType>
    void save_user_asset_group(manager_object& objects, ArchiveType& archive)
    {
        auto& mappings = cereal::get_user_data<mappings_manager_game_save>(archive);
        const auto& storage = objects.template get_user_asset_storage<AssetType>();
        std::vector<recipe_object_entry<recipe_object_user_asset<AssetType>>> recipes = {};
        make_recipes_for(recipes, storage.assets, mappings.objects.user_assets.template get_mapping<AssetType>());
        archive(cereal::make_nvp("assets", recipes));
    }

    template <typename AssetType, typename ArchiveType>
    void load_user_asset_group(manager_object& objects, ArchiveType& archive)
    {
        mappings_manager_game_load& mappings = cereal::get_user_data<mappings_manager_game_load>(archive);
        auto& storage = objects.template get_user_asset_storage<AssetType>();
        std::vector<recipe_object_entry<recipe_object_user_asset<AssetType>>> recipes = {};
        archive(cereal::make_nvp("assets", recipes));
        for (auto& entry : recipes) {
            container_cache<object_user_asset<AssetType>>* cell = apply_recipe(objects, storage.assets, entry.recipe);
            if (cell == nullptr) {
                LUCARIA_DEBUG_ERROR("Failed to apply user asset recipe");
                continue;
            }
            mappings.objects.user_assets.template set<AssetType>(entry.save_id, cell);
        }
    }

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

            _component_type.binary_save = [](object_scene& scene, cereal::PortableBinaryOutputArchive& archive) {
                const detail::mappings_manager_game_save& _mappings = cereal::get_user_data<detail::mappings_manager_game_save>(archive);
                std::vector<recipe_object_scene_component<ComponentType>> _components = {};
                scene.components.view<ComponentType>().each(
                    [&](entt::entity entity, ComponentType& component) {
                        recipe_object_scene_component<ComponentType>& _back = _components.emplace_back();
                        _back.component_save_id = _mappings.scenes.save_map_scene_entities.at(&scene.components).at(entity);
                        _back.component_save = &component;
                    });
                archive(cereal::make_nvp("components", _components));
            };

            _component_type.binary_load = [](object_scene& scene, cereal::PortableBinaryInputArchive& archive) {
                std::vector<recipe_object_scene_component<ComponentType>> components = {};
                archive(cereal::make_nvp("components", components));
            };

            _component_type.json_save = [](object_scene& scene, cereal::JSONOutputArchive& archive) {
                const detail::mappings_manager_game_save& _mappings = cereal::get_user_data<detail::mappings_manager_game_save>(archive);
                std::vector<recipe_object_scene_component<ComponentType>> _components = {};
                scene.components.view<ComponentType>().each(
                    [&](entt::entity entity, ComponentType& component) {
                        recipe_object_scene_component<ComponentType>& _back = _components.emplace_back();
                        _back.component_save_id = _mappings.scenes.save_map_scene_entities.at(&scene.components).at(entity);
                        _back.component_save = &component;
                    });
                archive(cereal::make_nvp("components", _components));
            };

            _component_type.json_load = [](object_scene& scene, cereal::JSONInputArchive& archive) {
                std::vector<recipe_object_scene_component<ComponentType>> components = {};
                archive(cereal::make_nvp("components", components));
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
        object_scene* scene = nullptr;
        object_scene_type_callbacks* scene_type_callbacks = nullptr;

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

            manager_scene& manager = *game_mappings.loading_scene_manager;

            auto it = manager.scene_types.find(type_id);
            if (it == manager.scene_types.end()) {
                LUCARIA_DEBUG_ERROR("Unknown scene type while loading snapshot");
                return;
            }

            std::unique_ptr<object_scene>& scene_ptr = manager.scenes.emplace_back();
            scene_ptr = std::make_unique<object_scene>();

            object_scene& loaded_scene = *scene_ptr;
            loaded_scene.type_id = type_id;

            if (it->second.construct) {
                it->second.construct(loaded_scene);
            }

            game_mappings.scenes.load_map_scenes[scene_save_id] = &loaded_scene.components;

            object_scene* previous_scene = game_mappings.loading_scene;
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

    [[nodiscard]] recipe_object_scene make_recipe(const context_game& game, object_scene& scene, const object_scene_type_callbacks& callbacks);
    void apply_recipe(object_scene& scene, uint32 scene_save_id, mappings_manager_scene_load& mappings, recipe_object_scene_registry& registry_recipe);

    struct recipe_manager_scene {
        std::vector<recipe_object_scene> scenes = {};

        template <typename ArchiveType>
        void serialize(ArchiveType& archive)
        {
            archive(cereal::make_nvp("scenes", scenes));
        }
    };

    [[nodiscard]] recipe_manager_scene make_recipe(manager_scene& manager, mappings_manager_scene_save& mappings);
    void apply_recipe(context_game& game, manager_scene& manager, mappings_manager_scene_load& mappings, recipe_manager_scene& recipe);

}
}
