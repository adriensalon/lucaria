#pragma once

#include <cassert>
#include <limits>
#include <typeindex>

#include <lucaria/core/manager_assets.hpp>
#include <lucaria/core/storage_registry.hpp>
#include <lucaria/core/user_scene.hpp>

namespace lucaria {

struct context_game;

namespace detail {

    struct manager_game;

    struct entt_emplace_factory_default_tag { };

    template <typename ComponentType>
    struct entt_emplace_factory : entt_emplace_factory_default_tag {

        template <typename ArchiveType>
        static ComponentType& emplace(ArchiveType&, container_segment_registry_cpu& registry, object_entity entity)
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

        std::vector<object_user_scene> scenes = {};
        container_segment_registry_cpu segment_registry_cpu = {};
        // container_segment_registry_gpu segment_registry_gpu = {};
        object_entity_scene_index index_for_context = 0;

        std::unordered_map<std::string, user_scene_type_callbacks> scene_types = {};
        std::unordered_map<std::type_index, std::string> scene_type_ids = {};
        std::unordered_map<std::string, user_component_type_callbacks> user_component_types = {};
        std::unordered_map<std::type_index, std::string> user_component_type_ids = {};

        template <typename... ComponentTypes, typename Callback>
        void each_view(Callback&& callback)
        {
            segment_registry_cpu.view<ComponentTypes...>().each(std::forward<Callback>(callback));
        }

        template <typename... ComponentTypes, typename... ExcludeComponentTypes, typename Callback>
        void each_view(exclude_t<ExcludeComponentTypes...> exclude, Callback&& callback)
        {
            segment_registry_cpu.view<ComponentTypes...>(exclude).each(std::forward<Callback>(callback));
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
            if constexpr (has_user_scene_stop_v<SceneType>) {
                _scene_type.stop = [](context_game& game, object_user_scene& scene) {
                    SceneType& _typed_scene = std::any_cast<SceneType&>(scene.user_data);
                    _typed_scene.stop(game);
                };
            }

            _scene_type.binary_save = [](object_user_scene& scene, cereal::PortableBinaryOutputArchive& archive) {
                SceneType& _typed_scene = std::any_cast<SceneType&>(scene.user_data);
                archive(cereal::make_nvp("user_scene", _typed_scene));
            };

            _scene_type.binary_load = [](object_user_scene& scene, cereal::PortableBinaryInputArchive& archive) {
                SceneType& _typed_scene = std::any_cast<SceneType&>(scene.user_data);
                archive(cereal::make_nvp("user_scene", _typed_scene));
            };

            _scene_type.json_save = [](object_user_scene& scene, cereal::JSONOutputArchive& archive) {
                SceneType& _typed_scene = std::any_cast<SceneType&>(scene.user_data);
                archive(cereal::make_nvp("user_scene", _typed_scene));
            };

            _scene_type.json_load = [](object_user_scene& scene, cereal::JSONInputArchive& archive) {
                SceneType& _typed_scene = std::any_cast<SceneType&>(scene.user_data);
                archive(cereal::make_nvp("user_scene", _typed_scene));
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
        void update_callbacks(context_game& game);
        void update_systems(manager_game& game);
    };

}
}
