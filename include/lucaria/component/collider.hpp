#pragma once

#include <lucaria/core/layer.hpp>
#include <lucaria/core/shape.hpp>

namespace lucaria {
namespace detail {
    struct dynamics_system;
}

struct collider_component {
    collider_component() = default;
    collider_component(const collider_component& other) = delete;
    collider_component& operator=(const collider_component& other) = delete;
    collider_component(collider_component&& other) = default;
    collider_component& operator=(collider_component&& other) = default;
    ~collider_component();

    collider_component& use_shape(shape& from);
    collider_component& use_shape(fetched<shape>& from);

    collider_component& set_group_layer(const collision_layer layer, const bool enable = true);
    collider_component& set_mask_layer(const collision_layer layer, const bool enable = true);

private:
    bool _is_added = false;
    detail::fetched_container<shape> _shape = {};
    std::unique_ptr<btDefaultMotionState> _state = nullptr;
    std::unique_ptr<btRigidBody> _rigidbody = nullptr;
    std::int16_t _group = 0;
    std::int16_t _mask = 0;
    friend struct detail::dynamics_system;
};

}
