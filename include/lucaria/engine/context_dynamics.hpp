#pragma once

#include <lucaria/bin/types_math.hpp>
#include <lucaria/bin/types_containers.hpp>

namespace lucaria {
namespace detail {
    struct system_dynamics;
}

/// @brief
enum struct collision_layer : std::int16_t {
    layer_0 = 1 << 0,
    layer_1 = 1 << 1,
    layer_2 = 1 << 2,
    layer_3 = 1 << 3,
    layer_4 = 1 << 4,
    layer_5 = 1 << 5,
    layer_6 = 1 << 6,
    layer_7 = 1 << 7,
    layer_8 = 1 << 8,
    layer_9 = 1 << 9,
    layer_10 = 1 << 10,
    layer_11 = 1 << 11,
    layer_12 = 1 << 12,
    layer_13 = 1 << 13,
    layer_14 = 1 << 14
};

/// @brief
struct collision {
    float32 distance;
    float32x3 position;
    float32x3 normal;
};

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
