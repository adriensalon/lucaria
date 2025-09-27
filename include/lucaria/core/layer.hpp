#pragma once

#include <cstdint>

namespace lucaria {
namespace detail {
    constexpr std::int16_t bulletgroupID_kinematic_rigidbody = 1 << 0; // interacts with ?collider_ground, ?collider_wall, ?collider_layerX
    constexpr std::int16_t bulletgroupID_dynamic_rigidbody = 1 << 1; // interacts with ?dynamic_rigidbody, collider_ground, collider_wall
    constexpr std::int16_t bulletgroupID_collider_ground = 1 << 2; // interacts with kinematic_rigidbody, dynamic_rigidbody
    constexpr std::int16_t bulletgroupID_collider_wall = 1 << 3; // interacts with kinematic_rigidbody, dynamic_rigidbody
    constexpr std::int16_t bulletgroupID_collider_layer_0 = 1 << 4; // interacts with kinematic_rigidbody
    constexpr std::int16_t bulletgroupID_collider_layer_1 = 1 << 5; // interacts with kinematic_rigidbody
    constexpr std::int16_t bulletgroupID_collider_layer_2 = 1 << 6; // interacts with kinematic_rigidbody
    constexpr std::int16_t bulletgroupID_collider_layer_3 = 1 << 7; // interacts with kinematic_rigidbody
    constexpr std::int16_t bulletgroupID_collider_layer_4 = 1 << 8; // interacts with kinematic_rigidbody
    constexpr std::int16_t bulletgroupID_collider_layer_5 = 1 << 9; // interacts with kinematic_rigidbody
    constexpr std::int16_t bulletgroupID_collider_layer_6 = 1 << 10; // interacts with kinematic_rigidbody
    constexpr std::int16_t bulletgroupID_collider_layer_7 = 1 << 11; // interacts with kinematic_rigidbody
}

enum struct kinematic_layer : std::int16_t {
    layer_0 = detail::bulletgroupID_collider_layer_0,
    layer_1 = detail::bulletgroupID_collider_layer_1,
    layer_2 = detail::bulletgroupID_collider_layer_2,
    layer_3 = detail::bulletgroupID_collider_layer_3,
    layer_4 = detail::bulletgroupID_collider_layer_4,
    layer_5 = detail::bulletgroupID_collider_layer_5,
    layer_6 = detail::bulletgroupID_collider_layer_6,
    layer_7 = detail::bulletgroupID_collider_layer_7,
};

inline bool contains_layer(const std::int16_t containing, const std::int16_t contained)
{
    return (containing & contained) == contained;
}

inline std::int16_t remove_layer(const std::int16_t containing, const std::int16_t to_remove)
{
    return containing & ~to_remove;
}

}
