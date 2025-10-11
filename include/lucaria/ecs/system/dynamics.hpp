#pragma once

#include <glm/glm.hpp>

#include <optional>

namespace lucaria {
namespace ecs {

    namespace dynamics {

        /// @brief
        struct raycast_collision {
            glm::vec3 position;
            glm::vec3 normal;
        };

        /// @brief
        /// @param from
        /// @param to
        [[nodiscard]] std::optional<raycast_collision> compute_raycast(const glm::vec3& from, const glm::vec3& to);

        /// @brief
        /// @param newtons
        void set_world_gravity(const glm::vec3& newtons);

    }
}

namespace detail {

    struct dynamics_system {
        dynamics_system() = delete;
        dynamics_system(const dynamics_system& other) = delete;
        dynamics_system& operator=(const dynamics_system& other) = delete;
        dynamics_system(dynamics_system&& other) = delete;
        dynamics_system& operator=(dynamics_system&& other) = delete;

        static void step_simulation();
        static void compute_collisions();
        static void collect_debug_guizmos();
    };
}
}
