#pragma once

#include <lucaria/public/handle_asset.hpp>
#include <lucaria/core/object_shape.hpp>
#include <lucaria/core/utils_refcount.hpp>

namespace lucaria {

/// @brief Represents runtime geometry meant for collision detection on the device
struct handle_shape : handle_asset<detail::object_shape> {
    using handle_asset<detail::object_shape>::handle_asset;

    friend struct context_object;
    friend struct component_rigidbody_passive;
    friend struct component_rigidbody_kinematic;
    friend struct component_rigidbody_dynamic;
    friend struct detail::system_motion;
    friend struct detail::system_dynamics;
    friend class cereal::access;
};

}
