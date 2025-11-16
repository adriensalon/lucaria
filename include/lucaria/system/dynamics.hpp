#pragma once

#include <glm/glm.hpp>

#include <optional>

namespace lucaria {

/// @brief Represents a detected collision from a raycast
struct raycast_collision {
    glm::vec3 position;
    glm::vec3 normal;
};

/// @brief Raycasts shapes geometry
/// @param from the position to raycast from
/// @param to the position to raycast to
[[nodiscard]] std::optional<raycast_collision> raycast(const glm::vec3& from, const glm::vec3& to);

/// @brief Sets the global gravity
/// @param newtons global gravity along -Y axis
void set_world_gravity(const glm::vec3& newtons);

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
