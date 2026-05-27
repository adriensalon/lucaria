#pragma once

#include <any>
#include <memory>

#include <entt/entt.hpp>

#include <lucaria/bin/types_math.hpp>
#include <lucaria/core/utils_detection.hpp>

namespace lucaria {

struct context_game;
struct component_animator;
struct component_interface_screen;
struct component_interface_spatial;
struct component_model_blockout;
struct component_model_unlit;
struct component_rigidbody_passive;
struct component_rigidbody_kinematic;
struct component_rigidbody_dynamic;
struct component_speaker_spatial;
struct component_transform;

namespace detail {

    struct object_scene {
        std::string type_id = {};
        bool is_marked_erase = false;
        std::vector<entt::entity> entities_marked_erase = {};
        entt::registry components = {};
        std::any user_data = {};
    };

    // recipes

    template <typename ComponentType>
    struct component_emplace_factory {
        template <typename ArchiveType>
        static ComponentType& emplace(
            ArchiveType&,
            entt::registry& registry,
            entt::entity entity)
        {
            if constexpr (std::is_default_constructible_v<ComponentType>) {
                return registry.emplace_or_replace<ComponentType>(entity);
            } else {
                static_assert(
                    std::is_default_constructible_v<ComponentType>,
                    "Component is not default constructible. Provide component_emplace_factory specialization.");
            }
        }
    };

}
}
