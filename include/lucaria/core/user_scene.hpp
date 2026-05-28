#pragma once

#include <any>
#include <type_traits>

#include <entt/entt.hpp>

#include <lucaria/bin/types_containers.hpp>

namespace lucaria {

struct context_game;

namespace detail {

    template <typename, typename = void>
    struct has_start : std::false_type { };

    template <typename T>
    struct has_start<T, std::void_t<decltype(std::declval<T&>().start(std::declval<::lucaria::context_game&>()))>> : std::true_type { };

    template <typename, typename = void>
    struct has_update : std::false_type { };

    template <typename T>
    struct has_update<T, std::void_t<decltype(std::declval<T&>().update(std::declval<::lucaria::context_game&>()))>> : std::true_type { };

    template <typename, typename = void>
    struct has_stop : std::false_type { };

    template <typename T>
    struct has_stop<T, std::void_t<decltype(std::declval<T&>().stop(std::declval<::lucaria::context_game&>()))>> : std::true_type { };

    template <typename T>
    inline constexpr bool has_start_v = has_start<T>::value;

    template <typename T>
    inline constexpr bool has_update_v = has_update<T>::value;

    template <typename T>
    inline constexpr bool has_stop_v = has_stop<T>::value;

    struct object_scene {
        std::string type_id = {};
        bool is_marked_erase = false;
        std::vector<entt::entity> entities_marked_erase = {};
        entt::registry components = {};
        std::any user_data = {};
    };

}
}