#pragma once

#include <cassert>
#include <deque>
#include <limits>
#include <typeindex>

#include <lucaria/core/app_error.hpp>
#include <lucaria/core/gsl_compiler.hpp>
#include <lucaria/core/manager_assets.hpp>
#include <lucaria/core/scenes_dispatch.hpp>
#include <lucaria/core/scenes_registry.hpp>
#include <lucaria/core/serialize_context.hpp>
#include <lucaria/core/user_scenes.hpp>

namespace lucaria {

struct context_game;

namespace detail {

    struct manager_game;

    struct entt_emplace_factory_default_tag { };

    template <typename ComponentType>
    struct entt_emplace_factory : entt_emplace_factory_default_tag {

        template <typename ArchiveType>
        static ComponentType& emplace(ArchiveType&, storage_registry& registry, object_entity entity)
        {
            static_assert(std::is_default_constructible_v<ComponentType>, "Component is not default constructible. Provide entt_emplace_factory specialization");
            return registry.emplace_or_replace<ComponentType>(entity);
        }
    };

    struct user_component_type_callbacks {
        std::function<void(object_user_scene&, cereal::PortableBinaryOutputArchive&)> binary_save = nullptr;
        std::function<void(object_user_scene&, cereal::PortableBinaryInputArchive&)> binary_load = nullptr;
        std::function<void(object_user_scene&, cereal::JSONOutputArchive&)> json_save = nullptr;
        std::function<void(object_user_scene&, cereal::JSONInputArchive&)> json_load = nullptr;
    };

    struct user_scene_type_callbacks {
        std::function<void(object_user_scene&)> construct = nullptr;
        std::function<void(context_game&, object_user_scene&)> start = nullptr;
        std::function<void(context_game&, object_user_scene&)> update = nullptr;
        std::function<void(context_game&, object_user_scene&)> stop = nullptr;
        std::function<void(object_user_scene&, cereal::PortableBinaryOutputArchive&)> binary_save = nullptr;
        std::function<void(object_user_scene&, cereal::PortableBinaryInputArchive&)> binary_load = nullptr;
        std::function<void(object_user_scene&, cereal::JSONOutputArchive&)> json_save = nullptr;
        std::function<void(object_user_scene&, cereal::JSONInputArchive&)> json_load = nullptr;
    };

    struct manager_scenes {
        manager_scenes() = default;
        manager_scenes(const manager_scenes& other) = delete;
        manager_scenes& operator=(const manager_scenes& other) = delete;
        manager_scenes(manager_scenes&& other) = delete;
        manager_scenes& operator=(manager_scenes&& other) = delete;

#if !defined(LUCARIA_DISABLE_COMPUTE_SPIRV)
        std::unique_ptr<gsl_compiler> compiler = nullptr; // -> move to threads manager
#endif
        std::vector<gsl_system_info> gsl_systems = {}; // -> move to threads manager

        // ecs
        object_entity_scene_index index_for_context = 0;
        std::vector<object_user_scene> scenes = {};
        storage_registry registry = {};

        // user
        std::unordered_map<std::string, user_scene_type_callbacks> scene_types = {};
        std::unordered_map<std::type_index, std::string> scene_type_ids = {};
        std::unordered_map<std::string, user_component_type_callbacks> user_component_types = {};
        std::unordered_map<std::type_index, std::string> user_component_type_ids = {};

        template <typename... ComponentTypes, typename Callback>
        void each_view(Callback&& callback)
        {
            registry.view<ComponentTypes...>(exclude<>).each(std::forward<Callback>(callback));
        }

        template <typename... ComponentTypes, typename... ExcludeComponentTypes, typename Callback>
        void each_view(exclude_t<ExcludeComponentTypes...> exclude, Callback&& callback)
        {
            registry.view<ComponentTypes...>(exclude).each(std::forward<Callback>(callback));
        }

        template <auto SystemFunction>
        void run_dispatch_compute()
        {
            using components_type = lgsl_system_component_list_t<SystemFunction>;
            detail::run_dispatch_compute_cpu_fallback<SystemFunction>(registry, components_type {});
        }

        template <auto SystemFunction>
        void register_gsl_system(const char* name, const char* gsl_id, const char* gsl_source, gsl_system_info** info_slot, const char* file, int line)
        {
            gsl_system_info _info = {};
            _info.name = name;
            _info.gsl_id = gsl_id;
            _info.gsl_source = gsl_source;
            _info.file = file;
            _info.line = line;
            _info.function_ptr = reinterpret_cast<void*>(SystemFunction);
            gsl_systems.push_back(_info);
            if (info_slot) {
                *info_slot = &gsl_systems.back();
            }
        }

        template <typename SceneType>
        void register_user_scene(std::string type_id)
        {
            user_scene_type_callbacks _scene_type = {};

            _scene_type.construct = [](object_user_scene& scene) {
                scene.user_data.emplace<SceneType>();
            };

            if constexpr (has_user_scene_start_v<SceneType>) {
                _scene_type.start = [](context_game& game, object_user_scene& scene) {
                    SceneType& _typed_scene = std::any_cast<SceneType&>(scene.user_data);
                    _typed_scene.start(game);
                };
            }
            if constexpr (has_user_scene_update_v<SceneType>) {
                _scene_type.update = [](context_game& game, object_user_scene& scene) {
                    SceneType& _typed_scene = std::any_cast<SceneType&>(scene.user_data);
                    _typed_scene.update(game);
                };
            }
            if constexpr (has_user_scene_destroy_v<SceneType>) {
                _scene_type.stop = [](context_game& game, object_user_scene& scene) {
                    SceneType& _typed_scene = std::any_cast<SceneType&>(scene.user_data);
                    _typed_scene.destroy(game);
                };
            } else if constexpr (has_user_scene_stop_v<SceneType>) {
                _scene_type.stop = [](context_game& game, object_user_scene& scene) {
                    SceneType& _typed_scene = std::any_cast<SceneType&>(scene.user_data);
                    _typed_scene.stop(game);
                };
            }

            _scene_type.binary_save = [](object_user_scene& scene, cereal::PortableBinaryOutputArchive& archive) {
                SceneType& _typed_scene = std::any_cast<SceneType&>(scene.user_data);
                if constexpr (has_user_scene_save_game_v<SceneType>) {
                    mappings_manager_game_save& _mappings = cereal::get_user_data<mappings_manager_game_save>(archive);
                    if (_mappings.saving_objects == nullptr || _mappings.scenes.saving_scene_manager == nullptr) {
                        LUCARIA_DEBUG_ERROR("Missing game context while saving user scene");
                        return;
                    }
                    game_save_context _context { archive, *_mappings.saving_objects, *_mappings.scenes.saving_scene_manager, _mappings };
                    _context.field("user_scene", _typed_scene);
                } else {
                    archive(cereal::make_nvp("user_scene", _typed_scene));
                }
            };

            _scene_type.binary_load = [](object_user_scene& scene, cereal::PortableBinaryInputArchive& archive) {
                SceneType& _typed_scene = std::any_cast<SceneType&>(scene.user_data);
                if constexpr (has_user_scene_load_game_v<SceneType>) {
                    mappings_manager_game_load& _mappings = cereal::get_user_data<mappings_manager_game_load>(archive);
                    if (_mappings.loading_objects == nullptr || _mappings.loading_scene_manager == nullptr) {
                        LUCARIA_DEBUG_ERROR("Missing game context while loading user scene");
                        return;
                    }
                    game_load_context _context { archive, *_mappings.loading_objects, *_mappings.loading_scene_manager, _mappings.dynamics, _mappings };
                    _context.field("user_scene", _typed_scene);
                } else {
                    archive(cereal::make_nvp("user_scene", _typed_scene));
                }
            };

            _scene_type.json_save = [](object_user_scene& scene, cereal::JSONOutputArchive& archive) {
                SceneType& _typed_scene = std::any_cast<SceneType&>(scene.user_data);
                if constexpr (has_user_scene_save_game_v<SceneType>) {
                    mappings_manager_game_save& _mappings = cereal::get_user_data<mappings_manager_game_save>(archive);
                    if (_mappings.saving_objects == nullptr || _mappings.scenes.saving_scene_manager == nullptr) {
                        LUCARIA_DEBUG_ERROR("Missing game context while saving user scene");
                        return;
                    }
                    game_save_context _context { archive, *_mappings.saving_objects, *_mappings.scenes.saving_scene_manager, _mappings };
                    _context.field("user_scene", _typed_scene);
                } else {
                    archive(cereal::make_nvp("user_scene", _typed_scene));
                }
            };

            _scene_type.json_load = [](object_user_scene& scene, cereal::JSONInputArchive& archive) {
                SceneType& _typed_scene = std::any_cast<SceneType&>(scene.user_data);
                if constexpr (has_user_scene_load_game_v<SceneType>) {
                    mappings_manager_game_load& _mappings = cereal::get_user_data<mappings_manager_game_load>(archive);
                    if (_mappings.loading_objects == nullptr || _mappings.loading_scene_manager == nullptr) {
                        LUCARIA_DEBUG_ERROR("Missing game context while loading user scene");
                        return;
                    }
                    game_load_context _context { archive, *_mappings.loading_objects, *_mappings.loading_scene_manager, _mappings.dynamics, _mappings };
                    _context.field("user_scene", _typed_scene);
                } else {
                    archive(cereal::make_nvp("user_scene", _typed_scene));
                }
            };

            scene_type_ids[std::type_index(typeid(SceneType))] = type_id;
            scene_types[std::move(type_id)] = std::move(_scene_type);
        }

        // implemented in serialize_scenes.hpp
        template <typename ComponentType>
        void register_user_component(std::string type_id);

        template <typename SceneType>
        [[nodiscard]] std::pair<SceneType&, object_user_scene&> construct_scene()
        {
            assert(scenes.size() <= std::numeric_limits<object_entity_scene_index>::max());

            index_for_context = static_cast<uint16>(scenes.size());
            object_user_scene& scene = scenes.emplace_back();
            scene.type_id = scene_type_ids.at(std::type_index(typeid(SceneType)));
            scene.index_for_context = index_for_context;
            SceneType& typed_scene = scene.user_data.emplace<SceneType>();
            return { typed_scene, scene };
        }

        void start_scene(context_game& game, object_user_scene& scene);
        void clear_runtime_for_reload(context_game& game);
        void clear_plugin_registrations_for_reload();
        void update_callbacks(context_game& game);
        void update_systems(manager_game& game);

        void save(game_save_context& context);
        void load(game_load_context& context);
    };

}

template <typename ArchiveType>
void handle_entity::save(ArchiveType& archive) const
{
    const detail::mappings_manager_game_save& _mappings = cereal::get_user_data<detail::mappings_manager_game_save>(archive);
    const detail::object_entity_scene_index  scene = detail::entity_scene(_entity);
    const uint32 _scene_id = _mappings.scenes.save_map_scene_ids.at(scene);
    const uint32 _entity_id = _mappings.scenes.save_map_scene_entities.at(scene).at(_entity);
    archive(cereal::make_nvp("scene_save_id", _scene_id));
    archive(cereal::make_nvp("entity_save_id", _entity_id));
}

template <typename ArchiveType>
void handle_entity::load(ArchiveType& archive)
{
    uint32 scene_id = 0;
    uint32 entity_id = 0;

    archive(cereal::make_nvp("scene_save_id", scene_id));
    archive(cereal::make_nvp("entity_save_id", entity_id));

    if (scene_id == 0 || entity_id == 0) {
        _entity = {};
        return;
    }

    detail::mappings_manager_game_load& mappings = cereal::get_user_data<detail::mappings_manager_game_load>(archive);

    auto scene_it = mappings.scenes.load_map_scenes.find(scene_id);
    if (scene_it == mappings.scenes.load_map_scenes.end()) {
        LUCARIA_DEBUG_ERROR("Failed to resolve scene while loading entity handle");
        _entity = {};
        return;
    }

    auto entities_it = mappings.scenes.load_map_scene_entities.find(scene_id);
    if (entities_it == mappings.scenes.load_map_scene_entities.end()) {
        LUCARIA_DEBUG_ERROR("Failed to resolve scene entity map while loading entity handle");
        _entity = {};
        return;
    }

    auto entity_it = entities_it->second.find(entity_id);
    if (entity_it == entities_it->second.end()) {
        LUCARIA_DEBUG_ERROR("Failed to resolve entity while loading entity handle");
        _entity = {};
        return;
    }

    if (detail::entity_scene(entity_it->second) != scene_it->second
        || mappings.loading_scene_manager == nullptr
        || !mappings.loading_scene_manager->registry.valid(entity_it->second)) {
        LUCARIA_DEBUG_ERROR("Failed to resolve valid entity while loading entity handle");
        _entity = {};
        return;
    }

    _entity = entity_it->second;
}

}
