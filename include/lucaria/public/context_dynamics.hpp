#pragma once

#include <lucaria/core/system_dynamics.hpp>

namespace lucaria {

/// @brief Runtime API for dynamics system configuration and usage.
/// This context provides functionnality for raycasting and interacting with the
/// bullet dynamics world.
struct context_dynamics {

    /// @brief Raycasts shapes geometry
    /// @param from the position to raycast from
    /// @param to the position to raycast to
    std::optional<collision> raycast(const float32x3& from, const float32x3& to);

    /// @brief Sets the global gravity
    /// @param newtons global gravity along -Y axis
    void set_world_gravity(const float32 newtons);

private:
	detail::system_dynamics* _system;
	friend struct component_rigidbody_passive;
	friend struct component_rigidbody_kinematic;
	friend struct component_rigidbody_dynamic;
	friend struct access_context;
};

}
