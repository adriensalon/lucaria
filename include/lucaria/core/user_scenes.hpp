#pragma once

#include <any>

#include <lucaria/bin/types_containers.hpp>
#include <lucaria/core/scenes_entity.hpp>
#include <lucaria/core/user_components.hpp>

namespace lucaria {

struct context_game;
namespace detail {

    struct game_save_context;
    struct game_load_context;

    template <typename, typename = void>
    struct has_user_scene_start : std::false_type { };

    template <typename T>
    struct has_user_scene_start<T, std::void_t<decltype(std::declval<T&>().start(std::declval<::lucaria::context_game&>()))>> : std::true_type { };

    template <typename SceneType>
    inline constexpr bool has_user_scene_start_v = has_user_scene_start<SceneType>::value;

    template <typename, typename = void>
    struct has_user_scene_update : std::false_type { };

    template <typename T>
    struct has_user_scene_update<T, std::void_t<decltype(std::declval<T&>().update(std::declval<::lucaria::context_game&>()))>> : std::true_type { };

    template <typename SceneType>
    inline constexpr bool has_user_scene_update_v = has_user_scene_update<SceneType>::value;

    template <typename, typename = void>
    struct has_user_scene_stop : std::false_type { };

    template <typename T>
    struct has_user_scene_stop<T, std::void_t<decltype(std::declval<T&>().stop(std::declval<::lucaria::context_game&>()))>> : std::true_type { };

    template <typename SceneType>
    inline constexpr bool has_user_scene_stop_v = has_user_scene_stop<SceneType>::value;

    template <typename, typename = void>
    struct has_user_scene_destroy : std::false_type { };

    template <typename T>
    struct has_user_scene_destroy<T, std::void_t<decltype(std::declval<T&>().destroy(std::declval<::lucaria::context_game&>()))>> : std::true_type { };

    template <typename SceneType>
    inline constexpr bool has_user_scene_destroy_v = has_user_scene_destroy<SceneType>::value;

    template <typename, typename = void>
    struct has_user_scene_save_game : std::false_type { };

    template <typename T>
    struct has_user_scene_save_game<T, std::void_t<decltype(std::declval<const T&>().save(std::declval<game_save_context&>()))>> : std::true_type { };

    template <typename SceneType>
    inline constexpr bool has_user_scene_save_game_v = has_user_scene_save_game<SceneType>::value;

    template <typename, typename = void>
    struct has_user_scene_load_game : std::false_type { };

    template <typename T>
    struct has_user_scene_load_game<T, std::void_t<decltype(std::declval<T&>().load(std::declval<game_load_context&>()))>> : std::true_type { };

    template <typename SceneType>
    inline constexpr bool has_user_scene_load_game_v = has_user_scene_load_game<SceneType>::value;

    template <typename, typename = void>
    struct has_user_component_save_game : std::false_type { };

    template <typename T>
    struct has_user_component_save_game<T, std::void_t<decltype(std::declval<const T&>().save(std::declval<game_save_context&>()))>> : std::true_type { };

    template <typename ComponentType>
    inline constexpr bool has_user_component_save_game_v = has_user_component_save_game<ComponentType>::value;

    template <typename, typename = void>
    struct has_user_component_load_game : std::false_type { };

    template <typename T>
    struct has_user_component_load_game<T, std::void_t<decltype(std::declval<T&>().load(std::declval<game_load_context&>()))>> : std::true_type { };

    template <typename ComponentType>
    inline constexpr bool has_user_component_load_game_v = has_user_component_load_game<ComponentType>::value;

    template <typename SceneType>
    inline constexpr bool is_user_scene_v = is_any_compatible_v<SceneType> && (is_cereal_compatible_v<SceneType> || (has_user_scene_save_game_v<SceneType> && has_user_scene_load_game_v<SceneType>));

    template <typename SceneType>
    inline constexpr void static_assert_user_scene()
    {
        if constexpr (!is_user_scene_v<SceneType>) {
            static_assert(is_any_compatible_v<SceneType>, "User scene type must be compatible with std::any storage");
            static_assert(is_cereal_compatible_v<SceneType> || (has_user_scene_save_game_v<SceneType> && has_user_scene_load_game_v<SceneType>), "User scene type must be compatible with cereal serialization or implement save(game_save_context&) and load(game_load_context&)");
        }
    }

    template <typename ComponentType>
    inline constexpr bool is_user_component_v = is_entt_compatible_v<ComponentType> && (is_cereal_compatible_v<ComponentType> || (has_user_component_save_game_v<ComponentType> && has_user_component_load_game_v<ComponentType>));

    template <typename ComponentType>
    inline constexpr void static_assert_user_component()
    {
        if constexpr (!is_user_component_v<ComponentType>) {
            static_assert(is_entt_compatible_v<ComponentType>, "User component type must be compatible with entt storage");
            static_assert(is_cereal_compatible_v<ComponentType> || (has_user_component_save_game_v<ComponentType> && has_user_component_load_game_v<ComponentType>), "User component type must be compatible with cereal serialization or implement save(game_save_context&) and load(game_load_context&)");
        }
    }

    struct object_user_scene {
        std::string type_id = {};
        bool is_marked_erase = false;
        std::vector<object_entity> entities_marked_erase = {};
        std::any user_data = {};
        object_entity_scene_index index_for_context = 0;
    };

}
}
