#pragma once

#include <any>

#include <lucaria/bin/types_containers.hpp>
#include <lucaria/core/user_traits.hpp>

namespace lucaria {

struct context_game;

namespace detail {

    template <typename SceneType>
    inline constexpr bool is_user_scene_v = is_any_compatible_v<SceneType> && is_cereal_compatible_v<SceneType>;

    template <typename SceneType>
    inline constexpr void static_assert_user_scene()
    {
        if constexpr (!is_user_scene_v<SceneType>) {
            static_assert(is_any_compatible_v<SceneType>, "User scene type must be compatible with std::any storage");
            static_assert(is_cereal_compatible_v<SceneType>, "User scene type must be compatible with cereal serialization");
        }
    }

    template <typename ComponentType>
    inline constexpr bool is_user_component_v = is_entt_compatible_v<ComponentType> && is_cereal_compatible_v<ComponentType>;

    template <typename ComponentType>
    inline constexpr void static_assert_user_component()
    {
        if constexpr (!is_user_component_v<ComponentType>) {
            static_assert(is_entt_compatible_v<ComponentType>, "User component type must be compatible with entt storage");
            static_assert(is_cereal_compatible_v<ComponentType>, "User component type must be compatible with cereal serialization");
        }
    }

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

    struct object_user_scene {
        std::string type_id = {};
        bool is_marked_erase = false;
        std::vector<entt::entity> entities_marked_erase = {};
        entt::registry components = {};
        std::any user_data = {};
    };

}
}