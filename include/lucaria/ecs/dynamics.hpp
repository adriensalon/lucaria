#pragma once

#include <optional>

#include <lucaria/core/collision.hpp>

namespace lucaria {

/// @brief Raycasts shapes geometry
/// @param from the position to raycast from
/// @param to the position to raycast to
[[nodiscard]] std::optional<collision> raycast(const glm::vec3& from, const glm::vec3& to);

/// @brief Sets the global gravity
/// @param newtons global gravity along -Y axis
void set_world_gravity(const glm::float32 newtons);

}
